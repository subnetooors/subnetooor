/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#ifndef COMET_H
#define COMET_H

#include "../utils/options.h"
#include "../utils/logger.h"

#include <unordered_set>
#include <vector>

/// Comet driver states
enum class CometState {
  STOPPED          =  0, ///< Comet is in stopped state (no worker thread started)
  STARTED          =  1, ///< Comet worker thread just started
  CONFIGURING      =  2, ///< Starting to set up comet config
  CONFIGURED       =  3, ///< Finished setting up comet config
  INSPECTING_COMET =  4, ///< Starting cometbft inspect
  INSPECT_RUNNING  =  5, ///< Application can use setPauseState(INSPECT_RUNNING) to keep cometbft inspect open.
  INSPECTED_COMET  =  6, ///< Stopped cometbft inspect; all tests passed
  STARTING_ABCI    =  7, ///< Starting ABCI server on our end
  STARTED_ABCI     =  8, ///< Started ABCI server
  STARTING_COMET   =  9, ///< Running cometbft start
  STARTED_COMET    = 10, ///< cometbft start successful
  TESTING_COMET    = 11, ///< Starting to test cometbft connection
  TESTED_COMET     = 12, ///< Finished cometbft connection test; all tests passed
  RUNNING          = 13, ///< Comet is running
  TERMINATED       = 14, ///< Comet worker somehow ran out of work (this is always an error)
  FINISHED         = 15, ///< Comet worker loop quit (stopped for some explicit reason)
  NONE             = 16  ///< Dummy state to disable state stepping
};

/// Comet error codes so that the application can decide to e.g. retry cometbft start or not
enum class CometError {
  NONE               =  0, ///< No error
  ERROR              =  1, ///< Generic error (when no explicit error code is set prior to an exception being thrown)
  FATAL              =  2, ///< Generic fatal error
  CONFIG             =  3, ///< cometbft config files/dirs error
  DATA               =  4, ///< cometbft data directory has bad/corrupted/unexpected data
  RUN                =  5, ///< Error trying to launch cometbft
  RUN_TIMEOUT        =  6, ///< Timed out while waiting for a cometbft run to complete (e.g. cometbft show-node-id)
  FAIL               =  7, ///< Unexpected failure that might be transient (if not, it may be cometbft data corruption)
  RPC_TIMEOUT        =  8, ///< Timed out while trying to connect to the cometbft RPC port
  RPC_CALL_FAILED    =  9, ///< JSON-RPC call returned an error (JSON-RPC or HTTP error)
  RPC_BAD_RESPONSE   = 10, ///< cometbft RPC method response is invalid (parse error)
  ABCI_SERVER_FAILED = 11, ///< ABCI server (socket connections) closed
  ABCI_TIMEOUT       = 12  ///< Timed out while waiting for an expected ABCI callblack from cometbft
};

/**
 * The result of executing a transaction.
 */
struct CometExecTxResult {
  int64_t     gasWanted = 0; ///< Gas limit set by the transaction.
  int64_t     gasUsed = 0; ///< Gas used by the execution of this transaction as part of the actual block it was included in.
  Bytes       data; ///< Return data generated by the code invoked by the transaction, if any.
  uint32_t    code = 0; ///< 0 for success, any other value is a failure.
};

/**
 * An update to the validator set (adding or removing a validator).
 * A validator removal sets the power to 0, and a validator addition or power change is with a positive voting power value.
 */
struct CometValidatorUpdate {
  Bytes       publicKey; ///< Validator's public key bytes.
  int64_t     power; ///< Validator's new voting power value (0 to remove validator).
};

/**
 * Special values for CometTxStatus::height that signal different reasons for a transaction not being included in a block yet.
 */
enum CometTxStatusHeight : int64_t {
  NONE       = -9, ///< A state that is never set on the object.
  SUBMITTING = -3, ///< Sent the transaction to the cometbft RPC port.
  REJECTED   = -2, ///< cometbft rejected the transaction or it never reached it due to some local communication error.
  SUBMITTED  = -1  ///< cometbft acknowledged and accepted it.
};

/**
 * The current status of a transaction in the network.
 * Instances of this struct will be indexed by the sha3 hash of the transaction in a hash table.
 * If there isn't a CometTxStatus entry in the hash table at all, it means the Comet driver has no idea about it,
 * but that doesn't mean the network doesn't know about it (there's an unconfirmed-transactions method in the
 * cometbft RPC server that can be queried to figure it out, but you'll need cometbft transaction hashes, i.e.
 * SHA256 to locate it).
 */
struct CometTxStatus {
  int64_t           height; ///< Height of the block in which this transaction was included (< 0 if not included in any block).
  int32_t           index; ///< Index of the transaction in the block in which it was included (valid only if height >= 0).
  std::string       cometTxHash; ///< cometbft tx hash (i.e. SHA256) if known.
  CometExecTxResult result; ///< Transaction execution result (valid only if height >= 0).
};

/**
 * The Comet class notifies its user of events through the CometListener interface.
 * Users of the Comet class must implement a CometListener class and pass a pointer
 * to a CometListener object to Comet so that they can receive Comet events.
 *
 * NOTE: These callbacks may or may not be invoked in parallel. You should not assume
 * that these are invoked in any particular order or that they aren't concurrent.
 *
 * NOTE: the return value of all callbacks is set to void because they are reserved
 * for e.g. some status or error handling use; all user return values are outparams.
 *
 * NOTE: Althrough Comet could retrieve the current validator set from cometbft via
 * RPC, it doesn't do that -- the Comet driver does not keep state w.r.t  the current
 * validator set at all -- since validator set changes are entirely controlled by the
 * application anyway; tracking the validator set is to be done by the application,
 * and the application can know deterministically when the validator set changes it
 * drives via CometListener::incomingBlock(..., &validatorUpdates) will take effect:
 *
 * FinalizeBlockResponse.validator_updates, triggered by block H, affect validation for
 * blocks H+1, H+2, and H+3. Heights following a validator update are affected in the
 * following way:
 * - Height H+1: NextValidatorsHash includes the new validator_updates value.
 * - Height H+2: The validator set change takes effect and ValidatorsHash is updated.
 * - Height H+3: *_last_commit fields in PrepareProposal, ProcessProposal, and
 *               FinalizeBlock now include the altered validator set.
 *
 * From: https://docs.cometbft.com/v1.0/spec/abci/abci++_methods#finalizeblock
 */
class CometListener {
  public:
    /**
     * Called upon starting a fresh blockchain (i.e. empty block storage) from a given genesis config.
     * @param genesisTime Genesis block time in seconds since epoch.
     * @param chainId The chain ID.
     * @param initialAppState Serialized initial application state.
     * @param initialHeight The application's initial block height.
     * @param initialValidators The genesis validator set.
     * @param appHash Outparam to be set to the application's initial app hash.
     */
    virtual void initChain(
      const uint64_t genesisTime, const std::string& chainId, const Bytes& initialAppState, const uint64_t initialHeight,
      const std::vector<CometValidatorUpdate>& initialValidators, Bytes& appHash
    ) {
      appHash.clear();
    }

    /**
     * Check if a transaction is valid.
     * @param tx The transaction to check.
     * @param gasWanted Outparam to be set to the gas_limit of this transaction (leave unmodified if unknown/irrelevant).
     * @param accept Outparam to be set to `true` if the transaction is valid, `false` if it is invalid.
     */
    virtual void checkTx(const Bytes& tx, int64_t& gasWanted, bool& accept) {
      accept = true;
    }

    /**
     * Notification of a new finalized block added to the chain.
     * @param height The block height of the new finalized block at the new head of the chain.
     * @param syncingToHeight If the blockchain is doing a replay, syncingToHeight > height, otherwise syncingToHeight == height.
     * @param txs All transactions included in the block, which need to be processed into the application state.
     * @param proposerAddr Address of the validator that proposed the block.
     * @param timeNanos Block timestamp in nanoseconds since epoch.
     * @param appHash Outparam to be set with the hash of the application state after all `txs` are processed into it.
     * @param txResults Outparam to be filled in with the result of executing each transaction in the `txs` vector (indices must match).
     * @param validatorUpdates Outparam to be filled with the validator updates generated as a side-effect of applying this block to the app state.
     */
    virtual void incomingBlock(
      const uint64_t height, const uint64_t syncingToHeight, const std::vector<Bytes>& txs, const Bytes& proposerAddr, const uint64_t timeNanos,
      Bytes& appHash, std::vector<CometExecTxResult>& txResults, std::vector<CometValidatorUpdate>& validatorUpdates
    ) {
      appHash.clear();
      txResults.resize(txs.size());
    }

    /**
     * Validator node that is the block proposer now needs to build a block.
     * @param txs Transactions that cometbft took from the mempool and that it wants to include in the block proposal.
     * @param delTxIds Outparam to be set with the indices in `txs`  that are to be excluded from the block proposal.
     */
    virtual void buildBlockProposal(const std::vector<Bytes>& txs, std::unordered_set<size_t>& delTxIds) {
      // By default, just copy the recommended txs from the mempool into the proposal.
      // This requires all block size and limit related params to be set in such a way that the
      // total byte size of txs in the request don't exceed the total byte size allowed for a block.
    }

    /**
     * Validator node receives a block proposal from the block proposer, and must check if the proposal is a valid one.
     * @param height The block height of the new block being proposed.
     * @param txs All transactions included in the block, which need to be verified.
     * @param accept Outparam to be set to `true` if the proposed block is valid, `false` otherwise.
     */
    virtual void validateBlockProposal(const uint64_t height, const std::vector<Bytes>& txs, bool& accept) {
      accept = true;
    }

    /**
     * Callback from cometbft to check what is the current state of the application.
     * @param height Outparam to be set with the height of the last block processed to generate the current application state.
     * @param appHash Outparam to be set with the hash of the current application state (i.e. the state at `height`).
     * @param appSemVer Outparam to be set with the application's semantic version string e.g. "1.0.0" (logged in every block).
     * @param appVersion Outparam with the uint version of the application (0 if never modified from its original/genesis version).
     */
    virtual void getCurrentState(uint64_t& height, Bytes& appHash, std::string& appSemVer, uint64_t& appVersion) {
      height = 0;
      appHash.clear();
      appSemVer = "1.0.0";
      appVersion = 0;
    }

    /**
     * Callback from cometbft to check if it can prune some old blocks from its block store.
     * @param height Outparam to be set with the height of the earliest block that must be kept (all earlier blocks are deleted).
     */
    virtual void getBlockRetainHeight(uint64_t& height) {
      height = 0;
    }

    /**
     * Notification of what the cometbft block store height is. If the application is ahead, it can bring itself to a height
     * that is equal or lower than this, or it can prepare to report a correct app_hash and validator set changes after
     * each incomingBlock() callback that it will get with a height that is lower or equal than its current heigh without
     * computing state (i.e. feeding it to Comet via stored historical data that it has computed in the past or that it has
     * obtained off-band). The application is free to block this callback for any amount of time.
     * @param height The current head height in the cometbft block store (the height that the cometbft node/db is at).
     */
    virtual void currentCometBFTHeight(const uint64_t height) {
    }

    /**
     * Notification of completing a Comet::sendTransaction() RPC request.
     * @param tId The sendTransaction() request ticket ID.
     * @param tx Transaction that was previously sent via `Comet::sendTransaction()`.
     * @param success `true` if sendTransaction() succeeded, `false` if the transaction failed to send.
     * @param txHash Transaction hash as CometBFT sees it (SHA256: https://docs.cometbft.com/main/spec/core/encoding).
     * @param response The full JSON-RPC response returned by cometbft.
     */
    virtual void sendTransactionResult(
      const uint64_t tId, const Bytes& tx, const bool success, const std::string& txHash, const json& response
    ) {
    }

    /**
     * Notification of completing a Comet::checkTransaction(txHash) RPC request.
     * @param tId The checkTransaction() request ticket ID.
     * @param txHash The transaction hash that was checked.
     * @param success Whether the transaction check ('tx' JSON-RPC method call) was successful or not.
     * @param response The full JSON-RPC response returned by cometbft.
     */
    virtual void checkTransactionResult(
      const uint64_t tId, const std::string& txHash, const bool success, const json& response
    ) {
    }

    /**
     * Notification of completing an asynchronous Comet::rpcAsyncCall() RPC request.
     * @param tId The checkTransaction() request ticket ID.
     * @param method Method name for the RPC.
     * @param params Parameters for the RPC.
     * @param success Whether the RPC was successful or not.
     * @param response The full JSON-RPC response returned by cometbft.
     */
    virtual void rpcAsyncCallResult(
      const uint64_t tId, const std::string& method, const json& params, const bool success, const json& response
    ) {
    }

    /**
     * Notification of a state change in the Comet consensus engine driver.
     *
     * In TERMINATED, FINISHED or STOPPED state, the receiver of this callback can (in *another thread*)
     * can call stop(), then start() again if they want to retry. At the point the TERMINATED state is
     * reached, the error code and the error string are already set.
     *
     * There will be no retries, i.e. no "continue;" in the WorkerLoopInner. Every error is a fatal error
     * reported here for the application to figure out.
     *
     * @param newState The new CometState.
     * @param oldState The previous CometState.
     */
    virtual void cometStateTransition(const CometState newState, const CometState oldState) {
    }
};

class CometImpl;

/**
 * The Comet class is instantiated by the BDK node to serve as an interface to CometBFT.
 * Most of its implementation details are private and contained in CometImpl, which is
 * declared and defined in comet.cpp, in order to keep this header short and simple.
 *
 * NOTE: Comet first searches for a cometbft executable in the current working directory;
 * if it is not found, then it will search for it in the system PATH.
 */
class Comet : public Log::LogicalLocationProvider {
  private:
    const std::string instanceIdStr_; ///< Identifier for logging
    std::unique_ptr<CometImpl> impl_; ///< Private implementation details of a Comet instance.

  public:
    /**
     * Constructor.
     * NOTE: Comet only reads configuration options from the "cometBFT" key in Options.
     * These are the top-level keys found under the "cometBFT" key:
     * - "genesis.json": fully overwrites config/genesis.json
     * - "priv_validator_key.json": fully overwrites config/priv_validator_key.json
     * - "node_key.json": fully overwrites config/node_key.json
     * - "config.toml": json object that describes hierarchical key/value pairs to
     *   overwrite individual entries in the config/config.toml TOML config file
     * For example, the P2P and RPC ports are set like this in Options:
     *   "cometBFT": {
     *     "config.toml": {
     *       "p2p": {
     *          "laddr": "tcp://0.0.0.0:26656"
     *       },
     *       "rpc": {
     *          "laddr": "tcp://0.0.0.0:26657"
     *       }
     *     }
     *   }
     * NOTE: Some values in config.toml are hardcoded in the Comet driver and are
     * overwritten irrespective of what's specified in cometBFT::
     * @param listener Pointer to an object of a class that implements the CometListener interface
     *                 and that will receive event callbacks from this Comet instance.
     * @param instanceIdStr Instance ID string to use for logging.
     * @param options Reference to the Options singleton.
     */
    explicit Comet(CometListener* listener, std::string instanceIdStr, const Options& options);

    /**
     * Destructor; ensures all subordinate jobs are stopped.
     */
    virtual ~Comet();

    /**
     * Set the transaction cache size (if 0, the cache is disabled).
     * When the cache is enabled, transactions will get their eth-compatible hash computed,
     * and Comet will keep the most recent transactions it has seen in the cache with their
     * status (submitted, included in a block at some height, etc).
     * @param cacheSize New cache capacity, in transaction count (if zero, disable the cache).
     */
    void setTransactionCacheSize(const uint64_t cacheSize);

    /**
     * Get the global status of the Comet worker.
     * @return `true` if it is in a working state, `false` if it is in a failed/terminated state.
     */
    bool getStatus();

    /**
     * Get the last error message from the Comet worker (not synchronized; call only after
     * getStatus() returns false).
     * @return Error message or empty string if no error.
     */
    const std::string& getErrorStr();

    /**
     * Get the last error code from the Comet worker (not synchronized; call only after
     * getStatus() returns false).
     * @return Error code (of CometError enum type); CometError::NONE if no error.
     */
    CometError getErrorCode();

    /**
     * Get the current state of the Comet worker (or last known state before failure/termination;
     * must also call getStatus()).
     * @return The current state.
     */
    CometState getState();

    /**
     * Set the pause state (this is not set by start(), but is set to NONE by stop()).
     * The pause state is a state that the comet worker thread stops at when it is reached, which
     * is useful for writing unit tests for the Comet class.
     * @param pauseState State to pause at, or CometState::NONE to disable/unpause.
     */
    void setPauseState(const CometState pauseState = CometState::NONE);

    /**
     * Get the pause state.
     * @return The current pause state.
     */
    CometState getPauseState();

    /**
     * Busy wait for the pause state, a given timeout in milliseconds, or an error status.
     * @param timeoutMillis Timeout in milliseconds, or zero to wait forever.
     * @return If error status set, the error string, or "TIMEOUT" in case of timeout, or an empty string if OK.
     */
    std::string waitPauseState(uint64_t timeoutMillis);

    /**
     * Get the cometbft node ID; this is only guaranteed to be set from the INSPECTED_COMET state and ownards.
     * @return The cometbft node id string, or an empty string if unknown (not yet computed) at this point.
     */
    std::string getNodeID();

    /**
     * Enqueues a transaction to be sent to the cometbft node; will retry until the localhost cometbft
     * instance is running and it acknowledges the receipt of the transaction bytes, meaning it should
     * be in its mempool from now on (if it passes CheckTx, etc).
     * If the ethash parameter is not provided, the transaction will not be added to the transaction
     * cache at the time of its creation.
     * @param tx The raw bytes of the transaction object.
     * @param ethHash If ethHash is nullptr, won't compute the sha3 hash and won't track the transaction
     * in the internal tx cache. If ethHash points to a shared_ptr that is nullptr, then it will create
     * a shared Hash and store it in ethHash, and track the transaction in the tx cache. If ethHash
     * points to a shared_ptr that is not nullptr, the function will assume the Hash value provided is
     * the eth hash of tx and use it when storing the tx in the tx cache.
     * @return If > 0, the ticket number for the transaction send request (unique for one Comet object
     * instantiation), or 0 if ethHash is not nullptr, the internal tx cache is enabled, and the
     * transaction was not sent because there's already an entry for the transaction in the cache and
     * its height is not CometTxStatusHeight::REJECTED (which allows for the transaction to be resent).
     */
    uint64_t sendTransaction(const Bytes& tx, std::shared_ptr<Hash>* ethHash = nullptr);

    /**
     * Enqueue a request to check the status of a transaction given its hash (CometBFT hash, i.e. SHA256).
     * This will query the cometbft RPC /tx endpoint, not the internal tx cache.
     * @param txHash A hex string (no "0x") with the SHA256 hex of the transaction to look for.
     * @return The ticket number for the transaction check request, or 0 on error (RPC call not made).
     */
    uint64_t checkTransaction(const std::string& txHash);

    /**
     * Check the status of a transaction given its eth hash.
     * This will query the internal tx cache (which is also updated when FinalizedBlock is processed),
     * instead of the cometbft RPC /tx endpoint and so returns the result immediately instead of via
     * CometListener::checkTransactionResult().
     * @param txEthHash The binary Ethereum-compatible hash of the transaction.
     * @param txStatus Outparam to be filled with the current status of the transaction if it is found.
     * @return `true` if a CometTxStatus for the tx was found, `false` otherwise (txStatus is unmodified).
     */
    bool checkTransactionInCache(const Hash& txEthHash, CometTxStatus& txStatus);

    /**
     * Make a JSON-RPC call to the RPC endpoint. The Comet engine must be in a state that has cometbft
     * running, with the RPC connection already established, otherwise this method will error out.
     * @param method Name of the JSON-RPC method to call.
     * @param params Parameters to the method call.
     * @param outResult Outparam set to the resulting json (positive error codes are local errors).
     * @return `true` if the call succeeded, `false` if any error occurred ("error" response is set).
     */
    bool rpcSyncCall(const std::string& method, const json& params, json& outResult);

    /**
     * Make an async JSON-RPC call to the RPC endpoint. The Comet engine must be in a state that has cometbft
     * running, with the RPC connection already established, otherwise this method will error out.
     * @param method Name of the JSON-RPC method to call.
     * @param params Parameters to the method call.
     * @return The JSON-RPC request ID generated for the call, or 0 on error.
     */
    uint64_t rpcAsyncCall(const std::string& method, const json& params);

    /**
     * Start (or restart) the consensus engine loop.
     * @return `true` if started, `false` if it was already started (need to call stop() first).
     */
    bool start();

    /**
     * Stop the consensus engine loop; sets the pause state to CometState::NONE and clears out
     * any error message or error code (so collect those before calling stop()).
     * @return `true` if stopped, `false` if it was already stopped (need to call start() first).
     */
    bool stop();

    /**
     * Runs cometbft and returns stdout/stderr produced by it. Throws on any error.
     * NOTE: This is a blocking call intended for running cometbft as a terminal tool
     * (cometbft version, cometbft init, cometbft show-node-id, etc) and not as a server,
     * so if cometbft doesn't return in a few seconds, it is killed and this errors out.
     * @param cometArgs Arguments to pass on to cometbft.
     * @param outStdout Outparam to set to all data sent to stdout (if not nullptr).
     * @param outStderr Outparam to set to all data sent to stderr (if not nullptr).
     */
    static void runCometBFT(const std::vector<std::string>& cometArgs, std::string* outStdout = nullptr, std::string* outStderr = nullptr);

    /**
     * Check that cometbft can be found and has a version that is supported.
     * Throws on any error -- if it doesn't throw an error, then the check passed.
     */
    static void checkCometBFT();

    /**
     * Return an instance (object) identifier for all LOGxxx() messages emitted this class.
     */
    std::string getLogicalLocation() const override { return instanceIdStr_; }
};

///@{
/** Options key "cometBFT" has these configuration option json subkeys. */
inline const std::string COMET_OPTION_GENESIS_JSON = "genesis.json";
inline const std::string COMET_OPTION_PRIV_VALIDATOR_KEY_JSON = "priv_validator_key.json";
inline const std::string COMET_OPTION_NODE_KEY_JSON = "node_key.json";
inline const std::string COMET_OPTION_CONFIG_TOML = "config.toml";
///@}

/// Consensus parameter: PBTS Clock drift.
/// This must be at least 500ms due to leap seconds.
/// Give 1 whole second for clock drift, which is plenty.
#define COMETBFT_PBTS_SYNCHRONY_PARAM_PRECISION_SECONDS 1

/// Consensus parameter: PBTS Maximum network delay for proposal (header) propagation.
/// This cannot be too small or else performance is affected.
/// If it is too large, malicious validators can timestamp blocks up to this amount in the future,
/// which slows downs the network (need to wait for real time to pass until it catches up with the
/// timestamp of the previous block) but that's externally observable and punishable by governance.
/// Five seconds is plenty for the propagation of a timestamp worldwide.
#define COMETBFT_PBTS_SYNCHRONY_PARAM_MESSAGE_DELAY_SECONDS 5

// Use/keep the genesis file parameters instead.
//
// Consensus parameter: Maximum block size in bytes.
// The hard-coded maximum block size in cometbft is 100MB.
//#define COMETBFT_BLOCK_PARAM_MAX_BYTES 104857600
//
// Consensus parameter: Maximum gas in a block. -1 means we give no gas limit for cometbft to
// work with and so we have to deal with gas limits ourselves when generating valid block proposals.
//#define COMETBFT_BLOCK_PARAM_MAX_GAS -1

#endif