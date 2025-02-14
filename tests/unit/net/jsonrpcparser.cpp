/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#include "libs/catch2/catch_amalgamated.hpp"

#include "net/http/jsonrpc/blocktag.h" // parser.h

#include "bytes/random.h"
#include "bytes/hex.h"

namespace THTTPJSONRPCParser {
  TEST_CASE("JSON RPC Parser Tests", "[unit][net][jsonrpcparser]") {
    SECTION("operator()") {
      Hash h = bytes::random();
      std::vector<uint64_t> v = {10, 20, 30, 40, 50};

      // Parser regex REQUIRES hex prefix (0x)
      json jsonHash = h.hex(true).get();
      json jsonAdd = Address(bytes::hex("0x0000111122223333444455556666777788889999")).hex(true).get();
      json jsonBytes = Hex::fromBytes(Bytes{0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF}, true).get();
      json jsonBool = true;
      json jsonFloat = 13.37f;
      json jsonUint = uint64_t(3926591489);
      json jsonUintStr = "0xea0b0801";
      json jsonOptional = nullptr;
      json jsonVariant = uint64_t(12345);
      json jsonBlockTagLatest = "latest";
      json jsonBlockTagEarliest = "earliest";
      json jsonBlockTagPending = "pending";
      json jsonVectorArr = v;
      json jsonVectorObj = json::object();
      for (int i = 0; i < v.size(); i++) jsonVectorObj[std::to_string(i)] = v[i];

      Hash resHash = jsonrpc::parse<Hash>(jsonHash);
      Address resAdd = jsonrpc::parse<Address>(jsonAdd);
      Bytes resBytes = jsonrpc::parse<Bytes>(jsonBytes);
      bool resBool = jsonrpc::parse<bool>(jsonBool);
      float resFloat = jsonrpc::parse<float>(jsonFloat);
      uint64_t resUint = jsonrpc::parse<uint64_t>(jsonUint);
      uint64_t resUintStr = jsonrpc::parse<uint64_t>(jsonUintStr);
      std::optional<uint64_t> resOptional = jsonrpc::parse<std::optional<uint64_t>>(jsonOptional);
      std::variant<uint64_t, bool> resVariant = jsonrpc::parse<std::variant<uint64_t, bool>>(jsonVariant);
      jsonrpc::BlockTag resBlockTagLatest = jsonrpc::parse<jsonrpc::BlockTag>(jsonBlockTagLatest);
      jsonrpc::BlockTag resBlockTagEarliest = jsonrpc::parse<jsonrpc::BlockTag>(jsonBlockTagEarliest);
      jsonrpc::BlockTag resBlockTagPending = jsonrpc::parse<jsonrpc::BlockTag>(jsonBlockTagPending);
      std::vector<uint64_t> resVectorArr = jsonrpc::parse<std::vector<uint64_t>>(jsonVectorArr);
      std::vector<uint64_t> resVectorObj = jsonrpc::parse<std::vector<uint64_t>>(jsonVectorObj);

      REQUIRE(resHash == h);
      REQUIRE(resAdd == Address(bytes::hex("0x0000111122223333444455556666777788889999")));
      REQUIRE(resBytes == Bytes{0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF});
      REQUIRE(resBool == true);
      REQUIRE(resFloat == 13.37f);
      REQUIRE(resUint == uint64_t(3926591489));
      REQUIRE(resUintStr == uint64_t(3926591489));
      REQUIRE(resOptional == std::nullopt);
      REQUIRE(std::get<uint64_t>(resVariant) == 12345);
      REQUIRE(resBlockTagLatest == jsonrpc::BlockTag::LATEST);
      REQUIRE(resBlockTagEarliest == jsonrpc::BlockTag::EARLIEST);
      REQUIRE(resBlockTagPending == jsonrpc::BlockTag::PENDING);
      REQUIRE(resVectorArr == v);
      REQUIRE(resVectorObj == v);
    }

    SECTION("operator() (throws)") {
      // Same thing but everything is wrong on purpose to cover throw cases
      json hashWrongType = json::array(); // Type is not string (or the required type)
      json hashWrongFormat = Hash(bytes::random()).hex().get(); // No "0x"
      json addWrongType = json::array();
      json addWrongFormat = Address(bytes::hex("0x0000111122223333444455556666777788889999")).hex().get();
      json bytesWrongType = json::array();
      json bytesWrongFormat = "0x000g"; // Invalid hex (0-9a-fA-F)
      json boolWrongType = json::array();
      json floatWrongType = json::array();
      json uintWrongType = json::array();
      json uintWrongFormat = "ea0b0801";
      json uintWrongNumber = "hhhh"; // Invalid number
      json blockTagWrongType = json::array();
      json blockTagWrongFormat = "holding"; // Invalid tag ("latest", "earliest", "pending")
      json vectorWrongType = -1; // Not array or object
      REQUIRE_THROWS(jsonrpc::parse<Hash>(hashWrongType));
      REQUIRE_THROWS(jsonrpc::parse<Hash>(hashWrongFormat));
      REQUIRE_THROWS(jsonrpc::parse<Address>(addWrongType));
      REQUIRE_THROWS(jsonrpc::parse<Address>(addWrongFormat));
      REQUIRE_THROWS(jsonrpc::parse<Bytes>(bytesWrongType));
      REQUIRE_THROWS(jsonrpc::parse<Bytes>(bytesWrongFormat));
      REQUIRE_THROWS(jsonrpc::parse<bool>(boolWrongType));
      REQUIRE_THROWS(jsonrpc::parse<float>(floatWrongType));
      REQUIRE_THROWS(jsonrpc::parse<uint64_t>(uintWrongType));
      REQUIRE_THROWS(jsonrpc::parse<uint64_t>(uintWrongFormat));
      REQUIRE_THROWS(jsonrpc::parse<uint64_t>(uintWrongNumber));
      REQUIRE_THROWS(jsonrpc::parse<jsonrpc::BlockTag>(blockTagWrongType));
      REQUIRE_THROWS(jsonrpc::parse<jsonrpc::BlockTag>(blockTagWrongFormat));
      REQUIRE_THROWS(jsonrpc::parse<std::vector<uint64_t>>(vectorWrongType));
    }
  }
}

