/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#include "libs/catch2/catch_amalgamated.hpp"

#include "core/blockchain.h"

#include "../../sdktestsuite.hpp"

#include <filesystem>

/**
 * Tests for the new Blockchain class.
 * Here we are using and testing the Blockchain class itself, not the SDKTestSuite test helper subclass.
 */
namespace TBlockchain {
  auto createTestDumpPath = SDKTestSuite::createTestDumpPath;
  TEST_CASE("Blockchain Class", "[integration][core][blockchain]") {
    std::string testDumpPath = Utils::getTestDumpPath();

    // TODO: various Blockchain class RPC tests

    SECTION("BlockchainBootTest") {
      std::string testDumpPath = createTestDumpPath("BlockchainBootTest");

      GLOGDEBUG("TEST: creating Options for Blockchain");

      // Standard single node test setup
      int p2p_port = SDKTestSuite::getTestPort();
      int rpc_port = SDKTestSuite::getTestPort();
      const Options options = SDKTestSuite::getOptionsForTest(testDumpPath, false, "", p2p_port, rpc_port, 0, 1, {}, 0);

      GLOGDEBUG("TEST: creating Blockchain");

      // Create a blockchain node
      Blockchain blockchain(options, testDumpPath);

      GLOGDEBUG("TEST: starting Blockchain");

      // Start the blockchain node
      // This waits until it reaches CometState::RUNNING, then starts the RPC endpoint.
      blockchain.start();

      GLOGDEBUG("TEST: stopping Blockchain");

      // Then just check that we can stop it without any problems.
      blockchain.stop();

      GLOGDEBUG("TEST: done");
    }

    SECTION("BlockchainAccountNonceTest") {
      // Perform various account/nonce tests (repeat nonce, nonce sequence w/ same account in same block, ...)
      std::string testDumpPath = createTestDumpPath("BlockchainAccountNonceTest");

      int p2p_port = SDKTestSuite::getTestPort();
      int rpc_port = SDKTestSuite::getTestPort();
      const Options options = SDKTestSuite::getOptionsForTest(testDumpPath, false, "", p2p_port, rpc_port, 0, 1, {}, 0);

      Blockchain blockchain(options, testDumpPath);

      // For this test, we will not do blockchain start() and stop().
      // Instead, we will just fool Blockchain/State and inject some
      //  TxBlock and FinalizedBlock objects we create here, which is
      //  faster, doesn't involve networking, and allows better control.
      // We can write a networked test as well which takes transactions
      //  via the BDK RPC or the CometBFT RPC port (blockchain.comet().rpcSyncCall('tx',... )
      //  but the BDK RPC will validate the tx and prevent us from sending invalid txs to
      //  the mempool.

      // give lots of tokens to account A
      // tx A --> AA nonce 0 X token
      // tx A --> AA nonce 1 X token
      // tx A --> AA nonce 2 X token
      // tx A --> AA nonce 3 X token
      // verify all included in block, balance of AA is now 4*X token

      TestAccount accValidator = TestAccount::newRandomAccount();

      // Here we have to create the CometBFT address that corresponds to the Eth address that we want the coinbase to be set to.
      // Unfortunately this has to be valid otherwise the coinbase processing step in State::processBlock() will blow up.
      Bytes accValidatorPubKeyBytes = Secp256k1::toPub(accValidator.privKey).asBytes();
      blockchain.setValidators({{accValidatorPubKeyBytes, 10}}); // second arg is voting power (irrelevant)
      Bytes accValidatorCometAddress = Comet::getCometAddressFromPubKey(accValidatorPubKeyBytes);

      TestAccount accA = TestAccount::newRandomAccount();
      TestAccount accAA = TestAccount::newRandomAccount();

      blockchain.state().setBalance(accA.address, uint256_t("1000000000000000000000")); // +1,000 eth
      blockchain.state().setBalance(accAA.address, uint256_t("1000000000000000000000")); // +1,000 eth

      uint256_t accAbal0 = blockchain.state().getNativeBalance(accA.address);
      uint256_t accAAbal0 = blockchain.state().getNativeBalance(accAA.address);
      GLOGDEBUG("TEST: accA starting balance: " + accAbal0.str());
      GLOGDEBUG("TEST: accAA starting balance: " + accAAbal0.str());
      REQUIRE(accAbal0 == uint256_t("1000000000000000000000"));
      REQUIRE(accAAbal0 == uint256_t("1000000000000000000000"));

      TxBlock tx_A_AA_0 (
        accAA.address, // to
        accA.address, // from
        Bytes(), // data
        options.getChainID(), // chainId
        0, // nonce
        1000000000000000000, // value
        21000, // maxPriorityFeePerGas
        1000000000, // maxFeePerGas
        1000000000, // gasLimit
        accA.privKey // privKey
      );

      TxBlock tx_A_AA_1 (
        accAA.address, // to
        accA.address, // from
        Bytes(), // data
        options.getChainID(), // chainId
        1, // nonce
        1000000000000000000, // value
        21000, // maxPriorityFeePerGas
        1000000000, // maxFeePerGas
        1000000000, // gasLimit
        accA.privKey // privKey
      );

      TxBlock tx_A_AA_2 (
        accAA.address, // to
        accA.address, // from
        Bytes(), // data
        options.getChainID(), // chainId
        2, // nonce
        1000000000000000000, // value
        21000, // maxPriorityFeePerGas
        1000000000, // maxFeePerGas
        1000000000, // gasLimit
        accA.privKey // privKey
      );

      TxBlock tx_A_AA_3 (
        accAA.address, // to
        accA.address, // from
        Bytes(), // data
        options.getChainID(), // chainId
        3, // nonce
        1000000000000000000, // value
        21000, // maxPriorityFeePerGas
        1000000000, // maxFeePerGas
        1000000000, // gasLimit
        accA.privKey // privKey
      );

      REQUIRE(blockchain.state().validateTransaction(tx_A_AA_0, true) == true);
      REQUIRE(blockchain.state().validateTransaction(tx_A_AA_1, true) == true);
      REQUIRE(blockchain.state().validateTransaction(tx_A_AA_2, true) == true);
      REQUIRE(blockchain.state().validateTransaction(tx_A_AA_3, true) == true);

      // Fake an ABCI block here with the transactions
      CometBlock cometBlock;
      cometBlock.height = 1;
      cometBlock.timeNanos = 1;
      cometBlock.proposerAddr = accValidatorCometAddress;
      cometBlock.txs.push_back(tx_A_AA_0.rlpSerialize(true));
      cometBlock.txs.push_back(tx_A_AA_1.rlpSerialize(true));
      cometBlock.txs.push_back(tx_A_AA_2.rlpSerialize(true));
      cometBlock.txs.push_back(tx_A_AA_3.rlpSerialize(true));
      cometBlock.txs.push_back(Utils::randBytes(32)); // append a randomHash non-tx tx (required by our protocol / FinalizedBlock::fromCometBlock())
      cometBlock.hash.resize(32); // The block hash can be whatever, it's not checked.
      cometBlock.prevHash.resize(32); // The prev block hash can be whatever, it's not checked.

      // Create a BDK FinalizedBlock from the fake ABCI block and send it to the machine state
      std::vector<bool> succeeded;
      std::vector<uint64_t> gasUsed;
      FinalizedBlock finBlock1 = FinalizedBlock::fromCometBlock(cometBlock);
      blockchain.state().processBlock(finBlock1, succeeded, gasUsed);

      REQUIRE(succeeded.size() == 4);
      REQUIRE(succeeded[0] == true);
      REQUIRE(succeeded[1] == true);
      REQUIRE(succeeded[2] == true);
      REQUIRE(succeeded[3] == true);
      REQUIRE(gasUsed[0] == 21000);
      REQUIRE(gasUsed[1] == 21000);
      REQUIRE(gasUsed[2] == 21000);
      REQUIRE(gasUsed[3] == 21000);

      uint256_t accNonce1 = blockchain.state().getNativeNonce(accA.address);
      REQUIRE(accNonce1 == 4);

      uint256_t accAbal1 = blockchain.state().getNativeBalance(accA.address);
      uint256_t accAAbal1 = blockchain.state().getNativeBalance(accAA.address);
      GLOGDEBUG("TEST: accA block 1 balance: " + accAbal1.str());
      GLOGDEBUG("TEST: accAA block 1 balance: " + accAAbal1.str());
      REQUIRE(accAbal1 == uint256_t("995999916000000000000")); // REVIEW: 21 * 4 = 84; 100 - 84 = 16 (21K * 1M = gasPrice for each tx)
      REQUIRE(accAAbal1 == uint256_t("1004000000000000000000"));

      // For the next tests, we will go even deeper and pretend we are the ABCI, and call
      // Blockchain::buildBlockProposal directly with a bunch of weird transactions.
      // This is easier than torturing CometBFT to stuff "bad" transactions in the mempool
      // (which would require us to actually bypass checks that are in the Blockchain class)
      // Afterwards, we manually check for what the block builder did.

      // Test nonce in past, nonce in future OK, and nonce in future with a hole (fails)

      // Build the transactions

      TxBlock tx_A_AA_3_PastNonce (
        accAA.address, // to
        accA.address, // from
        Bytes(), // data
        options.getChainID(), // chainId
        3, // nonce
        1000000000000000000, // value
        21000, // maxPriorityFeePerGas
        1000000000, // maxFeePerGas
        1000000000, // gasLimit
        accA.privKey // privKey
      );

      TxBlock tx_A_AA_4_PresentNonce (
        accAA.address, // to
        accA.address, // from
        Bytes(), // data
        options.getChainID(), // chainId
        4, // nonce
        1000000000000000000, // value
        21000, // maxPriorityFeePerGas
        1000000000, // maxFeePerGas
        1000000000, // gasLimit
        accA.privKey // privKey
      );

      TxBlock tx_A_AA_5_FutureNonce (
        accAA.address, // to
        accA.address, // from
        Bytes(), // data
        options.getChainID(), // chainId
        5, // nonce
        1000000000000000000, // value
        21000, // maxPriorityFeePerGas
        1000000000, // maxFeePerGas
        1000000000, // gasLimit
        accA.privKey // privKey
      );

      TxBlock tx_A_AA_6_TooExpensive (
        accAA.address, // to
        accA.address, // from
        Bytes(), // data
        options.getChainID(), // chainId
        6, // nonce
        uint256_t("10000000000000000000000"), // value 10,000 eth: can't pay so will reject
        21000, // maxPriorityFeePerGas
        1000000000, // maxFeePerGas
        1000000000, // gasLimit
        accA.privKey // privKey
      );

      // Nonce 6 is missing because the tx with nonce 6 is too expensive so it was never added to the mempool
      TxBlock tx_A_AA_7_MissingNonce6 (
        accAA.address, // to
        accA.address, // from
        Bytes(), // data
        options.getChainID(), // chainId
        7, // nonce
        1000000000000000000, // value
        21000, // maxPriorityFeePerGas
        1000000000, // maxFeePerGas
        1000000000, // gasLimit
        accA.privKey // privKey
      );

      // Fake CheckTx calls (not really necessary, but mimics what would happen more closely)
      REQUIRE(blockchain.state().validateTransaction(tx_A_AA_3_PastNonce, true) == false); // expected to fail
      REQUIRE(blockchain.state().validateTransaction(tx_A_AA_4_PresentNonce, true) == true);
      REQUIRE(blockchain.state().validateTransaction(tx_A_AA_5_FutureNonce, true) == true);
      REQUIRE(blockchain.state().validateTransaction(tx_A_AA_6_TooExpensive, true) == false); // expected to fail
      REQUIRE(blockchain.state().validateTransaction(tx_A_AA_7_MissingNonce6, true) == false); // expected to fail

      // Create a more profitable alternative for nonce 4
      TxBlock tx_A_AA_4_PresentNonce_MoreProfitable (
        accAA.address, // to
        accA.address, // from
        Bytes(), // data
        options.getChainID(), // chainId
        4, // nonce
        100000000000000000, // value (10x smaller than value of tx_A_AA_4_PresentNonce)
        21000, // maxPriorityFeePerGas
        2000000000, // maxFeePerGas
        2000000000, // gasLimit
        accA.privKey // privKey
      );
      REQUIRE(blockchain.state().validateTransaction(tx_A_AA_4_PresentNonce_MoreProfitable, true) == true);

      // Fake the CometBlock that is the pre-proposal (just stuff all the txs there)
      cometBlock.height = 2;
      cometBlock.timeNanos = 2;
      cometBlock.txs.clear();
      cometBlock.txs.push_back(tx_A_AA_3_PastNonce.rlpSerialize(true)); // should be excluded by block builder
      cometBlock.txs.push_back(tx_A_AA_4_PresentNonce.rlpSerialize(true)); // should be excluded by block builder
      cometBlock.txs.push_back(tx_A_AA_4_PresentNonce_MoreProfitable.rlpSerialize(true));
      cometBlock.txs.push_back(tx_A_AA_5_FutureNonce.rlpSerialize(true));
      cometBlock.txs.push_back(tx_A_AA_7_MissingNonce6.rlpSerialize(true)); // should be excluded by block builder

      // Call buildBlockProposal
      GLOGDEBUG("TEST: calling buildBlockProposal");
      bool noChange;
      std::vector<size_t> txIds;
      std::vector<Bytes> injectTxs;
      blockchain.buildBlockProposal(100'000'000, cometBlock, noChange, txIds, injectTxs);
      for (const size_t txId : txIds) {
        GLOGDEBUG("TEST: proposal has included txId: " + std::to_string(txId));
      }
      REQUIRE(txIds.size() == 2);
      REQUIRE(txIds[0] == 2); // tx_A_AA_4_PresentNonce_MoreProfitable
      REQUIRE(txIds[1] == 3); // tx_A_AA_5_FutureNonce

      // Fix the block according to the block builder
      std::vector<Bytes> newTxs;
      for (const size_t txId : txIds) {
        newTxs.push_back(cometBlock.txs[txId]);
      }
      cometBlock.txs = newTxs;
      REQUIRE(cometBlock.txs.size() == 2);

      // Send it to processBlock() for good measure.
      succeeded.clear();
      gasUsed.clear();
      cometBlock.txs.push_back(Utils::randBytes(32)); // append a randomHash non-tx tx (required by our protocol / FinalizedBlock::fromCometBlock())
      FinalizedBlock finBlock2 = FinalizedBlock::fromCometBlock(cometBlock);
      blockchain.state().processBlock(finBlock2, succeeded, gasUsed);

      // Check that the transactions picked by the block builder each have the expected outcome.
      uint256_t accAbal2 = blockchain.state().getNativeBalance(accA.address);
      uint256_t accAAbal2 = blockchain.state().getNativeBalance(accAA.address);
      GLOGDEBUG("TEST: accA block 2 balance: " + accAbal2.str());
      GLOGDEBUG("TEST: accAA block 2 balance: " + accAAbal2.str());
      REQUIRE(accAbal2 == uint256_t("994899853000000000000"));
      REQUIRE(accAAbal2 == uint256_t("1005100000000000000000"));

      // Test eject tx from memory model then build a nonce sequence that depends on it.

      // Massively expensive tx
      TxBlock tx_A_AA_6 (
        accAA.address, // to
        accA.address, // from
        Bytes(), // data
        options.getChainID(), // chainId
        6, // nonce
        uint256_t("900000000000000000000"), // value is 900 ETH, account A has 994.89... ETH right now
        21000, // maxPriorityFeePerGas
        1000000000, // maxFeePerGas
        1000000000, // gasLimit
        accA.privKey // privKey
      );

      // Regular tx
      TxBlock tx_A_AA_7 (
        accAA.address, // to
        accA.address, // from
        Bytes(), // data
        options.getChainID(), // chainId
        7, // nonce
        1000000000000000000, // value
        21000, // maxPriorityFeePerGas
        1000000000, // maxFeePerGas
        1000000000, // gasLimit
        accA.privKey // privKey
      );

      // For now, we can afford both, emulate CheckTx
      REQUIRE(blockchain.state().validateTransaction(tx_A_AA_6, true) == true);
      REQUIRE(blockchain.state().validateTransaction(tx_A_AA_7, true) == true);

      // Take 900 ETH out of A, so now it can't afford tx 6, though it can afford tx 7
      uint256_t preHackBalance = accAbal2;
      uint256_t hackedBalance = accAbal2 - uint256_t("900000000000000000000");
      blockchain.state().setBalance(accA.address, hackedBalance); // -900 eth
      uint256_t accAbal2hacked = blockchain.state().getNativeBalance(accA.address);
      GLOGDEBUG("TEST: accA block 2 balance (after -900 eth hack): " + accAbal2hacked.str());
      REQUIRE(accAbal2hacked == uint256_t("94899853000000000000"));

      // Do a recheck for tx 6, but turn off affectsMempool so it will flag it as
      //   ejected in the State's mempoolModel_ instead of just removing it.
      GLOGDEBUG("TEST: forcing flag tx nonce=6 as ejected in the State's mempool model due to insufficient balance");
      REQUIRE(blockchain.state().validateTransaction(tx_A_AA_6, false) == false);

      // Give the money back to A, so now it can afford tx 6 again.
      // But it should no longer matter, as tx 6 has already been ejected from the mempool
      //   according to the State's mempool model.
      GLOGDEBUG("TEST: restoring accA block 2 balance");
      blockchain.state().setBalance(accA.address, preHackBalance); // +900 eth

      // Now send both txs to the block builder, both should be excluded
      cometBlock.height = 2;
      cometBlock.timeNanos = 2;
      cometBlock.txs.clear();
      cometBlock.txs.push_back(tx_A_AA_6.rlpSerialize(true)); // should be excluded by block builder: flagged as ejected
      cometBlock.txs.push_back(tx_A_AA_7.rlpSerialize(true)); // should be excluded by block builder: nonce path deleted
      txIds.clear();
      blockchain.buildBlockProposal(100'000'000, cometBlock, noChange, txIds, injectTxs);
      REQUIRE(txIds.size() == 0);
    }

    SECTION("BlockchainStateDumpTriggerTest") {
      std::string testDumpPath = createTestDumpPath("BlockchainStateDumpTriggerTest");

      GLOGDEBUG("TEST: creating Options for Blockchain");

      int snapshotCount = 5;
      int stateDumpTrigger = 4; // # blocks between auto snapshot save
      int p2p_port = SDKTestSuite::getTestPort();
      int rpc_port = SDKTestSuite::getTestPort();
      const Options options = SDKTestSuite::getOptionsForTest(
        testDumpPath, false, "", p2p_port, rpc_port, 0, 1, {}, 0, stateDumpTrigger, "100ms"
      );

      GLOGDEBUG("TEST: creating Blockchain with stateDumpTrigger = " + std::to_string(stateDumpTrigger));

      Blockchain blockchain(options, testDumpPath);

      GLOGDEBUG("TEST: starting Blockchain");

      blockchain.start();

      GLOGDEBUG("TEST: starting Blockchain");

      // Wait until a minimum height is reached
      while (blockchain.state().getHeight() < stateDumpTrigger * snapshotCount) {
        GLOGDEBUG("TEST: Blockchain height = " + std::to_string(blockchain.state().getHeight()));
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      }

      GLOGDEBUG("TEST: stopping Blockchain");

      blockchain.stop();

      // Check we have created snapshotCount snapshot directories
      std::filesystem::path snapshotsDir = std::filesystem::path(testDumpPath) / "snapshots";
      REQUIRE(std::filesystem::exists(snapshotsDir));
      REQUIRE(std::filesystem::is_directory(snapshotsDir));
      for (int i = 1; i <= snapshotCount; ++i) {
        int expectedHeight = i * stateDumpTrigger;
        std::filesystem::path expectedSnapshotDir = snapshotsDir / std::to_string(expectedHeight);
        GLOGDEBUG("TEST: Checking existence of snapshot directory: " + expectedSnapshotDir.string());
        REQUIRE(std::filesystem::exists(expectedSnapshotDir));
        REQUIRE(std::filesystem::is_directory(expectedSnapshotDir));
      }

      GLOGDEBUG("TEST: done");
    }
  }
}

