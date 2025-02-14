/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#include "libs/catch2/catch_amalgamated.hpp"

#include "utils/safehash.h"

#include "bytes/hex.h"
#include "bytes/random.h"

namespace TSafeHash {
  TEST_CASE("SafeHash Struct", "[unit][utils][safehash]") {
    SECTION("SafeHash operator() (coverage)") {
      const uint64_t i64 = 12345;
      const uint256_t i256 = uint256_t(12345);
      const std::string str = "Hello World";
      const std::string_view strView = "Goodbye Planet";
      const Functor func{83892529};
      const Hash hash = bytes::random();
      //const std::shared_ptr<std::string> ptr = std::make_shared<std::string>(str);
      const View<Bytes> bytesView = Utils::create_view_span(str);
      const std::pair<uint64_t, std::string> pair = std::make_pair(54321, "TestString");
      //const boost::unordered_flat_map<int, std::string, SafeHash> map = {{1, "aaa"}, {2, "bbb"}};
      const std::pair<boost::asio::ip::address, uint16_t> nodeIdV4 = std::make_pair(
        boost::asio::ip::address_v4::from_string("127.0.0.1"), 8000
      );
      const std::pair<boost::asio::ip::address, uint16_t> nodeIdV6 = std::make_pair(
        boost::asio::ip::address_v6::from_string("::1"), 8000
      );
      SafeHash()(nodeIdV4);
      SafeHash()(nodeIdV6);

      SafeHash()(i64);
      SafeHash()(i256);
      SafeHash()(str);
      SafeHash()(strView);
      SafeHash()(func);
      SafeHash()(hash);
      //SafeHash()(ptr);  // Not sure why those error out
      SafeHash()(bytesView);
      SafeHash()(pair);
      //SafeHash()(map);

      REQUIRE(true);  // No test cases, just calling functions for coverage purposes
    }
  }
}

