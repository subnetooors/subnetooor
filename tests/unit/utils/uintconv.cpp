/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#include "libs/catch2/catch_amalgamated.hpp"

#include "utils/uintconv.h"
#include "utils/utils.h" // strings.h -> hex.h

using Catch::Matchers::Equals;

namespace TUtils {
  TEST_CASE("UintConv Namespace", "[unit][utils][uintconv]") {
    SECTION("uint256ToBytes") {
      uint256_t uint256Input = uint256_t("91830918212381802449294565349763096207758814059154440393436864477986483867239");
      auto uint256Output = UintConv::uint256ToBytes(uint256Input);
      BytesArr<32> uint256ExpectedOutput = BytesArr<32> {0xcb, 0x06, 0x75, 0x32, 0x90, 0xff, 0xac, 0x16, 0x72, 0x05, 0xd0, 0xf5, 0x3b, 0x64, 0xac, 0xfd, 0x80, 0xbe, 0x11, 0xed, 0xbb, 0x26, 0xa2, 0x24, 0xbe, 0xd9, 0x23, 0x9a, 0xe6, 0x74, 0x0e, 0x67};
      REQUIRE(uint256Output == uint256ExpectedOutput);
    }

    SECTION("uint248ToBytes") {
      uint248_t uint248Input = uint248_t("452312848503266318373324160190117140051835877600158453279131187530910662650");
      auto uint248Output = UintConv::uint248ToBytes(uint248Input);
      BytesArr<31> uint248ExpectedOutput = BytesArr<31> {0xff, 0xff, 0xff, 0xff, 0x3d, 0x87, 0xd5, 0x8f, 0x2e, 0x16, 0xa4, 0x50, 0x99, 0xdf, 0xde, 0x83, 0xe8, 0x45, 0xd0, 0x13, 0x5e, 0x48, 0x6c, 0xf6, 0x6b, 0xa7, 0xff, 0xff, 0xff, 0xff, 0xfa};
      REQUIRE(uint248Output == uint248ExpectedOutput);
    }

    SECTION("uint240ToBytes") {
      uint240_t uint240Input = uint240_t("1766847064778384329583297500742918515827483896875618958121606201292619770");
      auto uint240Output = UintConv::uint240ToBytes(uint240Input);
      BytesArr<30> uint240ExpectedOutput = BytesArr<30>{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfa};
      REQUIRE(uint240Output == uint240ExpectedOutput);
    }

    SECTION("uint232ToBytes") {
      uint232_t uint232Input = uint232_t("6901746346790563717434755872277015452451108972170386555162524123799295");
      auto uint232Output = UintConv::uint232ToBytes(uint232Input);
      BytesArr<29> uint232ExpectedOutput = BytesArr<29>{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x44, 0xe8, 0x0e, 0x70, 0x5c, 0x5f, 0x03, 0xf7, 0x10, 0xc1, 0x0f, 0x75, 0x8a, 0x39, 0x68, 0x1e, 0xb1, 0x5f, 0xfa, 0x0a, 0x1e, 0xff};
      REQUIRE(uint232Output == uint232ExpectedOutput);
    }

    SECTION("uint224ToBytes") {
      uint224_t uint224Input = uint224_t("26959946667150639194667015087019630673637144422540512481103110219215");
      auto uint224Output = UintConv::uint224ToBytes(uint224Input);
      BytesArr<28> uint224ExpectedOutput = BytesArr<28> {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x65, 0x76, 0x9d, 0x7c, 0x26, 0x91, 0x98, 0xd1, 0x0f, 0x67, 0xe0, 0xc2, 0x0d, 0xbe, 0xd2, 0xd6, 0x51, 0x43, 0xac, 0x25, 0xcf};
      REQUIRE(uint224Output == uint224ExpectedOutput);
    }

    SECTION("uint216ToBytes") {
      uint216_t uint216Input = uint216_t("105312291618557186697918027613670432318815095400549111254310917535");
      auto uint216Output = UintConv::uint216ToBytes(uint216Input);
      BytesArr<27> uint216ExpectedOutput = BytesArr<27> {0xff, 0xff, 0xff, 0xfd, 0xf5, 0xf9, 0xc3, 0xb5, 0xf8, 0x4a, 0xed, 0x4c, 0x56, 0x99, 0xc5, 0xec, 0xdb, 0xbe, 0xe1, 0xb8, 0x32, 0x77, 0x5d, 0xaf, 0xff, 0x15, 0x9f};
      REQUIRE(uint216Output == uint216ExpectedOutput);
    }

    SECTION("uint208ToBytes") {
      uint208_t uint208Input = uint208_t("411176139330301510518742295639337626245613966408394965837151255");
      auto uint208Output = UintConv::uint208ToBytes(uint208Input);
      BytesArr<26> uint208ExpectedOutput = BytesArr<26> {0xff, 0xe0, 0x23, 0x5e, 0x91, 0xfb, 0x47, 0x91, 0xd9, 0x68, 0xea, 0x08, 0x5f, 0xb3, 0x4e, 0x6b, 0xcc, 0xfe, 0xad, 0xd4, 0xab, 0xe5, 0x20, 0x3f, 0xfc, 0x17};
      REQUIRE(uint208Output == uint208ExpectedOutput);
    }

    SECTION("uint200ToBytes") {
      uint200_t uint200Input = uint200_t("1606931844258990275541962019341162602522202913782792831301375");
      auto uint200Output = UintConv::uint200ToBytes(uint200Input);
      BytesArr<25> uint200ExpectedOutput = BytesArr<25> {0xff, 0xff, 0xbf, 0x44, 0xdb, 0xda, 0x25, 0xf4, 0xcf, 0xa0, 0x53, 0x3c, 0x00, 0xc9, 0x61, 0xdb, 0xfa, 0x45, 0x00, 0xd2, 0x92, 0x81, 0xba, 0xf6, 0xff};
      REQUIRE(uint200Output == uint200ExpectedOutput);
    }

    SECTION("uint192ToBytes") {
      uint192_t uint192Input = uint192_t("6277101735386680163835789423207166416102355144464034112895");
      auto uint192Output = UintConv::uint192ToBytes(uint192Input);
      BytesArr<24> uint192ExpectedOutput = BytesArr<24> {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf9, 0x1c, 0xc2, 0x2b, 0x56, 0x36, 0x47, 0xc5, 0x7c, 0xa2, 0x1a, 0x93, 0xf3, 0x08, 0x5a, 0x8b, 0x25, 0x7f};
      REQUIRE(uint192Output == uint192ExpectedOutput);
    }

    SECTION("uint184ToBytes") {
      uint184_t uint184Input = uint184_t("24519128653854221731733552434404946137899825954931634815");
      auto uint184Output = UintConv::uint184ToBytes(uint184Input);
      BytesArr<23> uint184ExpectedOutput = BytesArr<23> {0xff, 0xfd, 0xdc, 0x9e, 0x27, 0x50, 0x33, 0x6b, 0x4a, 0x91, 0xcc, 0xe8, 0x44, 0x49, 0xbb, 0x90, 0x0d, 0xd0, 0x74, 0xe7, 0x24, 0x72, 0x7f};
      REQUIRE(uint184Output == uint184ExpectedOutput);
    }

    SECTION("uint176ToBytes") {
      uint176_t uint176Input = uint176_t("95180971304118053147396689196894323971171195136471135");
      auto uint176Output = UintConv::uint176ToBytes(uint176Input);
      BytesArr<22> uint176ExpectedOutput = BytesArr<22> {0xfe, 0x65, 0x76, 0x9d, 0x7c, 0x26, 0x91, 0x38, 0x85, 0x28, 0x29, 0xfc, 0x3e, 0x82, 0xe5, 0xf0, 0x6c, 0xfe, 0xc8, 0x1f, 0x70, 0x5f};
      REQUIRE(uint176Output == uint176ExpectedOutput);
    }

    SECTION("uint168ToBytes") {
      uint168_t uint168Input = uint168_t("374114419156711147010143317175368451031918731001155");
      auto uint168Output = UintConv::uint168ToBytes(uint168Input);
      BytesArr<21> uint168ExpectedOutput = BytesArr<21> {0xff, 0xfa, 0xbe, 0xc0, 0xe3, 0xb0, 0x36, 0x48, 0x66, 0x50, 0x4b, 0x1c, 0x90, 0xef, 0x13, 0xad, 0x42, 0x36, 0x72, 0xfd, 0x43};
      REQUIRE(uint168Output == uint168ExpectedOutput);
    }

    SECTION("uint160ToBytes") {
      uint160_t uint160Input = uint160_t("506797479317435130489084083375319966488594602593");
      auto uint160Output = UintConv::uint160ToBytes(uint160Input);
      BytesArr<20> uint160ExpectedOutput = BytesArr<20> {0x58, 0xc5, 0x95, 0xbe, 0xdf, 0x1d, 0xea, 0x53, 0x2c, 0xf0, 0x6a, 0xf9, 0x09, 0x1a, 0x51, 0xb7, 0x5a, 0x11, 0xda, 0x61};
      REQUIRE(uint160Output == uint160ExpectedOutput);
    }

    SECTION("uint152ToBytes") {
      uint152_t uint152Input = uint152_t("5701991770823839524233143877797980545130986491");
      auto uint152Output = UintConv::uint152ToBytes(uint152Input);
      BytesArr<19> uint152ExpectedOutput = BytesArr<19> {0xff, 0xaf, 0xa7, 0xc9, 0x9f, 0xec, 0x62, 0x36, 0xc7, 0x93, 0x33, 0xe4, 0x70, 0x1a, 0x7f, 0xe8, 0x28, 0x7b, 0xfb};
      REQUIRE(uint152Output == uint152ExpectedOutput);
    }

    SECTION("uint144ToBytes") {
      uint144_t uint144Input = uint144_t("22300745118530623141515718272648361505980415");
      auto uint144Output = UintConv::uint144ToBytes(uint144Input);
      BytesArr<18> uint144ExpectedOutput = BytesArr<18> {0xff, 0xff, 0xff, 0xf0, 0x97, 0xb2, 0x0a, 0x93, 0xbd, 0xc2, 0x0f, 0xd1, 0xf9, 0x6a, 0x9b, 0x7f, 0xff, 0xff};
      REQUIRE(uint144Output == uint144ExpectedOutput);
    }

    SECTION("uint136ToBytes") {
      uint136_t uint136Input = uint136_t("87112285131760246616623899502532662132135");
      auto uint136Output = UintConv::uint136ToBytes(uint136Input);
      BytesArr<17> uint136ExpectedOutput = BytesArr<17> {0xff, 0xff, 0xff, 0xd8, 0x8e, 0x94, 0x95, 0xee, 0xc9, 0x84, 0xf6, 0x26, 0xc7, 0xe9, 0x3f, 0xfd, 0xa7};
      REQUIRE(uint136Output == uint136ExpectedOutput);
    }

    SECTION("uint128ToBytes") {
      uint128_t uint128Input = uint128_t("340282366920938463463374607431768211401");
      auto uint128Output = UintConv::uint128ToBytes(uint128Input);
      BytesArr<16> uint128ExpectedOutput = BytesArr<16> {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc9 };
      REQUIRE(uint128Output == uint128ExpectedOutput);
    }

    SECTION("uint120ToBytes") {
      uint120_t uint120Input = uint120_t("1329227915784915812903807060281344575");
      auto uint120Output = UintConv::uint120ToBytes(uint120Input);
      BytesArr<15> uint120ExpectedOutput = BytesArr<15> {0xff, 0xff, 0xfe, 0xfd, 0x81, 0x8d, 0x0a, 0xce, 0x2d, 0x1a, 0xdc, 0x44, 0x9f, 0x42, 0x3f};
      REQUIRE(uint120Output == uint120ExpectedOutput);
    }

    SECTION("uint112ToBytes") {
      uint112_t uint112Input = uint112_t("5191296858514827628530496129220091");
      auto uint112Output = UintConv::uint112ToBytes(uint112Input);
      BytesArr<14> uint112ExpectedOutput = BytesArr<14> {0xff, 0xf3, 0x60, 0xd3, 0x5e, 0xf3, 0x85, 0xc9, 0x7e, 0xa0, 0x4f, 0x94, 0x3d, 0xfb};
      REQUIRE(uint112Output == uint112ExpectedOutput);
    }

    SECTION("uint104ToBytes") {
      uint104_t uint104Input = uint104_t("20212409603611670423941251286011");
      auto uint104Output = UintConv::uint104ToBytes(uint104Input);
      BytesArr<13> uint104ExpectedOutput = BytesArr<13> {0xff, 0x1d, 0xd1, 0x5b, 0x6a, 0x21, 0xe0, 0x63, 0x45, 0x02, 0xbf, 0x43, 0xfb};
      REQUIRE(uint104Output == uint104ExpectedOutput);
    }

    SECTION("uint96ToBytes") {
      uint96_t uint96Input = uint96_t("79128162514264331593543951331");
      auto uint96Output = UintConv::uint96ToBytes(uint96Input);
      BytesArr<12> uint96ExpectedOutput = BytesArr<12> {0xff, 0xad, 0x48, 0x2d, 0x23, 0x37, 0xed, 0xb8, 0x20, 0x21, 0xa3, 0xe3};
      REQUIRE(uint96Output == uint96ExpectedOutput);
    }

    SECTION("uint88ToBytes") {
      uint88_t uint88Input = uint88_t("309185109821345061724781051");
      auto uint88Output = UintConv::uint88ToBytes(uint88Input);
      BytesArr<11> uint88ExpectedOutput = BytesArr<11> {0xff, 0xc0, 0x7e, 0x63, 0x6f, 0xba, 0x4a, 0xde, 0x54, 0x79, 0xfb};
      REQUIRE(uint88Output == uint88ExpectedOutput);
    }

    SECTION("uint80ToBytes") {
      uint80_t uint80Input = uint80_t("1208925819614629174106171");
      auto uint80Output = UintConv::uint80ToBytes(uint80Input);
      BytesArr<10> uint80ExpectedOutput = BytesArr<10> {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf6, 0xd8, 0x3b};
      REQUIRE(uint80Output == uint80ExpectedOutput);
    }

    SECTION("uint72ToBytes") {
      uint72_t uint72Input = uint72_t("1722366182819645213691");
      auto uint72Output = UintConv::uint72ToBytes(uint72Input);
      BytesArr<9> uint72ExpectedOutput = BytesArr<9> {0x5d, 0x5e, 0xa1, 0xe5, 0xc9, 0x8e, 0x75, 0xcb, 0xfb};
      REQUIRE(uint72Output == uint72ExpectedOutput);
    }

    SECTION("uint64ToBytes") {
      uint64_t uint64Input = uint64_t(11155010102558518614ULL);
      auto uint64Output = UintConv::uint64ToBytes(uint64Input);
      BytesArr<8> uint64ExpectedOutput = BytesArr<8> {0x9a, 0xce, 0x8e, 0x96, 0x24, 0xe4, 0xed, 0x56};
      REQUIRE(uint64Output == uint64ExpectedOutput);
    }

    SECTION("uint56ToBytes") {
      uint56_t uint56Input = uint56_t(72051594031927931);
      auto uint56Output = UintConv::uint56ToBytes(uint56Input);
      BytesArr<7> uint56ExpectedOutput = BytesArr<7> {0xff, 0xfa, 0x8b, 0x03, 0xc6, 0x12, 0x7b};
      REQUIRE(uint56Output == uint56ExpectedOutput);
    }

    SECTION("uint48ToBytes") {
      uint48_t uint48Input = uint48_t(281414176110651);
      auto uint48Output = UintConv::uint48ToBytes(uint48Input);
      BytesArr<6> uint48ExpectedOutput = BytesArr<6> {0xff, 0xf1, 0xd8, 0x00, 0x78, 0x3b};
      REQUIRE(uint48Output == uint48ExpectedOutput);
    }

    SECTION("uint40ToBytes") {
      uint40_t uint40Input = uint40_t(1019511621771);
      auto uint40Output = UintConv::uint40ToBytes(uint40Input);
      BytesArr<5> uint40ExpectedOutput = BytesArr<5> {0xed, 0x5f, 0xa0, 0xc8, 0x8b};
      REQUIRE(uint40Output == uint40ExpectedOutput);
    }

    SECTION("uint32ToBytes") {
      uint32_t uint32Input = 2004601498;
      auto uint32Output = UintConv::uint32ToBytes(uint32Input);
      BytesArr<4> uint32ExpectedOutput = BytesArr<4> {0x77, 0x7b, 0xca, 0x9a};
      REQUIRE(uint32Output == uint32ExpectedOutput);
    }

    SECTION("uint24ToBytes") {
      uint24_t uint24Input = 16117211;
      auto uint24Output = UintConv::uint24ToBytes(uint24Input);
      BytesArr<3> uint24ExpectedOutput = BytesArr<3> {0xf5, 0xed, 0xdb};
      REQUIRE(uint24Output == uint24ExpectedOutput);
    }

    SECTION("uint16ToBytes") {
      uint16_t uint16Input = 65452;
      auto uint16Output = UintConv::uint16ToBytes(uint16Input);
      BytesArr<2> uint16ExpectedOutput = BytesArr<2> {0xff, 0xac};
      REQUIRE(uint16Output == uint16ExpectedOutput);
    }

    SECTION("uint8ToBytes") {
      uint8_t uint8Input = 120;
      auto uint8Output = UintConv::uint8ToBytes(uint8Input);
      BytesArr<1> uint8ExpectedOutput = BytesArr<1> {0x78};
      REQUIRE(uint8Output == uint8ExpectedOutput);
    }

    SECTION("bytesToUint256") {
      FixedBytes<32> bytesStr(bytes::view("\xcb\x06\x75\x32\x90\xff\xac\x16\x72\x05\xd0\xf5\x3b\x64\xac\xfd\x80\xbe\x11\xed\xbb\x26\xa2\x24\xbe\xd9\x23\x9a\xe6\x74\x0e\x67"));
      auto uint256Output = UintConv::bytesToUint256(bytesStr);
      uint256_t uint256ExpectedOutput = uint256_t("91830918212381802449294565349763096207758814059154440393436864477986483867239");
      REQUIRE(uint256Output == uint256ExpectedOutput);

      bool catchLo = false;
      bool catchHi = false;
      Bytes loStr = Hex::toBytes("0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
      Bytes hiStr = Hex::toBytes("0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
      try { UintConv::bytesToUint256(loStr); } catch (std::exception &e) { catchLo = true; }
      try { UintConv::bytesToUint256(hiStr); } catch (std::exception &e) { catchHi = true; }
      REQUIRE(catchLo == true);
      REQUIRE(catchHi == true);
    }

    SECTION("bytesToUint248") {
      BytesArr<31> bytesArr = BytesArr<31> {0xff, 0xff, 0xff, 0xff, 0x3d, 0x87, 0xd5, 0x8f, 0x2e, 0x16, 0xa4, 0x50, 0x99, 0xdf, 0xde, 0x83, 0xe8, 0x45, 0xd0, 0x13, 0x5e, 0x48, 0x6c, 0xf6, 0x6b, 0xa7, 0xff, 0xff, 0xff, 0xff, 0xfa};
      auto uint248Output = UintConv::bytesToUint248(bytesArr);
      uint248_t uint248ExpectedOutput = uint248_t("452312848503266318373324160190117140051835877600158453279131187530910662650");
      REQUIRE(uint248Output == uint248ExpectedOutput);

      bool catchLo = false;
      bool catchHi = false;
      Bytes loStr = Hex::toBytes("0xfffffffffffffffffffffffffffffffffffffffffffffffffffff");
      Bytes hiStr = Hex::toBytes("0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
      try { UintConv::bytesToUint248(loStr); } catch (std::exception &e) { catchLo = true; }
      try { UintConv::bytesToUint248(hiStr); } catch (std::exception &e) { catchHi = true; }
      REQUIRE(catchLo == true);
      REQUIRE(catchHi == true);
    }

    SECTION("bytesToUint240") {
      BytesArr<30> bytesArr = BytesArr<30>{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfa};
      auto uint240Output = UintConv::bytesToUint240(bytesArr);
      uint240_t uint240ExpectedOutput = uint240_t("1766847064778384329583297500742918515827483896875618958121606201292619770");
      REQUIRE(uint240Output == uint240ExpectedOutput);

      bool catchLo = false;
      bool catchHi = false;
      Bytes loStr = Hex::toBytes("0xffffffffffffffffffffffffffffffffffffffffffffffffffff");
      Bytes hiStr = Hex::toBytes("0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
      try { UintConv::bytesToUint240(loStr); } catch (std::exception &e) { catchLo = true; }
      try { UintConv::bytesToUint240(hiStr); } catch (std::exception &e) { catchHi = true; }
      REQUIRE(catchLo == true);
      REQUIRE(catchHi == true);
    }

    SECTION("bytesToUint232") {
      BytesArr<29> bytesArr = BytesArr<29>{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x44, 0xe8, 0x0e, 0x70, 0x5c, 0x5f, 0x03, 0xf7, 0x10, 0xc1, 0x0f, 0x75, 0x8a, 0x39, 0x68, 0x1e, 0xb1, 0x5f, 0xfa, 0x0a, 0x1e, 0xff};
      auto uint232Output = UintConv::bytesToUint232(bytesArr);
      uint232_t uint232ExpectedOutput = uint232_t("6901746346790563717434755872277015452451108972170386555162524123799295");
      REQUIRE(uint232Output == uint232ExpectedOutput);

      bool catchLo = false;
      bool catchHi = false;
      Bytes loStr = Hex::toBytes("0xffffffffffffffffffffffffffffffffffffffffffffffffff");
      Bytes hiStr = Hex::toBytes("0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
      try { UintConv::bytesToUint232(loStr); } catch (std::exception &e) { catchLo = true; }
      try { UintConv::bytesToUint232(hiStr); } catch (std::exception &e) { catchHi = true; }
      REQUIRE(catchLo == true);
      REQUIRE(catchHi == true);

    }

    SECTION("bytesToUint224") {
      BytesArr<28> bytesArr = BytesArr<28> {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x65, 0x76, 0x9d, 0x7c, 0x26, 0x91, 0x98, 0xd1, 0x0f, 0x67, 0xe0, 0xc2, 0x0d, 0xbe, 0xd2, 0xd6, 0x51, 0x43, 0xac, 0x25, 0xcf};
      auto uint224Output = UintConv::bytesToUint224(bytesArr);
      uint224_t uint224ExpectedOutput = uint224_t("26959946667150639194667015087019630673637144422540512481103110219215");
      REQUIRE(uint224Output == uint224ExpectedOutput);

      bool catchLo = false;
      bool catchHi = false;
      Bytes loStr = Hex::toBytes("0xffffffffffffffffffffffffffffffffffffffffffffffffff");
      Bytes hiStr = Hex::toBytes("0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
      try { UintConv::bytesToUint224(loStr); } catch (std::exception &e) { catchLo = true; }
      try { UintConv::bytesToUint224(hiStr); } catch (std::exception &e) { catchHi = true; }
      REQUIRE(catchLo == true);
      REQUIRE(catchHi == true);

    }

    SECTION("bytesToUint216") {
      BytesArr<27> bytesArr = BytesArr<27> {0xff, 0xff, 0xff, 0xfd, 0xf5, 0xf9, 0xc3, 0xb5, 0xf8, 0x4a, 0xed, 0x4c, 0x56, 0x99, 0xc5, 0xec, 0xdb, 0xbe, 0xe1, 0xb8, 0x32, 0x77, 0x5d, 0xaf, 0xff, 0x15, 0x9f};
      auto uint216Output = UintConv::bytesToUint216(bytesArr);
      uint216_t uint216ExpectedOutput = uint216_t("105312291618557186697918027613670432318815095400549111254310917535");
      REQUIRE(uint216Output == uint216ExpectedOutput);

      bool catchLo = false;
      bool catchHi = false;
      Bytes loStr = Hex::toBytes("0xffffffffffffffffffffffffffffffffffffffffffffffffff");
      Bytes hiStr = Hex::toBytes("0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
      try { UintConv::bytesToUint216(loStr); } catch (std::exception &e) { catchLo = true; }
      try { UintConv::bytesToUint216(hiStr); } catch (std::exception &e) { catchHi = true; }
      REQUIRE(catchLo == true);
      REQUIRE(catchHi == true);

    }

    SECTION("bytesToUint208") {
      BytesArr<26> bytesArr = BytesArr<26> {0xff, 0xe0, 0x23, 0x5e, 0x91, 0xfb, 0x47, 0x91, 0xd9, 0x68, 0xea, 0x08, 0x5f, 0xb3, 0x4e, 0x6b, 0xcc, 0xfe, 0xad, 0xd4, 0xab, 0xe5, 0x20, 0x3f, 0xfc, 0x17};
      auto uint208Output = UintConv::bytesToUint208(bytesArr);
      uint208_t uint208ExpectedOutput = uint208_t("411176139330301510518742295639337626245613966408394965837151255");
      REQUIRE(uint208Output == uint208ExpectedOutput);

      bool catchLo = false;
      bool catchHi = false;
      Bytes loStr = Hex::toBytes("0xffffffffffffffffffffffffffffffffffffffffffffffffff");
      Bytes hiStr = Hex::toBytes("0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
      try { UintConv::bytesToUint208(loStr); } catch (std::exception &e) { catchLo = true; }
      try { UintConv::bytesToUint208(hiStr); } catch (std::exception &e) { catchHi = true; }
      REQUIRE(catchLo == true);
      REQUIRE(catchHi == true);

    }

    SECTION("bytesToUint200") {
      BytesArr<25> bytesArr = BytesArr<25> {0xff, 0xff, 0xbf, 0x44, 0xdb, 0xda, 0x25, 0xf4, 0xcf, 0xa0, 0x53, 0x3c, 0x00, 0xc9, 0x61, 0xdb, 0xfa, 0x45, 0x00, 0xd2, 0x92, 0x81, 0xba, 0xf6, 0xff};
      auto uint200Output = UintConv::bytesToUint200(bytesArr);
      uint200_t uint200ExpectedOutput = uint200_t("1606931844258990275541962019341162602522202913782792831301375");
      REQUIRE(uint200Output == uint200ExpectedOutput);

      bool catchLo = false;
      bool catchHi = false;
      Bytes loStr = Hex::toBytes("0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
      Bytes hiStr = Hex::toBytes("0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
      try { UintConv::bytesToUint200(loStr); } catch (std::exception &e) { catchLo = true; }
      try { UintConv::bytesToUint200(hiStr); } catch (std::exception &e) { catchHi = true; }
      REQUIRE(catchLo == true);
      REQUIRE(catchHi == true);

    }

    SECTION("bytesToUint192") {
      std::vector<int> vec;
      FixedBytes<24> bytesStr = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf9, 0x1c, 0xc2, 0x2b, 0x56, 0x36, 0x47, 0xc5, 0x7c, 0xa2, 0x1a, 0x93, 0xf3, 0x08, 0x5a, 0x8b, 0x25, 0x7f};
      auto uint192Output = UintConv::bytesToUint192(bytesStr);
      uint192_t uint192ExpectedOutput = uint192_t("6277101735386680163835789423207166416102355144464034112895");
      REQUIRE(uint192Output == uint192ExpectedOutput);

      bool catchLo = false;
      bool catchHi = false;
      Bytes loStr = Hex::toBytes("0xffffffffffffffffffffffffffffffffffffffffffffffffff");
      Bytes hiStr = Hex::toBytes("0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
      try { UintConv::bytesToUint192(loStr); } catch (std::exception &e) { catchLo = true; }
      try { UintConv::bytesToUint192(hiStr); } catch (std::exception &e) { catchHi = true; }
      REQUIRE(catchLo == true);
      REQUIRE(catchHi == true);

    }

    SECTION("bytesToUint184") {
      BytesArr<23> bytesArr = BytesArr<23> {0xff, 0xfd, 0xdc, 0x9e, 0x27, 0x50, 0x33, 0x6b, 0x4a, 0x91, 0xcc, 0xe8, 0x44, 0x49, 0xbb, 0x90, 0x0d, 0xd0, 0x74, 0xe7, 0x24, 0x72, 0x7f};
      auto uint184Output = UintConv::bytesToUint184(bytesArr);
      uint184_t uint184ExpectedOutput = uint184_t("24519128653854221731733552434404946137899825954931634815");
      REQUIRE(uint184Output == uint184ExpectedOutput);

      bool catchLo = false;
      bool catchHi = false;
      Bytes loStr = Hex::toBytes("0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
      Bytes hiStr = Hex::toBytes("0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
      try { UintConv::bytesToUint184(loStr); } catch (std::exception &e) { catchLo = true; }
      try { UintConv::bytesToUint184(hiStr); } catch (std::exception &e) { catchHi = true; }
      REQUIRE(catchLo == true);
      REQUIRE(catchHi == true);

    }

    SECTION("bytesToUint176") {
      BytesArr<22> bytesArr = BytesArr<22> {0xff, 0xff, 0xff, 0xff, 0x3d, 0x87, 0xd5, 0x8f, 0x2e, 0x16, 0xa4, 0x50, 0x99, 0xdf, 0xde, 0x83, 0xe8, 0x45, 0xd0, 0x13, 0x5e, 0x48};
      auto uint176Output = UintConv::bytesToUint176(bytesArr);
      uint176_t uint176ExpectedOutput = uint176_t("95780971287177379879234105465257117190357643745779272");
      REQUIRE(uint176Output == uint176ExpectedOutput);

      bool catchLo = false;
      bool catchHi = false;
      Bytes loStr = Hex::toBytes("0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
      Bytes hiStr = Hex::toBytes("0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
      try { UintConv::bytesToUint176(loStr); } catch (std::exception &e) { catchLo = true; }
      try { UintConv::bytesToUint176(hiStr); } catch (std::exception &e) { catchHi = true; }
      REQUIRE(catchLo == true);
      REQUIRE(catchHi == true);

    }

    SECTION("bytesToUint168") {
      BytesArr<21> bytesArr = BytesArr<21> {0xff, 0xff, 0xff, 0x3d, 0x87, 0xd5, 0x8f, 0x2e, 0x16, 0xa4, 0x50, 0x99, 0xdf, 0xde, 0x83, 0xe8, 0x45, 0xd0, 0x13, 0x5e, 0x48};
      auto uint168Output = UintConv::bytesToUint168(bytesArr);
      uint168_t uint168ExpectedOutput = uint168_t("374144402216037378897559585538161667218367340305992");
      REQUIRE(uint168Output == uint168ExpectedOutput);

      bool catchLo = false;
      bool catchHi = false;
      Bytes loStr = Hex::toBytes("0xfffffffffffffffffffffffffffffffffffffffffff");
      Bytes hiStr = Hex::toBytes("0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
      try { UintConv::bytesToUint168(loStr); } catch (std::exception &e) { catchLo = true; }
      try { UintConv::bytesToUint168(hiStr); } catch (std::exception &e) { catchHi = true; }
      REQUIRE(catchLo == true);
      REQUIRE(catchHi == true);

    }

    SECTION("bytesToUint160") {
      FixedBytes<20> bytesStr(bytes::view("\x58\xc5\x95\xbe\xdf\x1d\xea\x53\x2c\xf0\x6a\xf9\x09\x1a\x51\xb7\x5a\x11\xda\x61"));
      auto uint160Output = UintConv::bytesToUint160(bytesStr);
      uint160_t uint160ExpectedOutput = uint160_t("506797479317435130489084083375319966488594602593");
      REQUIRE(uint160Output == uint160ExpectedOutput);

      bool catchLo = false;
      bool catchHi = false;
      Bytes loStr = Hex::toBytes("0xffffffffffffffffffffffffffffffffffffff");
      Bytes hiStr = Hex::toBytes("0xffffffffffffffffffffffffffffffffffffffffff");
      try { UintConv::bytesToUint160(loStr); } catch (std::exception &e) { catchLo = true; }
      try { UintConv::bytesToUint160(hiStr); } catch (std::exception &e) { catchHi = true; }
      REQUIRE(catchLo == true);
      REQUIRE(catchHi == true);
    }

    SECTION("bytesToUint152") {
      BytesArr<19> bytesArr = BytesArr<19> {0xff, 0xaf, 0xa7, 0xc9, 0x9f, 0xec, 0x62, 0x36, 0xc7, 0x93, 0x33, 0xe4, 0x70, 0x1a, 0x7f, 0xe8, 0x28, 0x7b, 0xfb};
      auto uint152Output = UintConv::bytesToUint152(bytesArr);
      uint152_t uint152ExpectedOutput = uint152_t("5701991770823839524233143877797980545130986491");
      REQUIRE(uint152Output == uint152ExpectedOutput);

      bool catchLo = false;
      bool catchHi = false;
      Bytes loStr = Hex::toBytes("0xfffffffffffffffffffffffffffffffffffffffffffffffff");
      Bytes hiStr = Hex::toBytes("0xfffffffffffffffffffffffffffffffffffffffffffffffffff");
      try { UintConv::bytesToUint152(loStr); } catch (std::exception &e) { catchLo = true; }
      try { UintConv::bytesToUint152(hiStr); } catch (std::exception &e) { catchHi = true; }
      REQUIRE(catchLo == true);
      REQUIRE(catchHi == true);
    }

    SECTION("bytesToUint144") {
      BytesArr<18> bytesArr = BytesArr<18> {0xff, 0xff, 0xff, 0xf0, 0x97, 0xb2, 0x0a, 0x93, 0xbd, 0xc2, 0x0f, 0xd1, 0xf9, 0x6a, 0x9b, 0x7f, 0xff, 0xff};
      auto uint144Output = UintConv::bytesToUint144(bytesArr);
      uint144_t uint144ExpectedOutput = uint144_t("22300745118530623141515718272648361505980415");
      REQUIRE(uint144Output == uint144ExpectedOutput);

      bool catchLo = false;
      bool catchHi = false;
      Bytes loStr = Hex::toBytes("0xffffffffffffffffffffffffffffffffffffffffffffff");
      Bytes hiStr = Hex::toBytes("0xfffffffffffffffffffffffffffffffffffffffffffffff");
      try { UintConv::bytesToUint144(loStr); } catch (std::exception &e) { catchLo = true; }
      try { UintConv::bytesToUint144(hiStr); } catch (std::exception &e) { catchHi = true; }
      REQUIRE(catchLo == true);
      REQUIRE(catchHi == true);
    }

    SECTION("bytesToUint136") {
      BytesArr<17> bytesArr = BytesArr<17> {0xff, 0xff, 0xff, 0xd8, 0x8e, 0x94, 0x95, 0xee, 0xc9, 0x84, 0xf6, 0x26, 0xc7, 0xe9, 0x3f, 0xfd, 0xa7};
      auto uint136Output = UintConv::bytesToUint136(bytesArr);
      uint136_t uint136ExpectedOutput = uint136_t("87112285131760246616623899502532662132135");
      REQUIRE(uint136Output == uint136ExpectedOutput);

      bool catchLo = false;
      bool catchHi = false;
      Bytes loStr = Hex::toBytes("0xffffffffffffffffffffffffffffffffffffffffffffffff");
      Bytes hiStr = Hex::toBytes("0xfffffffffffffffffffffffffffffffffffffffffffffffff");
      try { UintConv::bytesToUint136(loStr); } catch (std::exception &e) { catchLo = true; }
      try { UintConv::bytesToUint136(hiStr); } catch (std::exception &e) { catchHi = true; }
      REQUIRE(catchLo == true);
      REQUIRE(catchHi == true);
    }

    SECTION("bytesToUint128") {
      BytesArr<16> bytesArr = BytesArr<16> {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc9 };
      auto uint128Output = UintConv::bytesToUint128(bytesArr);
      uint128_t uint128ExpectedOutput = uint128_t("340282366920938463463374607431768211401");
      REQUIRE(uint128Output == uint128ExpectedOutput);

      bool catchLo = false;
      bool catchHi = false;
      Bytes loStr = Hex::toBytes("0xfffffffffffffffffffffffffffff");
      Bytes hiStr = Hex::toBytes("0xfffffffffffffffffffffffffffffffff");
      try { UintConv::bytesToUint128(loStr); } catch (std::exception &e) { catchLo = true; }
      try { UintConv::bytesToUint128(hiStr); } catch (std::exception &e) { catchHi = true; }
      REQUIRE(catchLo == true);
      REQUIRE(catchHi == true);

    }

    SECTION("bytesToUint120") {
      BytesArr<15> bytesArr = BytesArr<15> {0xff, 0xff, 0xff, 0xff, 0x3d, 0x87, 0xd5, 0x8f, 0x2e, 0x16, 0xa4, 0x50, 0x99, 0xdf, 0xde};
      auto uint120Output = UintConv::bytesToUint120(bytesArr);
      uint120_t uint120ExpectedOutput = uint120_t("1329227995549816797027838164786077662");
      REQUIRE(uint120Output == uint120ExpectedOutput);

      bool catchLo = false;
      bool catchHi = false;
      Bytes loStr = Hex::toBytes("0xffffffffffffffffffffffffffffffffffff");
      Bytes hiStr = Hex::toBytes("0xfffffffffffffffffffffffffffffffffffffff");
      try { UintConv::bytesToUint120(loStr); } catch (std::exception &e) { catchLo = true; }
      try { UintConv::bytesToUint120(hiStr); } catch (std::exception &e) { catchHi = true; }
      REQUIRE(catchLo == true);
      REQUIRE(catchHi == true);

    }

    SECTION("bytesToUint112") {
      BytesArr<14> bytesArr = BytesArr<14> {0xff, 0xf3, 0x60, 0xd3, 0x5e, 0xf3, 0x85, 0xc9, 0x7e, 0xa0, 0x4f, 0x94, 0x3d, 0xfb};
      auto uint112Output = UintConv::bytesToUint112(bytesArr);
      uint112_t uint112ExpectedOutput = uint112_t("5191296858514827628530496129220091");
      REQUIRE(uint112Output == uint112ExpectedOutput);

      bool catchLo = false;
      bool catchHi = false;
      Bytes loStr = Hex::toBytes("0xfffffffffffffffffffffffffffffffffff");
      Bytes hiStr = Hex::toBytes("0xffffffffffffffffffffffffffffffffffff");
      try { UintConv::bytesToUint112(loStr); } catch (std::exception &e) { catchLo = true; }
      try { UintConv::bytesToUint112(hiStr); } catch (std::exception &e) { catchHi = true; }
      REQUIRE(catchLo == true);
      REQUIRE(catchHi == true);

    }

    SECTION("bytesToUint104") {
      BytesArr<13> bytesArr = BytesArr<13> {0xff, 0x1d, 0xd1, 0x5b, 0x6a, 0x21, 0xe0, 0x63, 0x45, 0x02, 0xbf, 0x43, 0xfb};
      auto uint104Output = UintConv::bytesToUint104(bytesArr);
      uint104_t uint104ExpectedOutput = uint104_t("20212409603611670423941251286011");
      REQUIRE(uint104Output == uint104ExpectedOutput);

      bool catchLo = false;
      bool catchHi = false;
      Bytes loStr = Hex::toBytes("0xfffffffffffffffffffffffffffffffff");
      Bytes hiStr = Hex::toBytes("0xfffffffffffffffffffffffffffffffffff");
      try { UintConv::bytesToUint104(loStr); } catch (std::exception &e) { catchLo = true; }
      try { UintConv::bytesToUint104(hiStr); } catch (std::exception &e) { catchHi = true; }
      REQUIRE(catchLo == true);
      REQUIRE(catchHi == true);

    }


    SECTION("bytesToUint96") {
      BytesArr<12> bytesArr = BytesArr<12> {0xff, 0xad, 0x48, 0x2d, 0x23, 0x37, 0xed, 0xb8, 0x20, 0x21, 0xa3, 0xe3};
      auto uint96Output = UintConv::bytesToUint96(bytesArr);
      uint96_t uint96ExpectedOutput = uint96_t("79128162514264331593543951331");
      REQUIRE(uint96Output == uint96ExpectedOutput);

      bool catchLo = false;
      bool catchHi = false;
      Bytes loStr = Hex::toBytes("0xfffffffffffffffffffffffff");
      Bytes hiStr = Hex::toBytes("0xffffffffffffffffffffffffff");
      try { UintConv::bytesToUint96(loStr); } catch (std::exception &e) { catchLo = true; }
      try { UintConv::bytesToUint96(hiStr); } catch (std::exception &e) { catchHi = true; }
      REQUIRE(catchLo == true);
      REQUIRE(catchHi == true);
    }

    SECTION("bytesToUint88") {
      BytesArr<11> bytesArr = BytesArr<11> {0xff, 0xc0, 0x7e, 0x63, 0x6f, 0xba, 0x4a, 0xde, 0x54, 0x79, 0xfb};
      auto uint88Output = UintConv::bytesToUint88(bytesArr);
      uint88_t uint88ExpectedOutput = uint88_t("309185109821345061724781051");
      REQUIRE(uint88Output == uint88ExpectedOutput);

      bool catchLo = false;
      bool catchHi = false;
      Bytes loStr = Hex::toBytes("0xfffffffffffffffffffffff");
      Bytes hiStr = Hex::toBytes("0xffffffffffffffffffffffff");
      try { UintConv::bytesToUint88(loStr); } catch (std::exception &e) { catchLo = true; }
      try { UintConv::bytesToUint88(hiStr); } catch (std::exception &e) { catchHi = true; }
      REQUIRE(catchLo == true);
      REQUIRE(catchHi == true);
    }


    SECTION("bytesToUint80") {
      BytesArr<10> bytesArr = BytesArr<10> {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf6, 0xd8, 0x3b};
      auto uint80Output = UintConv::bytesToUint80(bytesArr);
      uint80_t uint80ExpectedOutput = uint80_t("1208925819614629174106171");
      REQUIRE(uint80Output == uint80ExpectedOutput);

      bool catchLo = false;
      bool catchHi = false;
      Bytes loStr = Hex::toBytes("0xffffffffffffffffffffff");
      Bytes hiStr = Hex::toBytes("0xffffffffffffffffffffffff");
      try { UintConv::bytesToUint80(loStr); } catch (std::exception &e) { catchLo = true; }
      try { UintConv::bytesToUint80(hiStr); } catch (std::exception &e) { catchHi = true; }
      REQUIRE(catchLo == true);
      REQUIRE(catchHi == true);
    }

    SECTION("bytesToUint72") {
      BytesArr<9> bytesArr = BytesArr<9> {0x5d, 0x5e, 0xa1, 0xe5, 0xc9, 0x8e, 0x75, 0xcb, 0xfb};
      auto uint72Output = UintConv::bytesToUint72(bytesArr);
      uint72_t uint72ExpectedOutput = uint72_t("1722366182819645213691");
      REQUIRE(uint72Output == uint72ExpectedOutput);

      bool catchLo = false;
      bool catchHi = false;
      Bytes loStr = Hex::toBytes("0xffffffffffffffffffff");
      Bytes hiStr = Hex::toBytes("0xffffffffffffffffffffff");
      try { UintConv::bytesToUint72(loStr); } catch (std::exception &e) { catchLo = true; }
      try { UintConv::bytesToUint72(hiStr); } catch (std::exception &e) { catchHi = true; }
      REQUIRE(catchLo == true);
      REQUIRE(catchHi == true);
    }

    SECTION("bytesToUint64") {
      FixedBytes<8> bytesStr(bytes::view("\x9a\xce\x8e\x96\x24\xe4\xed\x56"));
      auto uint64Output = UintConv::bytesToUint64(bytesStr);
      uint64_t uint64ExpectedOutput = uint64_t(11155010102558518614ULL);
      REQUIRE(uint64Output == uint64ExpectedOutput);

      bool catchLo = false;
      bool catchHi = false;
      Bytes loStr = Hex::toBytes("0xffffffffffffff");
      Bytes hiStr = Hex::toBytes("0xffffffffffffffffff");
      try { UintConv::bytesToUint64(loStr); } catch (std::exception &e) { catchLo = true; }
      try { UintConv::bytesToUint64(hiStr); } catch (std::exception &e) { catchHi = true; }
      REQUIRE(catchLo == true);
      REQUIRE(catchHi == true);
    }

    SECTION("bytesToUint56") {
      BytesArr<7> bytesArr = BytesArr<7> {0xff, 0xfa, 0x8b, 0x03, 0xc6, 0x12, 0x7b};
      auto uint56Output = UintConv::bytesToUint56(bytesArr);
      uint64_t uint56ExpectedOutput = uint64_t(72051594031927931);
      REQUIRE(uint56Output == uint56ExpectedOutput);

      bool catchLo = false;
      bool catchHi = false;
      Bytes loStr = Hex::toBytes("0xfffffffffffffff");
      Bytes hiStr = Hex::toBytes("0xffffffffffffffff");
      try { UintConv::bytesToUint56(loStr); } catch (std::exception &e) { catchLo = true; }
      try { UintConv::bytesToUint56(hiStr); } catch (std::exception &e) { catchHi = true; }
      REQUIRE(catchLo == true);
      REQUIRE(catchHi == true);
    }

    SECTION("bytesToUint48") {
      BytesArr<6> bytesArr = BytesArr<6> {0xff, 0xf1, 0xd8, 0x00, 0x78, 0x3b};
      auto uint48Output = UintConv::bytesToUint48(bytesArr);
      uint64_t uint48ExpectedOutput = uint64_t(281414176110651);
      REQUIRE(uint48Output == uint48ExpectedOutput);

      bool catchLo = false;
      bool catchHi = false;
      Bytes loStr = Hex::toBytes("0xfffffffffffff");
      Bytes hiStr = Hex::toBytes("0xffffffffffffff");
      try { UintConv::bytesToUint48(loStr); } catch (std::exception &e) { catchLo = true; }
      try { UintConv::bytesToUint48(hiStr); } catch (std::exception &e) { catchHi = true; }
      REQUIRE(catchLo == true);
      REQUIRE(catchHi == true);
    }

    SECTION("bytesToUint40") {
      BytesArr<5> bytesArr = BytesArr<5> {0xed, 0x5f, 0xa0, 0xc8, 0x8b};
      auto uint40Output = UintConv::bytesToUint40(bytesArr);
      uint64_t uint40ExpectedOutput = uint64_t(1019511621771);
      REQUIRE(uint40Output == uint40ExpectedOutput);

      bool catchLo = false;
      bool catchHi = false;
      Bytes loStr = Hex::toBytes("0xfffffffffff");
      Bytes hiStr = Hex::toBytes("0xffffffffffff");
      try { UintConv::bytesToUint40(loStr); } catch (std::exception &e) { catchLo = true; }
      try { UintConv::bytesToUint40(hiStr); } catch (std::exception &e) { catchHi = true; }
      REQUIRE(catchLo == true);
      REQUIRE(catchHi == true);
    }

    SECTION("bytesToUint32") {
      FixedBytes<4> bytesStr(bytes::view("\x77\x7b\xca\x9a"));
      auto uint32Output = UintConv::bytesToUint32(bytesStr);
      uint32_t uint32ExpectedOutput = 2004601498;
      REQUIRE(uint32Output == uint32ExpectedOutput);

      bool catchLo = false;
      bool catchHi = false;
      Bytes loStr = Hex::toBytes("0xffffff");
      Bytes hiStr = Hex::toBytes("0xffffffffff");
      try { UintConv::bytesToUint32(loStr); } catch (std::exception &e) { catchLo = true; }
      try { UintConv::bytesToUint32(hiStr); } catch (std::exception &e) { catchHi = true; }
      REQUIRE(catchLo == true);
      REQUIRE(catchHi == true);
    }

    SECTION("bytesToUint24") {
      BytesArr<3> bytesArr = BytesArr<3> {0xf5, 0xed, 0xdb};
      auto uint24Output = UintConv::bytesToUint24(bytesArr);
      uint32_t uint24ExpectedOutput = uint32_t(16117211);
      REQUIRE(uint24Output == uint24ExpectedOutput);

      bool catchLo = false;
      bool catchHi = false;
      Bytes loStr = Hex::toBytes("0xfffffff");
      Bytes hiStr = Hex::toBytes("0xffffffff");
      try { UintConv::bytesToUint24(loStr); } catch (std::exception &e) { catchLo = true; }
      try { UintConv::bytesToUint24(hiStr); } catch (std::exception &e) { catchHi = true; }
      REQUIRE(catchLo == true);
      REQUIRE(catchHi == true);
    }

    SECTION("bytesToUint16") {
      FixedBytes<2> bytesStr(bytes::view("\xff\xac"));
      auto uint16Output = UintConv::bytesToUint16(bytesStr);
      uint16_t uint16ExpectedOutput = 65452;
      REQUIRE(uint16Output == uint16ExpectedOutput);

      bool catchLo = false;
      bool catchHi = false;
      Bytes loStr = Hex::toBytes("0xff");
      Bytes hiStr = Hex::toBytes("0xffffff");
      try { UintConv::bytesToUint16(loStr); } catch (std::exception &e) { catchLo = true; }
      try { UintConv::bytesToUint16(hiStr); } catch (std::exception &e) { catchHi = true; }
      REQUIRE(catchLo == true);
      REQUIRE(catchHi == true);
    }

    SECTION("bytesToUint8") {
      FixedBytes<1> bytesStr(bytes::view("\x78"));
      auto uint8Output = UintConv::bytesToUint8(bytesStr);
      uint8_t uint8ExpectedOutput = 120;
      REQUIRE(uint8Output == uint8ExpectedOutput);

      bool catchLo = false;
      bool catchHi = false;
      Bytes loStr = Hex::toBytes("0x");
      Bytes hiStr = Hex::toBytes("0xffff");
      try { UintConv::bytesToUint8(loStr); } catch (std::exception &e) { catchLo = true; }
      try { UintConv::bytesToUint8(hiStr); } catch (std::exception &e) { catchHi = true; }
      REQUIRE(catchLo == true);
      REQUIRE(catchHi == true);
    }
  }
}

