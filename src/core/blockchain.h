/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H

#include "consensus.h"
#include "storage.h"
#include "state.h"
#include "rdpos.h"
#include "state.h"

#include "../net/p2p/managerbase.h"
#include "../net/p2p/nodeconns.h"
#include "../net/http/httpserver.h"
#include "../utils/options.h"
#include "../utils/db.h"

/**
 * Helper class that syncs the node with other nodes in the network.
 * Currently it's *single threaded*, meaning that it doesn't require mutexes.
 */
class Syncer : public Log::LogicalLocationProvider {
  private:
    P2P::ManagerNormal& p2p_;  ///< Reference to the P2P networking engine.
    const Storage& storage_;   ///< Reference to the blockchain storage.
    State& state_;             ///< reference to the blockchain state.
    std::atomic<bool> synced_ = false;  ///< Indicates whether or not the syncer is synced.

  public:
    /**
     * Constructor.
     * @param nodeConns Reference to the NodeConns object.
     * @param storage Reference to the blockchain storage.
     */
    explicit Syncer(P2P::ManagerNormal& p2p, const Storage& storage, State& state) :
      p2p_(p2p), storage_(storage), state_(state) {}

    std::string getLogicalLocation() const override { return p2p_.getLogicalLocation(); } ///< Log instance from P2P

    /**
     * Synchronize this node to the latest known blocks among all connected peers at the time this method is called.
     * @param blocksPerRequest How many blocks (at most) you want to obtain on each requestto the remote node.
     * @param bytesPerRequestLimit Maximum byte size of each block download response (will download 1 block minimum).
     * @param waitForPeersSecs Seconds to wait for at least one connection to be established (cumulative).
     * @param tries If zero, try forever, otherwise try block downloads a set number of times.
     * @return `true` if successfully synced, `false` otherwise.
     */
    bool sync(
      uint64_t blocksPerRequest, uint64_t bytesPerRequestLimit,
      int waitForPeersSecs = 15, int tries = 0
    );

    /**
     * Helper function that does the sync loop. Used exclusively by sync().
     * @param blocksPerRequest How many blocks (at most) you want to obtain on each requestto the remote node.
     * @param bytesPerRequestLimit Maximum byte size of each block download response (will download 1 block minimum).
     * @param waitForPeersSecs Seconds to wait for at least one connection to be established (cumulative).
     * @param tries If zero, try forever, otherwise try block downloads a set number of times.
     * @param highestNode The highest known node to use as the target height to end the sync process.
     * @return `true` when loop is successful, `false` when loop is aborted.
     */
    bool syncLoop(
      const uint64_t& blocksPerRequest, const uint64_t& bytesPerRequestLimit,
      int& waitForPeersSecs, int& tries,
      std::pair<P2P::NodeID, uint64_t>& highestNode
    );

    ///@{
    /** Getter. */
    const std::atomic<bool>& isSynced() const { return this->synced_; }
    ///@}
};

/**
 * Master class that represents the blockchain as a whole.
 * Contains, and acts as the middleman of, every other part of the core and net protocols.
 * Those parts interact with one another by communicating through this class.
 */
class Blockchain : public Log::LogicalLocationProvider {
  private:
    Options options_;           ///< Options singleton.
    const DB db_;               ///< Database.
    Storage storage_;           ///< Blockchain storage.
    State state_;               ///< Blockchain state.
    P2P::ManagerNormal p2p_;    ///< P2P connection manager. NOTE: must be initialized first due to getLogicalLocation()
    HTTPServer http_;           ///< HTTP server.
    Syncer syncer_;             ///< Blockchain syncer.
    Consensus consensus_;       ///< Block and transaction processing.
    ExternalEngine ext_;

  public:
    /**
     * Constructor.
     * @param blockchainPath Root path of the blockchain.
     */
    explicit Blockchain(const std::string& blockchainPath);
    ~Blockchain() = default;  ///< Default destructor.
    std::string getLogicalLocation() const override { return p2p_.getLogicalLocation(); } ///< Log instance from P2P
    void start(); ///< Start the blockchain. Initializes P2P, HTTP and Syncer, in this order.
    void stop();  ///< Stop/shutdown the blockchain. Stops Syncer, HTTP and P2P, in this order (reverse order of start()).

    ///@{
    /** Getter. */
    Options& getOptions() { return this->options_; }
    const DB& getDB() const { return this->db_; }
    Storage& getStorage() { return this->storage_; }
    State& getState() { return this->state_; }
    P2P::ManagerNormal& getP2P() { return this->p2p_; }
    HTTPServer& getHTTP() { return this->http_; }
    Syncer& getSyncer() { return this->syncer_; }
    Consensus& getConsensus() { return this->consensus_; }
    ///@}

    /// Check if the blockchain is synced.
    const std::atomic<bool>& isSynced() const { return this->syncer_.isSynced(); }
};

#endif // BLOCKCHAIN_H
