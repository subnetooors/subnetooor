/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#include "libs/catch2/catch_amalgamated.hpp"

#include "utils/options.h" // finalizedblock.h -> merkle.h -> tx.h -> ecdsa.h -> utils.h -> filesystem
#include "utils/utils.h" // getTestDumpPath() - need to include

namespace TOptions {
  TEST_CASE("Options Class", "[unit][utils][options]") {
    std::string testDumpPath = Utils::getTestDumpPath();
    SECTION("Options from File (default)") {
      if (std::filesystem::exists(testDumpPath + "/optionClassFromFile")) {
        std::filesystem::remove_all(testDumpPath + "/optionClassFromFile");
      }

      Options options(
        testDumpPath + "/optionClassFromFile",
        "BDK/cpp/linux_x86-64/0.2.0",
        1,
        8080,
        Address(Hex::toBytes("0x00dead00665771855a34155f5e7405489df2c3c6")),
        8080,
        2000,
        10000,
        1000,
        4,
        IndexingMode::RPC,
        json::object()  // TODO: CometBFT config here
      );

      Options optionsFromFile(Options::fromFile(testDumpPath + "/optionClassFromFile"));
      REQUIRE(optionsFromFile.getRootPath() == options.getRootPath());
      REQUIRE(optionsFromFile.getMajorSDKVersion() == options.getMajorSDKVersion());
      REQUIRE(optionsFromFile.getMinorSDKVersion() == options.getMinorSDKVersion());
      REQUIRE(optionsFromFile.getPatchSDKVersion() == options.getPatchSDKVersion());
      REQUIRE(optionsFromFile.getWeb3ClientVersion() == options.getWeb3ClientVersion());
      REQUIRE(optionsFromFile.getVersion() == options.getVersion());
      REQUIRE(optionsFromFile.getChainOwner() == options.getChainOwner());
      REQUIRE(optionsFromFile.getChainID() == options.getChainID());
      REQUIRE(optionsFromFile.getHttpPort() == options.getHttpPort());
      REQUIRE(optionsFromFile.getEventBlockCap() == options.getEventBlockCap());
      REQUIRE(optionsFromFile.getEventLogCap() == options.getEventLogCap());
      REQUIRE(optionsFromFile.getStateDumpTrigger() == options.getStateDumpTrigger());
      // TODO: test CometBFT output here
    }

    SECTION("IndexingMode Coverage") {
      IndexingMode idx1(IndexingMode::DISABLED);
      IndexingMode idx2(IndexingMode::RPC);
      IndexingMode idx3(IndexingMode::RPC_TRACE);
      IndexingMode idx1Str("DISABLED");
      IndexingMode idx2Str("RPC");
      IndexingMode idx3Str("RPC_TRACE");
      std::string_view idx1View = idx1.toString();
      std::string_view idx2View = idx2.toString();
      std::string_view idx3View = idx3.toString();
      REQUIRE(idx1 == IndexingMode::DISABLED);
      REQUIRE(idx2 == IndexingMode::RPC);
      REQUIRE(idx3 == IndexingMode::RPC_TRACE);
      REQUIRE(idx1 == idx1Str);
      REQUIRE(idx2 == idx2Str);
      REQUIRE(idx3 == idx3Str);
      REQUIRE(idx1View == "DISABLED");
      REQUIRE(idx2View == "RPC");
      REQUIRE(idx3View == "RPC_TRACE");
      REQUIRE_THROWS(IndexingMode("unknown"));
    }
  }
}

