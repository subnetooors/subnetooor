/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H

#include "../utils/options.h"
#include "../utils/bucketcache.h"
#include "../utils/finalizedblock.h"
#include "comet.h"
#include "state.h"
#include "storage.h"
#include "../net/http/httpserver.h"
#include "../net/http/noderpcinterface.h"

/**
 * A BDK node.
 * All components must be thread-safe.
 *
 * NOTE: Each Blockchain instance can have one or more file-backed DBs, which
 * will all be managed by storage_ (Storage class) to implement internal
 * features as needed. However, these should not store:
 * (i) any data that the consensus engine (cometbft) already stores and
 * that we have no reason to duplicate, such as blocks (OK to cache in RAM),
 * (ii) State (consistent contract checkpoints/snapshots at some execution
 * height) as those should be serialized/deserialized from/to their own
 * file-backed data structures (we are using the dump-to-fresh-speedb system)
 * (iii) the list of contract types/templates that exist, since that pertains
 * to the binary itself, and should be built statically in RAM on startup (const)
 * (it is OK-ish to store in Storage the range of block heights for which a
 * template is or isn't available, while keeping in mind that these are really
 * consensus parameters, that is, changing these means the protocol is forked).
 * What we are going to store in the node db:
 * - Activation block height for contract templates (if absent, the contract
 * template is not yet enabled) -- this should be directly modifiable by node
 * operators when enabling new contracts (these are soft forks).
 * - Events (we will keep these in the BDK to retain control and speed it up)
 * - Any State metadata we may need (e.g. name of latest state snapshot file
 * that was saved, and the last executed, locally seen, final block height).
 */
class Blockchain : public CometListener, public NodeRPCInterface, public Log::LogicalLocationProvider {
  protected: // protected is correct; don't change.
    /// A <blockHeight, blockIndex> tuple.
    struct TxCacheValueType {
      uint64_t blockHeight; ///< Height of the block this transaction is included in.
      uint64_t blockIndex; ///< Index inside the block this transaction is included in.
    };

    /**
     * RAM cache of FinalizedBlock objects.
     * NOTE: This class is thread-safe.
     */
    class FinalizedBlockCache
    {
    public:
      explicit FinalizedBlockCache(size_t capacity);
      void insert(std::shared_ptr<const FinalizedBlock> x);
      std::shared_ptr<const FinalizedBlock> getByHeight(uint64_t height);
      std::shared_ptr<const FinalizedBlock> getByHash(const Hash& hash);
    private:
      void evictIndices(const std::shared_ptr<const FinalizedBlock>& x);
      std::vector<std::shared_ptr<const FinalizedBlock>> ring_;
      size_t nextInsertPos_;
      size_t capacity_;
      std::map<uint64_t, std::shared_ptr<const FinalizedBlock>> byHeight_;
      std::unordered_map<Hash, std::shared_ptr<const FinalizedBlock>, SafeHash> byHash_;
      std::mutex mutex_;
    };

    const std::string instanceId_; ///< Instance ID for logging.

    // NOTE: Comet should be destructed (stopped) before State, hence it should be declared after it.
    Options options_; ///< Options singleton.
    Storage storage_; ///< BDK persistent store front-end.
    State state_;     ///< Blockchain state.
    Comet comet_;     ///< CometBFT consensus engine driver.
    HTTPServer http_; ///< HTTP server.

    // Pointer to the "latest" block in the blockchain.
    // This is not necessarily the latest block in CometBFT's block store. Rather, it is the
    // block that matches State::getHeight(); e.g. if the machine height is 20 but the CometBFT
    // block store has 100 blocks from genesis, latest_ will point to block 20.
    // In other words, this is the last processed block by the current State.
    // Since the State cannot be ahead of the CometBFT block store, this is guaranteed to exist
    // (unless the machine height is 0, that is, genesis).
    std::atomic<std::shared_ptr<const FinalizedBlock>> latest_;

    // Cache of transaction hash to (block height, block index).
    // It is too expensive to clean up these mappings every time we get a FinalizedBlock
    // evicted from fbCache_. Instead, we just oversize the buckets in txCache_ to make sure
    // they can hold more entries than what will be ever present in the fbCache_.
    BucketCache<Hash, TxCacheValueType, SafeHash> txCache_;

    // Block height to block hash cache
    BucketCache<uint64_t, Hash, SafeHash> blockHeightToHashCache_;

    // RAM finalized block/tx cache
    // We will be doing miscellaneous queries for blocks and assembling FinalizedBlock
    // objects from the query results. We need to cache the FinalizedBlock objects,
    // otherwise series of calls to e.g. getTxByBlockHashAndIndex() will be slow.
    // Since we cache FinalizedBlock's here, we will also use this as the back-end for
    // the GetTx() / GetTxBy...() methods.
    FinalizedBlockCache fbCache_;

    // Cache of pending transactions (substitutes for access to the mempool, for
    // answering RPC queries).
    // We don't actually have access to the CometBFT mempool, but since CometBFT only
    // removes transactions from its mempool when we return false from CheckTx or the
    // transaction is included in a block, we can mirror it exactly from our side.
    std::unordered_map<Hash, std::shared_ptr<TxBlock>, SafeHash> mempool_;

    std::shared_mutex mempoolMutex_; ///< Mutex protecting mempool_.

    bool syncing_ = false; ///< Updated by Blockchain::incomingBlock() when syncingToHeight > height.

    uint64_t persistStateSkipCount_ = 0; ///< Count of non-syncing_ Blockchain::persistState() calls that skipped saveSnapshot().

    std::atomic<bool> started_ = false; ///< Flag to protect the start()/stop() cycle.

    std::mutex incomingBlockLockMutex_; ///< For locking/unlocking block processing.

    std::atomic<bool> incomingBlockLock_ = false; ///< `true` if incomingBlock() is locked.

    /**
     * Helper for BDK RPC services, fetches a CometBFT block via CometBFT RPC.
     */
    bool getBlockRPC(const Hash& blockHash, json& ret);

    /**
     * Helper for BDK RPC services, fetches a CometBFT block via CometBFT RPC.
     */
    bool getBlockRPC(const uint64_t blockHeight, json& ret);

    /**
     * Helper for BDK RPC services, fetches a CometBFT tx via CometBFT RPC.
     */
    bool getTxRPC(const Hash& txHash, json& ret);

    /**
     * Stores a mapping of transaction hash (sha3) to block height and index
     * within the block in the txChache_.
     */
    void putTx(const Hash& tx, const TxCacheValueType& val);

    /**
     * Helper for BDK RPC services.
     */
    static std::tuple<Address, Address, Gas, uint256_t, Bytes> parseMessage(const json& request, bool recipientRequired);

    /**
     * Helper for BDK RPC services.
     */
    json getBlockJson(const FinalizedBlock *block, bool includeTransactions);

  public:

    /// A <TxBlock, blockIndex, blockHeight> tuple.
    struct GetTxResultType {
      std::shared_ptr<TxBlock> txBlockPtr; ///< The transaction object.
      uint64_t blockIndex; ///< Index inside the block this transaction is included in.
      uint64_t blockHeight; ///< Height of the block this transaction is included in.
    };

    // ------------------------------------------------------------------
    // CometListener
    // ------------------------------------------------------------------

    virtual void initChain(
      const uint64_t genesisTime, const std::string& chainId, const Bytes& initialAppState, const uint64_t initialHeight,
      const std::vector<CometValidatorUpdate>& initialValidators, Bytes& appHash
    ) override;
    virtual void checkTx(const Bytes& tx, const bool recheck, int64_t& gasWanted, bool& accept) override;
    virtual void incomingBlock(
      const uint64_t syncingToHeight, std::unique_ptr<CometBlock> block, Bytes& appHash,
      std::vector<CometExecTxResult>& txResults, std::vector<CometValidatorUpdate>& validatorUpdates
    ) override;
    virtual void buildBlockProposal(
      const uint64_t maxTxBytes, const CometBlock& block, bool& noChange, std::vector<size_t>& txIds,
      std::vector<Bytes>& injectTxs
    ) override;
    virtual void validateBlockProposal(const CometBlock& block, bool& accept) override;
    virtual void getCurrentState(uint64_t& height, Bytes& appHash, std::string& appSemVer, uint64_t& appVersion) override;
    virtual void persistState(uint64_t& height) override;
    virtual void currentCometBFTHeight(const uint64_t height, const json& lastBlock) override;
    virtual void sendTransactionResult(const uint64_t tId, const bool success, const json& response, const std::string& txHash, const Bytes& tx) override;
    virtual void checkTransactionResult(const uint64_t tId, const bool success, const json& response, const std::string& txHash) override;
    virtual void rpcAsyncCallResult(const uint64_t tId, const bool success, const json& response, const std::string& method, const json& params) override;
    virtual void cometStateTransition(const CometState newState, const CometState oldState) override;

    // ------------------------------------------------------------------
    // NodeRPCInterface
    // ------------------------------------------------------------------

    virtual json web3_clientVersion(const json& request) override;
    virtual json web3_sha3(const json& request) override;
    virtual json net_version(const json& request) override;
    virtual json net_listening(const json& request) override;
    virtual json eth_protocolVersion(const json& request) override;
    virtual json net_peerCount(const json& request) override;
    virtual json eth_getBlockByHash(const json& request)override;
    virtual json eth_getBlockByNumber(const json& request) override;
    virtual json eth_getBlockTransactionCountByHash(const json& request) override;
    virtual json eth_getBlockTransactionCountByNumber(const json& request) override;
    virtual json eth_chainId(const json& request) override;
    virtual json eth_syncing(const json& request) override;
    virtual json eth_coinbase(const json& request) override;
    virtual json eth_blockNumber(const json& request) override;
    virtual json eth_call(const json& request) override;
    virtual json eth_estimateGas(const json& request) override;
    virtual json eth_gasPrice(const json& request) override;
    virtual json eth_feeHistory(const json& request) override;
    virtual json eth_getLogs(const json& request) override;
    virtual json eth_getBalance(const json& request) override;
    virtual json eth_getTransactionCount(const json& request) override;
    virtual json eth_getCode(const json& request) override;
    virtual json eth_sendRawTransaction(const json& request) override;
    virtual json eth_getTransactionByHash(const json& request) override;
    virtual json eth_getTransactionByBlockHashAndIndex(const json& request) override;
    virtual json eth_getTransactionByBlockNumberAndIndex(const json& request) override;
    virtual json eth_getTransactionReceipt(const json& request) override;
    virtual json eth_getUncleByBlockHashAndIndex(const json& request) override;
    virtual json debug_traceBlockByNumber(const json& request) override;
    virtual json debug_traceTransaction(const json& request) override;
    virtual json txpool_content(const json& request) override;

    std::string getLogicalLocation() const override { return instanceId_; }

    /**
     * Constructor.
     * @param blockchainPath Root filesystem path for this blockchain node reading/writing data.
     * @param instanceId Runtime object instance identifier for logging (for multi-node unit tests).
     */
    explicit Blockchain(const std::string& blockchainPath, std::string instanceId = "");

    /**
     * Constructor.
     * @param options Explicit options object to use.
     * @param instanceId Runtime object instance identifier for logging (for multi-node unit tests).
     */
    explicit Blockchain(const Options& options, std::string instanceId = "");

    virtual ~Blockchain(); ///< Destructor.

    /**
     * Lock block processing.
     * Prevents incomingBlock() Comet callback from executing or being in execution after this call.
     * Blockchain::state().getHeight() will only return a non-changing value after this call.
     */
    void lockBlockProcessing();

    /**
     * Cancels lockBlockProcessing(), unlocking incomingBlock().
     */
    void unlockBlockProcessing();

    /**
     * Get number of unconfirmed txs in the CometBFT mempool.
     * @return Integer value returned by RPC num_unconfirmed_txs in result::num_txs, or -1 on error.
     */
    int getNumUnconfirmedTxs();

    /**
     * Set the size of the GetTx() cache.
     * If you set it to 0, you turn off the cache.
     * NOTE: CometBFT has a lag between finalizing a block and indexing transactions,
     * so if you turn off the cache, you need to take that into consideration when
     * using methods like Storage::getTx() which will hit cometbft with a 'tx' RPC
     * call and possibly fail because the transaction hasn't been indexed yet.
     * @param cacheSize Maximum size in entries for each bucket (two rotating buckets).
     */
    void setGetTxCacheSize(const uint64_t cacheSize);

    /**
     * Save the current machine state to <blockchainPath>/snapshots/<current-State-height>.
     * May throw on error.
     */
    void saveSnapshot();

    void start(); ///< Start the blockchain node.

    void stop(); ///< Stop the blockchain node.

    std::shared_ptr<const FinalizedBlock> latest() const; ///< Get latest finalized block.

    uint64_t getLatestHeight() const; ///< Get the height of the lastest finalized block, or 0 if no blocks (genesis state).

    /**
     * Get a block from the chain using a given hash.
     * @param hash The block hash to get.
     * @return A pointer to the found block, or `nullptr` if block is not found.
     */
    std::shared_ptr<const FinalizedBlock> getBlock(const Hash& hash);

    /**
     * Get a block from the chain using a given height.
     * @param height The block height to get.
     * @return A pointer to the found block, or `nullptr` if block is not found.
     */
    std::shared_ptr<const FinalizedBlock> getBlock(uint64_t height);

    /**
     * Get the block hash given a block height.
     * @param height The block height.
     * @param forceRPC Bypass all local caches and force getting the block hash from CometBFT.
     * @return The block hash, or an empty hash if can't find it.
     */
    Hash getBlockHash(const uint64_t height, bool forceRPC = false);

    /**
     * Get a transaction from the chain using a given hash.
     * @param tx The transaction hash to get.
     * @return A tuple with the found shared_ptr<TxBlock>, transaction index in the block, and block height.
     * @throw DynamicException on hash mismatch.
     */
    Blockchain::GetTxResultType getTx(const Hash& tx);

    /**
     * Get a transaction from a block with a specific index.
     * @param blockHash The block hash
     * @param blockIndex the index within the block
     * @return A tuple with the found shared_ptr<TxBlock>, transaction index in the block, and block height.
     * @throw DynamicException on hash mismatch.
     */
    Blockchain::GetTxResultType getTxByBlockHashAndIndex(const Hash& blockHash, const uint64_t blockIndex);

    /**
     * Get a transaction from a block with a specific index.
     * @param blockHeight The block height
     * @param blockIndex The index within the block.
     * @return A tuple with the found shared_ptr<TxBlock>, transaction index in the block, and block height.
     */
    Blockchain::GetTxResultType getTxByBlockNumberAndIndex(uint64_t blockHeight, uint64_t blockIndex);

    /**
     * Get an uncofirmed transaction from our mirror mempool.
     * @param txHash Hash of the transaction.alignas
     * @return An empty pointer if it is not in the mirror mempool, or a shared_ptr to a TxBlock of the
     * pending transaction if it was found.
     */
    std::shared_ptr<TxBlock> getUnconfirmedTx(const Hash& txHash);

    ///@{
    /** Getter. */
    Options& opt() { return this->options_; }
    Comet& comet() { return this->comet_; }
    State& state() { return this->state_; }
    Storage& storage() { return this->storage_; }
    HTTPServer& http() { return this->http_; }
    ///@}
};

#endif // BLOCKCHAIN_H
