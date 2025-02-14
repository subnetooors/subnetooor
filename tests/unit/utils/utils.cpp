/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#include "libs/catch2/catch_amalgamated.hpp"

#include "utils/utils.h" // (strings.h -> hex.h), (bytes/join.h -> bytes/range.h -> bytes/view.h)
#include "utils/strconv.h"

namespace TUtils {
  TEST_CASE("Utils Namespace", "[unit][utils][utilsitself]") {
    SECTION("fail (HTTP Fail Message) + logToFile, for coverage)") {
      boost::beast::error_code ec;
      fail("Utils", "fail", ec, "testing fail log");
      REQUIRE(!ec);
      Utils::logToFile("testing log to file");
    }

    SECTION("Account struct") {
      // Copy constructor
      Account a(uint256_t(1000), uint64_t(0));
      REQUIRE(a.balance == uint256_t(1000));
      REQUIRE(a.nonce == uint64_t(0));
      // Move constructor
      uint256_t bal = 2000;
      uint64_t nonce = 1;
      Account b(std::move(bal), std::move(nonce));
      REQUIRE(b.balance == 2000);
      REQUIRE(b.nonce == 1);
      // Deserialize constructor
      Bytes ser = a.serialize();
      Account c(ser);
      REQUIRE(c.balance == 1000);
      REQUIRE(c.nonce == 0);
      // For coverage
      ser[72] = 0xFF; // Invalid contract type
      REQUIRE_THROWS(Account(ser));
      ser.pop_back(); // Invalid bytes size (one less)
      REQUIRE_THROWS(Account(ser));
    }

    SECTION("safePrint") {
      Utils::safePrint("Fail!");
      Log::logToCout = true;
      Utils::safePrint("Win!");
      Log::logToCout = false; // Other tests litter terminal output with extra info if we forget to do this
    }

    SECTION("sha3") {
      std::string sha3Input = "My SHA3 Input";
      auto sha3Output = Utils::sha3(StrConv::stringToBytes(sha3Input));
      Bytes sha3ExpectedOutput = Bytes{0x10, 0x11, 0x40, 0xd6, 0xe7, 0x50, 0x6f, 0x80, 0x4c, 0xf7, 0xb0, 0x37, 0x0f, 0xa9, 0x0b, 0x04, 0xc5, 0xe9, 0x37, 0x4d, 0xdb, 0x0c, 0x8c, 0xbe, 0x12, 0xaf, 0x15, 0x0c, 0x8f, 0xf3, 0xee, 0x36};
      REQUIRE(sha3Output == Hash(sha3ExpectedOutput));
    }

    SECTION("randBytes") {
      Bytes randBytesOutput = Utils::randBytes(32);
      REQUIRE(randBytesOutput.size() == 32);
    }

    SECTION("readConfigFile") {
      json cfg = Utils::readConfigFile();
      REQUIRE(cfg.dump() == "{\"rpcport\":8080,\"p2pport\":8081,\"seedNodes\":[\"127.0.0.1:8086\",\"127.0.0.1:8087\",\"127.0.0.1:8088\",\"127.0.0.1:8089\"]}");
    }

    SECTION("getSignalName") {
      REQUIRE(Utils::getSignalName(0) == "Unknown signal (0)");
      REQUIRE(Utils::getSignalName(1) == "SIGHUP (1)");
      REQUIRE(Utils::getSignalName(2) == "SIGINT (2)");
    }

    SECTION("fromBigEndian") {
      std::string_view inputBytes("\x10\x11\x40\xd6\xe7\x50\x6f\x80\x4c\xf7\xb0\x37\x0f\xa9\x0b\x04\xc5\xe9\x37\x4d\xdb\x0c\x8c\xbe\x12\xaf\x15\x0c\x8f\xf3\xee\x36");
      auto uint256Output = Utils::fromBigEndian<uint256_t>(inputBytes);
      auto uint64Ouput12To20 = Utils::fromBigEndian<uint64_t>(inputBytes.substr(12, 8));
      auto uint64Ouput20To28 = Utils::fromBigEndian<uint64_t>(inputBytes.substr(20, 8));
      auto uint64Output24To28 = Utils::fromBigEndian<uint64_t>(inputBytes.substr(24, 4));
      auto uint32Output28To32 = Utils::fromBigEndian<uint32_t>(inputBytes.substr(28, 4));
      auto uint160Output5To25 = Utils::fromBigEndian<uint160_t>(inputBytes.substr(5, 20));
      uint256_t uint256ExpectedOutput = uint256_t("7267489482988504755957722036644729207517128093499486419604741885099068616246");
      uint64_t uint64ExpectedOutput12To20 = uint64_t(1128445296761190221);
      uint64_t uint64ExpectedOutput20To28 = uint64_t(15784145542011884812ULL);
      uint64_t uint64ExpectedOutput24To28 = uint64_t(313464076);
      uint32_t uint32ExpectedOutput28To32 = uint32_t(2415128118);
      uint160_t uint160ExpectedOutput5To25 = uint160_t("459205820946237488389499242237511570682479951378");
      REQUIRE(uint256Output == uint256ExpectedOutput);
      REQUIRE(uint64Ouput12To20 == uint64ExpectedOutput12To20);
      REQUIRE(uint64Ouput20To28 == uint64ExpectedOutput20To28);
      REQUIRE(uint64Output24To28 == uint64ExpectedOutput24To28);
      REQUIRE(uint32Output28To32 == uint32ExpectedOutput28To32);
      REQUIRE(uint160Output5To25 == uint160ExpectedOutput5To25);
    }

    SECTION("appendBytes") {
      Bytes int1 = Bytes{0x78, 0xF0, 0xB2, 0x91};
      Bytes int2 = Bytes{0xAC, 0x26, 0x0E, 0x43};
      Bytes res = Bytes{0x78, 0xF0, 0xB2, 0x91, 0xAC, 0x26, 0x0E, 0x43};
      Utils::appendBytes(int1, int2);
      REQUIRE(int1 == res);
    }

    SECTION("create_view_span") {
      Bytes b{0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
      std::string_view sv("abcdef");
      View<Bytes> v1 = Utils::create_view_span(b, 0, 6);
      View<Bytes> v2 = Utils::create_view_span(sv, 0, 6);
      REQUIRE(Hex::fromBytes(v1).get() == "0a0b0c0d0e0f");
      REQUIRE(Hex::fromBytes(v2).get() == "616263646566");
      REQUIRE_THROWS(Utils::create_view_span(b, 0, 12));
      REQUIRE_THROWS(Utils::create_view_span(sv, 0, 12));
    }

    SECTION("readConfigFile") {
      if (std::filesystem::exists("config.json")) std::filesystem::remove("config.json");
      json j = Utils::readConfigFile(); // Creating from scratch
      json j2 = Utils::readConfigFile(); // Loading from default
      REQUIRE(j == j2);
      REQUIRE(j["rpcport"] == 8080);
      REQUIRE(j["p2pport"] == 8081);
      REQUIRE(j["seedNodes"][0] == "127.0.0.1:8086");
      REQUIRE(j["seedNodes"][1] == "127.0.0.1:8087");
      REQUIRE(j["seedNodes"][2] == "127.0.0.1:8088");
      REQUIRE(j["seedNodes"][3] == "127.0.0.1:8089");
    }
  }
}

