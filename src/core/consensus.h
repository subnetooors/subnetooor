/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#ifndef CONSENSUS_H
#define CONSENSUS_H

#include <thread>

#include "rdpos.h"

#include "../utils/ecdsa.h"
#include "../utils/logger.h"
#include "../utils/strings.h"
#include "../utils/tx.h"

class Blockchain; // Forward declaration.

// TODO: tests for Consensus (if necessary)

/// Class responsible for processing blocks and transactions.
class Consensus : public Log::LogicalLocationProvider {
  private:
    State& state_; ///< Reference to the State object.
    P2P::ManagerNormal& p2p_; ///< Reference to the P2P connection manager.
    const Storage& storage_; ///< Reference to the blockchain storage.
    const Options& options_; ///< Reference to the Options singleton.

    std::future<void> loopFuture_;  ///< Future object holding the thread for the consensus loop.
    std::atomic<bool> stop_ = false; ///< Flag for stopping the consensus processing.

    /**
     * Create and broadcast a Validator block (called by validatorLoop()).
     * If the node is a Validator and it has to create a new block,
     * this function will be called, the new block will be created based on the
     * current State and rdPoS objects, and then it will be broadcast.
     * @throw DynamicException if block is invalid.
     */
    void doValidatorBlock();

    /**
     * Wait for a new block (called by validatorLoop()).
     * If the node is a Validator, this function will be called to make the
     * node wait until it receives a new block.
     */
    void doValidatorTx(const uint64_t& nHeight, const Validator& me);

  public:
    /**
     * Constructor.
     * @param state Reference to the State object.
     * @param p2p Reference to the P2P connection manager.
     * @param storage Reference to the blockchain storage.
     * @param options Reference to the Options singleton.
     */
    explicit Consensus(State& state, P2P::ManagerNormal& p2p, const Storage& storage, const Options& options) :
      state_(state), p2p_(p2p), storage_(storage), options_(options) {}

    std::string getLogicalLocation() const override { return p2p_.getLogicalLocation(); } ///< Log instance from P2P

    /**
     * Entry function for the worker thread (runs the workerLoop() function).
     * @return `true` when done running.
     */
    bool workerLoop();
    void validatorLoop(); ///< Routine loop for when the node is a Validator.

    void start(); ///< Start the consensus loop. Should only be called after node is synced.
    void stop();  ///< Stop the consensus loop.
};

// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------
// TODO: move to own file

#include <boost/process.hpp>
#include <thread>
#include <grpcpp/grpcpp.h>

class ExternalEngine {
public:
  ExternalEngine() : started_(false), grpcServerStarted_(false) {
  }

  // TODO: use
  //       the idea is to use the system PATH to find the executable, instead
  //       of adding more configuration overhead.
  //       the name of the executable can also be hard-coded for now.
  //void setLocation(const std::string& location) { location_ = location; }
  //std::string getLocation() { return location_; }

  // start if stopped
  bool start();

  // stop if started
  bool stop();

  // if in started state
  bool isStarted();

  // if started, returns whether the engine is running or not (i.e. pid exists)
  // if stopped, returns false
  bool isRunning();


  // this makes no sense; first we can assume that if the cometbft PID is alive
  //   then it is connected to us (if it got the GRPC server endpoint address
  //   right and we did open it so we are reachable)
  //
  // if started and running, check if connected to the engine
  //bool isConnected();



private:

  void grpcServerRun();

  //std::string location_; // TODO: use, or maybe not; leave in the path; or use for executable name

  std::atomic<bool> started_;

  std::atomic<bool> grpcServerStarted_;
  std::atomic<bool> grpcServerRunning_;

  std::optional<boost::process::child> process_;

  std::unique_ptr<grpc::Server> grpcServer_;

  std::optional<std::thread> grpcServerThread_;

};

// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------

#endif  // CONSENSUS_H
