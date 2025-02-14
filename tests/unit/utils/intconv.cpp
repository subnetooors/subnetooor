/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#include "libs/catch2/catch_amalgamated.hpp"

#include "utils/intconv.h"
#include "utils/utils.h" // strings.h -> hex.h

using Catch::Matchers::Equals;

namespace TUtils {
  TEST_CASE("IntConv Namespace", "[unit][utils][intconv]") {
    SECTION("int256ToBytes") {
      // Positive int
      int256_t int256Input = int256_t("23961171024934392974276419658924811645511170606486123646020719529926645772697");
      auto int256Output = IntConv::int256ToBytes(int256Input);
      BytesArr<32> int256ExpectedOutput = BytesArr<32> {0x34, 0xf9, 0x8a, 0xcd, 0x6f, 0x00, 0x53, 0xe9, 0x8d, 0xfa, 0x2f, 0x0a, 0xc4, 0x9b, 0x53, 0x02, 0x7f, 0x41, 0xee, 0x12, 0x44, 0xd9, 0x5d, 0xdb, 0x41, 0x26, 0xdc, 0x65, 0x19, 0x8b, 0xf1, 0x99};
      REQUIRE(int256Output == int256ExpectedOutput);

      // Negative int
      int256_t int256Input2 = int256_t("-23961171024934392974276419658924811645511170606486123646020719529926645772697");
      auto int256Output2 = IntConv::int256ToBytes(int256Input2);
      BytesArr<32> int256ExpectedOutput2 = BytesArr<32> {0xcb, 0x06, 0x75, 0x32, 0x90, 0xff, 0xac, 0x16, 0x72, 0x05, 0xd0, 0xf5, 0x3b, 0x64, 0xac, 0xfd, 0x80, 0xbe, 0x11, 0xed, 0xbb, 0x26, 0xa2, 0x24, 0xbe, 0xd9, 0x23, 0x9a, 0xe6, 0x74, 0x0e, 0x67};
      REQUIRE(int256Output2 == int256ExpectedOutput2);
    }

    SECTION("int136ToBytes") {
      int136_t int136Input = int136_t("87112285131760246616623899502532662132135");
      auto int136Output = IntConv::int136ToBytes(int136Input);
      BytesArr<17> int136ExpectedOutput = BytesArr<17> {0xff, 0xff, 0xff, 0xd8, 0x8e, 0x94, 0x95, 0xee, 0xc9, 0x84, 0xf6, 0x26, 0xc7, 0xe9, 0x3f, 0xfd, 0xa7};
      REQUIRE(int136Output == int136ExpectedOutput);
    }

    SECTION("int64ToBytes") {
      int64_t int64Input = int64_t(1155010102558518614LL);
      auto int64Output = IntConv::int64ToBytes(int64Input);
      BytesArr<8> int64ExpectedOutput = BytesArr<8> {0x10, 0x07, 0x6b, 0x91, 0x9a, 0xfc, 0xed, 0x56};
      REQUIRE(int64Output == int64ExpectedOutput);
    }

    SECTION("bytesToInt256") {
      FixedBytes<32> bytesStr(bytes::view("\xcb\x06\x75\x32\x90\xff\xac\x16\x72\x05\xd0\xf5\x3b\x64\xac\xfd\x80\xbe\x11\xed\xbb\x26\xa2\x24\xbe\xd9\x23\x9a\xe6\x74\x0e\x67"));
      auto int256Output = IntConv::bytesToInt256(bytesStr);
      int256_t int256ExpectedOutput = int256_t("-23961171024934392974276419658924811645511170606486123646020719529926645772697");
      REQUIRE(int256Output == int256ExpectedOutput);

      bool catchLo = false;
      bool catchHi = false;
      Bytes loStr = Hex::toBytes("0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
      Bytes hiStr = Hex::toBytes("0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
      try { IntConv::bytesToInt256(loStr); } catch (std::exception &e) { catchLo = true; }
      try { IntConv::bytesToInt256(hiStr); } catch (std::exception &e) { catchHi = true; }
      REQUIRE(catchLo == true);
      REQUIRE(catchHi == true);
    }

    SECTION("bytesToInt136") {
      BytesArr<17> bytesArr = BytesArr<17> {0xff, 0xff, 0xff, 0xd8, 0x8e, 0x94, 0x95, 0xee, 0xc9, 0x84, 0xf6, 0x26, 0xc7, 0xe9, 0x3f, 0xfd, 0xa7};
      auto int136Output = IntConv::bytesToInt136(bytesArr);
      int136_t int136ExpectedOutput = int136_t("87112285131760246616623899502532662132135");
      REQUIRE(int136Output == int136ExpectedOutput);

      bool catchLo = false;
      bool catchHi = false;
      Bytes loStr = Hex::toBytes("0xffffffffffffffffffffffffffffffffffffffffffffffff");
      Bytes hiStr = Hex::toBytes("0xfffffffffffffffffffffffffffffffffffffffffffffffff");
      try { IntConv::bytesToInt136(loStr); } catch (std::exception &e) { catchLo = true; }
      try { IntConv::bytesToInt136(hiStr); } catch (std::exception &e) { catchHi = true; }
      REQUIRE(catchLo == true);
      REQUIRE(catchHi == true);
    }

    SECTION("bytesToInt64") {
      FixedBytes<8> bytesStr(bytes::view("\x10\x07\x6b\x91\x9a\xfc\xed\x56"));
      auto int64Output = IntConv::bytesToInt64(bytesStr);
      int64_t int64ExpectedOutput = int64_t(1155010102558518614ULL);
      REQUIRE(int64Output == int64ExpectedOutput);

      bool catchLo = false;
      bool catchHi = false;
      Bytes loStr = Hex::toBytes("0xffffffffffffff");
      Bytes hiStr = Hex::toBytes("0xffffffffffffffffff");
      try { IntConv::bytesToInt64(loStr); } catch (std::exception &e) { catchLo = true; }
      try { IntConv::bytesToInt64(hiStr); } catch (std::exception &e) { catchHi = true; }
      REQUIRE(catchLo == true);
      REQUIRE(catchHi == true);
    }
  }
}

