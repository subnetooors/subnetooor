/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#include "consensus.h"
#include "blockchain.h"

void Consensus::validatorLoop() {
  LOGINFO("Starting validator loop.");
  Validator me(Secp256k1::toAddress(Secp256k1::toUPub(this->options_.getValidatorPrivKey())));
  while (!this->stop_) {
    std::shared_ptr<const FinalizedBlock> latestBlock = this->storage_.latest();

    // Check if validator is within the current validator list.
    const auto currentRandomList = this->state_.rdposGetRandomList();
    bool isBlockCreator = false;
    if (currentRandomList[0] == me) {
      isBlockCreator = true;
      this->doValidatorBlock();
    }
    if (this->stop_) return;
    if (!isBlockCreator) this->doValidatorTx(latestBlock->getNHeight() + 1, me);

    // Keep looping while we don't reach the latest block
    bool logged = false;
    while (latestBlock == this->storage_.latest() && !this->stop_) {
      if (!logged) {
        LOGDEBUG("Waiting for next block to be created.");
        logged = true;
      }
      // Wait for next block to be created.
      std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
  }
}

void Consensus::doValidatorBlock() {
  // TODO: Improve this somehow.
  // Wait until we are ready to create the block
  auto start = std::chrono::high_resolution_clock::now();
  LOGDEBUG("Block creator: waiting for txs");
  uint64_t validatorMempoolSize = 0;
  std::unique_ptr<uint64_t> lastLog = nullptr;
  while (validatorMempoolSize != this->state_.rdposGetMinValidators() * 2 && !this->stop_) {
    if (lastLog == nullptr || *lastLog != validatorMempoolSize) {
      lastLog = std::make_unique<uint64_t>(validatorMempoolSize);
      LOGDEBUG("Block creator has: " + std::to_string(validatorMempoolSize) + " transactions in mempool");
    }
    validatorMempoolSize = this->state_.rdposGetMempoolSize();
    // Try to get more transactions from other nodes within the network
    for (const auto& nodeId : this->p2p_.getSessionsIDs(P2P::NodeType::NORMAL_NODE)) {
      auto txList = this->p2p_.requestValidatorTxs(nodeId);
      if (this->stop_) return;
      for (const auto& tx : txList) this->state_.addValidatorTx(tx);
    }
    std::this_thread::sleep_for(std::chrono::microseconds(10));
  }
  LOGDEBUG("Validator ready to create a block");

  // Wait until we have all required transactions to create the block.
  auto waitForTxs = std::chrono::high_resolution_clock::now();
  bool logged = false;
  while (this->state_.getMempoolSize() < 1) {
    if (!logged) {
      logged = true;
      LOGDEBUG("Waiting for at least one transaction in the mempool.");
    }
    if (this->stop_) return;

    // Try to get transactions from the network.
    for (const auto& nodeId : this->p2p_.getSessionsIDs(P2P::NodeType::NORMAL_NODE)) {
      LOGDEBUG("Requesting txs...");
      if (this->stop_) break;
      auto txList = this->p2p_.requestTxs(nodeId);
      if (this->stop_) break;
      for (const auto& tx : txList) {
        TxBlock txBlock(tx);
        this->state_.addTx(std::move(txBlock));
      }
    }
    std::this_thread::sleep_for(std::chrono::microseconds(10));
  }

  auto creatingBlock = std::chrono::high_resolution_clock::now();

  // Create the block.
  LOGDEBUG("Ordering transactions and creating block");
  if (this->stop_) return;
  auto mempool = this->state_.rdposGetMempool();
  const auto randomList = this->state_.rdposGetRandomList();

  // Order the transactions in the proper manner.
  std::vector<TxValidator> randomHashTxs;
  std::vector<TxValidator> randomnessTxs;
  uint64_t i = 1;
  while (randomHashTxs.size() != this->state_.rdposGetMinValidators()) {
    for (const auto& [txHash, tx] : mempool) {
      if (this->stop_) return;
      // 0xcfffe746 == 3489654598
      if (tx.getFrom() == randomList[i] && tx.getFunctor().value == 3489654598) {
        randomHashTxs.emplace_back(tx);
        i++;
        break;
      }
    }
  }
  i = 1;
  while (randomnessTxs.size() != this->state_.rdposGetMinValidators()) {
    for (const auto& [txHash, tx] : mempool) {
      // 0x6fc5a2d6 == 1875223254
      if (tx.getFrom() == randomList[i] && tx.getFunctor().value == 1875223254) {
        randomnessTxs.emplace_back(tx);
        i++;
        break;
      }
    }
  }
  if (this->stop_) return;

  // Create the block and append to all chains, we can use any storage for latest block.
  const std::shared_ptr<const FinalizedBlock> latestBlock = this->storage_.latest();

  // Append all validator transactions to a single vector (Will be moved to the new block)
  std::vector<TxValidator> validatorTxs;
  for (const auto& tx: randomHashTxs) validatorTxs.emplace_back(tx);
  for (const auto& tx: randomnessTxs) validatorTxs.emplace_back(tx);
  if (this->stop_) return;

  // Get a copy of the mempool and current timestamp
  auto chainTxs = this->state_.getMempool();
  uint64_t timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
    std::chrono::system_clock::now().time_since_epoch()
  ).count();

  // To create a valid block according to block validation rules, the
  //  timestamp provided to the new block must be equal or greater (>=)
  //  than the timestamp of the previous block.
  timestamp = std::max(timestamp, latestBlock->getTimestamp());

  LOGDEBUG("Create a new valid block.");
  auto block = FinalizedBlock::createNewValidBlock(
    std::move(chainTxs), std::move(validatorTxs), latestBlock->getHash(),
    timestamp, latestBlock->getNHeight() + 1, this->options_.getValidatorPrivKey()
  );
  LOGDEBUG("Block created, validating.");
  Hash latestBlockHash = block.getHash();
  if (
    BlockValidationStatus bvs = state_.tryProcessNextBlock(std::move(block));
    bvs != BlockValidationStatus::valid
  ) {
    LOGERROR("Block is not valid!");
    throw DynamicException("Block is not valid!");
  }
  if (this->stop_) return;
  if (this->storage_.latest()->getHash() != latestBlockHash) {
    LOGERROR("Block is not valid!");
    throw DynamicException("Block is not valid!");
  }

  // Broadcast the block through P2P
  LOGDEBUG("Broadcasting block.");
  if (this->stop_) return;
  this->p2p_.getBroadcaster().broadcastBlock(this->storage_.latest());
  auto end = std::chrono::high_resolution_clock::now();
  long double duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
  long double timeToConsensus = std::chrono::duration_cast<std::chrono::milliseconds>(waitForTxs - start).count();
  long double timeToTxs = std::chrono::duration_cast<std::chrono::milliseconds>(creatingBlock - waitForTxs).count();
  long double timeToBlock = std::chrono::duration_cast<std::chrono::milliseconds>(end - creatingBlock).count();
  LOGDEBUG("Block created in: " + std::to_string(duration) + "ms, " +
    "Time to consensus: " + std::to_string(timeToConsensus) + "ms, " +
    "Time to txs: " + std::to_string(timeToTxs) + "ms, " +
    "Time to block: " + std::to_string(timeToBlock) + "ms"
  );
}

void Consensus::doValidatorTx(const uint64_t& nHeight, const Validator& me) {
  Hash randomness = Hash::random();
  Hash randomHash = Utils::sha3(randomness);
  LOGDEBUG("Creating random Hash transaction");
  Bytes randomHashBytes = Hex::toBytes("0xcfffe746");
  randomHashBytes.insert(randomHashBytes.end(), randomHash.begin(), randomHash.end());
  TxValidator randomHashTx(
    me.address(),
    randomHashBytes,
    this->options_.getChainID(),
    nHeight,
    this->options_.getValidatorPrivKey()
  );

  Bytes seedBytes = Hex::toBytes("0x6fc5a2d6");
  seedBytes.insert(seedBytes.end(), randomness.begin(), randomness.end());
  TxValidator seedTx(
    me.address(),
    seedBytes,
    this->options_.getChainID(),
    nHeight,
    this->options_.getValidatorPrivKey()
  );

  // Sanity check if tx is valid
  bytes::View randomHashTxView(randomHashTx.getData());
  bytes::View randomSeedTxView(seedTx.getData());
  if (Utils::sha3(randomSeedTxView.subspan(4)) != Hash(randomHashTxView.subspan(4))) {
    LOGDEBUG("RandomHash transaction is not valid!!!");
    return;
  }

  // Append to mempool and broadcast the transaction across all nodes.
  LOGDEBUG("Broadcasting randomHash transaction");
  this->state_.rdposAddValidatorTx(randomHashTx);
  this->p2p_.getBroadcaster().broadcastTxValidator(randomHashTx);

  // Wait until we received all randomHash transactions to broadcast the randomness transaction
  LOGDEBUG("Waiting for randomHash transactions to be broadcasted");
  uint64_t validatorMempoolSize = 0;
  std::unique_ptr<uint64_t> lastLog = nullptr;
  while (validatorMempoolSize < this->state_.rdposGetMinValidators() && !this->stop_) {
    if (lastLog == nullptr || *lastLog != validatorMempoolSize) {
      lastLog = std::make_unique<uint64_t>(validatorMempoolSize);
      LOGDEBUG("Validator has: " + std::to_string(validatorMempoolSize) + " transactions in mempool");
    }
    validatorMempoolSize = this->state_.rdposGetMempoolSize();
    // Try to get more transactions from other nodes within the network
    for (const auto& nodeId : this->p2p_.getSessionsIDs(P2P::NodeType::NORMAL_NODE)) {
      if (this->stop_) return;
      auto txList = this->p2p_.requestValidatorTxs(nodeId);
      for (const auto& tx : txList) this->state_.addValidatorTx(tx);
    }
    std::this_thread::sleep_for(std::chrono::microseconds(10));
  }

  LOGDEBUG("Broadcasting random transaction");
  // Append and broadcast the randomness transaction.
  this->state_.addValidatorTx(seedTx);
  this->p2p_.getBroadcaster().broadcastTxValidator(seedTx);
}

void Consensus::start() {
  if (this->state_.rdposGetIsValidator() && !this->loopFuture_.valid()) {
    this->loopFuture_ = std::async(std::launch::async, &Consensus::validatorLoop, this);
  }
}

void Consensus::stop() {
  if (this->loopFuture_.valid()) {
    this->stop_ = true;
    this->loopFuture_.wait();
    this->loopFuture_.get();
  }
}

// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------
// TODO: Move to own file


// Generated from .proto files
#include <cometbft/abci/v1/service.grpc.pb.h>
#include <cometbft/abci/v1/types.grpc.pb.h>

using namespace cometbft::abci::v1;
using namespace cometbft::crypto::v1;
using namespace cometbft::types::v1;

class ABCIServiceImpl final : public cometbft::abci::v1::ABCIService::Service {
public:
    ABCIServiceImpl() = default;

    ~ABCIServiceImpl() override = default;

    grpc::Status Echo(grpc::ServerContext* context, const EchoRequest* request, EchoResponse* response) override {
        return grpc::Status::OK;
    }

    grpc::Status Flush(grpc::ServerContext* context, const FlushRequest* request, FlushResponse* response) override {
        return grpc::Status::OK;
    }

    grpc::Status Info(grpc::ServerContext* context, const InfoRequest* request, InfoResponse* response) override {
        return grpc::Status::OK;
    }

    grpc::Status CheckTx(grpc::ServerContext* context, const CheckTxRequest* request, CheckTxResponse* response) override {
        return grpc::Status::OK;
    }

    grpc::Status Query(grpc::ServerContext* context, const QueryRequest* request, QueryResponse* response) override {
        return grpc::Status::OK;
    }

    grpc::Status Commit(grpc::ServerContext* context, const CommitRequest* request, CommitResponse* response) override {
        return grpc::Status::OK;
    }

    grpc::Status InitChain(grpc::ServerContext* context, const InitChainRequest* request, InitChainResponse* response) override {
        return grpc::Status::OK;
    }

    grpc::Status ListSnapshots(grpc::ServerContext* context, const ListSnapshotsRequest* request, ListSnapshotsResponse* response) override {
        return grpc::Status::OK;
    }

    grpc::Status OfferSnapshot(grpc::ServerContext* context, const OfferSnapshotRequest* request, OfferSnapshotResponse* response) override {
        return grpc::Status::OK;
    }

    grpc::Status LoadSnapshotChunk(grpc::ServerContext* context, const LoadSnapshotChunkRequest* request, LoadSnapshotChunkResponse* response) override {
        return grpc::Status::OK;
    }

    grpc::Status ApplySnapshotChunk(grpc::ServerContext* context, const ApplySnapshotChunkRequest* request, ApplySnapshotChunkResponse* response) override {
        return grpc::Status::OK;
    }

    grpc::Status PrepareProposal(grpc::ServerContext* context, const PrepareProposalRequest* request, PrepareProposalResponse* response) override {
        return grpc::Status::OK;
    }

    grpc::Status ProcessProposal(grpc::ServerContext* context, const ProcessProposalRequest* request, ProcessProposalResponse* response) override {
        return grpc::Status::OK;
    }

    grpc::Status ExtendVote(grpc::ServerContext* context, const ExtendVoteRequest* request, ExtendVoteResponse* response) override {
        return grpc::Status::OK;
    }

    grpc::Status VerifyVoteExtension(grpc::ServerContext* context, const VerifyVoteExtensionRequest* request, VerifyVoteExtensionResponse* response) override {
        return grpc::Status::OK;
    }

    grpc::Status FinalizeBlock(grpc::ServerContext* context, const FinalizeBlockRequest* request, FinalizeBlockResponse* response) override {
        return grpc::Status::OK;
    }
};

void ExternalEngine::grpcServerRun() {

  // TODO: This is a member var that is set with the necessary references (the BDK engine objects
  //       that do the actual work the external consensus engine callbacks require.)
  ABCIServiceImpl service;

  // Create the GRPC listen socket/endpoint that the external engine will connect to 
  grpc::ServerBuilder builder;
  // InsecureServerCredentials is probably correct, we're not adding a security bureaucracy to a local RPC. use firewalls.
  builder.AddListeningPort("127.0.0.1:50567", grpc::InsecureServerCredentials()); // FIXME: need another node configuration parameter which is the listening port for GRPC
  builder.RegisterService(&service);
  grpcServer_ = builder.BuildAndStart();

  if (!grpcServer_) {
    // failed to start
    // set this to unlock the while (!grpcServerStarted_) barrier, but never set the grpcServerRunning_ flag since we 
    //   always were in a failed state if this is the case.
    grpcServerStarted_ = true;
    std::cerr << "Failed to start the gRPC server!" << std::endl;
    return;
  }

  // need to set this before grpcServerStarted_ since that flag is used as the sync barrier in ExternalEngine::start() and
  //  after that point, detecting grpcServerRunning_ == false indicates that we are no longer running, i.e. it exited or failed.
  grpcServerRunning_ = true; // true when we know Wait() is going to be called

  // After this is set, other threads can wait a bit and then check grpcServerRunning_
  //   to guess whether everything is working as expected.
  grpcServerStarted_ = true;

  std::cout << "gRPC Server started." << std::endl;

  // This blocks until we call grpcServer_->Shutdown() from another thread 
  grpcServer_->Wait();

  grpcServerRunning_ = false; // when past Wait(), we are no longer running

  std::cout << "gRPC Server stopped." << std::endl;
}

bool ExternalEngine::start() {

  if (started_) return true; // already started so technically succeeded in starting it

  // Ensure these are reset
  grpcServerStarted_ = false;
  grpcServerRunning_ = false;

  // start the GRPC server
  // assert (!server_thread)
  grpcServerThread_.emplace(&ExternalEngine::grpcServerRun, this);

  // wait until we are past opening the grpc server
  while (!grpcServerStarted_) {
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }

  // the right thing would be to connect to ourselves here and test the connection before firing up 
  //   the external engine, but a massive enough sleep here (like 1 second) should do the job.
  //   If 1s is not enough, make it e.g. 3s then. or use the GRPC async API which is significantly 
  //   more complex.
  // All we are waiting here is for the other thread to be in scheduling to be able to get into Wait()
  //   and do a blocking read on a TCP listen socket that is already opened (or instantly fail on some
  //   read or create error), which is almost zero work.
  // Unless the machine's CPU is completely overloaded, a massive sleep should be able to get that
  //   scheduled in and executed.
  std::this_thread::sleep_for(std::chrono::seconds(1));

  // if it is not running by now then it is probably because starting the GRPC server failed.
  if (!grpcServerRunning_) {
    std::cout << "ExternalEngine::start() failed: gRPC server failed to start." << std::endl;

    // cleanup failed grpc server/thread startup
    grpcServerThread_->join();
    grpcServerThread_.reset();
    grpcServer_.reset();

    return false;
  }

  // run cometbft which will connect to our GRPC server
  //
  //  cometbft start --proxy_app="tcp://127.0.0.1:26658" --abci grpc
  //
  try {

    // Search for the executable in the system's PATH
    boost::filesystem::path exec_path = boost::process::search_path("cometbft");

    if (exec_path.empty()) {
      std::cerr << "Error: Executable not found in system PATH." << std::endl;
      return false;
    }

    // TODO:
    // - all database/config/home_dir initialization and management (integrity checks, ...)
    // - correct runtime configuration for actual running-the-network

    // Launch the process
    process_ = boost::process::child(exec_path /*, "(arguments go here --abci grpc etc.)"" */);

    std::cout << "Executable launched with PID: " << process_->id() << std::endl;

  } catch (const std::exception& ex) {
      std::cerr << "Error: " << ex.what() << std::endl;
      return false;
  }

  started_ = true;
  return true;
}


bool ExternalEngine::stop() {

  if (!started_) return true;

  // This if protects from a grpcServer optional that isn't set
  //  usually this would be an impossible condition (given that started_ == true) so this may be an useless check   
  if (grpcServer_) {
      // this makes the grpc server thread actually terminate so we can join it 
      grpcServer_->Shutdown();
  }

  // Wait for the server thread to finish
  grpcServerThread_->join();
  grpcServerThread_.reset();

  // get rid of the grpcServer since it is shut down
  grpcServer_.reset();

  // Force-reset these for good measure
  grpcServerStarted_ = false;
  grpcServerRunning_ = false;

  // process shutdown -- TODO: assuming this is needed i.e. it doesn't shut down when the connected application goes away?

  // never happens because the optional should be set when started_ == true
  if (!process_.has_value()) {
      std::cerr << "Error: No process is running to terminate." << std::endl;
      started_ = false;
      return true;
  }

  // terminate the process
  try {
      process_->terminate();
      std::cout << "Process with PID " << process_->id() << " terminated." << std::endl;
      process_->wait();  // Ensure the process is fully terminated
      std::cout << "Process with PID " << process_->id() << " joined." << std::endl;
  } catch (const std::exception& ex) {

      // TODO: this is bad, and if it actually happens, we need to be able to do something else here to ensure the process disappears
      //   because we don't want a process using the data directory

      std::cerr << "Error: Failed to terminate process: " << ex.what() << std::endl;
      
      // actually we are just cutting it loose (what else could we do? force a kill -9 on the PID?)
      //return false;
  }

  // we need to get rid of the process_ instance in any case so we can start another
  process_.reset();

  started_ = false;
  return true;
}

bool ExternalEngine::isStarted() {
  return started_;
}

bool ExternalEngine::isRunning() {
  return started_ && process_.has_value() && process_->running();
}

// not needed; implement other cometbft process health checks as they are actually needed 
//bool ExternalEngine::isConnected() {
// if (!isRunning()) return false;
//  // FIXME/TODO check if connected
//  return false;
//}
