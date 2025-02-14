/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#include "libs/catch2/catch_amalgamated.hpp"
#include "libs/base64.hpp" // To decode the base64-encoded key strings

#include "core/comet.h"

#include "utils/uintconv.h"

#include "../../sdktestsuite.hpp"

#include <sys/prctl.h> // For prctl and PR_SET_PDEATHSIG
#include <signal.h>    // For SIGTERM

// Helper to convert an int64_t into Bytes
Bytes serializeInt64(int64_t value) {
  BytesArr<8> arr = UintConv::uint64ToBytes(static_cast<uint64_t>(value));
  return Bytes(arr.begin(), arr.end());
}

// Binary hash deserialization helper (to hex string without "0x" prefix)
std::string bytesToString(const Bytes& appHash) {
  return Hex::fromBytes(appHash, false).get();
}

/**
 * A simple stateful execution environment to test a Comet blockchain.
 *
 * Transactions must be ASCII strings in the following space-separated format:
 *  "<Signature> <Nonce> <Operation> <Value>"
 * Nonce is any string (whatever makes sense for the testcase).
 * The machine has a single memory cell that stores a signed integer and starts at 0.
 * A valid signature is "SIG", an invalid signature is "BADSIG", anything else is a badly formatted transaction.
 * Valid operations are + (add), - (subtract), = (set) and ? (assert value) to the memory cell.
 * The apphash is just set to the current block height at the end (h_).
 */
class TestMachine : public CometListener {
private:
  /**
   * Parse a TestMachine transaction string into its four components.
   * @param tx Transaction to be parsed (it is a plain ASCII string).
   * @param sig Parsed signature (outparam).
   * @param nonce Parsed nonce string (outparam).
   * @param op Parsed operation char (outparam) to be applied over m_.
   * @param val Parsed numeric value operand (outparam) to be appled to op over m_.
   * @return `true` if the transaction is valid and could be fully parsed, `false` otherwise.
   */
  bool parseTransaction(const Bytes& tx, std::string &sig, std::string& nonce, std::string& op, std::string& val) {
    std::string tx_str(tx.begin(), tx.end());
    std::istringstream iss(tx_str);
    if (!(iss >> sig >> nonce >> op >> val)) { return false; }
    std::string extra;
    if (iss >> extra) { return false; }
    if (sig == "BADSIG") { return false; }
    if (sig != "SIG") { return false; }
    if (op != "+" && op != "-" && op != "=" && op != "?" && op != "REVERT") { return false; }
    char* endptr;
    strtoll(val.c_str(), &endptr, 10);
    if (*endptr != '\0') { return false; }
    return true;
  }

  std::atomic<bool> enableAppHash_ = false; // Enables computing the app_hash based on m_ (instead of leaving it empty)

public:
  std::atomic<int64_t> m_ = 0; // machine state
  std::atomic<uint64_t> h_ = 0; // current block height (0 = genesis)
  std::atomic<uint64_t> incomingHeight_ = 0; // value of height we got from incomingBlock() (0 if none yet)
  std::atomic<uint64_t> incomingSyncingToHeight_ = 0; // value of syncingToHeight we got from incomingBlock() (0 if none yet)
  std::atomic<uint64_t> requiredSyncingToHeight_ = 0; // If set to != 0, requires incomingBlock(syncingToHeight) to match this value
  std::atomic<int> initChainCount_ = 0; // Flag for syncing with the cometbft InitChain callback
  Bytes appHash_; // Lastest apphash corresponding to m_ (if enableApphash_ == true)

  // ---------------------------------------------------------------------------
  // Transaction result tracker
  // ---------------------------------------------------------------------------

  // Test transaction serialization helper
  static Bytes toBytes(const std::string& str) {
    return Bytes(str.begin(), str.end());
  }

  struct TransactionDetails {
      // Fields from sendTransactionResult
      Bytes tx;
      uint64_t txId;
      bool sendSuccess;
      std::string txHash;
      std::string sendResponse;

      // Fields from checkTransactionResult
      bool checkResultArrived = false; // Indicates if a check result has been processed
      bool checkSuccess;
      std::string checkResponse;
  };
  // Map to store transaction details, indexed by txId
  std::mutex transactionMapMutex_;
  std::unordered_map<uint64_t, TransactionDetails> transactionMap_;

  std::string getSendTransactionHash(uint64_t txId) {
    std::lock_guard<std::mutex> lock(transactionMapMutex_);
    auto it = transactionMap_.find(txId);
    if (it == transactionMap_.end()) {
      LOGFATALP_THROW("Transaction ID not found: " + std::to_string(txId));
    }
    if (!it->second.sendSuccess) {
      LOGFATALP_THROW("Transaction ID wasn't successfully sent (failed or did not succeed yet): " + std::to_string(txId));
    }
    return it->second.txHash;
  }

  virtual void sendTransactionResult(
    const uint64_t tId, const bool success, const json& response,
    const std::string& txHash, const Bytes& tx
  ) override {
    GLOGDEBUG("TEST: TestMachine: Got sendTransactionResult : " + std::to_string(tId) + " hash: " + txHash + " success: " + std::to_string(success)
    + ", response: " + response.dump());
    TransactionDetails details = {tx, tId, success, txHash, response.dump(), false, false, ""};
    std::lock_guard<std::mutex> lock(transactionMapMutex_);
    transactionMap_[tId] = details;
  }

  virtual void checkTransactionResult(
    const uint64_t tId, const bool success, const json& response, const std::string& txHash
  ) override {
    GLOGDEBUG("TEST: TestMachine: Got checkTransactionResult : " + txHash + ", success: " + std::to_string(success) + ", response: " + response.dump());
    std::lock_guard<std::mutex> lock(transactionMapMutex_);
    for (auto& [txId, details] : transactionMap_) {
      if (details.txHash == txHash) {
        details.checkResultArrived = true;
        details.checkSuccess = success;
        details.checkResponse = response.dump();
        return;
      }
    }
    LOGFATALP_THROW("No matching transaction found for the given txHash.");
  }

  std::string getCheckTransactionResult(uint64_t txId) {
    std::lock_guard<std::mutex> lock(transactionMapMutex_);
    auto it = transactionMap_.find(txId);
    if (it == transactionMap_.end()) {
      LOGFATALP_THROW("Transaction ID not found: " + std::to_string(txId));
    }
    TransactionDetails& details = it->second;
    if (!details.checkResultArrived) {
      LOGFATALP_THROW("Transaction ID no check result arrived: " + std::to_string(txId));
    }
    if (!details.checkSuccess) {
      LOGFATALP_THROW("Transaction ID check result was failure: " + std::to_string(txId));
    }
    return details.checkResponse;
  }

  // ---------------------------------------------------------------------------

  TestMachine(bool enableAppHash = false) : enableAppHash_(enableAppHash) {
    updateAppHash();
  }

  /**
   * Get the appHash as a string.
   */
  std::string getAppHashString() {
    std::string ret;
    if (enableAppHash_) {
      ret = bytesToString(appHash_);
    }
    return ret;
  }

  /**
   * Recompute appHash_ based on m_ if enableAppHash_ == true.
   */
  void updateAppHash() {
    // Changing apphash generates empty blocks, making stepMode_ significantly less useful.
    // So, only compute the apphash if the testcase asks for it (via enableApphash_).
    // Don't use a proper hash here because that's just harder to debug/understand.
    appHash_.clear();
    if (enableAppHash_) {
      appHash_ = serializeInt64(m_);
    }
  }

  /**
   * Unfortunately, cometbft cannot be behind the application state, and by default if we don't have an app state
   * snapshot that we kept to match that "height" then we just reset to genesis.
   * TODO: allow TestMachine to store snapshots (i.e. values of m_) for all heights it processes and let tests use
   * them as they need it.
   * TODO: this callback should also forward the current validator set here because that's also state that the application
   * needs to be aware of.
   */
  void currentCometBFTHeight(const uint64_t height) override {
    if (h_ > height) {
      // comply by resetting to genesis
      GLOGDEBUG("TEST: TestMachine: currentCometBFTHeight " + std::to_string(height) + " > " + std::to_string(h_));
      h_ = 0;
      m_ = 0;
      updateAppHash();
    }
  }

  /**
   * InitChain ABCI callback.
   */
  void initChain(
    const uint64_t genesisTime, const std::string& chainId, const Bytes& initialAppState, const uint64_t initialHeight,
    const std::vector<CometValidatorUpdate>& initialValidators, Bytes& appHash) override
  {
    GLOGDEBUG("TEST: TestMachine: initChain()");
    m_ = 0;
    h_ = 0;
    incomingHeight_ = 0;
    incomingSyncingToHeight_ = 0;
    requiredSyncingToHeight_ = 0;
    ++initChainCount_;
    updateAppHash();
    appHash = appHash_;
  }

  /**
   * CheckTx ABCI callback.
   * @param tx Transaction to check.
   * @param accept Outparam to be set to `true` if the transaction is valid, `false` otherwise.
   */
  void checkTx(const Bytes& tx, const bool, int64_t&, bool& accept) override {
    GLOGDEBUG("TEST: TestMachine: checkTx()");
    std::string sig, nonce, op, val;
    accept = parseTransaction(tx, sig, nonce, op, val);
  }

  /**
   * FinalizedBlock ABCI callback.
   * @param height The height of the block that is being delivered for processing to advance the app state.
   * @param syncingToHeight If the execution is still catching up to head (block replay) this is less than height.
   * @param txs All transactions in the block that need to be executed.
   * @param appHash Outparam that needs to be filled with the new state hash of the application, if any.
   */
  void incomingBlock(
    const uint64_t syncingToHeight, std::unique_ptr<CometBlock> block, Bytes& appHash,
    std::vector<CometExecTxResult>& txResults, std::vector<CometValidatorUpdate>& validatorUpdates
  ) override
  {
    GLOGDEBUG("TEST: TestMachine: incomingBlock(): height=" + std::to_string(block->height) + "; syncingToheight="+std::to_string(syncingToHeight) + "; txs.size()="+std::to_string(block->txs.size()));
    incomingHeight_ = block->height;
    incomingSyncingToHeight_ = syncingToHeight;
    if (requiredSyncingToHeight_ != 0) {
      if (syncingToHeight != requiredSyncingToHeight_) {
        GLOGFATAL_THROW("incomingBlock with unexpected syncingToHeight=" + std::to_string(syncingToHeight) + "; required=" + std::to_string(requiredSyncingToHeight_));
      }
    }

    // If we get a finalized block height that is different from what our internal model is,
    //  that's an error: the consensus process would be finalizing a duplicate block, meaning
    //  it didn't correctly synchronize with the applicaiton.
    // It does not matter whether it is syncing to the height or not; our own current state should have
    //  been synchronized correctly via getCurrentState(). Whatever we report as the current height via
    //  getCurrentState() should be respected by cometbft so it doesn't give us a block that doesn't
    //  respect that current state, ever.
    if (block->height != h_ + 1) {
      GLOGFATAL_THROW("incomingBlock with out-of-sync height " + std::to_string(block->height) + "; app current height = " + std::to_string(h_));
    }

    // We need to process each transaction
    try {
      GLOGTRACE("incomingBlock: transaction count: " + std::to_string(block->txs.size()));
      for (const Bytes& tx : block->txs) {
        CometExecTxResult txRes; // Default tx result execution object: code: 0 (success)
        // parse and validate the transaction
        std::string sig, nonce, op, val;
        bool accept = parseTransaction(tx, sig, nonce, op, val);
        if (!accept) {
          GLOGFATAL_THROW("incomingBlock: the transaction is somehow invalid");
        }
        // parse value as int
        char* endptr;
        int64_t value = strtoll(val.c_str(), &endptr, 10);
        if (*endptr != '\0') {
          // Should not reach here since we validated the tx already
          GLOGFATAL_THROW("incomingBlock: transaction has an invalid value operand");
        }
        if (op == "+") {
          m_ += value;
        } else if (op == "-") {
          m_ -= value;
        } else if (op == "=") {
          m_ = value;
        } else if (op == "?") {
          // Reverts the transaction if the assert fails
          txRes.code = (m_ != value);
        } else if (op == "REVERT") {
          txRes.code = 1;
        } else {
          // Should not reach here since we validated the tx already
          GLOGFATAL_THROW("incomingBlock: transaction has an invalid operation");
        }
        GLOGXTRACE("TestMachine: incomingBlock updated m_ == " + std::to_string(m_));
        // Must provide Comet a tx result object for each transaction executed
        txResults.push_back(txRes);
      }
      // If all transactions are processed successfully, advance the height
      h_ = block->height;
      GLOGXTRACE("TestMachine: incomingBlock updated h_ == " + std::to_string(h_));
      // recompute the app_hash and return it
      updateAppHash();
      appHash = appHash_;
    } catch (...) {
      GLOGFATAL_THROW("incomingBlock: unexpected exception caugth");
    }
  }

  /**
   * PrepareProposal ABCI callback.
   * @param block The block being proposed (for reading; if you need to keep it, you must copy it explicitly).
   * @param accept Outparam to be set to `true` if the proposed block is valid, `false` otherwise.
   */
  void validateBlockProposal(const CometBlock& block, bool& accept) override {
    GLOGDEBUG("TEST: TestMachine: validateBlockProposal(): height=" + std::to_string(block.height) + "; txs.size()="+std::to_string(block.txs.size()));
    accept = true;
    for (const Bytes& tx : block.txs) {
      bool tx_accept;
      int64_t gas_limit;
      checkTx(tx, false, gas_limit, tx_accept);
      if (!tx_accept) {
        accept = false;
        return;
      }
    }
  }

  /**
   * Info ABCI callback.
   * @param height Outparam that must be set to the current block height that the execution environment is in.
   * @param appHash Outparam that must be set to the current state hash of the application, if any.
   */
  void getCurrentState(uint64_t& height, Bytes& appHash, std::string& appSemVer, uint64_t& appVersion) override {
    GLOGDEBUG("TEST: TestMachine: getCurrentState(): h_=" + std::to_string(h_));
    // return the currently computed apphash and the current height
    appHash = appHash_;
    height = h_;
    appSemVer = "1.0.0";
    appVersion = 0;
  }

  /**
   * Answers cometbft about what's the earliest block in the chain it shouldn't prune.
   * @param height Height of the first block in the chain that should not be deleted.
   */
  void persistState(uint64_t& height) override {
    GLOGDEBUG("TEST: TestMachine: persistState()");
    height = 0; // retain all blocks forever
  }
};

// FIXME/TODO: must time out of all future threads so testcases will eventually cleanup/exit
//             Or, actually, we should probably create helper classes (with or without help from macros)
//             that do away with the std::future boilerplate we are using to wait for conditions then
//             REQUIRE() that they are met.
namespace TComet {
  auto createTestDumpPath = SDKTestSuite::createTestDumpPath;
  auto generateCometTestPorts = SDKTestSuite::generateCometTestPorts;

  TEST_CASE("Comet Tests", "[integration][core][comet]") {
    // Very simple test flow that runs a single cometbft node that runs a single-validator blockchain
    // that can thus advance with a single validator producing blocks.
    // NOTE: This test showcases the ideal version of our std::future boilerplate for waiting for a condition to be met,
    //       but perhaps instead of copying it everywhere we should just replace this with something shorter (we would
    //       probably need macros that then call the REQUIRE() etc. macros within them).
    SECTION("CometBootTest") {
      std::string testDumpPath = createTestDumpPath("CometBootTest");

      GLOGDEBUG("TEST: Constructing Comet");

      // get free ports to run tests on
      int p2p_port = SDKTestSuite::getTestPort();
      int rpc_port = SDKTestSuite::getTestPort();

      const Options options = SDKTestSuite::getOptionsForTest(testDumpPath, false, "", p2p_port, rpc_port);

      // Create a simple listener that just records that we got InitChain and what the current height is.
      class TestCometListener : public CometListener {
      public:
        std::atomic<bool> gotInitChain = false;
        std::atomic<uint64_t> finalizedHeight = 0;
        virtual void initChain(
          const uint64_t genesisTime, const std::string& chainId, const Bytes& initialAppState, const uint64_t initialHeight,
          const std::vector<CometValidatorUpdate>& initialValidators, Bytes& appHash
        ) override
        {
          appHash.clear();
          GLOGDEBUG("TestCometListener: got initChain");
          gotInitChain = true;
        }
        virtual void incomingBlock(
          const uint64_t syncingToHeight, std::unique_ptr<CometBlock> block, Bytes& appHash,
          std::vector<CometExecTxResult>& txResults, std::vector<CometValidatorUpdate>& validatorUpdates
        ) override
        {
          GLOGDEBUG("TestCometListener: got incomingBlock " + std::to_string(block->height));
          finalizedHeight = block->height;
          appHash.clear();
          txResults.resize(block->txs.size());

          // Here we just show that Eth addresses and CometBFT addresses differ,
          //   even though they are both derived from the same Secp256k1 private key.
          // Validator 0 should have these (keys are base64):
          // address: "A146C3E02DB4F8AAD5E859E35F4F7BCC094F0B13"
          // pubkey: "AiA6uTAC2S62d1DcwRAAj0hSosCdkCa1aTWlXWJeFA9W"
          // privkey: "+8+j8W0W3B9H68JbLoUTieIU4aNWjsumkuU8fQPN6tY=",
          Bytes addressBytes = Hex::toBytes(cometTestKeys[0].address);
          REQUIRE(Hex::toBytes("A146C3E02DB4F8AAD5E859E35F4F7BCC094F0B13") == addressBytes);
          Bytes pubBytes = base64::decode_into<Bytes>(cometTestKeys[0].pub_key);
          Bytes privBytes = base64::decode_into<Bytes>(cometTestKeys[0].priv_key);
          REQUIRE(block->proposerAddr == addressBytes);
          PubKey pubKey(pubBytes); // Compressed key (33 bytes)
          Address generatedAddress = Secp256k1::toAddress(pubKey); // Generate Eth address from validator pub key
          Address expectedAddress(addressBytes); // Validator address given to us by CometBFT, computed from the same validator pub key
          REQUIRE(generatedAddress != expectedAddress); // They are not the same, as expected
          Bytes emulCometAddrBytes = Comet::getCometAddressFromPubKey(pubBytes); // Let's emulate what CometBFT does to compute an address
          Address emulCometAddr(emulCometAddrBytes);
          GLOGDEBUG("Expected CometBFT address: " + Hex::fromBytes(addressBytes).get());
          GLOGDEBUG("Computed CometBFT address: " + Hex::fromBytes(emulCometAddrBytes).get());
          REQUIRE(emulCometAddr == expectedAddress); // Now they do match, so we know how to compute CometBFT addresses on our end
        }
      };
      TestCometListener cometListener;

      // Set up comet with single validator
      Comet comet(&cometListener, "", options);

      // Set pause at configured
      comet.setPauseState(CometState::CONFIGURED);

      GLOGDEBUG("TEST: Starting");

      // Start comet.
      comet.start();

      // --- config check ---

      GLOGDEBUG("TEST: Waiting configuration");

      // Waits for the pause state or error status
      REQUIRE(comet.waitPauseState(10000) == "");

      // --- start ABCI server ---

      comet.setPauseState(CometState::STARTED_ABCI);

      GLOGDEBUG("TEST: Waiting for ABCI server to be successfully started");

      // Waits for the pause state or error status
      REQUIRE(comet.waitPauseState(10000) == "");

      // --- start cometbft ---

      comet.setPauseState(CometState::STARTED_COMET);

      GLOGDEBUG("TEST: Waiting for 'cometbft start' to be successfully started");

      // Waits for the pause state or error status
      REQUIRE(comet.waitPauseState(10000) == "");

      // --- test ABCI connection ---

      // Set pause at tested the comet gRPC connection
      comet.setPauseState(CometState::TESTED_COMET);

      GLOGDEBUG("TEST: Waiting for ABCI connection test");

      // Waits for the pause state or error status
      REQUIRE(comet.waitPauseState(10000) == "");

      // --- Wait for an InitChain ABCI callback ---

      GLOGDEBUG("TEST: Waiting for CometBFT InitChain");

      const int initChainTimeoutSecs = 5;
      auto futureInitChain = std::async(std::launch::async, [&]() {
        auto startTime = std::chrono::steady_clock::now();
        auto endTime = startTime + std::chrono::seconds(initChainTimeoutSecs + 1);
        while (!cometListener.gotInitChain && std::chrono::steady_clock::now() < endTime) {
          std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
      });
      REQUIRE(futureInitChain.wait_for(std::chrono::seconds(initChainTimeoutSecs)) != std::future_status::timeout);
      REQUIRE(cometListener.gotInitChain);
      REQUIRE_NOTHROW(futureInitChain.get());

      // --- Wait for a FinalizeBlock ABCI callback for a few blocks ---

      GLOGDEBUG("TEST: Waiting for CometBFT FinalizeBlock for 3 blocks");
      const int targetHeight = 3;
      const int finalizeBlockTimeoutSecs = 60;
      auto futureFinalizeBlock = std::async(std::launch::async, [&]() {
        auto startTime = std::chrono::steady_clock::now();
        auto endTime = startTime + std::chrono::seconds(finalizeBlockTimeoutSecs + 1);
        while (cometListener.finalizedHeight < targetHeight && std::chrono::steady_clock::now() < endTime) {
          std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
      });
      REQUIRE(futureFinalizeBlock.wait_for(std::chrono::seconds(finalizeBlockTimeoutSecs)) != std::future_status::timeout);
      REQUIRE(cometListener.finalizedHeight >= targetHeight);
      REQUIRE_NOTHROW(futureFinalizeBlock.get());

      // --- stop ---

      GLOGDEBUG("TEST: Stopping...");

      REQUIRE(comet.getStatus()); // no error reported (must check before stop())

      // Stop comet.
      comet.stop();

      GLOGDEBUG("TEST: Stopped");

      REQUIRE(comet.getState() == CometState::STOPPED);

      GLOGDEBUG("TEST: Finished");
    }

    // Simple test that runs a blockchain/genesis with two validators, that is:
    //       getOptionsForCometTest( path , 0 , 2 );   // instance 1 of 2
    //       getOptionsForCometTest( path , 1 , 2 );   // instance 2 of 2
    // This is just another trivial test that ensures we can run a CometBFT blockchain
    //   with more than one validator being required to finalize and advance state.
    SECTION("CometBootTest2") {
      GLOGDEBUG("TEST: Constructing two Comet instances");

      // get free ports to run tests on
      auto ports = generateCometTestPorts(2);

      // Create two test dump (i.e. BDK options rootPath) directories, one for each comet instance.
      // This is needed because each BDK instance only supports one running comet instance normally,
      //   so each options/rootPath has one "comet" subdirectory in it to be the cometbft home dir.
      std::string testDumpPath0 = createTestDumpPath("CometBootTest2_0");
      const Options options0 = SDKTestSuite::getOptionsForTest(testDumpPath0, false, "", ports[0].p2p, ports[0].rpc, 0, 2, ports); // key 0 (totals 2 keys: 0 and 1)

      std::string testDumpPath1 = createTestDumpPath("CometBootTest2_1");
      const Options options1 = SDKTestSuite::getOptionsForTest(testDumpPath1, false, "", ports[1].p2p, ports[1].rpc, 1, 2, ports); // key 1 (totals 2 keys: 0 and 1)

      // Create a simple listener that just records that we got InitChain and what the current height is.
      class TestCometListener : public CometListener {
      public:
        std::atomic<bool> gotInitChain = false;
        std::atomic<uint64_t> finalizedHeight = 0;
        virtual void initChain(
          const uint64_t genesisTime, const std::string& chainId, const Bytes& initialAppState, const uint64_t initialHeight,
          const std::vector<CometValidatorUpdate>& initialValidators, Bytes& appHash
        ) override
        {
          appHash.clear();
          GLOGDEBUG("TestCometListener: got initChain");
          gotInitChain = true;
        }
        virtual void incomingBlock(
          const uint64_t syncingToHeight, std::unique_ptr<CometBlock> block, Bytes& appHash,
          std::vector<CometExecTxResult>& txResults, std::vector<CometValidatorUpdate>& validatorUpdates
        ) override
        {
          GLOGDEBUG("TestCometListener: got incomingBlock " + std::to_string(block->height));
          finalizedHeight = block->height;
          appHash.clear();
          txResults.resize(block->txs.size());
        }
      };

      // Instantiate the listener object twice, one for each running Comet instance
      TestCometListener cometListener0;
      TestCometListener cometListener1;

      // Set up our two running Comet instances
      Comet comet0(&cometListener0, "Comet0", options0);
      Comet comet1(&cometListener1, "Comet1", options1);

      // Start both Comet instances.
      GLOGDEBUG("TEST: Starting both Comet instances");
      comet0.start();
      comet1.start();

      // Wait for both Comet instances to find one finalized block
      GLOGDEBUG("TEST: Waiting for CometBFT FinalizeBlock to be called 3 times on both instances");
      int targetHeight = 3;
      auto futureFinalizeBlock = std::async(std::launch::async, [&]() {
        while (cometListener0.finalizedHeight < targetHeight || cometListener1.finalizedHeight < targetHeight) {
          std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
      });
      REQUIRE(futureFinalizeBlock.wait_for(std::chrono::seconds(30)) != std::future_status::timeout);
      REQUIRE(cometListener0.finalizedHeight >= targetHeight);
      REQUIRE(cometListener1.finalizedHeight >= targetHeight);

      // Stop both cometbft instances
      GLOGDEBUG("TEST: Stopping both instances...");
      REQUIRE(comet0.getStatus()); // no error reported (must check before stop())
      comet0.stop();
      REQUIRE(comet1.getStatus()); // no error reported (must check before stop())
      comet1.stop();
      GLOGDEBUG("TEST: Stopped both instances");
      REQUIRE(comet0.getState() == CometState::STOPPED);
      REQUIRE(comet1.getState() == CometState::STOPPED);
      GLOGDEBUG("TEST: Finished");
    }

    // Test Comet::getBlock() --> CometListener::getBlockResult()
    SECTION("CometGetBlockTest") {
      std::string testDumpPath = createTestDumpPath("CometGetBlockTest");

      GLOGDEBUG("TEST: Constructing Comet");

      const int64_t initialAppState = 0;
      const std::string initialAppHashString = bytesToString(serializeInt64(initialAppState));

      int p2p_port = SDKTestSuite::getTestPort();
      int rpc_port = SDKTestSuite::getTestPort();
      const Options options = SDKTestSuite::getOptionsForTest(testDumpPath, false, initialAppHashString, p2p_port, rpc_port);

      // Create a listener to trap getBlockResult
      class TestCometListener : public CometListener {
      public:
        std::atomic<int64_t> state = initialAppState;
        std::atomic<uint64_t> finalizedHeight = 0;
        std::atomic<uint64_t> blockResults = 0;
        std::string finalizedBlockHashStr;
        virtual void initChain(
          const uint64_t genesisTime, const std::string& chainId, const Bytes& initialAppState, const uint64_t initialHeight,
          const std::vector<CometValidatorUpdate>& initialValidators, Bytes& appHash
        ) {
          appHash = serializeInt64(state);
        }
        virtual void getCurrentState(uint64_t& height, Bytes& appHash, std::string& appSemVer, uint64_t& appVersion) {
          height = finalizedHeight;
          appHash = serializeInt64(state);
          appSemVer = "1.0.0";
          appVersion = 0;
        }
        virtual void sendTransactionResult(const uint64_t tId, const bool success, const json& response, const std::string& txHash, const Bytes& tx) override {
          GLOGDEBUG("TestCometListener: got sendTransactionResult(): " + response.dump() + ", txHash: " + txHash + ", success: " + std::to_string(success));
          REQUIRE(success == true);
        }
        virtual void incomingBlock(
          const uint64_t syncingToHeight, std::unique_ptr<CometBlock> block, Bytes& appHash,
          std::vector<CometExecTxResult>& txResults, std::vector<CometValidatorUpdate>& validatorUpdates
        ) override
        {
          // false = no 0x ; true = uppercase ABCDEF (since that's what CometBFT gives us when answering the getblock RPC call)
          finalizedBlockHashStr = Hash(block->hash).hex(false, true).get();
          GLOGDEBUG(
            "TestCometListener: got incomingBlock(): height = " + std::to_string(block->height) +
            ", tx count: " + std::to_string(block->txs.size()) +
            ", hash = " + finalizedBlockHashStr
          );
          finalizedHeight = block->height;
          state += block->txs.size(); // state is a transaction counter, tx content is empty/ignored
          appHash = serializeInt64(state);
          txResults.resize(block->txs.size()); // just accept
        }
        virtual void getBlockResult(
          const uint64_t tId, const bool success, const json& response, const uint64_t blockHeight
        ) override {
          GLOGDEBUG(
            "TestCometListener: getBlockResult(): height = " + std::to_string(blockHeight) +
            " response = " + response.dump()
          );
          REQUIRE(success == true);
          REQUIRE(finalizedHeight == blockHeight);
          REQUIRE(finalizedBlockHashStr == response["result"]["block_id"]["hash"]);
          ++blockResults;
        }
      };
      TestCometListener cometListener;
      Comet comet(&cometListener, "", options);

      // Need to wait for RUNNING state
      comet.setPauseState(CometState::RUNNING);
      GLOGDEBUG("TEST: Starting comet...");
      comet.start();
      GLOGDEBUG("TEST: Waiting RUNNING state...");
      REQUIRE(comet.waitPauseState(30000) == "");
      comet.setPauseState();

      // Sleep a bit because we don't want to pick up the first block.
      // Just so we get a block JSON in the logs that has e.g. an actual previous block.
      std::this_thread::sleep_for(std::chrono::seconds(2));

      // Send three transactions that should be included in whatever the next block height happens to be.
      GLOGDEBUG("TEST: Send 3 transactions");
      REQUIRE(comet.sendTransaction({1}) > 0);
      REQUIRE(comet.sendTransaction({2}) > 0);
      REQUIRE(comet.sendTransaction({3}) > 0);

      // Wait until the state advances to a height that has the three transactions
      GLOGDEBUG("TEST: Wait for the 3 transactions to be in a finalized block");
      for (int millisecs = 0; cometListener.state < 3; ++millisecs) {
        REQUIRE(millisecs < 5000);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }

      // Whatever the head block is now, that's the one we are going to fetch
      // Capturing the height here works since blocks take a lot longer than 1ms to be produced
      uint64_t getBlockHeight = cometListener.finalizedHeight;

      // Request block via RPC
      GLOGDEBUG("TEST: getBlock(" + std::to_string(getBlockHeight) + ")");
      bool success = comet.getBlock(getBlockHeight);

      // Wait for block result
      GLOGDEBUG("TEST: Wait for getBlockResult()...");
      for (int millisecs = 0; cometListener.blockResults < 1; ++millisecs) {
        REQUIRE(millisecs < 5000);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }

      // Don't need to unpause, can just stop
      GLOGDEBUG("TEST: Stopping...");
      REQUIRE(comet.getStatus()); // no error reported (must check before stop())
      comet.stop();
      GLOGDEBUG("TEST: Stopped");
      REQUIRE(comet.getState() == CometState::STOPPED);
      GLOGDEBUG("TEST: Finished");
    }

    // Simple test that checks we can control block production with dispatching
    // one transaction per block while running cometbft in stepMode_ == true.
    SECTION("CometTxTest") {
      std::string testDumpPath = createTestDumpPath("CometTxTest");

      GLOGDEBUG("TEST: Constructing Comet");

      // get free ports to run tests on
      int p2p_port = SDKTestSuite::getTestPort();
      int rpc_port = SDKTestSuite::getTestPort();

      const Options options = SDKTestSuite::getOptionsForTest(testDumpPath, true, "", p2p_port, rpc_port); // stepMode enabled

      const int txSize = 1048576;

      const uint8_t txContentByte = 0xde;
      const uint8_t txBorderByte = 0xad;

      // Default sha256 computed by CometBFT
      //static const std::string transactionHash = "2A62F69DB37417A3EB7E72219BDE4D6ADCD1A9878527DA245D4CC30FD1F899AB";
      // Eth sha3 computed by our patched CometBFT
      static const std::string transactionHash = "98922D4C7850BC46439DFFE434AAC9523F64A5D711B74ADDF8D5665184E0F2C3";

      // Create a simple listener that just records that we got InitChain and what the current height is.
      class TestCometListener : public CometListener {
      public:
        std::atomic<bool> gotInitChain = false;
        std::atomic<uint64_t> finalizedHeight = 0;
        std::atomic<int> txCount = 0;
        std::atomic<int> gotTxCheck = 0;
        virtual void initChain(
          const uint64_t genesisTime, const std::string& chainId, const Bytes& initialAppState, const uint64_t initialHeight,
          const std::vector<CometValidatorUpdate>& initialValidators, Bytes& appHash
        ) override
        {
          appHash.clear();
          GLOGDEBUG("TestCometListener: got initChain");
          gotInitChain = true;
        }
        virtual void incomingBlock(
          const uint64_t syncingToHeight, std::unique_ptr<CometBlock> block, Bytes& appHash,
          std::vector<CometExecTxResult>& txResults, std::vector<CometValidatorUpdate>& validatorUpdates
        ) override
        {
          GLOGDEBUG("TestCometListener: got incomingBlock " + std::to_string(block->height));
          if (block->txs.size() != 0) {
            REQUIRE(block->txs.size() == 1);
            REQUIRE(block->txs[0].size() == txSize);
            REQUIRE(block->txs[0][0] == txBorderByte);
            bool err = false;
            for (int i = 1; i < block->txs[0].size() - 1; ++i) {
              if (block->txs[0][i] != txContentByte) {
                err = true;
                break;
              }
            }
            REQUIRE(err == false);
            REQUIRE(block->txs[0][block->txs[0].size()-1] == txBorderByte);
            ++txCount;
          }
          finalizedHeight = block->height;
          appHash.clear();
          txResults.resize(block->txs.size());
        }
        virtual void sendTransactionResult(const uint64_t tId, const bool success, const json& response, const std::string& txHash, const Bytes& tx) override {
          GLOGDEBUG("TestCometListener: got sendTransactionResult: " + response.dump() + ", txHash: " + txHash + ", success: " + std::to_string(success));
          REQUIRE(success == true);
          REQUIRE(tx.size() == txSize);
          REQUIRE(txHash == transactionHash);
        }
        virtual void checkTransactionResult(const uint64_t tId, const bool success, const json& response, const std::string& txHash) override {
          size_t jsonSize = response.dump().size();
          GLOGDEBUG("TestCometListener: got checkTransactionResult: " + std::to_string(jsonSize) + " response json bytes.");
          REQUIRE(jsonSize > txSize); // between json overhead and base64 encoding, this has to hold
          REQUIRE(success == true);
          ++gotTxCheck;
        }
      };
      TestCometListener cometListener;

      // Set up comet with single validator, no empty blocks and very large timeouts,
      //   which essentially makes cometbft only produce a block when we send a tx.
      Comet comet(&cometListener, "", options);

      // Start comet
      comet.start();

      // Wait for InitChain
      GLOGDEBUG("TEST: Waiting for CometBFT InitChain");
      auto futureInitChain = std::async(std::launch::async, [&]() {
        while (!cometListener.gotInitChain) {
          std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
      });
      REQUIRE(futureInitChain.wait_for(std::chrono::seconds(5)) != std::future_status::timeout);
      REQUIRE(cometListener.gotInitChain);

      // Wait for chain to advance to height==1
      // This also ensures we are in RUNNING state, which is required now for sendTransaction()
      // Apparently, even with produce empty blocks set to false, it produces the first block without
      //   any transactions for some reason.
      GLOGDEBUG("TEST: Waiting for CometBFT FinalizeBlock for height 1 (block 1 is created even when set to not create empty blocks)");
      auto futureFinalizeBlock1 = std::async(std::launch::async, [&]() {
        while (cometListener.finalizedHeight < 1) {
          std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
      });
      REQUIRE(futureFinalizeBlock1.wait_for(std::chrono::seconds(10)) != std::future_status::timeout);
      REQUIRE(cometListener.finalizedHeight == 1); // require exactly height 1

      // Sleep for a while: this is where block production could have
      //   most certainly advanced by now if we are allowing empty blocks
      //   to be produced and the block interval params were otherwise correct.
      // This 10s wait time is probably overkill, but we need at least one test
      //   to waste this time to ensure that stepMode_ is indeed working.
      GLOGDEBUG("TEST: Waiting to check that chain does not advance past height 1 until a transaction is sent (10s)");
      std::this_thread::sleep_for(std::chrono::seconds(10));

      // Ensure blockchain has indeed not advanced at all
      GLOGDEBUG("TEST: Checking that chain has not advanced past height 1 without a transaction");
      REQUIRE(cometListener.finalizedHeight == 1);

      // Build a large transaction
      Bytes largeTransaction(txSize, txContentByte);
      largeTransaction[0] = txBorderByte;
      largeTransaction[largeTransaction.size()-1] = txBorderByte;

      // Utils::sha256() should match the expected CometBFT-produced transaction hash
      // hex(false, true) returns a Hex object without "0x" and uppercase "ABCDEF" (char data)
      // Hex::get() returns itself as a std::string
      //Hash sha256txHash;
      //Utils::sha256(largeTransaction, sha256txHash);
      //REQUIRE(sha256txHash.hex(false, true).get() == transactionHash);
      //
      // With our patched CometBFT, it must match eth sha3 instead.
      Hash sha3txHash = Utils::sha3(largeTransaction);
      REQUIRE(sha3txHash.hex(false, true).get() == transactionHash);

      // Send the transaction to cause a block to be produced
      GLOGDEBUG("TEST: Sending transaction");
      uint64_t tId = comet.sendTransaction(largeTransaction);
      REQUIRE(tId > 0); // Ensure RPC was actually called

      // Wait for chain to advance
      GLOGDEBUG("TEST: Waiting for CometBFT FinalizeBlock for height 2");
      auto futureFinalizeBlock2 = std::async(std::launch::async, [&]() {
        while (cometListener.finalizedHeight < 2) {
          std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
      });
      REQUIRE(futureFinalizeBlock2.wait_for(std::chrono::seconds(10)) != std::future_status::timeout);
      REQUIRE(cometListener.finalizedHeight == 2); // require exactly height 2

      // Require successful processing of the transaction we sent
      REQUIRE(cometListener.txCount == 1);

      // Check the transaction
      // Actually, you need to loop making multiple checkTransaction() calls until you get one
      // that succeeds, since CometBFT takes some time to index the transaction AFTER it has successfully
      // been included in a block.
      // In any case, it has to be able to index the successful transaction that DID go in a block in
      // say 3 seconds, so it's fine if we just wait upfront and then send one check request (simpler).
      std::this_thread::sleep_for(std::chrono::seconds(3));
      comet.checkTransaction(transactionHash);
      GLOGDEBUG("TEST: Waiting for transaction check...");
      auto futureCheckTx = std::async(std::launch::async, [&]() {
        while (cometListener.gotTxCheck < 1) {
          std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
      });
      REQUIRE(futureCheckTx.wait_for(std::chrono::seconds(10)) != std::future_status::timeout);
      REQUIRE(cometListener.gotTxCheck == 1);

      // Stop
      GLOGDEBUG("TEST: Stopping...");
      REQUIRE(comet.getStatus()); // no error reported (must check before stop())
      comet.stop();
      GLOGDEBUG("TEST: Stopped");
      REQUIRE(comet.getState() == CometState::STOPPED);
      GLOGDEBUG("TEST: Finished");
    }

    // Simple test that checks that failed transactions are returned to the listener.
    SECTION("CometTxFailTest") {
      std::string testDumpPath = createTestDumpPath("CometTxFailTest");

      GLOGDEBUG("TEST: Constructing Comet");

      const int txSize = 10000000; // Transaction is too large (10 MB)

      // Create a simple listener that just records that we got InitChain and what the current height is.
      class TestCometListener : public CometListener {
      public:
        std::atomic<uint64_t> expectedTxId_ = 0;
        std::atomic<int> failTxCount = 0;
        virtual void sendTransactionResult(const uint64_t tId, const bool success, const json& response, const std::string& txHash, const Bytes& tx) override {
          GLOGDEBUG("TestCometListener: got sendTransactionResult: " + response.dump() + ", txHash: " + txHash);
          REQUIRE(success == false); // we expect the only tx sent by this testcase to fail
          REQUIRE(tId == expectedTxId_);
          REQUIRE(tx.size() == txSize);
          ++failTxCount;
        }
      };
      TestCometListener cometListener;

      // get free ports to run tests on
      int p2p_port = SDKTestSuite::getTestPort();
      int rpc_port = SDKTestSuite::getTestPort();

      const Options options = SDKTestSuite::getOptionsForTest(testDumpPath, true, "", p2p_port, rpc_port); // stepMode enabled

      // Set up comet with single validator
      Comet comet(&cometListener, "", options);

      // Start comet
      comet.start();

      // Need to wait for RUNNING state before sending transactions now unfortunately
      GLOGDEBUG("TEST: Waiting RUNNING state before sending transaction....");
      comet.setPauseState(CometState::RUNNING);
      REQUIRE(comet.waitPauseState(30000) == "");
      comet.setPauseState();

      // Send a transaction to cause a block to be produced
      GLOGDEBUG("TEST: Sending transaction");
      std::vector<uint8_t> largeTransaction(txSize, 0x00);
      cometListener.expectedTxId_ = comet.sendTransaction(largeTransaction);
      REQUIRE(cometListener.expectedTxId_ > 0); // ensure it got sent

      // Wait for failTxCount
      GLOGDEBUG("TEST: Waiting for sendTransaction to fail");
      auto futureFailTx = std::async(std::launch::async, [&]() {
        while (cometListener.failTxCount < 1) {
          std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
      });
      REQUIRE(futureFailTx.wait_for(std::chrono::seconds(10)) != std::future_status::timeout);

      // Require that the transaction has failed
      REQUIRE(cometListener.failTxCount == 1);

      // Stop
      GLOGDEBUG("TEST: Stopping...");
      REQUIRE(comet.getStatus()); // no error reported (must check before stop())
      comet.stop();
      GLOGDEBUG("TEST: Stopped");
      REQUIRE(comet.getState() == CometState::STOPPED);
      GLOGDEBUG("TEST: Finished");
    }

    // Test for transaction results such as reverted transaction or transaction return data
    SECTION("CometTxResultTest") {
      std::string testDumpPath = createTestDumpPath("CometTxResultTest");

      GLOGDEBUG("TEST: Constructing Comet");

      // Create a TestMachine with app_hash enabled.
      TestMachine cometListener(true);

      // get free ports to run tests on
      int p2p_port = SDKTestSuite::getTestPort();
      int rpc_port = SDKTestSuite::getTestPort();

      const Options options = SDKTestSuite::getOptionsForTest(testDumpPath, false, cometListener.getAppHashString(), p2p_port, rpc_port);

      // Set up comet with single validator and no stepMode.
      Comet comet(&cometListener, "", options);

      // Start comet.
      GLOGDEBUG("TEST: Starting Comet");
      comet.start();

      // Need to wait for RUNNING state before sending transactions now unfortunately
      GLOGDEBUG("TEST: Waiting RUNNING state before sending transaction....");
      comet.setPauseState(CometState::RUNNING);
      REQUIRE(comet.waitPauseState(30000) == "");
      comet.setPauseState();

      // Send transactions
      uint64_t succeedAssertTxId = comet.sendTransaction(TestMachine::toBytes("SIG 1 ? 0")); // m_ == 0 so this returndata is true
      REQUIRE(succeedAssertTxId > 0);
      uint64_t failAssertTxId = comet.sendTransaction(TestMachine::toBytes("SIG 2 ? 9876")); // m_ == 0 so this returndata is false
      REQUIRE(failAssertTxId > 0);
      uint64_t revertTxId = comet.sendTransaction(TestMachine::toBytes("SIG 3 REVERT 1111")); // operand (1111) is ignored when REVERT op
      REQUIRE(revertTxId > 0);

      // It's just simpler to wait for some large amount of time which guarantees that the transactions were included in a block
      std::this_thread::sleep_for(std::chrono::seconds(5));

      // After waiting a lot we can just call checkTransaction() to get the results since the txHash will already have resolved
      // and the /tx endpoint will return the transaction result (sufficient time for the successful tx indexing to run also)
      comet.checkTransaction(cometListener.getSendTransactionHash(succeedAssertTxId));
      comet.checkTransaction(cometListener.getSendTransactionHash(failAssertTxId));
      comet.checkTransaction(cometListener.getSendTransactionHash(revertTxId));

      // And now just wait a little longer so that the Comet engine can make the 3 /tx RPC calls for sure
      std::this_thread::sleep_for(std::chrono::seconds(3));

      // After this long wait we expect the whole send/check pipeline must have resolved for all transactions
      std::string succeedAssertResult = cometListener.getCheckTransactionResult(succeedAssertTxId);
      std::string failAssertResult = cometListener.getCheckTransactionResult(failAssertTxId);
      std::string revertResult = cometListener.getCheckTransactionResult(revertTxId);

      GLOGDEBUG("TEST: succeedAssertResult: " + succeedAssertResult);
      GLOGDEBUG("TEST: failAssertResult: " + failAssertResult);
      GLOGDEBUG("TEST: revertResult: " + revertResult);

      // It's faster if we just assert for the expected code field substring in the json string
      REQUIRE(succeedAssertResult.find("\"code\":0,") != std::string::npos); // code == 0: not reverted
      REQUIRE(failAssertResult.find("\"code\":1,") != std::string::npos); // code == 1: reverted
      REQUIRE(revertResult.find("\"code\":1,") != std::string::npos); // code == 1: reverted

      // Stop
      GLOGDEBUG("TEST: Stopping...");
      REQUIRE(comet.getStatus()); // no error reported (must check before stop())
      comet.stop();
      GLOGDEBUG("TEST: Stopped");
      REQUIRE(comet.getState() == CometState::STOPPED);
      GLOGDEBUG("TEST: Finished");
    }

    // Stop at block M and restart test with no block replay, 1 validator
    // (snapshotted state at M before shutdown reported by Info on restart)
    // Non-empty transactions and verify that it reaches the same end state
    SECTION("CometRestartTest") {
      std::string testDumpPath = createTestDumpPath("CometRestartTest");

      GLOGDEBUG("TEST: Constructing Comet");

      TestMachine cometListener;

      // get free ports to run tests on
      int p2p_port = SDKTestSuite::getTestPort();
      int rpc_port = SDKTestSuite::getTestPort();

      const Options options = SDKTestSuite::getOptionsForTest(testDumpPath, true, cometListener.getAppHashString(), p2p_port, rpc_port); // stepMode enabled

      // Set up comet with single validator
      Comet comet(&cometListener, "", options);

      // Start comet.
      GLOGDEBUG("TEST: Starting Comet");
      comet.start();

      // Need to wait for RUNNING state before sending transactions now unfortunately
      GLOGDEBUG("TEST: Waiting RUNNING state before sending transaction....");
      comet.setPauseState(CometState::RUNNING);
      REQUIRE(comet.waitPauseState(30000) == "");
      comet.setPauseState();

      // Should stop at height 10
      // Don't send a transation for height 1, since height 1 is seemingly produced
      // even without a transaction from the app (empty block even with produce
      // empty blocks option disabled).
      GLOGDEBUG("TEST: Sending several ++m_ transactions...");
      int targetHeight = 10;
      int expectedMachineMemoryValue = 0;
      for (int i = 1; i <= targetHeight; ++i) {

        GLOGDEBUG("TEST: Height=" + std::to_string(i));
        // send a transaction (skip height==1 as that height is produced regardless).
        if (i != 1) {
          // transaction increments the memory cell (++cometListener.m_)
          // The second parameter (i) is serving as the nonce (need to uniquify the transaction
          //   otherwise it is understood as a replay).
          std::string transaction = "SIG " + std::to_string(i) + " + 1";
          GLOGDEBUG("TEST: Sending transaction: " + transaction);
          Bytes transactionBytes(transaction.begin(), transaction.end());
          comet.sendTransaction(transactionBytes);
          ++expectedMachineMemoryValue;
        }

        // Wait for chain to advance
        auto futureFinalizeBlock = std::async(std::launch::async, [&]() {
          while (cometListener.h_ < i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
          }
        });
        REQUIRE(futureFinalizeBlock.wait_for(std::chrono::seconds(10)) != std::future_status::timeout);

        // Ensure the block and transaction had the intended effect on the machine
        GLOGDEBUG("TEST: Checking we are at block " + std::to_string(i) + " and m_ == " + std::to_string(expectedMachineMemoryValue));
        REQUIRE(cometListener.h_ == i);
        REQUIRE(cometListener.m_ == expectedMachineMemoryValue);
      }

      // Stop comet.
      GLOGDEBUG("TEST: Stopping comet (before restart step)");
      REQUIRE(comet.getStatus()); // no error reported (must check before stop())
      comet.stop();
      GLOGDEBUG("TEST: Stopped");
      REQUIRE(comet.getState() == CometState::STOPPED);

      // Before stopping again, ensure that we are in the running state (otherwise we can
      // finish replay before we get to opening our RPC connection, which generates
      // unnecessary logging and RPC connection retries).
      comet.setPauseState(CometState::RUNNING);

      // Restart comet
      GLOGDEBUG("TEST: Restarting");
      comet.start();
      GLOGDEBUG("TEST: Restarted");

      // This behavior actually depends on the app hash / commit hash changes;
      // cometbft produces empty blocks regardless of the create-empty-blocks setting if
      // there's a change in app hash / commit hash (not sure why or how that works exactly).
      // In any case, we can't depend on it; the empty blocks option is for optimizing the
      // network, not creating a step-mode for block production / debugging.
      //
      //   So it turns out that upon a successful restart, we should get a block produced
      //    that is empty (again, ignoring our "dont't produce empty blocks) config.
      //   That's actually useful, because it gives us a condition to wait for here.
      //   In addition, the TestMachine class will error out if the restart generates a
      //    bad incomingBlock() callback that would ignore what getCurrentState() is
      //    informing cometbft.
      //
      // Wait for the last non-empty block we created before (targetHeight)
      auto futureFinalizeBlock = std::async(std::launch::async, [&]() {
        while (cometListener.h_ < targetHeight) {
          std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
      });
      REQUIRE(futureFinalizeBlock.wait_for(std::chrono::seconds(10)) != std::future_status::timeout);
      REQUIRE(cometListener.h_ >= targetHeight); // We *might* produce more blocks (empty blocks), which is fine
      REQUIRE(cometListener.m_ == expectedMachineMemoryValue); // Double-check that we didn't screw up the previous state

      // Waits for RUNNING so we know we are past RPC connection established and so we can just stop now.
      REQUIRE(comet.waitPauseState(60000) == "");
      comet.setPauseState(); // unset the pause state (not needed since stop() does it, but nicer)

      // Stop
      GLOGDEBUG("TEST: Stopping...");
      REQUIRE(comet.getStatus()); // no error reported (must check before stop())
      comet.stop();
      GLOGDEBUG("TEST: Stopped");
      REQUIRE(comet.getState() == CometState::STOPPED);
      GLOGDEBUG("TEST: Finished");
    }

    // Stop at block M and restart test with block replay from M/2, 1 validator
    // (Info on restart reports block M/2 thus replays M/2+1 to M)
    // Non-empty transactions and verify that it reaches the same end state
    SECTION("CometReplayTest") {
      std::string testDumpPath = createTestDumpPath("CometReplayTest");

      GLOGDEBUG("TEST: Constructing Comet");

      TestMachine cometListener;

      // get free ports to run tests on
      int p2p_port = SDKTestSuite::getTestPort();
      int rpc_port = SDKTestSuite::getTestPort();

      const Options options = SDKTestSuite::getOptionsForTest(testDumpPath, true, cometListener.getAppHashString(), p2p_port, rpc_port); // stepMode enabled

      // Set up comet with single validator
      Comet comet(&cometListener, "", options);

      // Start comet.
      GLOGDEBUG("TEST: Starting Comet");
      comet.start();

      // Need to wait for RUNNING state before sending transactions now unfortunately
      GLOGDEBUG("TEST: Waiting RUNNING state before sending transaction....");
      comet.setPauseState(CometState::RUNNING);
      REQUIRE(comet.waitPauseState(30000) == "");
      comet.setPauseState();

      // Should stop at height 10
      // Don't send a transation for height 1, since height 1 is seemingly produced
      // even without a transaction from the app (empty block even with produce
      // empty blocks option disabled).
      GLOGDEBUG("TEST: Sending several ++m_ transactions...");
      int targetHeight = 10;
      int expectedMachineMemoryValue = 0;
      for (int i = 1; i < targetHeight; ++i) {

        GLOGDEBUG("TEST: Height=" + std::to_string(i));
        // send a transaction (skip height==1 as that height is produced regardless).
        if (i != 1) {
          // transaction increments the memory cell (++cometListener.m_)
          // The second parameter (i) is serving as the nonce (need to uniquify the transaction
          //   otherwise it is understood as a replay).
          std::string transaction = "SIG " + std::to_string(i) + " + 1";
          GLOGDEBUG("TEST: Sending transaction: " + transaction);
          Bytes transactionBytes(transaction.begin(), transaction.end());
          comet.sendTransaction(transactionBytes);
          ++expectedMachineMemoryValue;
        }

        // Wait for chain to advance
        auto futureFinalizeBlock = std::async(std::launch::async, [&]() {
          while (cometListener.h_ < i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
          }
        });
        REQUIRE(futureFinalizeBlock.wait_for(std::chrono::seconds(10)) != std::future_status::timeout);

        // Ensure the block and transaction had the intended effect on the machine
        GLOGDEBUG("TEST: Checking we are at block " + std::to_string(i) + " and m_ == " + std::to_string(expectedMachineMemoryValue));
        REQUIRE(cometListener.h_ == i);
        REQUIRE(cometListener.m_ == expectedMachineMemoryValue);
      }

      // Stop comet.
      GLOGDEBUG("TEST: Stopping comet (before restart step)");
      REQUIRE(comet.getStatus()); // no error reported (must check before stop())
      comet.stop();
      GLOGDEBUG("TEST: Stopped");
      REQUIRE(comet.getState() == CometState::STOPPED);

      // Here we will pretend that our node has crashed, and thus we have rolled back
      // our state to a previously-saved snapshot state, and so we need cometbft to
      // replay some blocks for us.
      GLOGDEBUG("TEST: Rolling back state (h_ == " + std::to_string(cometListener.h_) + ", m_ == " + std::to_string(cometListener.m_) + ")");
      int finalAppState = cometListener.m_;
      int finalAppHeight = cometListener.h_;
      int snapshotAppHeight = cometListener.h_ / 2;
      for (int i = finalAppHeight; i > snapshotAppHeight; --i) {
        // Since all transactions we send to the app do ++m_, then each
        //  height we unstack from the blockchain means we need to do --m_
        //  (remember that this doesn't apply to height==1 since the first
        //  block is empty because cometbft doesn't strictly respect the
        //  "don't produce empty blocks" option, but since we are unstacking
        //  it only back to h_/2, and h_ is like 9 here, then we should be
        //  well clear off of accidentally unstacking the NOP block 1.)
        --cometListener.h_;
        --cometListener.m_;
        cometListener.updateAppHash(); // since we changed m_
      }
      GLOGDEBUG("TEST: Rolled back state (h_ == " + std::to_string(cometListener.h_) + ", m_ == " + std::to_string(cometListener.m_) + ")");

      // Set the requiredSyncingToHeight_ into the machine (will throw if it doesn't match)
      cometListener.requiredSyncingToHeight_ = finalAppHeight;

      // Before stopping again, ensure that we are in the running state (otherwise we can
      // finish replay before we get to opening our RPC connection, which generates
      // unnecessary logging and RPC connection retries).
      comet.setPauseState(CometState::RUNNING);

      // Restart comet
      GLOGDEBUG("TEST: Restarting");
      comet.start();
      GLOGDEBUG("TEST: Restarted");

      // Wait until we get back to the finalAppHeight we recorded before the rollback.
      auto futureFinalizeBlock = std::async(std::launch::async, [&]() {
        while (cometListener.h_ < finalAppHeight) {
          std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
      });
      REQUIRE(futureFinalizeBlock.wait_for(std::chrono::seconds(10)) != std::future_status::timeout);
      REQUIRE(cometListener.h_ == finalAppHeight);
      REQUIRE(cometListener.m_ == finalAppState); // Double-check we reproduced the state after the replay

      // Reset the required syncing to height, no longer syncing
      cometListener.requiredSyncingToHeight_ = 0;

      // Sleep a bit and ensure we didn't get some garbage added to the chain afterwards
      std::this_thread::sleep_for(std::chrono::seconds(10));
      REQUIRE(cometListener.h_ == finalAppHeight);
      REQUIRE(cometListener.m_ == finalAppState);

      // Waits for RUNNING so we know we are past RPC connection established and so we can just stop now.
      REQUIRE(comet.waitPauseState(60000) == "");
      comet.setPauseState(); // unset the pause state (not needed since stop() does it, but nicer)

      // Stop
      GLOGDEBUG("TEST: Stopping...");
      REQUIRE(comet.getStatus()); // no error reported (must check before stop())
      comet.stop();
      GLOGDEBUG("TEST: Stopped");
      REQUIRE(comet.getState() == CometState::STOPPED);
      GLOGDEBUG("TEST: Finished");
    }

    // Produce some blocks (without stepMode), stop, rewind to genesis and replay,
    // with app_hash computing over m_ enabled.
    SECTION("CometReplayAppHashTest") {
      std::string testDumpPath = createTestDumpPath("CometReplayAppHashTest");

      GLOGDEBUG("TEST: Constructing Comet");

      // Create a TestMachine with app_hash enabled.
      TestMachine cometListener(true);

      // get free ports to run tests on
      int p2p_port = SDKTestSuite::getTestPort();
      int rpc_port = SDKTestSuite::getTestPort();

      const Options options = SDKTestSuite::getOptionsForTest(testDumpPath, false, cometListener.getAppHashString(), p2p_port, rpc_port);

      // Set up comet with single validator and no stepMode.
      Comet comet(&cometListener, "", options);

      // Start comet.
      GLOGDEBUG("TEST: Starting Comet");
      comet.start();

      // Need to wait for RUNNING state before sending transactions now unfortunately
      GLOGDEBUG("TEST: Waiting RUNNING state before sending transaction....");
      comet.setPauseState(CometState::RUNNING);
      REQUIRE(comet.waitPauseState(30000) == "");
      comet.setPauseState();

      // Send several transactions across different blocks (stepMode is disabled,
      // so cometbft can produce empty blocks).
      GLOGDEBUG("TEST: Sending several ++m_ transactions...");
      int numTxs = 10;
      int64_t expectedMachineMemoryValue = 0;
      for (int i = 0; i < numTxs; ++i) {

        // transaction increments the memory cell (++cometListener.m_)
        // The second parameter (i) is serving as the nonce (need to uniquify the transaction
        //   otherwise it is understood as a replay).
        std::string transaction = "SIG " + std::to_string(i) + " + 1";
        GLOGDEBUG("TEST: Sending transaction: " + transaction);
        Bytes transactionBytes(transaction.begin(), transaction.end());
        comet.sendTransaction(transactionBytes);
        ++expectedMachineMemoryValue;

        // Wait for memory cell to update (meaning the transaction was picked up)
        GLOGDEBUG("TEST: Waiting for m_ == " + std::to_string(expectedMachineMemoryValue));
        auto futureMemoryUpdated = std::async(std::launch::async, [&]() {
          while (cometListener.m_ != expectedMachineMemoryValue) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
          }
        });
        REQUIRE(futureMemoryUpdated.wait_for(std::chrono::seconds(10)) != std::future_status::timeout);

        // Log the appHash
        GLOGDEBUG("TEST: AppHash is now: " + bytesToString(cometListener.appHash_));

        // Ensure the block and transaction had the intended effect on the machine
        GLOGDEBUG("TEST: Checking m_ == " + std::to_string(expectedMachineMemoryValue));
        REQUIRE(cometListener.m_ == expectedMachineMemoryValue);
      }

      // Stop comet.
      GLOGDEBUG("TEST: Stopping comet (before restart step)");
      REQUIRE(comet.getStatus()); // no error reported (must check before stop())
      comet.stop();
      GLOGDEBUG("TEST: Stopped");
      REQUIRE(comet.getState() == CometState::STOPPED);

      // Roll back the application state to genesis state.
      GLOGDEBUG("TEST: Rolling back state to genesis");
      cometListener.h_ = 0;
      cometListener.m_ = 0;
      cometListener.updateAppHash(); // since we changed m_

      // Before stopping again, ensure that we are in the running state (otherwise we can
      // finish replay before we get to opening our RPC connection, which generates
      // unnecessary logging and RPC connection retries).
      comet.setPauseState(CometState::RUNNING);

      // Restart comet
      GLOGDEBUG("TEST: Restarting");
      comet.start();
      GLOGDEBUG("TEST: Restarted");

      // Wait until m_ is restored (we don't have to determine in what height this happens).
      GLOGDEBUG("TEST: Waiting for catch up to m_ == " + std::to_string(expectedMachineMemoryValue));
      auto futureMemoryUpdated = std::async(std::launch::async, [&]() {
        while (cometListener.m_ != expectedMachineMemoryValue) {
          std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
      });
      REQUIRE(futureMemoryUpdated.wait_for(std::chrono::seconds(60)) != std::future_status::timeout);
      GLOGDEBUG("TEST: Final check for m_ == " + std::to_string(expectedMachineMemoryValue));
      REQUIRE(cometListener.m_ == expectedMachineMemoryValue);

      // Waits for RUNNING so we know we are past RPC connection established and so we can just stop now.
      REQUIRE(comet.waitPauseState(60000) == "");
      comet.setPauseState(); // unset the pause state (not needed since stop() does it, but nicer)

      // Stop
      GLOGDEBUG("TEST: Stopping...");
      REQUIRE(comet.getStatus()); // no error reported (must check before stop())
      comet.stop();
      GLOGDEBUG("TEST: Stopped");
      REQUIRE(comet.getState() == CometState::STOPPED);
      GLOGDEBUG("TEST: Finished");
    }

    // Validator produces blocks, then launch another non-validator node
    // that connects to it and syncs from some block height.
    SECTION("CometSyncTest") {
      GLOGDEBUG("TEST: Constructing two Comet instances");

      // Instantiate the listener object twice, one for each running Comet instance
      TestMachine cometListener0(true);
      TestMachine cometListener1(true);

      // get free ports to run tests on
      auto ports = generateCometTestPorts(2);

      // Create two test dump (i.e. BDK options rootPath) directories, one for each comet instance.
      // This is needed because each BDK instance only supports one running comet instance normally,
      //   so each options/rootPath has one "comet" subdirectory in it to be the cometbft home dir.
      // numKeys == 2 (keys 0 and 1), and numNonValidators (last param) == 1, since only comet0 is
      //   a validator; comet1 is a nonvalidator that will sync to the chain mined only by comet0.
      std::string testDumpPath0 = createTestDumpPath("CometSyncTest_0");
      const Options options0 = SDKTestSuite::getOptionsForTest(testDumpPath0, false, cometListener0.getAppHashString(), ports[0].p2p, ports[0].rpc, 0, 2, ports, 1);

      std::string testDumpPath1 = createTestDumpPath("CometSyncTest_1");
      const Options options1 = SDKTestSuite::getOptionsForTest(testDumpPath1, false, cometListener1.getAppHashString(), ports[1].p2p, ports[1].rpc, 1, 2, ports, 1);

      // Set up our two running Comet instances
      Comet comet0(&cometListener0, "Comet0", options0);
      Comet comet1(&cometListener1, "Comet1", options1);

      // Start the validator first so the chain can advance to the target block height.
      GLOGDEBUG("TEST: Starting validator (node 0)");
      comet0.start();

      // Need to wait for RUNNING state before sending transactions now unfortunately
      GLOGDEBUG("TEST: Waiting RUNNING state before sending transaction....");
      comet0.setPauseState(CometState::RUNNING);
      REQUIRE(comet0.waitPauseState(30000) == "");
      comet0.setPauseState();

      // Send several transactions across different blocks (stepMode is disabled,
      // so cometbft can produce empty blocks).
      GLOGDEBUG("TEST: Sending several ++m_ transactions...");
      int numTxs = 3;
      int64_t expectedMachineMemoryValue = 0;
      for (int i = 0; i < numTxs; ++i) {

        // transaction increments the memory cell (++cometListener.m_)
        // The second parameter (i) is serving as the nonce (need to uniquify the transaction
        //   otherwise it is understood as a replay).
        std::string transaction = "SIG " + std::to_string(i) + " + 1";
        GLOGDEBUG("TEST: Sending transaction: " + transaction);
        Bytes transactionBytes(transaction.begin(), transaction.end());
        comet0.sendTransaction(transactionBytes);
        ++expectedMachineMemoryValue;

        // Wait for memory cell to update (meaning the transaction was picked up)
        GLOGDEBUG("TEST: Waiting for m_ == " + std::to_string(expectedMachineMemoryValue));
        auto futureMemoryUpdated = std::async(std::launch::async, [&]() {
          while (cometListener0.m_ != expectedMachineMemoryValue) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
          }
        });
        REQUIRE(futureMemoryUpdated.wait_for(std::chrono::seconds(10)) != std::future_status::timeout);

        // Log the appHash
        GLOGDEBUG("TEST: AppHash is now: " + cometListener0.getAppHashString()); //std::to_string(Utils::bytesToInt64(cometListener0.appHash_)));

        // Ensure the block and transaction had the intended effect on the machine
        GLOGDEBUG("TEST: Checking m_ == " + std::to_string(expectedMachineMemoryValue));
        REQUIRE(cometListener0.m_ == expectedMachineMemoryValue);
      }

      // Fetch the current block height (any value here is good, even if this is racing
      // block production, since any h_ value here is guaranteed to have the state after
      // the test transactions above, which is what matters).
      uint64_t comet1StartHeight = cometListener0.h_;
      uint64_t comet1StartMachineMemoryValue = cometListener0.m_;
      Bytes comet1StartAppHash = cometListener0.appHash_;
      std::string comet1StartAppHashStr = bytesToString(comet1StartAppHash);

      GLOGDEBUG(
        "TEST: comet1StartHeight will be " + std::to_string(comet1StartHeight) +
        ", comet1StartMachineMemoryValue will be " + std::to_string(comet1StartMachineMemoryValue) +
        ", comet1StartAppHash will be " + comet1StartAppHashStr
      );

      // Wait for comet0 to produce at least one more block
      auto futureFinalizeBlock = std::async(std::launch::async, [&]() {
        while (cometListener0.h_ <= comet1StartHeight) {
          std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
      });
      REQUIRE(futureFinalizeBlock.wait_for(std::chrono::seconds(10)) != std::future_status::timeout);
      REQUIRE(cometListener0.h_ > comet1StartHeight);

      // Add another set of transactions to produce a final target state
      // Send several transactions across different blocks (stepMode is disabled,
      // so cometbft can produce empty blocks).
      GLOGDEBUG("TEST: Sending more ++m_ transactions...");
      int startTxs = numTxs;
      numTxs = 3;
      for (int i = startTxs; i < startTxs + numTxs; ++i) {

        // transaction increments the memory cell (++cometListener.m_)
        // The second parameter (i) is serving as the nonce (need to uniquify the transaction
        //   otherwise it is understood as a replay).
        std::string transaction = "SIG " + std::to_string(i) + " + 1";
        GLOGDEBUG("TEST: Sending transaction: " + transaction);
        Bytes transactionBytes(transaction.begin(), transaction.end());
        comet0.sendTransaction(transactionBytes);
        ++expectedMachineMemoryValue;

        // Wait for memory cell to update (meaning the transaction was picked up)
        GLOGDEBUG("TEST: Waiting for m_ == " + std::to_string(expectedMachineMemoryValue));
        auto futureMemoryUpdated = std::async(std::launch::async, [&]() {
          while (cometListener0.m_ != expectedMachineMemoryValue) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
          }
        });
        REQUIRE(futureMemoryUpdated.wait_for(std::chrono::seconds(10)) != std::future_status::timeout);

        // Log the appHash
        GLOGDEBUG("TEST: AppHash is now: " + cometListener0.getAppHashString());

        // Ensure the block and transaction had the intended effect on the machine
        GLOGDEBUG("TEST: Checking m_ == " + std::to_string(expectedMachineMemoryValue));
        REQUIRE(cometListener0.m_ == expectedMachineMemoryValue);
      }

      // Here comet0 will just keep running.
      // Whatever h_ comet0 is in now will be the sync target for comet1, as it will already
      //   have the second batch of test transactions applied to it.
      uint64_t comet1TargetHeight = cometListener0.h_;
      uint64_t comet1TargetMachineMemoryValue = cometListener0.m_;
      Bytes comet1TargetAppHash = cometListener0.appHash_;
      std::string comet1TargetAppHashStr = bytesToString(comet1TargetAppHash);
      GLOGDEBUG(
        "TEST: comet1TargetHeight will be " + std::to_string(comet1TargetHeight) +
        ", comet1TargetMachineMemoryValue will be " + std::to_string(comet1TargetMachineMemoryValue) +
        ", comet1TargetAppHash will be " + comet1TargetAppHashStr
      );

      // Now we switch to testing comet1
      // Start by rigging the cometListener1 to start at the start height and memory value,
      //   as if it had loaded a BDK app state DB/snapshot.
      // NOTE: These will be simply ignored, since cometbft height is 0 (this is a fresh node).
      //       TestMachine will force its h_ to genesis when it is notified of this.
      // TODO: In a future version, the Comet driver will use cometbft state sync,
      //       snapshots, light-client verification, etc. instead and this test will
      //       actually start the node1 instance from comet1StartHeight and its current
      //       comet1StartMachineMemoryValue and comet1StartAppHash, without reverting to
      //       genesis state.
      cometListener1.h_ = comet1StartHeight;
      cometListener1.m_ = comet1StartMachineMemoryValue;
      cometListener1.appHash_ = comet1StartAppHash;

      // Start comet1 at the start height
      GLOGDEBUG("TEST: Starting non-validator (node 1) at comet1Start* values as logged above");
      comet1.start();

      // Wait for comet1 to reach the taget height
      auto futureFinalizeBlock1 = std::async(std::launch::async, [&]() {
        while (cometListener1.h_ <= comet1TargetHeight) {
          std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
      });
      REQUIRE(futureFinalizeBlock1.wait_for(std::chrono::seconds(10)) != std::future_status::timeout);
      REQUIRE(cometListener1.h_ > comet1TargetHeight);

      // comet1 must have synced to the target state
      REQUIRE(cometListener1.m_ == comet1TargetMachineMemoryValue);
      REQUIRE(cometListener1.appHash_ == comet1TargetAppHash);

      // Stop both cometbft instances
      GLOGDEBUG("TEST: Stopping both instances...");
      REQUIRE(comet0.getStatus()); // no error reported (must check before stop())
      comet0.stop();
      REQUIRE(comet1.getStatus()); // no error reported (must check before stop())
      comet1.stop();
      GLOGDEBUG("TEST: Stopped both instances");
      REQUIRE(comet0.getState() == CometState::STOPPED);
      REQUIRE(comet1.getState() == CometState::STOPPED);
      GLOGDEBUG("TEST: Finished");
    }

    // Start chain with two validators 0 and 1 (both are required to advance the chain since need 2/3 votes).
    // Change validator set to add validator 2.
    // Change validator set to remove validator 0 (now both 1 and 2 are required to advance the chain, but not 0).
    // Stop validator 0, verify that chain continues advancing normally.
    SECTION("CometValidatorSetTest") {
      GLOGDEBUG("TEST: Constructing three Comet instances");

      // IMPORTANT: Catch2 REQUIRE() isn't thread-safe!
      // Need to sync all the CometListener callbacks doing REQUIRE()s,
      static std::mutex requireMutex;

      // Listener that tracks the validator set
      class TestCometListener : public CometListener {
      public:
        std::atomic<uint64_t> finalizedHeight_ = 0;
        virtual void initChain(
          const uint64_t genesisTime, const std::string& chainId, const Bytes& initialAppState, const uint64_t initialHeight,
          const std::vector<CometValidatorUpdate>& initialValidators, Bytes& appHash
        ) override
        {
          appHash.clear();
          GLOGDEBUG("TestCometListener: got initChain");
          // the genesis state has nodes 0 and 1 as validators only
          {
            std::scoped_lock lock(requireMutex);
            REQUIRE(initialValidators.size() == 2);
            REQUIRE(initialValidators[0].publicKey.size() == 33); // Secp256k1 keys are 33 bytes
            REQUIRE(initialValidators[1].publicKey.size() == 33);
          }
          std::string vs0 = base64::encode_into<std::string>(initialValidators[0].publicKey.begin(), initialValidators[0].publicKey.end());
          std::string vs1 = base64::encode_into<std::string>(initialValidators[1].publicKey.begin(), initialValidators[1].publicKey.end());
          // for some reason they can be flipped around (we can't use the order in the vector, although we should be able to get them in order...)
          // so we have to search the validator keys in the whole vector
          bool foundKey0 = vs0 == cometTestKeys[0].pub_key || vs1 == cometTestKeys[0].pub_key;
          bool foundKey1 = vs0 == cometTestKeys[1].pub_key || vs1 == cometTestKeys[1].pub_key;
          {
            std::scoped_lock lock(requireMutex);
            REQUIRE(foundKey0);
            REQUIRE(foundKey1);
          }
        }
        virtual void incomingBlock(
          const uint64_t syncingToHeight, std::unique_ptr<CometBlock> block, Bytes& appHash,
          std::vector<CometExecTxResult>& txResults, std::vector<CometValidatorUpdate>& validatorUpdates
        ) override
        {
          GLOGDEBUG("TestCometListener: got incomingBlock " + std::to_string(block->height));
          finalizedHeight_ = block->height;
          appHash.clear();

          // At height == 2, we add node 2, so now the validator set has nodes 0, 1, and 2
          if (block->height == 2) {
            CometValidatorUpdate update;
            update.publicKey = base64::decode_into<Bytes>(cometTestKeys[2].pub_key);
            {
              std::scoped_lock lock(requireMutex);
              REQUIRE(update.publicKey.size() == 33);
            }
            update.power = 10;
            validatorUpdates.push_back(update);
          }

          // At height == 5, we remove node 0, so now the validator set has nodes 1 and 2 only
          if (block->height == 5) {
            CometValidatorUpdate update;
            update.publicKey = base64::decode_into<Bytes>(cometTestKeys[0].pub_key);
            {
              std::scoped_lock lock(requireMutex);
              REQUIRE(update.publicKey.size() == 33);
            }
            update.power = 0;
            validatorUpdates.push_back(update);
          }
        }
      };
      TestCometListener cometListener0;
      TestCometListener cometListener1;
      TestCometListener cometListener2;

      // get free ports to run tests on
      auto ports = generateCometTestPorts(3);

      // Create nodes 0 and 1 as validators, and node 2 as a non-validator (it will be promoted to validator later)
      // The validator/non-validator setup here affects the validator set for genesis; we're free to change the validator
      //   set as we go as the non-validator nodes also get public/private validator keypairs even if those aren't initially
      //   listed in the genesis validator set.
      std::string testDumpPath0 = createTestDumpPath("CometValidatorSetTest_0");
      const Options options0 = SDKTestSuite::getOptionsForTest(testDumpPath0, false, "", ports[0].p2p, ports[0].rpc, 0, 3, ports, 1);
      std::string testDumpPath1 = createTestDumpPath("CometValidatorSetTest_1");
      const Options options1 = SDKTestSuite::getOptionsForTest(testDumpPath1, false, "", ports[1].p2p, ports[1].rpc, 1, 3, ports, 1);
      std::string testDumpPath2 = createTestDumpPath("CometValidatorSetTest_2");
      const Options options2 = SDKTestSuite::getOptionsForTest(testDumpPath2, false, "", ports[2].p2p, ports[2].rpc, 2, 3, ports, 1);

      // Create the three nodes
      Comet comet0(&cometListener0, "Comet0", options0);
      Comet comet1(&cometListener1, "Comet1", options1);
      Comet comet2(&cometListener2, "Comet2", options2);

      // Start all of them
      GLOGDEBUG("TEST: Starting all validators");
      comet0.start();
      comet1.start();
      comet2.start();

      // Wait for block 8 on validator 1
      GLOGDEBUG("TEST: Waiting for node 1 to reach block 8...");
      auto futureFinalizeBlock = std::async(std::launch::async, [&]() {
        while (cometListener1.finalizedHeight_ < 8) {
          std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
      });
      REQUIRE(futureFinalizeBlock.wait_for(std::chrono::seconds(60)) != std::future_status::timeout);
      REQUIRE(cometListener1.finalizedHeight_ >= 8);

      // Stop node 0
      GLOGDEBUG("TEST: Stopping node 0 (chain must continue since node0 is no longer a validator)...");
      REQUIRE(comet0.getStatus()); // no error reported (must check before stop())
      comet0.stop();
      REQUIRE(comet0.getState() == CometState::STOPPED);
      GLOGDEBUG("TEST: Stopped node 0");

      // Wait for block 11 on validator 1
      GLOGDEBUG("TEST: Waiting for node 1 to reach block 11...");
      auto futureFinalizeBlock2 = std::async(std::launch::async, [&]() {
        while (cometListener1.finalizedHeight_ < 11) {
          std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
      });
      REQUIRE(futureFinalizeBlock2.wait_for(std::chrono::seconds(20)) != std::future_status::timeout);
      REQUIRE(cometListener1.finalizedHeight_ >= 11);

      // Stop both remaining cometbft instances
      GLOGDEBUG("TEST: Stopping node 1 and node 2...");
      REQUIRE(comet1.getStatus()); // no error reported (must check before stop())
      comet1.stop();
      REQUIRE(comet2.getStatus()); // no error reported (must check before stop())
      comet2.stop();
      GLOGDEBUG("TEST: Stopped node1 and node 2.");
      REQUIRE(comet1.getState() == CometState::STOPPED);
      REQUIRE(comet2.getState() == CometState::STOPPED);
      GLOGDEBUG("TEST: Finished");
    }

    // Test Comet RPC call API
    SECTION("CometRPCCallTest") {
      std::string testDumpPath = createTestDumpPath("CometRPCCallTest");

      GLOGDEBUG("TEST: Constructing Comet");

      int p2p_port = SDKTestSuite::getTestPort();
      int rpc_port = SDKTestSuite::getTestPort();
      const Options options = SDKTestSuite::getOptionsForTest(testDumpPath, false, "", p2p_port, rpc_port);

      // Just use the dummy default listener
      CometListener cometListener;
      Comet comet(&cometListener, "", options);

      // Set pause at inspect so we can use RPC calls on inspect
      comet.setPauseState(CometState::INSPECT_RUNNING);
      GLOGDEBUG("TEST: Starting comet...");
      comet.start();
      GLOGDEBUG("TEST: Waiting for cometbft inspect RPC to be up...");
      REQUIRE(comet.waitPauseState(10000) == "");

      // Make an RPC call
      GLOGDEBUG("TEST: Making rpcSyncCall()...");
      json healthResult;
      bool success = comet.rpcSyncCall("header", json::object(), healthResult);
      GLOGDEBUG("TEST: rpcSyncCall() result: " + healthResult.dump());
      REQUIRE(success == true);
      // expect null block header from latest block since the chain is empty
      REQUIRE(healthResult.contains("result"));
      REQUIRE(healthResult["result"].contains("header"));
      REQUIRE(healthResult["result"]["header"].is_null());

      // Don't need to unpause, can just stop
      GLOGDEBUG("TEST: Stopping...");
      REQUIRE(comet.getStatus()); // no error reported (must check before stop())
      comet.stop();
      GLOGDEBUG("TEST: Stopped");
      REQUIRE(comet.getState() == CometState::STOPPED);
      GLOGDEBUG("TEST: Finished");
    }

    // setpriv test. setpriv is not *really* optional -- you must have setpriv in your path
    // to run the tests, otherwise this test will just fail.
    SECTION("CometSetprivTest") {
      // setpriv must be available, if not always then at least for running tests
      boost::filesystem::path setpriv_path = boost::process::search_path("setpriv");
      REQUIRE(!setpriv_path.empty());

      std::string testDumpPath = createTestDumpPath("CometSetprivTest");
      int p2p_port = SDKTestSuite::getTestPort();
      int rpc_port = SDKTestSuite::getTestPort();
      const Options options = SDKTestSuite::getOptionsForTest(testDumpPath, false, "", p2p_port, rpc_port);
      CometListener cometListener;
      Comet comet(&cometListener, "", options);

      // Expected test behavior (if tasks take a minimally-reasonable time to complete):
      // < 5s: child process has cometbft inspect server running
      // at 5s: parent process sees RPC port to cometbft inspect is open
      // at 10s: child process terminates itself, making setpriv-wrapped cometbft inspect
      //         terminate itself soon after.
      // at 15s: parent process sees RPC port to cometbft inspect is closed

      // Spawn a disposable child process of the tester with a different PID that will actually call
      // comet.start(), and then the parent tester process will kill that process, which should kill
      // its child process (cometbft inspect) due to its setpriv wrapper.
      pid_t pid = fork();
      // Fork must have succeeded
      REQUIRE(pid != -1);
      if (pid == 0) {
        // Redirect stdout and stderr to /dev/null so we don't get any duplicate
        // Catch2 harness output generated, which is confusing and which we do get
        // if the prctrl() below goes into effect (i.e. we get a SIGTERM)
        int devnull = open("/dev/null", O_WRONLY);
        if (devnull == -1) {
          std::cerr << "ERROR: CometSetprivTest: can't open /dev/null" << std::endl;
          _exit(1); // _exit with underscore guarantees we immediately die
        }
        dup2(devnull, STDOUT_FILENO);
        dup2(devnull, STDERR_FILENO);
        close(devnull);
        // This is the tester child process, which wraps setpriv, which wraps cometbft inspect
        // Make sure this child process dies if the parent tester dies first for whatever reason
        // so we don't get dangling processes.
        std::cout << "CometSetprivTest: In child process." << std::endl;
        if (prctl(PR_SET_PDEATHSIG, SIGTERM) == -1) {
          std::cerr << "ERROR: CometSetprivTest: prctl() failed" << std::endl;
          _exit(1); // _exit with underscore guarantees we immediately die
        }
        // Hold the driver at "cometbft inspect" running, which is enough to test
        comet.setPauseState(CometState::INSPECT_RUNNING);
        // Only start the driver in the child process
        comet.start();
        comet.waitPauseState(10000);
        std::cout << "CometSetprivTest: Child process reached CometState::INSPECT_RUNNING" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(10));
        std::cout << "CometSetprivTest: Child process forcefully terminating itself" << std::endl;
        // We just kill ourselves after exactly 10 seconds -- we don't receive an
        // external SIGTERM, which is problematic. Instead, which is easier and simpler,
        // we just quit, so we can check that the setpriv mechanism is working.
        // This emulates any kind of failure condition where the BDK node (parent of the
        // cometbft process) dies for whatever reason.
        _exit(0); // _exit with underscore guarantees we immediately die
      }

      // Everything from here on runs in the parent tester process only
      GLOGDEBUG("TEST: Waiting 5s to test RPC port of child with PID = " + std::to_string(pid));

      // Give the child time to run prctrl() and then actually start cometbft inspect
      std::this_thread::sleep_for(std::chrono::seconds(5));

      // try to connect to the cometbft inspect RPC port. this should succeed, meaning that
      // starting cometbft inspect actually worked in the first place.
      GLOGDEBUG("TEST: Testing RPC port (must be open)");
      try {
        boost::asio::io_context io_context;
        boost::asio::ip::tcp::socket socket(io_context);
        boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::make_address("127.0.0.1"), rpc_port);
        socket.connect(endpoint);
        GLOGDEBUG("TEST: cometbft inspect RPC port is open as expected: " + std::to_string(rpc_port));
        socket.close();
      } catch (const std::exception& e) {
        FAIL("TEST: ERROR, failed to connect to RPC port when it should be open: " + std::to_string(rpc_port) + ": " + std::string(e.what()));
      }

      // Give the child time to decide to terminate on its own, and time for
      // the setpriv wrapper to detect that its parent process has died, so
      // it sends SIGTERM to cometbft inspect, which then closes the RPC port.
      GLOGDEBUG("TEST: Waiting another 10s to test RPC port of child with PID = " + std::to_string(pid));
      std::this_thread::sleep_for(std::chrono::seconds(10));

      // try to connect to the cometbft inspect RPC port. this should fail,
      // which is enough for us to conclude that the cometbft inspect process is dead.
      GLOGDEBUG("TEST: Testing RPC port (must be closed)");
      try {
        boost::asio::io_context io_context;
        boost::asio::ip::tcp::socket socket(io_context);
        boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::make_address("127.0.0.1"), rpc_port);
        socket.connect(endpoint);
        FAIL("TEST: ERROR: connection to cometbft inspect RPC port succeeded, but port should be closed: " + std::to_string(rpc_port));
        socket.close();
      } catch (const std::exception& e) {
        GLOGDEBUG("TEST: SUCCESS, failed to connect to cometbft inspect RPC port, which was expected: " + std::string(e.what()));
      }
    }
  }
}

