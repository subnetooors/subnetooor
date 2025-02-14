/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#include "libs/catch2/catch_amalgamated.hpp"

#include "utils/strconv.h"
#include "utils/hex.h"

using Catch::Matchers::Equals;

namespace TUtils {
  TEST_CASE("StrConv Namespace", "[unit][utils][strconv]") {
    SECTION("padLeft") {
      std::string in = "abcdef";
      std::string out = StrConv::padLeft(in, 10, '0');
      std::string out2 = StrConv::padLeft(in, 20, '1');
      std::string expOut = "0000abcdef";
      std::string expOut2 = "11111111111111abcdef";
      REQUIRE(out == expOut);
      REQUIRE(out2 == expOut2);
      // Check 0x (for coverage)
      in = "0xabcdef";
      out = StrConv::padLeft(in, 10, '0');
      out2 = StrConv::padLeft(in, 20, '1');
      expOut = "0x0000abcdef";
      expOut2 = "0x11111111111111abcdef";
      REQUIRE(out == expOut);
      REQUIRE(out2 == expOut2);
      // Check 0X (for coverage)
      in = "0Xabcdef";
      out = StrConv::padLeft(in, 10, '0');
      out2 = StrConv::padLeft(in, 20, '1');
      expOut = "0x0000abcdef";
      expOut2 = "0x11111111111111abcdef";
      REQUIRE(out == expOut);
      REQUIRE(out2 == expOut2);
      // Check no padding (for coverage)
      in = "abcdef";
      out = StrConv::padLeft(in, 4, '0');
      expOut = "abcdef";
      REQUIRE(out == expOut);
    }

    SECTION("padRight") {
      std::string in = "abcdef";
      std::string out = StrConv::padRight(in, 10, '0');
      std::string out2 = StrConv::padRight(in, 20, '1');
      std::string expOut = "abcdef0000";
      std::string expOut2 = "abcdef11111111111111";
      REQUIRE(out == expOut);
      REQUIRE(out2 == expOut2);
      // Check 0x (for coverage)
      in = "0xabcdef";
      out = StrConv::padRight(in, 10, '0');
      out2 = StrConv::padRight(in, 20, '1');
      expOut = "0xabcdef0000";
      expOut2 = "0xabcdef11111111111111";
      REQUIRE(out == expOut);
      REQUIRE(out2 == expOut2);
      // Check 0X (for coverage)
      in = "0Xabcdef";
      out = StrConv::padRight(in, 10, '0');
      out2 = StrConv::padRight(in, 20, '1');
      expOut = "0xabcdef0000";
      expOut2 = "0xabcdef11111111111111";
      REQUIRE(out == expOut);
      REQUIRE(out2 == expOut2);
      // Check no padding (for coverage)
      in = "abcdef";
      out = StrConv::padRight(in, 4, '0');
      expOut = "abcdef";
      REQUIRE(out == expOut);
    }

    SECTION("padLeftBytes") {
      Bytes in = Hex::toBytes("0xabcdef");
      Bytes out = StrConv::padLeftBytes(in, 10, 0x00);
      Bytes out2 = StrConv::padLeftBytes(in, 20, 0x11);
      Bytes expOut = Hex::toBytes("0x00000000000000abcdef");
      Bytes expOut2 = Hex::toBytes("0x1111111111111111111111111111111111abcdef");
      REQUIRE(out == expOut);
      REQUIRE(out2 == expOut2);
      // Check no padding (for coverage)
      in = Hex::toBytes("0xabcdef");
      out = StrConv::padLeftBytes(in, 2, 0x00);
      expOut = Hex::toBytes("0xabcdef");
      REQUIRE(out == expOut);
    }

    SECTION("padRightBytes") {
      Bytes in = Hex::toBytes("0xabcdef");
      Bytes out = StrConv::padRightBytes(in, 10, 0x00);
      Bytes out2 = StrConv::padRightBytes(in, 20, 0x11);
      Bytes expOut = Hex::toBytes("0xabcdef00000000000000");
      Bytes expOut2 = Hex::toBytes("0xabcdef1111111111111111111111111111111111");
      REQUIRE(out == expOut);
      REQUIRE(out2 == expOut2);
      // Check no padding (for coverage)
      in = Hex::toBytes("0xabcdef");
      out = StrConv::padRightBytes(in, 2, 0x00);
      expOut = Hex::toBytes("0xabcdef");
      REQUIRE(out == expOut);
    }

    SECTION("toLower") {
      std::string in = "ABCDEF";
      std::string out = in;
      StrConv::toLower(out);
      std::string expOut = "abcdef";
      REQUIRE_THAT(out, Equals(expOut));
    }

    SECTION("toUpper") {
      std::string in = "abcdef";
      std::string out = in;
      StrConv::toUpper(out);
      std::string expOut = "ABCDEF";
      REQUIRE_THAT(out, Equals(expOut));
    }

    SECTION("bytesToString") {
      Bytes b1 = Bytes{0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37};
      Bytes b2 = Bytes{0x30, 0x42, 0x34, 0x48, 0x52, 0x36, 0x33, 0x39};
      std::string s1 = StrConv::bytesToString(b1);
      std::string s2 = StrConv::bytesToString(b2);
      REQUIRE_THAT(s1, Equals("01234567"));
      REQUIRE_THAT(s2, Equals("0B4HR639"));
    }

    SECTION("stringToBytes") {
      std::string s1 = "01234567";
      std::string s2 = "0B4HR639";
      Bytes b1 = StrConv::stringToBytes(s1);
      Bytes b2 = StrConv::stringToBytes(s2);
      REQUIRE(b1 == Bytes{0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37});
      REQUIRE(b2 == Bytes{0x30, 0x42, 0x34, 0x48, 0x52, 0x36, 0x33, 0x39});
    }
  }
}

