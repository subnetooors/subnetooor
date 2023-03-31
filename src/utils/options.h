#ifndef OPTIONS_H
#define OPTIONS_H

#include "utils.h"
#include "ecdsa.h"

#include <filesystem>

class Options {
  private:
    /// Path to data root folder
    const std::string rootPath;

    /// Version of the OrbiterSDK
    const uint64_t SDKVersion = 1;

    /// clientVersion
    const std::string web3clientVersion;

    /// chain version
    const uint64_t version;

    /// ChainID
    const uint64_t chainID;

    /// Websocket Server Port
    const uint16_t wsPort;

    /// HTTP Server Port
    const uint16_t httpPort;

    /// Coinbase Address (if found), used by rdPoS.
    const Address coinbase;

    /// isValidator (Set by constructor or if found on file.
    const bool isValidator;

  public:
    /// Copy Constructor
    Options(const Options& other) :
      rootPath(other.rootPath),
      SDKVersion(other.SDKVersion),
      web3clientVersion(other.web3clientVersion),
      version(other.version),
      chainID(other.chainID),
      wsPort(other.wsPort),
      httpPort(other.httpPort),
      coinbase(other.coinbase),
      isValidator(other.isValidator) {}


    /// Constructor for Options
    /// Populates coinbase() and isValidator() with false
    /// Creates option.json file within rootPath
    /// @params rootPath: Path to data root folder
    /// @params web3clientVersion: Version of the OrbiterSDK
    /// @params version: chain version
    /// @params chainID: ChainID
    /// @params wsPort: Websocket Server Port
    /// @params httpPort: HTTP Server Port
    Options(const std::string& rootPath,
            const std::string& web3clientVersion,
            const uint64_t& version,
            const uint64_t& chainID,
            const uint16_t& wsPort,
            const uint16_t& httpPort);

    /// Constructor for Options
    /// Populates coinbase() and isValidator() with privKey address and true respectively
    /// Creates option.json file within rootPath
    /// @params rootPath: Path to data root folder
    /// @params web3clientVersion: Version of the OrbiterSDK
    /// @params version: chain version
    /// @params chainID: ChainID
    /// @params wsPort: Websocket Server Port
    /// @params httpPort: HTTP Server Port
    Options(const std::string& rootPath,
            const std::string& web3clientVersion,
            const uint64_t& version,
            const uint64_t& chainID,
            const uint16_t& wsPort,
            const uint16_t& httpPort,
            const PrivKey& privKey
            );

    /// Tries to load options.json from rootPath
    /// Defaults to Options(rootPath, "OrbiterSDK/cpp/linux_x86-64/0.0.1", 1, 8080, 8080, 8081) if not found.
    /// @params rootPath: Path to data root folder
    /// @returns Options: Options object
    static Options fromFile(const std::string& rootPath);


    /// Options Getters
    const std::string& getRootPath() { return this->rootPath; }
    const uint64_t& getSDKVersion() { return this->SDKVersion; }
    const std::string& getWeb3ClientVersion() { return this->web3clientVersion; }
    const uint64_t& getVersion() { return this->version; }
    const uint64_t& getChainID() { return this->chainID; }
    const uint16_t& getWsPort() { return this->wsPort; }
    const uint16_t& getHttpPort() { return this->httpPort; }
    const Address& getCoinbase() { return this->coinbase; }
    const bool& getIsValidator() { return this->isValidator; }

};


#endif // OPTIONS_H