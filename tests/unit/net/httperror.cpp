/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#include "libs/catch2/catch_amalgamated.hpp"

#include "net/http/jsonrpc/error.h"

namespace THTTPError {
  TEST_CASE("HTTP JSONRPC Error Tests", "[unit][net][httperror]") {
    SECTION("Constructor and Getters") {
      jsonrpc::Error e(157, "This vehicle is being robbed");
      REQUIRE(e.code() == 157);
      REQUIRE(e.message() == "This vehicle is being robbed");
    }

    SECTION("Pre-determined Messages") {
      jsonrpc::Error e1 = jsonrpc::Error::invalidType("integer", "double");
      REQUIRE(e1.code() == -32601);
      REQUIRE(e1.message() == "Parsing error: invalid type, exp 'integer' - got 'double'");
      jsonrpc::Error e2 = jsonrpc::Error::invalidFormat("weirdObject");
      REQUIRE(e2.code() == -32601);
      REQUIRE(e2.message() == "Parsing error: 'weirdObject' is in invalid format");
      jsonrpc::Error e3 = jsonrpc::Error::insufficientValues();
      REQUIRE(e3.code() == -32601);
      REQUIRE(e3.message() == "Parsing error: insufficient values in array");
      jsonrpc::Error e4 = jsonrpc::Error::methodNotAvailable("eth_get_rich_quick");
      REQUIRE(e4.code() == -32601);
      REQUIRE(e4.message() == "Method \"eth_get_rich_quick\" not found/available");
      jsonrpc::Error e5 = jsonrpc::Error::executionError("tried to divide by zero");
      REQUIRE(e5.code() == -32603);
      REQUIRE(e5.message() == "Execution error: tried to divide by zero");
    }
  }
}

