/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#include "comet.h"

#include "../utils/logger.h"
#include "../libs/toml.hpp"

#include "../net/abci/abciserver.h"
#include "../net/abci/abcihandler.h"   // ???

#include <boost/beast.hpp>
#include <boost/asio.hpp>

#include "../libs/base64.hpp"

// ---------------------------------------------------------------------------------------
// CometImpl class
// ---------------------------------------------------------------------------------------

// Maximum size of an RPC request (should be loopback only, so the limit can be relaxed)
#define COMET_RPC_MAX_BODY_BYTES 10000000

/**
 * CometImpl implements the interface to CometBFT using ABCIServer and ABCISession from
 * src/net/abci/*, which implement the TCP ABCI Socket server for a cometbft instance to
 * connect to. The ABCI server implementation uses proto/*, which is the Protobuf message
 * codec used to exchange objects through the ABCI with a running cometbft instance.
 * CometImpl also manages configuring, launching, monitoring and terminating the cometbft
 * process, as well as interfacing with its RPC port which is not exposed to BDK users.
 */
class CometImpl : public Log::LogicalLocationProvider, public ABCIHandler {
  private:
    CometListener* listener_; ///< Comet class application event listener/handler
    const std::string instanceIdStr_; ///< Identifier for logging
    const Options options_; ///< Copy of the supplied Options.
    //const std::vector<std::string> extraArgs_; ///< Extra arguments to 'cometbft start' (for testing).
    const bool stepMode_; ///< Set to 'true' if unit testing with cometbft in step mode (no empty blocks).

    std::unique_ptr<ABCIServer> abciServer_; ///< TCP server for cometbft ABCI connection
    std::optional<boost::process::child> process_; ///< boost::process that points to the running "cometbft start" process

    std::future<void> loopFuture_; ///< Future object holding the consensus engine thread.
    std::atomic<bool> stop_ = false; ///< Flag for stopping the consensus engine thread.
    std::atomic<bool> status_ = true; ///< Global status (true = OK, false = failed/terminated).
    std::string errorStr_; ///< Error message (if any).

    std::atomic<CometState> state_ = CometState::STOPPED; ///< Current step the Comet instance is in.
    std::atomic<CometState> pauseState_ = CometState::NONE; ///< Step to pause/hold the comet instance at, if any.

    std::atomic<int> infoCount_ = 0; ///< Simple counter for how many cometbft Info requests we got.

    std::atomic<int> rpcPort_ = 0; ///< RPC port that will be used by our cometbft instance.

    std::mutex nodeIdMutex_; ///< mutex to protect reading/writing nodeId_.
    std::string nodeId_; ///< Cometbft node id or an empty string if not yet retrieved.

    std::mutex txOutMutex_; ///< mutex to protect txOut_.
    std::deque<Bytes> txOut_; ///< Txs that are pending dispatch to the local cometbft node's mempool.

    // FIXME/TODO: remove connected_ and use socket_ instead ; socket_.reset() to set it to disconnected
    //             rename rpcIoc rpcSocket rpcRequestIdCounter
    boost::asio::io_context ioc_; ///< io_context for our persisted RPC socket connection.
    std::unique_ptr<boost::asio::ip::tcp::socket> socket_; ///< Persisted RPC socket connection.
    bool connected_ = false; ///< Check if the RPC connection is established.
    uint64_t requestIdCounter_ = 0; ///< Client-side request ID generator for our JSON-RPC calls to cometbft.

    void setState(const CometState& state); ///< Apply a state transition, possibly pausing at the new state.
    void setError(const std::string& errorStr); ///< Signal a fatal error condition.
    void resetError(); ///< Reset internal error condition.
    void startCometBFT(const std::string& cometPath); ///< Launch the CometBFT process_ ('cometbft start' for an async node run).
    void stopCometBFT(); ///< Terminate the CometBFT process_.
    int runCometBFT(const std::string& cometPath, std::vector<std::string> cometArgs, std::string &sout, std::string &serr); ///< Blocking run cometbft w/ args
    void workerLoop(); ///< Worker loop responsible for establishing and managing a connection to cometbft.
    void workerLoopInner(); ///< Called by workerLoop().

    bool startRPCConnection() {
      if (rpcPort_ == 0) return false; // we have not figured out what the rpcPort_ configuration is yet
      if (connected_) return true;
      try {
          socket_ = std::make_unique<boost::asio::ip::tcp::socket>(ioc_);
            boost::asio::ip::tcp::endpoint endpoint(
            boost::asio::ip::make_address("127.0.0.1"), rpcPort_
          );
          socket_->connect(endpoint);
          connected_ = true;
          return true;
      } catch (const std::exception& e) {
          connected_ = false;
          return false;
      }
    }

    void stopRPCConnection() {
        if (socket_ && connected_) {
            boost::system::error_code ec;
            socket_->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
            connected_ = false;
        }
    }

    bool makeJSONRPCCall(const std::string& method, const json& params, std::string& outResult, bool retry = true) {
        if (!connected_ && !startRPCConnection()) {
            outResult = "Connection failed";
            return false;
        }

        int requestId = ++requestIdCounter_;  // Generate a unique request ID
        json requestBody = {
            {"jsonrpc", "2.0"},
            {"method", method},
            {"params", params},
            {"id", requestId}
        };

        try {
            // Set up the POST request with keep-alive
            boost::beast::http::request<boost::beast::http::string_body> req{
                boost::beast::http::verb::post, "/", 11};
            req.set(boost::beast::http::field::host, "localhost");
            req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
            req.set(boost::beast::http::field::accept, "application/json");
            req.set(boost::beast::http::field::content_type, "application/json");
            req.set(boost::beast::http::field::connection, "keep-alive");  // Keep connection open
            req.body() = requestBody.dump();  // Serialize JSON to string
            req.prepare_payload();

            boost::beast::http::write(*socket_, req);

            boost::beast::flat_buffer buffer;
            boost::beast::http::response<boost::beast::http::dynamic_body> res;

            boost::beast::http::read(*socket_, buffer, res);

          // This is broken, we have to always return json.
          //if (res.result() != boost::beast::http::status::ok) {
          //    LOGDEBUG("ACTUAL RESULT STRING: [" + boost::beast::buffers_to_string(res.body().data()) + "]" );
          //    outResult = "HTTP error: " + std::to_string(res.result_int());
          //    return false;
          // }
          outResult = boost::beast::buffers_to_string(res.body().data());
          auto jsonResponse = json::parse(outResult);

          // Check both the JSON-RPC error field and the HTTP return code to ensure we don't miss errors.
          return ! ( jsonResponse.contains("error") || (res.result() != boost::beast::http::status::ok) );

        } catch (const std::exception& e) {
            stopRPCConnection();  // Close connection on error
            outResult = "Exception occurred: " + std::string(e.what());
            // Retry the call once, in case it is some transient error
            // TODO: review this; this retry is not be strictly necessary as 
            //       the caller should handle failures instead
            if (retry && startRPCConnection()) {
                return makeJSONRPCCall(method, params, outResult, false);
            }
            return false;
        }
    }

  public:

    std::string getLogicalLocation() const override { return instanceIdStr_; } ///< Log instance

    explicit CometImpl(CometListener* listener, std::string instanceIdStr, const Options& options, bool stepMode);
    //const std::vector<std::string>& extraArgs

    virtual ~CometImpl();

    bool getStatus();

    const std::string& getErrorStr();

    CometState getState();

    void setPauseState(const CometState pauseState = CometState::NONE);

    CometState getPauseState();

    std::string waitPauseState(uint64_t timeoutMillis);

    std::string getNodeID();

    void start();

    void stop();

    void sendTransaction(const Bytes& tx);

    // ---------------------------------------------------------------------------------------
    // ABCIHandler interface
    // ---------------------------------------------------------------------------------------

    virtual void echo(const cometbft::abci::v1::EchoRequest& req, cometbft::abci::v1::EchoResponse* res);
    virtual void flush(const cometbft::abci::v1::FlushRequest& req, cometbft::abci::v1::FlushResponse* res);
    virtual void info(const cometbft::abci::v1::InfoRequest& req, cometbft::abci::v1::InfoResponse* res);
    virtual void init_chain(const cometbft::abci::v1::InitChainRequest& req, cometbft::abci::v1::InitChainResponse* res);
    virtual void prepare_proposal(const cometbft::abci::v1::PrepareProposalRequest& req, cometbft::abci::v1::PrepareProposalResponse* res);
    virtual void process_proposal(const cometbft::abci::v1::ProcessProposalRequest& req, cometbft::abci::v1::ProcessProposalResponse* res);
    virtual void check_tx(const cometbft::abci::v1::CheckTxRequest& req, cometbft::abci::v1::CheckTxResponse* res);
    virtual void commit(const cometbft::abci::v1::CommitRequest& req, cometbft::abci::v1::CommitResponse* res);
    virtual void finalize_block(const cometbft::abci::v1::FinalizeBlockRequest& req, cometbft::abci::v1::FinalizeBlockResponse* res);
    virtual void query(const cometbft::abci::v1::QueryRequest& req, cometbft::abci::v1::QueryResponse* res);
    virtual void list_snapshots(const cometbft::abci::v1::ListSnapshotsRequest& req, cometbft::abci::v1::ListSnapshotsResponse* res);
    virtual void offer_snapshot(const cometbft::abci::v1::OfferSnapshotRequest& req, cometbft::abci::v1::OfferSnapshotResponse* res);
    virtual void load_snapshot_chunk(const cometbft::abci::v1::LoadSnapshotChunkRequest& req, cometbft::abci::v1::LoadSnapshotChunkResponse* res);
    virtual void apply_snapshot_chunk(const cometbft::abci::v1::ApplySnapshotChunkRequest& req, cometbft::abci::v1::ApplySnapshotChunkResponse* res);
    virtual void extend_vote(const cometbft::abci::v1::ExtendVoteRequest& req, cometbft::abci::v1::ExtendVoteResponse* res);
    virtual void verify_vote_extension(const cometbft::abci::v1::VerifyVoteExtensionRequest& req, cometbft::abci::v1::VerifyVoteExtensionResponse* res);
};

CometImpl::CometImpl(CometListener* listener, std::string instanceIdStr, const Options& options, bool stepMode/*const std::vector<std::string>& extraArgs*/)
  : listener_(listener), instanceIdStr_(instanceIdStr), options_(options), stepMode_(stepMode)// extraArgs_(extraArgs)//, resolver_(ioc_)
{
}

CometImpl::~CometImpl() {
  stop();
}

bool CometImpl::getStatus() {
  return status_;
}

const std::string& CometImpl::getErrorStr() {
  return errorStr_;
}

CometState CometImpl::getState() {
  return state_;
}

void CometImpl::setPauseState(const CometState pauseState) {
  pauseState_ = pauseState;
}

CometState CometImpl::getPauseState() {
  return pauseState_;
}

std::string CometImpl::waitPauseState(uint64_t timeoutMillis) {
  auto timeoutTime = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMillis);
  while (timeoutMillis == 0 || std::chrono::steady_clock::now() < timeoutTime) {
    if (!this->status_) {
      return this->errorStr_;
    }
    if (this->pauseState_ == this->state_ || this->pauseState_ == CometState::NONE) {
      return ""; // succeed if the pause state is reached or if pause is disabled (set to NONE)
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }
  return "TIMEOUT";
}

std::string CometImpl::getNodeID() {
  std::scoped_lock(this->nodeIdMutex_);
  return this->nodeId_;
}

void CometImpl::sendTransaction(const Bytes& tx) {
  // Add transaction to a queue that will try to push it to our cometbft instance.
  // Queue is NEVER reset on continue; etc. it is the same node, need to deliver tx to it eventually.
  // NOTE: sendTransaction is about queuing the tx object so that it is eventually dispatched to
  //       the localhost cometbft node. the only guarantee is that the local cometbft node will see it.
  //       it does NOT mean it is accepted by the application's blockchain network or anything else.
  std::lock_guard<std::mutex> lock(txOutMutex_);
  txOut_.push_back(tx);
}

void CometImpl::start() {
  if (!this->loopFuture_.valid()) {
    this->stop_ = false;
    resetError(); // ensure error status is off
    setState(CometState::STARTED);
    this->loopFuture_ = std::async(std::launch::async, &CometImpl::workerLoop, this);
  }
}

void CometImpl::stop() {

  if (this->loopFuture_.valid()) {
    this->stop_ = true;

    // Stop the RPC connection
    stopRPCConnection();

    // (1)
    // We should stop the cometbft process first; we are a server to the consensus engine,
    //   and the side making requests should be the one to be shut down.
    // We can use the stop_ flag on our side to know that we are stopping as we service 
    //   ABCI callbacks from the consensus engine, but it probably isn't needed.
    //
    // It actually does not make sense to shut down the gRPC server from our end, since
    //   the cometbft process will, in this case, simply attempt to re-establish the
    //   connection with the ABCI application (the gRPC server); it assumes it is running
    //   under some sort of process control that restarts it in case it fails.
    //
    // CometBFT should receive SIGTERM and then just terminate cleanly, including sending a
    //   socket disconnection or shutdown message to us, which should ultimately allow us
    //   to wind down our end of the gRPC connection gracefully.

    stopCometBFT();

    // (2)
    // We should know cometbft has disconnected from us when:
    //
    //     grpcServerRunning_ == false
    //
    // Which means the grpcServer_->Wait() call has exited, which it should, if the gRPC server
    //   on the other end has disconnected from us or died.
    //
    // TODO: We shouldn't rely on this, though. If a timeout elapses, we should consider
    //       doing something else such as forcing kill -9 on cometbft, instead of looping
    //       forever here without a timeout check.
    //
    if (abciServer_) {
      LOGDEBUG("Waiting for abciServer_ networking to stop running (a side-effect of the cometbft process exiting.)");
      while (abciServer_->running()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
      }
      LOGDEBUG("abciServer_ networking has stopped running, we can now stop the ABCI net engine.");
      abciServer_->stop();
      LOGDEBUG("abciServer_ networking engine stopped.");
      abciServer_.reset();
      LOGDEBUG("abciServer_ networking engine destroyed.");
    } else {
      LOGDEBUG("No abciServer_ instance, so nothing to do.");
    }

    this->setPauseState(); // must reset any pause state otherwise it won't ever finish
    this->loopFuture_.wait();
    this->loopFuture_.get();
    resetError(); // stop() clears any error status
    setState(CometState::STOPPED);
  }
}

void CometImpl::setState(const CometState& state) {
  LOGTRACE("Set comet state: " + std::to_string((int)state));
  this->state_ = state;
  if (this->pauseState_ == this->state_) {
    LOGTRACE("Pausing at comet state: " + std::to_string((int)state));
    while (this->pauseState_ == this->state_) {
      std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    LOGTRACE("Unpausing at comet state: " + std::to_string((int)state));
  }
}

void CometImpl::setError(const std::string& errorStr) {
  LOGDEBUG("Comet ERROR raised: " + errorStr);
  this->errorStr_ = errorStr;
  this->status_ = false;
}

void CometImpl::resetError() {
  this->status_ = true;
  this->errorStr_ = "";
}

void CometImpl::startCometBFT(const std::string& cometPath) {

  if (!abciServer_) {
    LOGFATALP_THROW("Cannot call startCometBFT() without having first started the ABCI app server.");
  }

    //boost::process::ipstream bpout;
    //boost::process::ipstream bperr;
    auto bpout = std::make_shared<boost::process::ipstream>();
    auto bperr = std::make_shared<boost::process::ipstream>();

      // Search for the executable in the system's PATH
      boost::filesystem::path exec_path = boost::process::search_path("cometbft");
      if (exec_path.empty()) {
        // This is a non-recoverable error
        // The gRPC server will be stopped/collected during stop(), which
        //   is also called by the destructor.
        setError("cometbft executable not found in system PATH");
        return;
      }

      // CometBFT arguments
      //
      // We are making it so that an abci.sock file within the root of the
      //   CometBFT home directory ("cometPath" below) is the proxy_app
      //   URL. That is, each home directory can only be used for one
      //   running and communicating pair of BDK <-> CometBFT.
      //
      std::vector<std::string> cometArgs = {
        "start",
        "--abci=socket",
        "--proxy_app=unix://" + abciServer_->getSocketPath(),
        "--home=" + cometPath
      };

      // extraArgs should be used only for arguments that differ between
      //  testing and production use and can't be made standard configs
      //  and it doesn't make sense to add them to BDK Options::CometBFT.
      //cometArgs.insert(cometArgs.end(), this->extraArgs_.begin(), this->extraArgs_.end());

      LOGDEBUG("Launching cometbft");// with arguments: " + cometArgs);

      // Launch the process
      process_ = boost::process::child(
        exec_path,
        boost::process::args(cometArgs),
        boost::process::std_out > *bpout,
        boost::process::std_err > *bperr
      );

    std::string pidStr = std::to_string(process_->id());
    LOGDEBUG("cometbft start launched with PID: " + pidStr);

    // Spawn two detached threads to pump stdout and stderr to bdk.log.
    // They should go away naturally when process_ is terminated.

    std::thread stdout_thread([bpout, pidStr]() {
        std::string line;
        while (*bpout && std::getline(*bpout, line) && !line.empty()) {
          // REVIEW: may want to upgrade this to DEBUG
          GLOGTRACE("[cometbft stdout]: " + line);
        }
        GLOGDEBUG("cometbft stdout stream pump thread finished, cometbft pid = " + pidStr);
    });

    std::thread stderr_thread([bperr, pidStr]() {
        std::string line;
        while (*bperr && std::getline(*bperr, line) && !line.empty()) {
          GLOGDEBUG("[cometbft stderr]: " + line);
        }
        GLOGDEBUG("cometbft stderr stream pump thread finished, cometbft pid = " + pidStr);
    });

    // Detach the threads to run independently, or join if preferred
    stdout_thread.detach();
    stderr_thread.detach();

}

void CometImpl::stopCometBFT() {
    // -- Begin stop process_ if any
    // process shutdown -- TODO: assuming this is needed i.e. it doesn't shut down when the connected application goes away?
    if (process_.has_value()) {
      LOGDEBUG("Terminating CometBFT process");
      LOGXTRACE("Term 1");
      // terminate the process
      pid_t pid = process_->id();
      try {
        LOGXTRACE("Term 2");
        process_->terminate(); // SIGTERM (graceful termination, equivalent to terminal CTRL+C)
        LOGXTRACE("Term 3");
        LOGDEBUG("Process with PID " + std::to_string(pid) + " terminated");
        LOGXTRACE("Term 4");
        process_->wait();  // Ensure the process is fully terminated
        LOGXTRACE("Term 5");
        LOGDEBUG("Process with PID " + std::to_string(pid) + " joined");
        LOGXTRACE("Term 6");
      } catch (const std::exception& ex) {
        // This is bad, and if it actually happens, we need to be able to do something else here to ensure the process disappears
        //   because we don't want a process using the data directory and using the socket ports.
        // TODO: is this the best we can do? (what about `cometbft debug kill`?)
        LOGWARNING("Failed to terminate process: " + std::string(ex.what()));
        // Fallback: Forcefully kill the process using kill -9
        try {
          std::string killCommand = "kill -9 " + std::to_string(pid);
          LOGINFO("Attempting to force kill process with PID " + std::to_string(pid) + " using kill -9");
          int result = std::system(killCommand.c_str());
          if (result == 0) {
            LOGINFO("Successfully killed process with PID " + std::to_string(pid) + " using kill -9");
          } else {
            LOGWARNING("Failed to kill process with PID " + std::to_string(pid) + " using kill -9. Error code: " + std::to_string(result));
          }
        } catch (const std::exception& ex2) {
          LOGERROR("Failed to execute kill -9: " + std::string(ex2.what()));
        }
      }
      LOGXTRACE("Term end");
    }
    LOGXTRACE("10");
    // we need to ensure we got rid of any process_ instance in any case so we can start another
    process_.reset();
    LOGDEBUG("CometBFT process terminated");
    // -- End stop process_ if any

}

// One-shot run cometbft with the given cometArgs and block until it finishes, returning both
//  the complete stdout and stderr streams as outparams and the exit_code as a return value.
// This is to be used with e.g. "cometbft show-node-id" etc.
int CometImpl::runCometBFT(const std::string& cometPath, std::vector<std::string> cometArgs, std::string &sout, std::string &serr) {
  boost::filesystem::path exec_path = boost::process::search_path("cometbft");
  if (exec_path.empty()) {
    setError("cometbft executable not found in system PATH");
    return -1;
  }
  auto bpout = std::make_shared<boost::process::ipstream>();
  auto bperr = std::make_shared<boost::process::ipstream>();
  // using a local "process" variable here since this is an one-shot run:
  boost::process::child process(
      exec_path,
      boost::process::args(cometArgs),
      boost::process::std_out > *bpout,
      boost::process::std_err > *bperr
  );
  // these threads will pump the output streams while we wait (blocking) for cometbft to finish
  // when the process terminates, they stop looping
  std::thread stdout_thread([bpout, &sout]() {
    std::string line;
    if (std::getline(*bpout, line)) { sout += line; }
    while (std::getline(*bpout, line)) { sout += '\n' + line; }
  });
  std::thread stderr_thread([bperr, &serr]() {
    std::string line;
    if (std::getline(*bperr, line)) { serr += line; }
    while (std::getline(*bperr, line)) { serr += '\n' + line; }
  });
  process.wait(); // blocking wait until the cometbft execution completes
  if (stdout_thread.joinable()) { stdout_thread.join(); }
  if (stderr_thread.joinable()) { stderr_thread.join(); }
  int exit_code = process.exit_code();
  return exit_code;
}

void CometImpl::workerLoop() {
  LOGDEBUG("Comet worker thread: started");
  try {
    workerLoopInner();
  } catch (const std::exception& ex) {
    setError("Exception caught in comet worker thread: " + std::string(ex.what()));
  }
  LOGDEBUG("Comet worker thread: finished");
}

void CometImpl::workerLoopInner() {

  LOGDEBUG("Comet worker: started");

  // If we are stopping, then quit
  while (!stop_) {

    LOGDEBUG("Comet worker: start loop");

    // --------------------------------------------------------------------------------------
    // If this is a continue; and we are restarting the cometbft workerloop, ensure that any
    //   state and connections from the previous attempt are wiped out, regardless of the
    //   state they were previously in.

    // If there's an old RPC connection, make sure it is gone
    stopRPCConnection();

    // Ensure that the CometBFT process is in a terminated state.
    // This should be a no-op; it should be stopped before the continue; statement.
    stopCometBFT();

    // Ensure the ABCI Server is in a stopped/destroyed state
    abciServer_.reset();

    // Reset any state we might have as an ABCIHandler
    infoCount_ = 0; // needed for the TESTING_COMET -> TESTED_COMET state transition.
    rpcPort_ = 0;
    std::unique_lock<std::mutex> resetInfoLock(this->nodeIdMutex_);
    this->nodeId_ = ""; // only known after "comet/config/node_key.json" is set and "cometbft show-node-id" is run.
    resetInfoLock.unlock();

    LOGDEBUG("Comet worker: running configuration step");

    // --------------------------------------------------------------------------------------
    // Run configuration step (writes to the comet/config/* files before running cometbft)
    //
    // The global option rootPath from options.json tells us the root data directory for BDK.
    // The Comet worker thread works by expecting a rootPath + /comet/ directory to be the
    //   home directory used by its managed cometbft instance.
    //
    // Before BDK will work with comet, it already needs to have access to all the consensus
    //   parameters it needs to configure cometbft with. These parameters must all be
    //   given to BDK itself, via e.g. options.json. So any parameters needed for cometbft
    //   must be first modeled as BDK parameters which are then passed through.
    //
    // If the home directory does not exist, it must be initialized using `cometbft init`,
    //   and then the configuration must be modified with the relevant parameters supplied
    //   to the BDK, such as validator keys.
    //
    // NOTE: It is cometbft's job to determine whether the node is acting as a
    //   validator or not. In options.json we can infer whether the node can ever be acting
    //   as a validator by checking whether cometBFT::privValidatorKey is set, but that is all;
    //   whether it is currently a validator or not is up to the running network. In the
    //   future we may want to unify the Options constructors into just one, and leave the
    //   initialization of "non-validator nodes" as the ones that set the
    //   cometBFT::privValidatorKey option to empty or undefined.
    //
    // node_key.json can also be forced to a known value. This should be the default behavior,
    //   since deleting the comet/* directory (as an e.g. recovery strategy) would cause a
    //   peer to otherwise regenerate their node key, which would impact its persistent
    //   connection to other nodes that have explicitly configured themselves to connect
    //   to the node id that specific node/peer.

    setState(CometState::CONFIGURING);

    const std::string rootPath = options_.getRootPath();
    const std::string cometPath = rootPath + "/comet/";
    const std::string cometConfigPath = cometPath + "config/";
    const std::string cometConfigGenesisPath = cometConfigPath + "genesis.json";
    const std::string cometConfigNodeKeyPath = cometConfigPath + "node_key.json";
    const std::string cometConfigPrivValidatorKeyPath = cometConfigPath + "priv_validator_key.json";
    const std::string cometConfigTomlPath = cometConfigPath + "config.toml";

    const std::string cometUNIXSocketPath = cometPath + "abci.sock";

    LOGDEBUG("Options RootPath: " + options_.getRootPath());

    const json& opt = options_.getCometBFT();

    if (opt.is_null()) {
      LOGWARNING("Configuration option cometBFT is null.");
    } else {
      LOGDEBUG("Configuration option cometBFT: " + opt.dump());
    }

    bool hasGenesis = opt.contains("genesis");
    json genesisJSON = json::object();
    if (hasGenesis) genesisJSON = opt["genesis"];

    bool hasPrivValidatorKey = opt.contains("privValidatorKey");
    json privValidatorKeyJSON = json::object();
    if (hasPrivValidatorKey) privValidatorKeyJSON = opt["privValidatorKey"];

    bool hasNodeKey = opt.contains("nodeKey");
    json nodeKeyJSON = json::object();
    if (hasNodeKey) nodeKeyJSON = opt["nodeKey"];

    bool hasP2PPort = opt.contains("p2p_port");
    json p2pPortJSON = json::object();
    if (hasP2PPort) p2pPortJSON = opt["p2p_port"];

    bool hasRPCPort = opt.contains("rpc_port");
    json rpcPortJSON = json::object();
    if (hasRPCPort) rpcPortJSON = opt["rpc_port"];

    bool hasPeers = opt.contains("peers");
    json peersJSON = json::object();
    if (hasPeers) peersJSON = opt["peers"];

    // --------------------------------------------------------------------------------------
    // Sanity check configuration: a comet genesis file must be explicitly given.

    if (!hasGenesis) {
      // Cannot proceed with an empty comet genesis spec on options.json.
      // E.g.: individual testcases or the test harness must fill in a valid
      //   cometBFT genesis config.
      throw DynamicException("Configuration option cometBFT::genesis is empty.");
    } else {
      LOGINFO("CometBFT::genesis config found: " + genesisJSON.dump());
    }

    if (!hasP2PPort) {
      throw DynamicException("Configuration option cometBFT:: p2p_port is empty.");
    } else {
      LOGINFO("CometBFT::p2p_port config found: " + p2pPortJSON.get<std::string>());
    }

    if (!hasRPCPort) {
      throw DynamicException("Configuration option cometBFT:: rpc_port is empty.");
    } else {
      LOGINFO("CometBFT::rpc_port config found: " + rpcPortJSON.get<std::string>());

      // Save it so that we can reach the cometbft node via RPC to e.g. send transactions.
      rpcPort_ = atoi(rpcPortJSON.get<std::string>().c_str());
    }

    if (!hasNodeKey) {
      // This is allowed (some nodes will not care about what node ID they get), so just log it.
      LOGINFO("Configuration option cometBFT::nodeKey is empty.");
    } else {
      LOGINFO("CometBFT::nodeKey config found: " + nodeKeyJSON.dump());
    }

    if (!hasPrivValidatorKey) {
      // This is allowed (some nodes are not validators), so just log it.
      LOGINFO("Configuration option cometBFT::privValidatorKey is empty.");
    } else {
      LOGINFO("CometBFT::privValidatorKey config found: " + privValidatorKeyJSON.dump());
    }

    // --------------------------------------------------------------------------------------
    // BDK root path must be set up before the Comet worker is started.

    // If rootPath does not exist for some reason, quit.
    if (!std::filesystem::exists(rootPath)) {
      throw DynamicException("Root path not found: " + rootPath);
    }

    // --------------------------------------------------------------------------------------
    // If comet home directory does not exist inside rootPath, then create it via
    //   cometbft init. It will be created with all required options with standard values,
    //   which is what we want.

    if (!std::filesystem::exists(cometPath)) {

      LOGDEBUG("Comet worker: creating comet directory");

      // run cometbft init cometPath to create the cometbft directory with default configs
      Utils::execute("cometbft init --home " + cometPath);

      // check it exists now, otherwise halt node
      if (!std::filesystem::exists(cometPath)) {
        throw DynamicException("Could not create cometbft home directory");
      }
    }

    if (!std::filesystem::exists(cometConfigPath)) {
      // comet/config/ does not exist for some reason, which means the comet/ directory is broken
      throw DynamicException("CometBFT home directory is broken: it doesn't have a config/ subdirectory");
    }

    LOGDEBUG("Comet worker: comet directory exists");

    // --------------------------------------------------------------------------------------
    // Comet home directory exists; check its configuration is consistent with the current
    //   BDK configuration options. If it isn't then sync them all here.

    // If cometBFT::nodeKey is set in options, write it over the default
    //   node_key.json comet file to ensure it is the same.
    if (hasNodeKey) {
      std::ofstream outputFile(cometConfigNodeKeyPath);
      if (outputFile.is_open()) {
        outputFile << nodeKeyJSON.dump(4);
        outputFile.close();
      } else {
        throw DynamicException("Cannot open comet nodeKey file for writing: " + cometConfigNodeKeyPath);
      }
    }

    // If cometBFT::privValidatorKey is set in options, write it over the default
    //   priv_validator_key.json comet file to ensure it is the same.
    if (hasPrivValidatorKey) {
      std::ofstream outputFile(cometConfigPrivValidatorKeyPath);
      if (outputFile.is_open()) {
        outputFile << privValidatorKeyJSON.dump(4);
        outputFile.close();
      } else {
        throw DynamicException("Cannot open comet privValidatorKey file for writing: " + cometConfigPrivValidatorKeyPath);
      }
    }

    // NOTE: If genesis option is required, must test for it earlier.
    // If cometBFT::genesis is set in options, write it over the default
    //   genesis.json comet file to ensure it is the same.
    if (hasGenesis) {
      std::ofstream outputFile(cometConfigGenesisPath);
      if (outputFile.is_open()) {
        outputFile << genesisJSON.dump(4);
        outputFile.close();
      } else {
        throw DynamicException("Cannot open comet genesis file for writing: " + cometConfigGenesisPath);
      }
    }

    // Sanity check the existence of the config.toml file
    if (!std::filesystem::exists(cometConfigTomlPath)) {
      throw DynamicException("Comet config.toml file does not exist: " + cometConfigTomlPath);
    }

    // Open and parse the main comet config file (config.toml)
    toml::table configToml;
    try {
      configToml = toml::parse_file(cometConfigTomlPath);
    } catch (const toml::parse_error& err) {
      throw DynamicException("Error parsing TOML file: " + std::string(err.description()));
    }

    // Force all relevant option values into config.toml
    configToml.insert_or_assign("abci", "socket"); // gets overwritten by --abci, and this is the default value anyway
    configToml.insert_or_assign("proxy_app", "unix://" + cometUNIXSocketPath); // gets overwritten by --proxy_app
    configToml["storage"].as_table()->insert_or_assign("discard_abci_responses", toml::value(true));
    std::string p2p_param = "tcp://0.0.0.0:" + p2pPortJSON.get<std::string>();
    std::string rpc_param = "tcp://0.0.0.0:" + rpcPortJSON.get<std::string>();
    configToml["p2p"].as_table()->insert_or_assign("laddr", p2p_param);
    configToml["rpc"].as_table()->insert_or_assign("laddr", rpc_param);

    // RPC options. Since the RPC port should be made accessible for local (loopback) connections only, it can
    //   accept unsafe comands if we need it to.
    // REVIEW: Maybe we should try and have the RPC endpoint be an unix:/// socket as well?
    configToml["rpc"].as_table()->insert_or_assign("max_body_bytes", COMET_RPC_MAX_BODY_BYTES);
    //configToml["rpc"].as_table()->insert_or_assign("unsafe", toml::value(true));

    // FIXME/TODO: right now we are just testing, so these security params are relaxed to allow
    //   testing on the same machine. these will have to be exposed as BDK options as well so
    //   they can be toggled for testing.
    configToml["p2p"].as_table()->insert_or_assign("allow_duplicate_ip", toml::value(true));
    configToml["p2p"].as_table()->insert_or_assign("addr_book_strict", toml::value(false));

    if (hasPeers) {
      // persistent_peers is a single string with the following format:
      // <ID>@<IP>:<PORT>
      // BDK validators should specify as many nodes as possible as persistent peers.
      // Additional methods for adding or discovering other validators and non-validators should also be available.
      //  (e.g. seeds/PEX).
      configToml["p2p"].as_table()->insert_or_assign("persistent_peers", peersJSON.get<std::string>());
    }

    // If running in stepMode (testing ONLY) set all the options required to make cometbft
    // never produce a block unless at least one transaction is there to be included in one,
    // and never generating null/timeout blocks.
    if (stepMode_) {
      LOGDEBUG("stepMode_ is set, setting step mode parameters for testing.");
      configToml["consensus"].as_table()->insert_or_assign("create_empty_blocks", toml::value(false));
      configToml["consensus"].as_table()->insert_or_assign("skip_timeout_commit", toml::value(true)); // speeds up testing in general
      configToml["consensus"].as_table()->insert_or_assign("timeout_propose", "1s");
      configToml["consensus"].as_table()->insert_or_assign("timeout_propose_delta", "0s");
      configToml["consensus"].as_table()->insert_or_assign("timeout_prevote", "1s");
      configToml["consensus"].as_table()->insert_or_assign("timeout_prevote_delta", "0s");
      configToml["consensus"].as_table()->insert_or_assign("timeout_precommit", "1s");
      configToml["consensus"].as_table()->insert_or_assign("timeout_precommit_delta", "0s");
      configToml["consensus"].as_table()->insert_or_assign("timeout_commit", "1s");
    }

    // Overwrite updated config.toml
    std::ofstream configTomlOutFile(cometConfigTomlPath);
    if (!configTomlOutFile.is_open()) {
      throw DynamicException("Could not open file for writing: " + cometConfigTomlPath);
    }
    configTomlOutFile << configToml;
    if (configTomlOutFile.fail()) {
      throw DynamicException("Could not write file: " + cometConfigTomlPath);
    }
    configTomlOutFile.close();
    if (configTomlOutFile.fail()) {
      throw DynamicException("Failed to close file properly: " + cometConfigTomlPath);
    }

    LOGDEBUG("Comet setting configured");

    setState(CometState::CONFIGURED);

    LOGDEBUG("Comet set configured");

    // --------------------------------------------------------------------------------------
    // Check if quitting
    if (stop_) break;

    // --------------------------------------------------------------------------------------
    // Run cometbft inspect and check that everything is as expected here (validator key,
    ///   state, ...). If anything is wrong that is fixable in a non-forceful fashion,
    //    then fix it that way and re-check, otherwise "fix it" by rm -rf the
    //    rootPath + /comet/  directory entirely and continue; this loop.
    // TODO: ensure cometbft is terminated if we are killed (prctl()?)

    setState(CometState::INSPECTING_COMET);

    // If there's a need to run cometbft inspect first, do it here.
    //
    // As we map out all the usual error conditions, we'll add more code here to catch them
    //  and recover from them. By default, recovery can be done by just wiping off the comet/
    //  directory and starting the node from scratch. Eventually, this fallback case can be
    //  augmented with e.g. using a local snapshots directory to speed up syncing.

    // Let's use the inspect state to get the node ID
    // run cometbft which will connect to our ABCI server
    try {
      std::string sout, serr;
      int exitCode = runCometBFT(cometPath, { "show-node-id", "--home=" + cometPath }, sout, serr);
      if (exitCode != 0) {
        LOGERROR("ERROR: Error while attempting to fetch the cometbft node id. Exit code: " + std::to_string(exitCode) + "; stdout: " + sout + "; stderr: " + serr);
        // Continue without knowing the node ID
        // TODO: review this; this probably needs to be a setError and "continue;" to retry
      } else {
        // TODO: here we want to assert that the node ID we got is a proper hex string of the correct size
        LOGDEBUG("Got comet node ID: [" + sout + "]");
        std::scoped_lock(this->nodeIdMutex_);
        this->nodeId_ = sout;
      }
    } catch (const std::exception& ex) {
      // TODO: maybe we should attempt a restart in this case, with a retry counter?
      setError("Exception caught when trying to run cometbft start: " + std::string(ex.what()));
      return;
    }

    /*
      //FIXME/TODO:  this here causes a crash
      // Probably just a lifetime problem; need to review all error handling/conditions
      //  w.r.t. this working loop, and return;ing from it or continue;ing, and the
      //  implications to the various startup/teardown calls to the
      //  create/connect/disconnect/destroy lifetime of the various components (abci,
      //    rpc, etc.)

      setError("whatever");
      return;
    */

    // --------------------------------------------------------------------------------------
    // Stop cometbft inspect server.

    // (Not needed so far; see above)

    setState(CometState::INSPECTED_COMET);

    // --------------------------------------------------------------------------------------
    // Check if quitting
    if (stop_) break;

    // --------------------------------------------------------------------------------------
    // Start our cometbft application gRPC server; make sure it is started.

    setState(CometState::STARTING_ABCI);

    // start the ABCI server
    abciServer_ = std::make_unique<ABCIServer>(this, cometUNIXSocketPath);
    abciServer_->start();

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
    if (!abciServer_->running()) {
      LOGERROR("Comet failed: ABCI server failed to start");

      // Retry
      //
      // TODO: So, the main idea of this loop we are in is that we will continously try and retry
      //       to set up comet. But every time we do a continue; we need to make sure that makes
      //       sense, i.e. there is a chance it will work next time; log it, maybe introduce a
      //       delay, etc. OR fail permanently with an error message.
      //
      continue;
    }

    setState(CometState::STARTED_ABCI);

    // --------------------------------------------------------------------------------------
    // Check if quitting
    if (stop_) break;

    // --------------------------------------------------------------------------------------
    // Run cometbft start, passing the socket address of our gRPC server as a parameter.
    // TODO: ensure cometbft is terminated if we are killed (prctl()?)
    //
    // NOTE: cannot simply continue; if we know the process_ exists; we need to handle any
    //       alive process_ in this loop iteration before a continue;
    //       the only other place where we stop process_ is in stop().

    setState(CometState::STARTING_COMET);

    // run cometbft which will connect to our ABCI server
    try {

      startCometBFT(cometPath);

    } catch (const std::exception& ex) {
      // TODO: maybe we should attempt a restart in this case, with a retry counter?
      setError("Exception caught when trying to run cometbft start: " + std::string(ex.what()));
      return;
    }

    // TODO: check if this wait is actually useful/needed
    //std::this_thread::sleep_for(std::chrono::seconds(1));

    // If any immediate error trying to start cometbft, quit here
    // TODO: must check this elsewhere as well?
    if (!this->status_) return;

    setState(CometState::STARTED_COMET);

    // --------------------------------------------------------------------------------------
    // Test the ABCI connection.
    // TODO: ensure cometbft is terminated if we are killed (prctl()?)

    setState(CometState::TESTING_COMET);

    uint64_t testingStartTime = Utils::getCurrentTimeMillisSinceEpoch();
    while (!stop_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        // Info is a reliable call; cometbft always calls it to figure out what
        //  is the last block height that we have so it will e.g. replay from there.
        if (this->infoCount_ > 0) {
          break;
        }

        // Time out in 10 seconds, should never happen
        uint64_t currentTime = Utils::getCurrentTimeMillisSinceEpoch();
        if (currentTime - testingStartTime >= 10000) {
          // Timeout
          // TODO: should warn and continue; instead?
          //       also, shouldn't setError + return always ensure cometbft and the abci server are killed (not running)?
          setError("Timed out while waiting for an Info call from cometbft");
          return;
        }
    }

    // Check if quitting
    if (stop_) break;

    // Test the RPC connection
    LOGDEBUG("Will connect to cometbft RPC at port: " + std::to_string(rpcPort_));
    int rpcTries = 50; //5s
    bool rpcSuccess = false;
    while (rpcTries-- > 0 && !stop_) {
      // wait first, otherwise 1st connection attempt always fails
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      if (startRPCConnection()) {
        rpcSuccess = true;
        break;
      }
      LOGDEBUG("Retrying RPC connection: " + std::to_string(rpcTries));
    }

    // Check if quitting
    if (stop_) break;

    if (!rpcSuccess) {
      setError("Can't connect to the cometbft RPC port.");
      // OR, alternatively, this could be a continue; so we retry the whole thing.
      return;
    }

    // Make a few sample RPC calls using the persisted connection
    for (int i = 0; i < 3; ++i) {
      std::string result;
      bool success = makeJSONRPCCall("health", json::array(), result);
      if (success) {
        LOGDEBUG("cometbft RPC health call returned OK: "+ result);
      } else {
        LOGERROR("ERROR: cometbft RPC health call failed: " + result);
        return;
      }
    }

    setState(CometState::TESTED_COMET);

    // --------------------------------------------------------------------------------------
    // Main loop.
    // If there are queued requests, send them to the comet process.
    // Monitor cometbft integration health until node is shutting down.
    // If something goes wrong, terminate cometbft and restart this worker
    //   (i.e. continue; this outer loop) and it will reconnect/restart comet.

    setState(CometState::RUNNING);

    // NOTE:
    // If this loop breaks for whatever reason without !stop being true, we will be
    //   in the TERMINATED state, unless we decide to continue; to retry, which is
    //   also possibly what we will end up doing here.
    //
    while (!stop_) {

        // TODO: here we are doing work such as:
        //   - polling/blocking at the outgoing transactions queue and pumping them into the running
        //     'cometbft start' process_

        // FIXME/TODO: replace the cometbft rpc connection with the websocket version
        // TODO: improve the RPC transaction pump to be asynchronous, which allows us to send
        //       multiple transactions at once and then wait for the responses. this is probably
        //       using the websocket version of the transaction pump.
        //       in the websocket version, once the tx is pulled from the main queue, it is
        //       inserted into an in-flight txsend map with the RPC request ID as the index
        //       and a timeout; the timeout is the primary way the in-flight request expires
        //       in case the response for the (client-side-generated req id) never arrives for
        //       some reason (like RPC connection remade, cometbft restarted, etc)
        //       if it expires or errors out, it is just reinserted in the main queue.
        //       if it succeeds, it is just removed from the in-flight txsend map.

        Bytes tx;
        while (true) {
          // Lock the queue and check if it's empty
          {
              std::lock_guard<std::mutex> lock(txOutMutex_);
              if (txOut_.empty()) {
                  break;  // Exit if the queue is empty
              }
              tx = txOut_.front();  // Copy the transaction from the front of the queue
          }

          // Encode transaction in base64 (assume base64_encode function is defined)
          std::string encodedTx = base64::encode_into<std::string>(tx.begin(), tx.end());

          // Create the JSON-RPC parameters with the encoded transaction
          json params = { {"tx", encodedTx} };
          std::string result;

          // Send the transaction using JSON-RPC
          LOGTRACE("Sending tx via RPC, size: " + std::to_string(tx.size()));
          bool success = makeJSONRPCCall("broadcast_tx_async", params, result);
          LOGTRACE("Got RPC result: " + result);


          // *********************************************************************************
          // FIXME/TODO: this has to be protected by try/catch otherwise it will error out
          //  the comet worker (exception throws it out of workerloopinner)
          // *********************************************************************************
          json response = json::parse(result);

          // *********************************************************************************
          // FIXME/TODO: attempting to send a transaction that is too large results in this:
          // Transaction send error. Result: {"jsonrpc":"2.0","error":{"code":-32600,"message":"Invalid Request","data":"error reading request body: http: request body too large"}}
          // In that case, the transaction will NEVER go through: the failure is not transient, and this would just loop forever.
          // Have to rethink the idea of retrying transaction sending here.
          // It would be better if the Comet API responded with whether the transaction could be delivered or not.
          // It could return the transaction via the CometListener so it can be resent (or dropped). Also forward the error we got in the listener.
          // *********************************************************************************

          // If the response is successful, remove the transaction from the queue
          if (success) {
              if (response.contains("result")) {
                  LOGTRACE("Transaction sent successfully. Response: " + result);

                  // Remove the transaction from the queue
                  std::lock_guard<std::mutex> lock(txOutMutex_);
                  txOut_.pop_front();
              } else {
                  // ???
                  LOGTRACE("Transaction send error. Result: " + result);
                  // On any error, always break out of the while (true) as we could be quitting
                  break;
              }
          } else {

              // Unfortunately, cometbft thinks that sending a transaction that it already knows
              // about is an error, when it isn't -- it just means the job is already done, the
              // sending of the transaction has been ultimately successful.
              if (response.contains("result") && response[result].contains("error") && response["result"]["error"].contains("data")) {

                  // TODO/REVIEW: do we actually need to do a string error message match to figure this out
                  //              instead of e.g. matching an int error code?
                  if (response["result"]["error"]["data"] == "tx already exists in cache") {
                    LOGTRACE("Transaction sent successfully. Response: " + result);

                    // Remove the transaction from the queue
                    std::lock_guard<std::mutex> lock(txOutMutex_);
                    txOut_.pop_front();

                    // Override error condition below by just continuing the tx pump loop
                    continue;
                  }
              }

              LOGTRACE("Transaction send error. Result: " + result);
              // On any error, always break out of the while (true) as we could be quitting
              break;
          }
        }

        // wait a little bit before we poll the transaction queue and the stop flag again
        // TODO: optimize to condition variables later (when everything else is done)
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    // Check if quitting
    if (stop_) break;

    // --------------------------------------------------------------------------------------
    // If the main loop exits and this is reached then, it is because we are shutting down.
    // Shut down cometbft, clean up and break loop
    // Reaching here is probably an error, and should actually not be possible.
    // This has been useful to detect bugs in the RUNNING state loop above so leave it here.

    setState(CometState::TERMINATED);

    LOGERROR("Comet worker: exiting (loop end reached); this is an error!");
    setError("Reached TERMINATED state (should not be possible).");
    return;
  }

  setState(CometState::FINISHED);

  LOGDEBUG("Comet worker: exiting (quit loop)");
}

// CometImpl's ABCIHandler implementation
//
// NOTE: the "bytes" field in a Protobuf .proto file is implemented as a std::string, which
//       is terrible. there is some support for absl::Cord for non-repeat bytes fields in
//       version 23 of Protobuf, and repeat bytes with absl::Cord is already supported internally
//       but is unreleased. not exactly sure how that would improve interfacing with our Bytes
//       (vector<uint8_t>) convention (maybe we'd change the Bytes typedef to absl::Cord), but
//       in the meantime we have to convert everything which involves one extra memory copy step
//       for every instance of a bytes field.

// Helper to convert the C++ implementation of a Protobuf "repeat bytes" field to std::vector<std::vector<uint8_t>>
std::vector<Bytes> toBytesVector(const google::protobuf::RepeatedPtrField<std::string>& repeatedField) {
  std::vector<Bytes> result;
  for (const auto& str : repeatedField) {
      result.emplace_back(str.begin(), str.end());
  }
  return result;
}

// Helper to convert the C++ implementation of a Protobuf "bytes" field to std::vector<uint8_t>
Bytes toBytes(const std::string& str) {
  return Bytes(str.begin(), str.end());
}

void CometImpl::echo(const cometbft::abci::v1::EchoRequest& req, cometbft::abci::v1::EchoResponse* res) {
  //res->set_message(req.message()); // This is done at the net/abci caller, we don't need to do it here.
  // This callback doesn't seem to be called for ABCI Sockets vs. ABCI gRPC? Not sure what's going on.
}

void CometImpl::flush(const cometbft::abci::v1::FlushRequest& req, cometbft::abci::v1::FlushResponse* res) {
  // Nothing to do for now as all handlers should be synchronous.
}

void CometImpl::info(const cometbft::abci::v1::InfoRequest& req, cometbft::abci::v1::InfoResponse* res) {
  uint64_t height;
  Bytes hashBytes;
  listener_->getCurrentState(height, hashBytes);
  std::string hashString(hashBytes.begin(), hashBytes.end());
  res->set_version("1.0.0"); // TODO: let caller decide/configure this
  res->set_last_block_height(height);
  res->set_last_block_app_hash(hashString);
  ++infoCount_;
}

void CometImpl::init_chain(const cometbft::abci::v1::InitChainRequest& req, cometbft::abci::v1::InitChainResponse* res) {
  listener_->initChain();
  // Populate InitChainResponse based on initial blockchain state
  // res->add_validators(...) if needed
}

void CometImpl::prepare_proposal(const cometbft::abci::v1::PrepareProposalRequest& req, cometbft::abci::v1::PrepareProposalResponse* res) {
  // Just copy the recommended txs from the mempool into the proposal.
  // This requires all block size and limit related params to be set in such a way that the
  //  total byte size of txs in the request don't exceed the total byte size allowed for a block.
  for (const auto& tx : req.txs()) {
    res->add_txs(tx);
  }
}

void CometImpl::process_proposal(const cometbft::abci::v1::ProcessProposalRequest& req, cometbft::abci::v1::ProcessProposalResponse* res) {
  bool accept = false;
  listener_->validateBlockProposal(req.height(), toBytesVector(req.txs()), accept);
  if (accept) {
    res->set_status(cometbft::abci::v1::PROCESS_PROPOSAL_STATUS_ACCEPT);
  } else {
    res->set_status(cometbft::abci::v1::PROCESS_PROPOSAL_STATUS_REJECT);
  }
}

void CometImpl::check_tx(const cometbft::abci::v1::CheckTxRequest& req, cometbft::abci::v1::CheckTxResponse* res) {
  bool accept = false;
  listener_->checkTx(toBytes(req.tx()), accept);
  int ret_code = 0;
  if (!accept) { ret_code = 1; }
  res->set_code(ret_code);
}

void CometImpl::commit(const cometbft::abci::v1::CommitRequest& req, cometbft::abci::v1::CommitResponse* res) {
  uint64_t height;
  listener_->getBlockRetainHeight(height);
  res->set_retain_height(height);
}

void CometImpl::finalize_block(const cometbft::abci::v1::FinalizeBlockRequest& req, cometbft::abci::v1::FinalizeBlockResponse* res) {
  Bytes hashBytes;
  listener_->incomingBlock(req.height(), req.syncing_to_height(), toBytesVector(req.txs()), hashBytes);
  std::string hashString(hashBytes.begin(), hashBytes.end());
  res->set_app_hash(hashString);

  // FIXME/TODO: We actually need to let the Comet user code set a whole lot of optional transaction execution result fields.
  // For now just set the transaction as successfully executed.
  for (const auto& tx : req.txs()) {
    cometbft::abci::v1::ExecTxResult* tx_result = res->add_tx_results();
    tx_result->set_code(0); // 0 == successful tx
  }
}

void CometImpl::query(const cometbft::abci::v1::QueryRequest& req, cometbft::abci::v1::QueryResponse* res) {
  // TODO
}

void CometImpl::list_snapshots(const cometbft::abci::v1::ListSnapshotsRequest& req, cometbft::abci::v1::ListSnapshotsResponse* res) {
  // TODO
}

void CometImpl::offer_snapshot(const cometbft::abci::v1::OfferSnapshotRequest& req, cometbft::abci::v1::OfferSnapshotResponse* res) {
  // TODO
}

void CometImpl::load_snapshot_chunk(const cometbft::abci::v1::LoadSnapshotChunkRequest& req, cometbft::abci::v1::LoadSnapshotChunkResponse* res) {
  // TODO
}

void CometImpl::apply_snapshot_chunk(const cometbft::abci::v1::ApplySnapshotChunkRequest& req, cometbft::abci::v1::ApplySnapshotChunkResponse* res) {
  // TODO
}

void CometImpl::extend_vote(const cometbft::abci::v1::ExtendVoteRequest& req, cometbft::abci::v1::ExtendVoteResponse* res) {
  // TODO -- not sure if this will be needed
}

void CometImpl::verify_vote_extension(const cometbft::abci::v1::VerifyVoteExtensionRequest& req, cometbft::abci::v1::VerifyVoteExtensionResponse* res) {
  // TODO -- not sure if this will be needed
}

// ---------------------------------------------------------------------------------------
// Comet class
// ---------------------------------------------------------------------------------------

Comet::Comet(CometListener* listener, std::string instanceIdStr, const Options& options, bool stepMode/*const std::vector<std::string>& extraArgs*/)
  : instanceIdStr_(instanceIdStr)
{
  impl_ = std::make_unique<CometImpl>(listener, instanceIdStr, options, stepMode/*extraArgs*/);
} 

Comet::~Comet() {
  impl_->stop();
}

bool Comet::getStatus() {
  return impl_->getStatus();
}

const std::string& Comet::getErrorStr() {
  return impl_->getErrorStr();
}

CometState Comet::getState() {
  return impl_->getState();
}

void Comet::setPauseState(const CometState pauseState) {
  return impl_->setPauseState(pauseState);
}

CometState Comet::getPauseState() {
  return impl_->getPauseState();
}

std::string Comet::waitPauseState(uint64_t timeoutMillis) {
  return impl_->waitPauseState(timeoutMillis);
}

std::string Comet::getNodeID() {
  return impl_->getNodeID();
}

void Comet::sendTransaction(const Bytes& tx) {
  impl_->sendTransaction(tx);
}

void Comet::start() {
  impl_->start();
}

void Comet::stop() {
  impl_->stop();
}


