/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#include "libs/catch2/catch_amalgamated.hpp"

#include "utils/hex.h"

using Catch::Matchers::Equals;

namespace THex {
  TEST_CASE("Hex Class", "[unit][utils][hex]") {
    SECTION("Default Constructor") {
      Hex hex;
      Hex hexStrict(true);
      REQUIRE_THAT(hex.get(), Equals(""));
      REQUIRE_THAT(hexStrict.get(), Equals("0x"));
    }

    SECTION("String View Constructor") {
      Hex hexA1(std::string_view("0x1234"), true);
      Hex hexA2(std::string_view("0X1234"), true);
      Hex hexA3(std::string_view("1234"), true);
      Hex hexB1(std::string_view("0x1234"), false);
      Hex hexB2(std::string_view("0X1234"), false);
      Hex hexB3(std::string_view("1234"), false);
      REQUIRE_THAT(hexA1.get(), Equals("0x1234"));
      REQUIRE_THAT(hexA2.get(), Equals("0x1234"));
      REQUIRE_THAT(hexA3.get(), Equals("0x1234"));
      REQUIRE_THAT(hexB1.get(), Equals("1234"));
      REQUIRE_THAT(hexB2.get(), Equals("1234"));
      REQUIRE_THAT(hexB3.get(), Equals("1234"));
      REQUIRE_THROWS(Hex(std::string_view("01234"), true));
      REQUIRE_THROWS(Hex(std::string_view("x1234"), true));
      REQUIRE_THROWS(Hex(std::string_view("x1234"), false));
    }

    SECTION("Copy Constructor") {
      Hex hexA1(std::string("0x1234"), true);
      Hex hexA2(std::string("0X1234"), true);
      Hex hexA3(std::string("1234"), true);
      Hex hexB1(std::string("0x1234"), false);
      Hex hexB2(std::string("0X1234"), false);
      Hex hexB3(std::string("1234"), false);
      REQUIRE_THAT(hexA1.get(), Equals("0x1234"));
      REQUIRE_THAT(hexA2.get(), Equals("0x1234"));
      REQUIRE_THAT(hexA3.get(), Equals("0x1234"));
      REQUIRE_THAT(hexB1.get(), Equals("1234"));
      REQUIRE_THAT(hexB2.get(), Equals("1234"));
      REQUIRE_THAT(hexB3.get(), Equals("1234"));
      REQUIRE_THROWS(Hex(std::string("01234"), true));
      REQUIRE_THROWS(Hex(std::string("x1234"), true));
      REQUIRE_THROWS(Hex(std::string("x1234"), false));
    }

    SECTION("Move Constructor") {
      Hex hexA1(std::move(std::string("0x1234")), true);
      Hex hexA2(std::move(std::string("0X1234")), true);
      Hex hexA3(std::move(std::string("1234")), true);
      Hex hexB1(std::move(std::string("0x1234")), false);
      Hex hexB2(std::move(std::string("0X1234")), false);
      Hex hexB3(std::move(std::string("1234")), false);
      REQUIRE_THAT(hexA1.get(), Equals("0x1234"));
      REQUIRE_THAT(hexA2.get(), Equals("0x1234"));
      REQUIRE_THAT(hexA3.get(), Equals("0x1234"));
      REQUIRE_THAT(hexB1.get(), Equals("1234"));
      REQUIRE_THAT(hexB2.get(), Equals("1234"));
      REQUIRE_THAT(hexB3.get(), Equals("1234"));
      REQUIRE_THROWS(Hex(std::move(std::string("01234")), true));
      REQUIRE_THROWS(Hex(std::move(std::string("x1234")), true));
      REQUIRE_THROWS(Hex(std::move(std::string("x1234")), false));
    }

    SECTION("fromBytes") {
      Bytes bytes{0x12, 0x34};
      Hex hex = Hex::fromBytes(bytes, false);
      Hex hexStrict = Hex::fromBytes(bytes, true);
      REQUIRE_THAT(hex.get(), Equals("1234"));
      REQUIRE_THAT(hexStrict.get(), Equals("0x1234"));
    }

    SECTION("fromUTF8") {
      std::string utf8 = "exemple";
      Hex hex = Hex::fromUTF8(utf8, false);
      Hex hexStrict = Hex::fromUTF8(utf8, true);
      REQUIRE_THAT(hex.get(), Equals("6578656d706c65"));
      REQUIRE_THAT(hexStrict.get(), Equals("0x6578656d706c65"));
    }

    SECTION("fromUint") {
      uint256_t value = 4660;
      Hex hex = Hex::fromUint(value, false);
      Hex hexStrict = Hex::fromUint(value, true);
      REQUIRE_THAT(hex.get(), Equals("1234"));
      REQUIRE_THAT(hexStrict.get(), Equals("0x1234"));
    }

    SECTION("isValid") {
      REQUIRE(Hex::isValid("0x1a2b3c4d5e6f7890", true));
      REQUIRE(Hex::isValid("0X1A2B3C4D5E6F7890", true));
      REQUIRE(Hex::isValid("1a2b3c4d5e6f7890", false));
      REQUIRE(!Hex::isValid("0x81684g837h3892j", true));
      REQUIRE(!Hex::isValid("81684g837h3892j", false));
      REQUIRE(!Hex::isValid("1a2b3c4d5e6f7890", true));
      REQUIRE(!Hex::isValid("0x1a2b3c4d5e6f7890", false));
    }

    SECTION("toBytes") {
      std::string_view hexStr("0x1234");
      std::string_view hexStr2("5678");
      Bytes bytesStr = Hex::toBytes(hexStr);
      Bytes bytesStr2 = Hex::toBytes(hexStr2);
      REQUIRE(Hex::fromBytes(bytesStr).get() == "1234");
      REQUIRE(Hex::fromBytes(bytesStr2).get() == "5678");
      REQUIRE_THROWS(Hex::toBytes(std::string_view("xyzw")));
      REQUIRE_THROWS(Hex::toBytes(std::string_view("0xghij")));
    }

    SECTION("bytes") {
      std::string hexStr = "0x1234";
      Hex hex(hexStr, false);
      Hex hexStrict(hexStr, true);
      REQUIRE(hex.bytes() == Bytes{0x12, 0x34});
      REQUIRE(hexStrict.bytes() == Bytes{0x12, 0x34});
      // Odd-numbered hex for coverage
      std::string hexStrOdd = "0x123";
      Hex hexOdd(hexStrOdd, false);
      Hex hexStrictOdd(hexStrOdd, true);
      REQUIRE(hexOdd.bytes() == Bytes{0x01, 0x23});
      REQUIRE(hexStrictOdd.bytes() == Bytes{0x01, 0x23});
    }

    SECTION("get") {
      std::string hexStr = "0x1234";
      Hex hex(hexStr, false);
      Hex hexStrict(hexStr, true);
      REQUIRE_THAT(hex.get(), Equals("1234"));
      REQUIRE_THAT(hexStrict.get(), Equals("0x1234"));
    }

    SECTION("getUint") {
      std::string hexStr = "0x1234";
      std::string oddHexStr = "0xfffff";
      std::string evenHexStr = "0x0fffff";
      std::string tooBigHexStr = "0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"; // 33 bytes
      Hex hex(hexStr, false);
      Hex hexStrict(hexStr, true);
      Hex oddHex(oddHexStr, true);
      Hex evenHex(evenHexStr, true);
      Hex tooBigHex(tooBigHexStr, true);
      REQUIRE(hex.getUint() == uint256_t(4660));
      REQUIRE(hexStrict.getUint() == uint256_t(4660));
      REQUIRE(oddHex.getUint() == uint256_t(1048575));
      REQUIRE(evenHex.getUint() == uint256_t(1048575));
      try {
        uint256_t wrongNumber = tooBigHex.getUint();
      } catch (std::length_error& e) {
        REQUIRE(e.what() == std::string("Hex too big for uint conversion"));
      }
    }

    SECTION("substr") {
      std::string hexStr = "0x1234";
      Hex hex(hexStr, false);
      Hex hexStrict(hexStr, true);
      REQUIRE_THAT(hex.substr(0, 2), Equals("12"));
      REQUIRE_THAT(hex.substr(2, 2), Equals("34"));
      REQUIRE_THAT(hexStrict.substr(0, 2), Equals("0x"));
      REQUIRE_THAT(hexStrict.substr(2, 2), Equals("12"));
      REQUIRE_THAT(hexStrict.substr(4, 2), Equals("34"));
    }

    // std::string() wrapper is required here because Catch2 can't discern overloaded types
    SECTION("substr_view") {
      std::string hexStr = "0x1234";
      Hex hex(hexStr, false);
      Hex hexStrict(hexStr, true);
      REQUIRE_THAT(std::string(hex.substr_view(0, 2)), Equals("12"));
      REQUIRE_THAT(std::string(hex.substr_view(2, 2)), Equals("34"));
      REQUIRE_THAT(std::string(hexStrict.substr_view(0, 2)), Equals("0x"));
      REQUIRE_THAT(std::string(hexStrict.substr_view(2, 2)), Equals("12"));
      REQUIRE_THAT(std::string(hexStrict.substr_view(4, 2)), Equals("34"));
    }

    SECTION("toInt") {
      // Yes I know this is overkill (it could be more tho) but you can never be too sure lol
      std::unordered_map<char, int> map = {
        {'0', 0}, {'1', 1}, {'2', 2}, {'3', 3}, {'4', 4}, {'5', 5}, {'6', 6},
        {'7', 7}, {'8', 8}, {'9', 9}, {'a', 10}, {'A', 10}, {'b', 11}, {'B', 11},
        {'c', 12}, {'C', 12}, {'d', 13}, {'D', 13}, {'e', 14}, {'E', 14},
        {'f', 15}, {'F', 15}, {'g', -1}, {'G', -1}, {'h', -1}, {'H', -1},
        {'!', -1}, {'@', -1}, {'#', -1}, {'$', -1}, {'%', -1}, {'&', -1},
        {'*', -1}, {'(', -1}, {')', -1}, {'-', -1}, {'+', -1}, {'_', -1},
        {'=', -1}, {',', -1}, {'.', -1}, {'<', -1}, {'>', -1}, {';', -1},
        {':', -1}, {'/', -1}, {'?', -1}, {'~', -1}, {'^', -1}, {'[', -1},
        {']', -1}, {'{', -1}, {'}', -1}, {'"', -1}, {'|', -1}
      };
      for (std::pair<char, int> p : map) REQUIRE(Hex::toInt(p.first) == p.second);
    }

    SECTION("forRPC") {
      Hex h1(std::string("0x41"), true);
      Hex h2(std::string("0x400"), true);
      Hex h3(std::string("0x"), true);
      Hex h4(std::string("0x0400"), true);
      Hex h5(std::string("ff"), false);
      Hex h6(std::string("0x00007a6f00"), true);
      REQUIRE_THAT(h1.forRPC(), Equals("0x41"));
      REQUIRE_THAT(h2.forRPC(), Equals("0x400"));
      REQUIRE_THAT(h3.forRPC(), Equals("0x0"));
      REQUIRE_THAT(h4.forRPC(), Equals("0x400"));
      REQUIRE_THAT(h5.forRPC(), Equals("0xff"));
      REQUIRE_THAT(h6.forRPC(), Equals("0x7a6f00"));
    }

    SECTION("operator+= (std::string)") {
      std::string hexStr = "0x1234";
      Hex hex(hexStr, false);
      Hex hexStrict(hexStr, true);
      hex += std::string("0x5678");
      hexStrict += std::string("0x5678");
      REQUIRE_THAT(hex.get(), Equals("12345678"));
      REQUIRE_THAT(hexStrict.get(), Equals("0x12345678"));

      bool catched = false;
      try { hex += std::string("xyzw"); } catch (std::exception &e) { catched = true; }
      REQUIRE(catched == true);
    }

    SECTION("operator+= (Hex)") {
      std::string hexStr = "0x1234";
      Hex hex(hexStr, false);
      Hex hexStrict(hexStr, true);
      hex += Hex(std::move(std::string("5678")), false);
      hexStrict += Hex(std::move(std::string("0x5678")), true);
      REQUIRE_THAT(hex.get(), Equals("12345678"));
      REQUIRE_THAT(hexStrict.get(), Equals("0x12345678"));
    }

    SECTION("operator<<") {
      std::string hexStr = "0x1234";
      Hex hex(hexStr, false);
      Hex hexStrict(hexStr, true);
      std::stringstream ss;
      std::stringstream ssStrict;
      ss << hex;
      ssStrict << hexStrict;
      REQUIRE_THAT(ss.str(), Equals("1234"));
      REQUIRE_THAT(ssStrict.str(), Equals("0x1234"));
    }
  }
}

