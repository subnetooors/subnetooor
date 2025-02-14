/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#include "libs/catch2/catch_amalgamated.hpp"

#include "contract/abi.h" // utils.h -> strings.h, libs/json.hpp -> vector

using Catch::Matchers::Equals;

namespace TABI {
TEST_CASE("ABI Namespace", "[unit][contract][abi]") {
  // This should cover all types for functor, including nested.
  SECTION("Encode Functor for FunnsiesFunc(std::tuple<std::vector<std::tuple<uint256_t, uint256_t, uint256_t, uint256_t>>, std::string, uint256_t, std::vector<std::tuple<std::string, std::tuple<uint256_t, uint256_t>, std::string>>> arg)")
  {
    struct Test {
      void FunnsiesFunc(const std::tuple<
        std::vector<std::tuple<uint256_t, uint256_t, uint256_t, uint256_t>>,
        std::string,
        uint256_t,
        std::vector<std::tuple<std::string, std::tuple<uint256_t, uint256_t>, std::string>>
      >& a){};
    };
    auto result = ABI::FunctorEncoder::encode<const std::tuple<
      std::vector<std::tuple<uint256_t, uint256_t, uint256_t, uint256_t>>,
      std::string,
      uint256_t,
      std::vector<std::tuple<std::string, std::tuple<uint256_t, uint256_t>, std::string>>
    >>("FunnsiesFunc");
    REQUIRE(result.value == 3730907155);
  }

  SECTION("Encode Uint256 (Single)") {
      auto functor = ABI::FunctorEncoder::encode<uint256_t>("testUint");
      Bytes eS = ABI::Encoder::encodeData(uint256_t("12038189571283151234217456623442137"));

      REQUIRE(functor.value == 3349244261);
      REQUIRE(eS == Hex::toBytes(
        "0000000000000000000000000000000000025187505f9a7cca5c5178e81858d9"
      ));
    }

  SECTION("Encode Uint256 (Multiple)") {
      auto eS = ABI::Encoder::encodeData(uint256_t("985521342366467353964568564348544758443523147426"),
        uint256_t("3453441424448154428346543455122894428593523456453894523"),
        uint256_t("238745423894452554435879784534423784946532544278453254451345"));
      auto functor = ABI::FunctorEncoder::encode<uint256_t, uint256_t, uint256_t>("testMultipleUint");

      REQUIRE(functor.value == 2863972667);

      REQUIRE(Bytes(eS.begin(), eS.begin() + 32) == Hex::toBytes(
        "000000000000000000000000aca04e2e6a9c731a64f56964ab72e6c8270786a2"
      ));
      REQUIRE(Bytes(eS.begin() + 32, eS.begin() + 32 * 2) == Hex::toBytes(
        "000000000000000000240e3c8296e085da6a626254ed08dd8b03286c83bbe17b"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 2, eS.begin() + 32 * 3) == Hex::toBytes(
        "000000000000002608ca87ae5c3b4312e205f41a89f288579ccd19f908317091"
      ));
    }

  SECTION("Encode Uint256 (Array)") {

      auto eS = ABI::Encoder::encodeData(std::vector<uint256_t>{
        uint256_t("19283178512315252514312458124312935128381523"),
        uint256_t("31482535189448189541125434144"),
        uint256_t("1123444185124184138124378143891242186794252455823414458"),
        uint256_t("215345189442554346421356134551234851234484")});

      auto functor = ABI::FunctorEncoder::encode<std::vector<uint256_t>>("testUintArr");

      REQUIRE(functor.value == 3517244486);
      REQUIRE(Bytes(eS.begin(), eS.begin() + 32) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000020"
      ));
      REQUIRE(Bytes(eS.begin() + 32, eS.begin() + 32 * 2) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000004"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 2, eS.begin() + 32 * 3) == Hex::toBytes(
        "0000000000000000000000000000dd5c2b23fbb5fb408500075ff573e1383853"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 3, eS.begin() + 32 * 4) == Hex::toBytes(
        "000000000000000000000000000000000000000065b9be246336b3f36607ab20"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 4, eS.begin() + 32 * 5) == Hex::toBytes(
        "0000000000000000000bbab3b46bd0328e1d17617db2abfefc19046a083e14ba"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 5, eS.begin() + 32 * 6) == Hex::toBytes(
        "00000000000000000000000000000278d7b6df6a4c94873a7d6559bcab95e2b4"
      ));
    }

  SECTION("Encode Int256 (Single)") {

    auto eS = ABI::Encoder::encodeData(int256_t("-123456789012345678901234567890123456789012345678901234567890"));
    auto functor = ABI::FunctorEncoder::encode<int256_t>("testInt");

      REQUIRE(functor.value == 1612174621);
      REQUIRE(Bytes(eS.begin(), eS.begin() + 32) == Hex::toBytes(
        "ffffffffffffffec550afb1b43e19de8c0785bc873c84b6373300e6931c0f52e"
      ));
    }

  SECTION("Encode Int256 (Multiple)") {

    auto eS = ABI::Encoder::encodeData(int256_t("-123456789012345678901234567890123456789012345678901234567890"),
      int256_t("123456789012345678901234567890123456789012345678901234567890"),
      int256_t("-56789012345678901234567890123456789012345678901234567890"),
      int256_t("56789012345678901234567890123456789012345678901234567890"));

      Functor functor = ABI::FunctorEncoder::encode<int256_t, int256_t>("testMultipleInt");

      REQUIRE(functor.value == 3288499546);
      REQUIRE(Bytes(eS.begin(), eS.begin() + 32) == Hex::toBytes(
        "ffffffffffffffec550afb1b43e19de8c0785bc873c84b6373300e6931c0f52e"
      ));
      REQUIRE(Bytes(eS.begin() + 32, eS.begin() + 32 * 2) == Hex::toBytes(
        "0000000000000013aaf504e4bc1e62173f87a4378c37b49c8ccff196ce3f0ad2"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 2, eS.begin() + 32 * 3) == Hex::toBytes(
        "fffffffffffffffffdaf1854f62f44391b682b86c9106cac85300e6931c0f52e"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 3, eS.begin() + 32 * 4) == Hex::toBytes(
        "00000000000000000250e7ab09d0bbc6e497d47936ef93537acff196ce3f0ad2"
      ));
    }

  SECTION("Encode Int256 (Array)") {

    auto eS = ABI::Encoder::encodeData(std::vector<int256_t>{
      int256_t("-123456789012345678901234567890123456789012345678901234567890"),
      int256_t("123456789012345678901234567890123456789012345678901234567890"),
      int256_t("-56789012345678901234567890123456789012345678901234567890"),
      int256_t("56789012345678901234567890123456789012345678901234567890")});

      Functor functor = ABI::FunctorEncoder::encode<std::vector<int256_t>>("testIntArr");

      REQUIRE(functor.value == 1195402566);
      REQUIRE(Bytes(eS.begin(), eS.begin() + 32) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000020"
      ));
      REQUIRE(Bytes(eS.begin() + 32, eS.begin() + 32 * 2) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000004"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 2, eS.begin() + 32 * 3) == Hex::toBytes(
        "ffffffffffffffec550afb1b43e19de8c0785bc873c84b6373300e6931c0f52e"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 3, eS.begin() + 32 * 4) == Hex::toBytes(
        "0000000000000013aaf504e4bc1e62173f87a4378c37b49c8ccff196ce3f0ad2"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 4, eS.begin() + 32 * 5) == Hex::toBytes(
        "fffffffffffffffffdaf1854f62f44391b682b86c9106cac85300e6931c0f52e"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 5, eS.begin() + 32 * 6) == Hex::toBytes(
        "00000000000000000250e7ab09d0bbc6e497d47936ef93537acff196ce3f0ad2"
      ));
    }

  SECTION("Encode String (Single)") {

      auto eS = ABI::Encoder::encodeData(std::string("Hello World!"));

      Functor functor = ABI::FunctorEncoder::encode<std::string>("testString");

      REQUIRE(functor.value == 1640716801);
      REQUIRE(Bytes(eS.begin(), eS.begin() + 32) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000020"
      ));
      REQUIRE(Bytes(eS.begin() + 32, eS.begin() + 32 * 2) == Hex::toBytes(
        "000000000000000000000000000000000000000000000000000000000000000c"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 2, eS.begin() + 32 * 3) == Hex::toBytes(
        "48656c6c6f20576f726c64210000000000000000000000000000000000000000"
      ));
    }

  SECTION("Encode Bool (Multiple)") {

      auto eS = ABI::Encoder::encodeData(true, false, true);

      Functor functor = ABI::FunctorEncoder::encode<bool, bool, bool>("testMultipleBool");

      REQUIRE(functor.value == 1241378576);
      REQUIRE(Bytes(eS.begin(), eS.begin() + 32) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000001"
      ));
      REQUIRE(Bytes(eS.begin() + 32, eS.begin() + 32 * 2) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000000"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 2, eS.begin() + 32 * 3) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000001"
      ));
    }

    SECTION("Encode String (Array) + Uint256 (Array)") {

      auto eS = ABI::Encoder::encodeData(std::vector<std::string>{
        "First String", "Second String", "Third String"
      }, std::vector<uint256_t>{
        uint256_t("129838151824165123321245841287434198"),
        uint256_t("2134584124125984418451243118545129854235"),
        uint256_t("1234812315823541285534458693557693548423844235"),
        uint256_t("32452893445892345238552138945234454324523194514")});

      Functor functor = ABI::FunctorEncoder::encode<std::vector<std::string>, std::vector<uint256_t>>("testStringArrWithUintArr");

      REQUIRE(functor.value == 37505630);
      REQUIRE(Bytes(eS.begin(), eS.begin() + 32) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000040"
      ));
      REQUIRE(Bytes(eS.begin() + 32, eS.begin() + 32 * 2) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000180"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 2, eS.begin() + 32 * 3) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000003"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 3, eS.begin() + 32 * 4) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000060"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 4, eS.begin() + 32 * 5) == Hex::toBytes(
        "00000000000000000000000000000000000000000000000000000000000000a0"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 5, eS.begin() + 32 * 6) == Hex::toBytes(
        "00000000000000000000000000000000000000000000000000000000000000e0"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 6, eS.begin() + 32 * 7) == Hex::toBytes(
        "000000000000000000000000000000000000000000000000000000000000000c"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 7, eS.begin() + 32 * 8) == Hex::toBytes(
        "466972737420537472696e670000000000000000000000000000000000000000"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 8, eS.begin() + 32 * 9) == Hex::toBytes(
        "000000000000000000000000000000000000000000000000000000000000000d"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 9, eS.begin() + 32 * 10) == Hex::toBytes(
        "5365636f6e6420537472696e6700000000000000000000000000000000000000"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 10, eS.begin() + 32 * 11) == Hex::toBytes(
        "000000000000000000000000000000000000000000000000000000000000000c"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 11, eS.begin() + 32 * 12) == Hex::toBytes(
        "546869726420537472696e670000000000000000000000000000000000000000"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 12, eS.begin() + 32 * 13) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000004"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 13, eS.begin() + 32 * 14) == Hex::toBytes(
        "0000000000000000000000000000000000190183df26aa795c3b01e079ae4fd6"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 14, eS.begin() + 32 * 15) == Hex::toBytes(
        "0000000000000000000000000000000645e1f2c6dad3d9f1675c8163df32551b"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 15, eS.begin() + 32 * 16)== Hex::toBytes(
        "00000000000000000000000000375ef34102454b2b5222061ed99b03a148918b"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 16, eS.begin() + 32 * 17) == Hex::toBytes(
        "00000000000000000000000005af3cf248a13bc919ac44299b86c8a94ba65892"
      ));
    }

    SECTION("Encode Address (Single)") {

      auto eS = ABI::Encoder::encodeData(Address(Hex::toBytes("0x873630b0fAE5F8c69392Abdabb3B15270D137Ca1")));

      Functor functor = ABI::FunctorEncoder::encode<Address>("testAddress");

      REQUIRE(functor.value == 1123309456);
      REQUIRE(Bytes(eS.begin(), eS.end()) == Hex::toBytes(
        "000000000000000000000000873630b0fae5f8c69392abdabb3b15270d137ca1"
      ));
    }

  SECTION("Encode Bytes (Single)") {

      auto eS = ABI::Encoder::encodeData(Hex::toBytes("0xc8191d2e98e7cd9201cef777f85bf857"));

      Functor functor = ABI::FunctorEncoder::encode<Bytes>("testBytes");

      REQUIRE(functor.value == 1017688487);
      REQUIRE(Bytes(eS.begin(), eS.begin() + 32) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000020"
      ));
      REQUIRE(Bytes(eS.begin() + 32, eS.begin() + 32 * 2) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000010"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 2, eS.begin() + 32 * 3) == Hex::toBytes(
        "c8191d2e98e7cd9201cef777f85bf85700000000000000000000000000000000"
      ));
    }

    SECTION("Encode Bytes (Array) + String (Array)") {

      auto eS = ABI::Encoder::encodeData(std::vector<Bytes>{
        Hex::toBytes("0x81a1217428d6d8ff7a419e87cfc948d2"),
        Hex::toBytes("0x2d96cf448d1d455d9013572ac07edefc"),
        Hex::toBytes("0xc584d0de5dbddca6e74686a3c154bb28"),
        Hex::toBytes("0xdb6f06ea16ab61dca14053001c6b5815")},
        std::vector<std::string>{
        "First String", "Second String", "Third String",
        "Fourth String"
      });

      Functor functor = ABI::FunctorEncoder::encode<std::vector<Bytes>, std::vector<std::string>>("testBytesArrWithStrArr");

      REQUIRE(functor.value == 4052229535);
      REQUIRE(Bytes(eS.begin(), eS.begin() + 32) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000040"
      ));
      REQUIRE(Bytes(eS.begin() + 32, eS.begin() + 32 * 2) == Hex::toBytes(
        "00000000000000000000000000000000000000000000000000000000000001e0"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 2, eS.begin() + 32 * 3) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000004"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 3, eS.begin() + 32 * 4) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000080"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 4, eS.begin() + 32 * 5) == Hex::toBytes(
        "00000000000000000000000000000000000000000000000000000000000000c0"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 5, eS.begin() + 32 * 6) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000100"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 6, eS.begin() + 32 * 7) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000140"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 7, eS.begin() + 32 * 8) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000010"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 8, eS.begin() + 32 * 9) == Hex::toBytes(
        "81a1217428d6d8ff7a419e87cfc948d200000000000000000000000000000000"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 9, eS.begin() + 32 * 10) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000010"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 10, eS.begin() + 32 * 11) == Hex::toBytes(
        "2d96cf448d1d455d9013572ac07edefc00000000000000000000000000000000"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 11, eS.begin() + 32 * 12) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000010"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 12, eS.begin() + 32 * 13) == Hex::toBytes(
        "c584d0de5dbddca6e74686a3c154bb2800000000000000000000000000000000"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 13, eS.begin() + 32 * 14) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000010"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 14, eS.begin() + 32 * 15) == Hex::toBytes(
        "db6f06ea16ab61dca14053001c6b581500000000000000000000000000000000"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 15, eS.begin() + 32 * 16) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000004"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 16, eS.begin() + 32 * 17) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000080"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 17, eS.begin() + 32 * 18) == Hex::toBytes(
        "00000000000000000000000000000000000000000000000000000000000000c0"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 18, eS.begin() + 32 * 19) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000100"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 19, eS.begin() + 32 * 20) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000140"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 20, eS.begin() + 32 * 21) == Hex::toBytes(
        "000000000000000000000000000000000000000000000000000000000000000c"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 21, eS.begin() + 32 * 22) == Hex::toBytes(
        "466972737420537472696e670000000000000000000000000000000000000000"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 22, eS.begin() + 32 * 23) == Hex::toBytes(
        "000000000000000000000000000000000000000000000000000000000000000d"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 23, eS.begin() + 32 * 24) == Hex::toBytes(
        "5365636f6e6420537472696e6700000000000000000000000000000000000000"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 24, eS.begin() + 32 * 25)== Hex::toBytes(
        "000000000000000000000000000000000000000000000000000000000000000c"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 25, eS.begin() + 32 * 26) == Hex::toBytes(
        "546869726420537472696e670000000000000000000000000000000000000000"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 26, eS.begin() + 32 * 27) == Hex::toBytes(
        "000000000000000000000000000000000000000000000000000000000000000d"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 27, eS.begin() + 32 * 28) == Hex::toBytes(
        "466f7572746820537472696e6700000000000000000000000000000000000000"
      ));
    }

  SECTION("Encode All") {

    auto eS = ABI::Encoder::encodeData(uint256_t("19283816759128317851231551416451212"),
      std::vector<uint256_t>{
        uint256_t("1239381517249318561241694412"),
        uint256_t("2395843472138412758912309213482574123672567"),
        uint256_t("9138482765346472349817634647689124123"),
        uint256_t("9234782382341248283491")
      },
      true,
      std::vector<bool>{false, true, false},
      Address(Hex::toBytes("0x873630b0fAE5F8c69392Abdabb3B15270D137Ca1")),
      std::vector<Address>{
        Address(Hex::toBytes("0x2D061c095b06efed6A54b6e9B3f50f1b55cce2FF")),
        Address(Hex::toBytes("0x873630b0fAE5F8c69392Abdabb3B15270D137Ca1")),
        Address(Hex::toBytes("0xA462f6A66CC4465fA2d5E90EFA6757f615125760"))
      },
      Hex::toBytes("0xec05537ed99fc9053e29368726573b25"),
      std::vector<Bytes>{
        Hex::toBytes("0xadfae295d92644d19f69e4f20f28d0ae"),
        Hex::toBytes("0x6777b56cd127407ae1b1cc309905521e"),
        Hex::toBytes("0x52719fe16375c2446b109dfcf9336c38"),
        Hex::toBytes("0x6763b32cbd1c695a694d66fe2e729c97")},
      std::string("This is a string"),
      std::vector<std::string>{"Yes", "This", "Is", "A", "String",
                                "Array", "How stupid lol"});

      Functor functor = ABI::FunctorEncoder::encode<uint256_t, std::vector<uint256_t>, bool, std::vector<bool>, Address, std::vector<Address>, Bytes, std::vector<Bytes>, std::string, std::vector<std::string>>("testAll");

      REQUIRE(functor.value == 3637667916);
      REQUIRE(Bytes(eS.begin(), eS.begin() + 32) == Hex::toBytes(
        "000000000000000000000000000000000003b6c3fc7f2d151440685a319c408c"
      ));
      REQUIRE(Bytes(eS.begin() + 32, eS.begin() + 32 * 2) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000140"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 2, eS.begin() + 32 * 3) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000001"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 3, eS.begin() + 32 * 4) == Hex::toBytes(
        "00000000000000000000000000000000000000000000000000000000000001e0"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 4, eS.begin() + 32 * 5) == Hex::toBytes(
        "000000000000000000000000873630b0fae5f8c69392abdabb3b15270d137ca1"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 5, eS.begin() + 32 * 6) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000260"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 6, eS.begin() + 32 * 7) == Hex::toBytes(
        "00000000000000000000000000000000000000000000000000000000000002e0"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 7, eS.begin() + 32 * 8) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000320"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 8, eS.begin() + 32 * 9) == Hex::toBytes(
        "00000000000000000000000000000000000000000000000000000000000004c0"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 9, eS.begin() + 32 * 10) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000500"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 10, eS.begin() + 32 * 11) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000004"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 11, eS.begin() + 32 * 12) == Hex::toBytes(
        "00000000000000000000000000000000000000000401313ead502c4caecd00cc"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 12, eS.begin() + 32 * 13) == Hex::toBytes(
        "00000000000000000000000000001b80c04c816f5d3f60d46e2c568de014a3f7"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 13, eS.begin() + 32 * 14) == Hex::toBytes(
        "0000000000000000000000000000000006e001fc95fc94cdd826174c57e3d91b"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 14, eS.begin() + 32 * 15) == Hex::toBytes(
        "0000000000000000000000000000000000000000000001f49e59b0c3edac7363"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 15, eS.begin() + 32 * 16) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000003"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 16, eS.begin() + 32 * 17) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000000"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 17, eS.begin() + 32 * 18) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000001"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 18, eS.begin() + 32 * 19) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000000"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 19, eS.begin() + 32 * 20) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000003"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 20, eS.begin() + 32 * 21) == Hex::toBytes(
        "0000000000000000000000002d061c095b06efed6a54b6e9b3f50f1b55cce2ff"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 21, eS.begin() + 32 * 22) == Hex::toBytes(
        "000000000000000000000000873630b0fae5f8c69392abdabb3b15270d137ca1"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 22, eS.begin() + 32 * 23)== Hex::toBytes(
        "000000000000000000000000a462f6a66cc4465fa2d5e90efa6757f615125760"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 23, eS.begin() + 32 * 24) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000010"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 24, eS.begin() + 32 * 25) == Hex::toBytes(
        "ec05537ed99fc9053e29368726573b2500000000000000000000000000000000"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 25, eS.begin() + 32 * 26) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000004"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 26, eS.begin() + 32 * 27) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000080"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 27, eS.begin() + 32 * 28) == Hex::toBytes(
        "00000000000000000000000000000000000000000000000000000000000000c0"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 28, eS.begin() + 32 * 29) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000100"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 29, eS.begin() + 32 * 30) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000140"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 30, eS.begin() + 32 * 31) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000010"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 31, eS.begin() + 32 * 32) == Hex::toBytes(
        "adfae295d92644d19f69e4f20f28d0ae00000000000000000000000000000000"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 32, eS.begin() + 32 * 33) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000010"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 33, eS.begin() + 32 * 34) == Hex::toBytes(
        "6777b56cd127407ae1b1cc309905521e00000000000000000000000000000000"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 34, eS.begin() + 32 * 35) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000010"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 35, eS.begin() + 32 * 36) == Hex::toBytes(
        "52719fe16375c2446b109dfcf9336c3800000000000000000000000000000000"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 36, eS.begin() + 32 * 37) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000010"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 37, eS.begin() + 32 * 38)== Hex::toBytes(
        "6763b32cbd1c695a694d66fe2e729c9700000000000000000000000000000000"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 38, eS.begin() + 32 * 39) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000010"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 39, eS.begin() + 32 * 40) == Hex::toBytes(
        "54686973206973206120737472696e6700000000000000000000000000000000"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 40, eS.begin() + 32 * 41) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000007"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 41, eS.begin() + 32 * 42) == Hex::toBytes(
        "00000000000000000000000000000000000000000000000000000000000000e0"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 42, eS.begin() + 32 * 43) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000120"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 43, eS.begin() + 32 * 44) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000160"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 44, eS.begin() + 32 * 45) == Hex::toBytes(
        "00000000000000000000000000000000000000000000000000000000000001a0"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 45, eS.begin() + 32 * 46) == Hex::toBytes(
        "00000000000000000000000000000000000000000000000000000000000001e0"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 46, eS.begin() + 32 * 47) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000220"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 47, eS.begin() + 32 * 48) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000260"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 48, eS.begin() + 32 * 49) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000003"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 49, eS.begin() + 32 * 50) == Hex::toBytes(
        "5965730000000000000000000000000000000000000000000000000000000000"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 50, eS.begin() + 32 * 51) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000004"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 51, eS.begin() + 32 * 52) == Hex::toBytes(
        "5468697300000000000000000000000000000000000000000000000000000000"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 52, eS.begin() + 32 * 53) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000002"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 53, eS.begin() + 32 * 54) == Hex::toBytes(
        "4973000000000000000000000000000000000000000000000000000000000000"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 54, eS.begin() + 32 * 55) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000001"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 55, eS.begin() + 32 * 56) == Hex::toBytes(
        "4100000000000000000000000000000000000000000000000000000000000000"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 56, eS.begin() + 32 * 57) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000006"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 57, eS.begin() + 32 * 58) == Hex::toBytes(
        "537472696e670000000000000000000000000000000000000000000000000000"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 58, eS.begin() + 32 * 59) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000005"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 59, eS.begin() + 32 * 60) == Hex::toBytes(
        "4172726179000000000000000000000000000000000000000000000000000000"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 60, eS.begin() + 32 * 61) == Hex::toBytes(
        "000000000000000000000000000000000000000000000000000000000000000e"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 61, eS.begin() + 32 * 62) == Hex::toBytes(
        "486f7720737475706964206c6f6c000000000000000000000000000000000000"
      ));
    }

    SECTION("Encode std::tuple<uint256_t,uint256_t,uint256_t>") {
      const auto eS = ABI::Encoder::encodeData<std::tuple<uint256_t, uint256_t, uint256_t>>({ 2312415123141231511, 2734526262645, 389234263123421 });
      REQUIRE(Bytes(eS.begin(), eS.begin() + 32) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000002017594d84130397"
      ));
      REQUIRE(Bytes(eS.begin() + 32, eS.begin() + 32 * 2) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000027cae776d75"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 2, eS.begin() + 32 * 3) == Hex::toBytes(
        "00000000000000000000000000000000000000000000000000016201a9fce5dd"
      ));
    }

    SECTION("Encode std::tuple<uint256_t, std::tuple<uint256_t, uint256_t>, uint256_t, uint256_t>") {
      const auto eS = ABI::Encoder::encodeData<std::tuple<uint256_t, std::tuple<uint256_t, uint256_t>, uint256_t, uint256_t>>(
        { 19238561734821, { 98125781723,9812738158 } , 81273854512, 172831642124124}
      );
      REQUIRE(Bytes(eS.begin(), eS.begin() + 32) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000117f53a360a5"
      ));
      REQUIRE(Bytes(eS.begin() + 32, eS.begin() + 32 * 2) == Hex::toBytes(
        "00000000000000000000000000000000000000000000000000000016d8c09adb"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 2, eS.begin() + 32 * 3) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000248e2806e"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 3, eS.begin() + 32 * 4) == Hex::toBytes(
        "00000000000000000000000000000000000000000000000000000012ec4c9a30"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 4, eS.begin() + 32 * 5) == Hex::toBytes(
        "00000000000000000000000000000000000000000000000000009d3080a27f5c"
      ));
    }

    SECTION("Encode std::tuple<uint256_t, std::string, uint256_t, std::string>") {
      const auto eS = ABI::Encoder::encodeData<std::tuple<uint256_t, std::string, uint256_t, std::string>>({81236712741283, "Hello World v1!", 81273854512, "Hello World v2!"});
      REQUIRE(Bytes(eS.begin(), eS.begin() + 32) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000020"
      ));
      REQUIRE(Bytes(eS.begin() + 32, eS.begin() + 32 * 2) == Hex::toBytes(
        "000000000000000000000000000000000000000000000000000049e26567d9a3"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 2, eS.begin() + 32 * 3) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000080"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 3, eS.begin() + 32 * 4) == Hex::toBytes(
        "00000000000000000000000000000000000000000000000000000012ec4c9a30"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 4, eS.begin() + 32 * 5) == Hex::toBytes(
        "00000000000000000000000000000000000000000000000000000000000000c0"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 5, eS.begin() + 32 * 6) == Hex::toBytes(
        "000000000000000000000000000000000000000000000000000000000000000f"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 6, eS.begin() + 32 * 7) == Hex::toBytes(
        "48656c6c6f20576f726c64207631210000000000000000000000000000000000"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 7, eS.begin() + 32 * 8) == Hex::toBytes(
        "000000000000000000000000000000000000000000000000000000000000000f"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 8, eS.begin() + 32 * 9) == Hex::toBytes(
        "48656c6c6f20576f726c64207632210000000000000000000000000000000000"
      ));
    }



    SECTION("Encode std::vector<std::tuple<uint256_t, std::string, uint256_t, std::string>>") {
      const auto eS = ABI::Encoder::encodeData<std::vector<std::tuple<uint256_t, std::string, uint256_t, std::string>>>({
        {81236712741283, "Hello World v1!", 81273854512, "Hello World v2!"},
        {3841267518723, "Hello World v3!", 5189372576123, "Hello World v4!"}
        }
      );
      REQUIRE(Bytes(eS.begin(), eS.begin() + 32) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000020"
      ));
      REQUIRE(Bytes(eS.begin() + 32, eS.begin() + 32 * 2) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000002"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 2, eS.begin() + 32 * 3) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000040"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 3, eS.begin() + 32 * 4) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000140"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 4, eS.begin() + 32 * 5) == Hex::toBytes(
        "000000000000000000000000000000000000000000000000000049e26567d9a3"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 5, eS.begin() + 32 * 6) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000080"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 6, eS.begin() + 32 * 7) == Hex::toBytes(
        "00000000000000000000000000000000000000000000000000000012ec4c9a30"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 7, eS.begin() + 32 * 8) == Hex::toBytes(
        "00000000000000000000000000000000000000000000000000000000000000c0"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 8, eS.begin() + 32 * 9) == Hex::toBytes(
        "000000000000000000000000000000000000000000000000000000000000000f"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 9, eS.begin() + 32 * 10) == Hex::toBytes(
        "48656c6c6f20576f726c64207631210000000000000000000000000000000000"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 10, eS.begin() + 32 * 11) == Hex::toBytes(
        "000000000000000000000000000000000000000000000000000000000000000f"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 11, eS.begin() + 32 * 12) == Hex::toBytes(
        "48656c6c6f20576f726c64207632210000000000000000000000000000000000"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 12, eS.begin() + 32 * 13) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000037e5d62cd03"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 13, eS.begin() + 32 * 14) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000080"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 14, eS.begin() + 32 * 15) == Hex::toBytes(
        "000000000000000000000000000000000000000000000000000004b83eb5817b"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 15, eS.begin() + 32 * 16) == Hex::toBytes(
        "00000000000000000000000000000000000000000000000000000000000000c0"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 16, eS.begin() + 32 * 17) == Hex::toBytes(
        "000000000000000000000000000000000000000000000000000000000000000f"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 17, eS.begin() + 32 * 18) == Hex::toBytes(
        "48656c6c6f20576f726c64207633210000000000000000000000000000000000"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 18, eS.begin() + 32 * 19) == Hex::toBytes(
        "000000000000000000000000000000000000000000000000000000000000000f"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 19, eS.begin() + 32 * 20) == Hex::toBytes(
        "48656c6c6f20576f726c64207634210000000000000000000000000000000000"
      ));
    }

    SECTION("Encode std::tuple<std::vector<std::tuple<uint256_t, std::string, uint256_t, std::string>>, std::string, uint256_t, std::vector<std::tuple<uint256_t, std::string, uint256_t, std::string>>>") {
    const auto eS = ABI::Encoder::encodeData<
      std::tuple<
        std::vector<
          std::tuple<uint256_t, std::string, uint256_t, std::string>
        >,
        std::string,
        uint256_t,
        std::vector<
          std::tuple<uint256_t, std::string, uint256_t, std::string>>>>(
      {
        {
          {81236712741283, "Hello World v1!", 81273854512, "Hello World v2!"},
          {3841267518723, "Hello World v3!", 5189372576123, "Hello World v4!"}
        },
        "Hello World v5!",
        1298318812,
        {
          {81236712741283, "Hello World v1!", 81273854512, "Hello World v2!"},
          {3841267518723, "Hello World v3!", 5189372576123, "Hello World v4!"}
        }
      });
      REQUIRE(Bytes(eS.begin(), eS.begin() + 32) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000020"
      ));
      REQUIRE(Bytes(eS.begin() + 32, eS.begin() + 32 * 2) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000080"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 2, eS.begin() + 32 * 3) == Hex::toBytes(
        "00000000000000000000000000000000000000000000000000000000000002e0"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 3, eS.begin() + 32 * 4) == Hex::toBytes(
        "000000000000000000000000000000000000000000000000000000004d62c5dc"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 4, eS.begin() + 32 * 5) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000320"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 5, eS.begin() + 32 * 6) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000002"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 6, eS.begin() + 32 * 7) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000040"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 7, eS.begin() + 32 * 8) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000140"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 8, eS.begin() + 32 * 9) == Hex::toBytes(
        "000000000000000000000000000000000000000000000000000049e26567d9a3"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 9, eS.begin() + 32 * 10) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000080"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 10, eS.begin() + 32 * 11) == Hex::toBytes(
        "00000000000000000000000000000000000000000000000000000012ec4c9a30"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 11, eS.begin() + 32 * 12) == Hex::toBytes(
        "00000000000000000000000000000000000000000000000000000000000000c0"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 12, eS.begin() + 32 * 13) == Hex::toBytes(
        "000000000000000000000000000000000000000000000000000000000000000f"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 13, eS.begin() + 32 * 14) == Hex::toBytes(
        "48656c6c6f20576f726c64207631210000000000000000000000000000000000"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 14, eS.begin() + 32 * 15) == Hex::toBytes(
        "000000000000000000000000000000000000000000000000000000000000000f"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 15, eS.begin() + 32 * 16) == Hex::toBytes(
        "48656c6c6f20576f726c64207632210000000000000000000000000000000000"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 16, eS.begin() + 32 * 17) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000037e5d62cd03"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 17, eS.begin() + 32 * 18) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000080"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 18, eS.begin() + 32 * 19) == Hex::toBytes(
        "000000000000000000000000000000000000000000000000000004b83eb5817b"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 19, eS.begin() + 32 * 20) == Hex::toBytes(
        "00000000000000000000000000000000000000000000000000000000000000c0"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 20, eS.begin() + 32 * 21) == Hex::toBytes(
        "000000000000000000000000000000000000000000000000000000000000000f"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 21, eS.begin() + 32 * 22) == Hex::toBytes(
        "48656c6c6f20576f726c64207633210000000000000000000000000000000000"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 22, eS.begin() + 32 * 23) == Hex::toBytes(
        "000000000000000000000000000000000000000000000000000000000000000f"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 23, eS.begin() + 32 * 24) == Hex::toBytes(
        "48656c6c6f20576f726c64207634210000000000000000000000000000000000"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 24, eS.begin() + 32 * 25) == Hex::toBytes(
        "000000000000000000000000000000000000000000000000000000000000000f"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 25, eS.begin() + 32 * 26) == Hex::toBytes(
        "48656c6c6f20576f726c64207635210000000000000000000000000000000000"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 26, eS.begin() + 32 * 27) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000002"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 27, eS.begin() + 32 * 28) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000040"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 28, eS.begin() + 32 * 29) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000140"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 29, eS.begin() + 32 * 30) == Hex::toBytes(
        "000000000000000000000000000000000000000000000000000049e26567d9a3"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 30, eS.begin() + 32 * 31) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000080"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 31, eS.begin() + 32 * 32) == Hex::toBytes(
        "00000000000000000000000000000000000000000000000000000012ec4c9a30"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 32, eS.begin() + 32 * 33) == Hex::toBytes(
        "00000000000000000000000000000000000000000000000000000000000000c0"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 33, eS.begin() + 32 * 34) == Hex::toBytes(
        "000000000000000000000000000000000000000000000000000000000000000f"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 34, eS.begin() + 32 * 35) == Hex::toBytes(
        "48656c6c6f20576f726c64207631210000000000000000000000000000000000"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 35, eS.begin() + 32 * 36) == Hex::toBytes(
        "000000000000000000000000000000000000000000000000000000000000000f"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 36, eS.begin() + 32 * 37) == Hex::toBytes(
        "48656c6c6f20576f726c64207632210000000000000000000000000000000000"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 37, eS.begin() + 32 * 38) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000037e5d62cd03"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 38, eS.begin() + 32 * 39) == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000080"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 39, eS.begin() + 32 * 40) == Hex::toBytes(
        "000000000000000000000000000000000000000000000000000004b83eb5817b"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 40, eS.begin() + 32 * 41) == Hex::toBytes(
        "00000000000000000000000000000000000000000000000000000000000000c0"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 41, eS.begin() + 32 * 42) == Hex::toBytes(
        "000000000000000000000000000000000000000000000000000000000000000f"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 42, eS.begin() + 32 * 43) == Hex::toBytes(
        "48656c6c6f20576f726c64207633210000000000000000000000000000000000"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 43, eS.begin() + 32 * 44) == Hex::toBytes(
        "000000000000000000000000000000000000000000000000000000000000000f"
      ));
      REQUIRE(Bytes(eS.begin() + 32 * 44, eS.begin() + 32 * 45) == Hex::toBytes(
        "48656c6c6f20576f726c64207634210000000000000000000000000000000000"
      ));
    }

    SECTION("Encode std::tuple<std::vector<std::tuple<std::vector<std::tuple<uint256_t, std::string, uint256_t, std::string>>,std::string,uint256_t,std::vector<std::tuple<uint256_t, std::tuple<std::string, uint256_t>, std::string>>>>,std::string,std::vector<std::tuple<std::vector<std::tuple<uint256_t, std::tuple<std::string, uint256_t>, std::string>>,std::string,uint256_t,std::vector<std::tuple<uint256_t, std::string, uint256_t, std::string>>>>>"){
      const auto eS = ABI::Encoder::encodeData<std::tuple<
                                           std::vector<
                                             std::tuple<
                                               std::vector<
                                                 std::tuple<uint256_t, std::string, uint256_t, std::string>
                                               >,
                                               std::string,
                                               uint256_t,
                                               std::vector<
                                                 std::tuple<uint256_t, std::tuple<std::string, uint256_t>, std::string>
                                               >
                                             >
                                           >,
                                           std::string,
                                           std::vector<
                                             std::tuple<
                                               std::vector<
                                                 std::tuple<uint256_t, std::tuple<std::string, uint256_t>, std::string>
                                               >,
                                               std::string,
                                               uint256_t,
                                               std::vector<
                                                 std::tuple<uint256_t, std::string, uint256_t, std::string>
                                               >
                                             >
                                           >
                                          >>( {
                                            {
                                              {
                                                {
                                                  {4564563453, "Hello World v1!", 38834534, "Hello World v2!"},
                                                  {4564564534, "Hello World v3!", 4564898948 , "Hello World v4!"}
                                                },
                                                "Hello World v34!",
                                                1298318812,
                                                {
                                                  {334896856, {"Hello World v1!", 34523742}, "Hello World v2!"},
                                                  {52546448, {"Hello World v3!", 56378953}, "Hello World v4!"}
                                                }
                                              },
                                              {
                                                {
                                                  {1235968763, "Hello World v7!", 72727537, "Hello World v8!"},
                                                  {357695652, "Hello World v9!", 9834651, "Hello World v10!"}
                                                },
                                                "Hello World v33!",
                                                38575343,
                                                {
                                                  {3546863423486, {"Hello World v11!", 38412343}, "Hello World v12!"},
                                                  {2579873228, {"Hello World v13!", 35489531}, "Hello World v14!"}
                                                }
                                              }
                                            },
                                            "bbbbbbbbbbbbb",
                                          {
                                            {
                                              {
                                                {45678973455364, {"Hello World v15!", 4537897321}, "Hello World v16!"},
                                                {789564, {"Hello World v17!", 56748923}, "Hello World v18!"}
                                              },
                                              "Hello World v32!",
                                              4567824123,
                                              {
                                                {543245, "Hello World v19!", 3756542341564 , "Hello World v20!"},
                                                {987324387943, "Hello World v21!", 6534654234212, "Hello World v22!"}
                                              }
                                            },
                                            {
                                              {
                                                {74567867456421, {"Hello World v23!", 564523426453413}, "Hello World v24!"},
                                                {246543678546, {"Hello World v25!", 89865243124856}, "Hello World v26!"}
                                              },
                                              "Hello World v31!",
                                              5453212,
                                              {
                                                {789763245678574, "Hello World v27!", 214545978566375, "Hello World v28!"},
                                                {898967565456789, "Hello World v29!", 3215678923489, "Hello World v30!"}
                                              }
                                            }
                                          }
                                        });
      REQUIRE(eS == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000020"
        "0000000000000000000000000000000000000000000000000000000000000060"
        "0000000000000000000000000000000000000000000000000000000000000c40"
        "0000000000000000000000000000000000000000000000000000000000000c80"
        "0000000000000000000000000000000000000000000000000000000000000002"
        "0000000000000000000000000000000000000000000000000000000000000040"
        "0000000000000000000000000000000000000000000000000000000000000600"
        "0000000000000000000000000000000000000000000000000000000000000080"
        "00000000000000000000000000000000000000000000000000000000000002e0"
        "000000000000000000000000000000000000000000000000000000004d62c5dc"
        "0000000000000000000000000000000000000000000000000000000000000320"
        "0000000000000000000000000000000000000000000000000000000000000002"
        "0000000000000000000000000000000000000000000000000000000000000040"
        "0000000000000000000000000000000000000000000000000000000000000140"
        "000000000000000000000000000000000000000000000000000000011011b5fd"
        "0000000000000000000000000000000000000000000000000000000000000080"
        "0000000000000000000000000000000000000000000000000000000002509166"
        "00000000000000000000000000000000000000000000000000000000000000c0"
        "000000000000000000000000000000000000000000000000000000000000000f"
        "48656c6c6f20576f726c64207631210000000000000000000000000000000000"
        "000000000000000000000000000000000000000000000000000000000000000f"
        "48656c6c6f20576f726c64207632210000000000000000000000000000000000"
        "000000000000000000000000000000000000000000000000000000011011ba36"
        "0000000000000000000000000000000000000000000000000000000000000080"
        "000000000000000000000000000000000000000000000000000000011016d484"
        "00000000000000000000000000000000000000000000000000000000000000c0"
        "000000000000000000000000000000000000000000000000000000000000000f"
        "48656c6c6f20576f726c64207633210000000000000000000000000000000000"
        "000000000000000000000000000000000000000000000000000000000000000f"
        "48656c6c6f20576f726c64207634210000000000000000000000000000000000"
        "0000000000000000000000000000000000000000000000000000000000000010"
        "48656c6c6f20576f726c64207633342100000000000000000000000000000000"
        "0000000000000000000000000000000000000000000000000000000000000002"
        "0000000000000000000000000000000000000000000000000000000000000040"
        "0000000000000000000000000000000000000000000000000000000000000160"
        "0000000000000000000000000000000000000000000000000000000013f61ed8"
        "0000000000000000000000000000000000000000000000000000000000000060"
        "00000000000000000000000000000000000000000000000000000000000000e0"
        "0000000000000000000000000000000000000000000000000000000000000040"
        "00000000000000000000000000000000000000000000000000000000020eca5e"
        "000000000000000000000000000000000000000000000000000000000000000f"
        "48656c6c6f20576f726c64207631210000000000000000000000000000000000"
        "000000000000000000000000000000000000000000000000000000000000000f"
        "48656c6c6f20576f726c64207632210000000000000000000000000000000000"
        "000000000000000000000000000000000000000000000000000000000321cb90"
        "0000000000000000000000000000000000000000000000000000000000000060"
        "00000000000000000000000000000000000000000000000000000000000000e0"
        "0000000000000000000000000000000000000000000000000000000000000040"
        "00000000000000000000000000000000000000000000000000000000035c4649"
        "000000000000000000000000000000000000000000000000000000000000000f"
        "48656c6c6f20576f726c64207633210000000000000000000000000000000000"
        "000000000000000000000000000000000000000000000000000000000000000f"
        "48656c6c6f20576f726c64207634210000000000000000000000000000000000"
        "0000000000000000000000000000000000000000000000000000000000000080"
        "00000000000000000000000000000000000000000000000000000000000002e0"
        "00000000000000000000000000000000000000000000000000000000024c9cef"
        "0000000000000000000000000000000000000000000000000000000000000320"
        "0000000000000000000000000000000000000000000000000000000000000002"
        "0000000000000000000000000000000000000000000000000000000000000040"
        "0000000000000000000000000000000000000000000000000000000000000140"
        "0000000000000000000000000000000000000000000000000000000049ab62fb"
        "0000000000000000000000000000000000000000000000000000000000000080"
        "000000000000000000000000000000000000000000000000000000000455bbf1"
        "00000000000000000000000000000000000000000000000000000000000000c0"
        "000000000000000000000000000000000000000000000000000000000000000f"
        "48656c6c6f20576f726c64207637210000000000000000000000000000000000"
        "000000000000000000000000000000000000000000000000000000000000000f"
        "48656c6c6f20576f726c64207638210000000000000000000000000000000000"
        "00000000000000000000000000000000000000000000000000000000155200a4"
        "0000000000000000000000000000000000000000000000000000000000000080"
        "000000000000000000000000000000000000000000000000000000000096109b"
        "00000000000000000000000000000000000000000000000000000000000000c0"
        "000000000000000000000000000000000000000000000000000000000000000f"
        "48656c6c6f20576f726c64207639210000000000000000000000000000000000"
        "0000000000000000000000000000000000000000000000000000000000000010"
        "48656c6c6f20576f726c64207631302100000000000000000000000000000000"
        "0000000000000000000000000000000000000000000000000000000000000010"
        "48656c6c6f20576f726c64207633332100000000000000000000000000000000"
        "0000000000000000000000000000000000000000000000000000000000000002"
        "0000000000000000000000000000000000000000000000000000000000000040"
        "0000000000000000000000000000000000000000000000000000000000000160"
        "00000000000000000000000000000000000000000000000000000339d188cffe"
        "0000000000000000000000000000000000000000000000000000000000000060"
        "00000000000000000000000000000000000000000000000000000000000000e0"
        "0000000000000000000000000000000000000000000000000000000000000040"
        "00000000000000000000000000000000000000000000000000000000024a2037"
        "0000000000000000000000000000000000000000000000000000000000000010"
        "48656c6c6f20576f726c64207631312100000000000000000000000000000000"
        "0000000000000000000000000000000000000000000000000000000000000010"
        "48656c6c6f20576f726c64207631322100000000000000000000000000000000"
        "0000000000000000000000000000000000000000000000000000000099c5bdcc"
        "0000000000000000000000000000000000000000000000000000000000000060"
        "00000000000000000000000000000000000000000000000000000000000000e0"
        "0000000000000000000000000000000000000000000000000000000000000040"
        "00000000000000000000000000000000000000000000000000000000021d86fb"
        "0000000000000000000000000000000000000000000000000000000000000010"
        "48656c6c6f20576f726c64207631332100000000000000000000000000000000"
        "0000000000000000000000000000000000000000000000000000000000000010"
        "48656c6c6f20576f726c64207631342100000000000000000000000000000000"
        "000000000000000000000000000000000000000000000000000000000000000d"
        "6262626262626262626262626200000000000000000000000000000000000000"
        "0000000000000000000000000000000000000000000000000000000000000002"
        "0000000000000000000000000000000000000000000000000000000000000040"
        "0000000000000000000000000000000000000000000000000000000000000600"
        "0000000000000000000000000000000000000000000000000000000000000080"
        "0000000000000000000000000000000000000000000000000000000000000320"
        "00000000000000000000000000000000000000000000000000000001104376fb"
        "0000000000000000000000000000000000000000000000000000000000000360"
        "0000000000000000000000000000000000000000000000000000000000000002"
        "0000000000000000000000000000000000000000000000000000000000000040"
        "0000000000000000000000000000000000000000000000000000000000000160"
        "0000000000000000000000000000000000000000000000000000298b76fc8c04"
        "0000000000000000000000000000000000000000000000000000000000000060"
        "00000000000000000000000000000000000000000000000000000000000000e0"
        "0000000000000000000000000000000000000000000000000000000000000040"
        "000000000000000000000000000000000000000000000000000000010e7ad169"
        "0000000000000000000000000000000000000000000000000000000000000010"
        "48656c6c6f20576f726c64207631352100000000000000000000000000000000"
        "0000000000000000000000000000000000000000000000000000000000000010"
        "48656c6c6f20576f726c64207631362100000000000000000000000000000000"
        "00000000000000000000000000000000000000000000000000000000000c0c3c"
        "0000000000000000000000000000000000000000000000000000000000000060"
        "00000000000000000000000000000000000000000000000000000000000000e0"
        "0000000000000000000000000000000000000000000000000000000000000040"
        "000000000000000000000000000000000000000000000000000000000361eb7b"
        "0000000000000000000000000000000000000000000000000000000000000010"
        "48656c6c6f20576f726c64207631372100000000000000000000000000000000"
        "0000000000000000000000000000000000000000000000000000000000000010"
        "48656c6c6f20576f726c64207631382100000000000000000000000000000000"
        "0000000000000000000000000000000000000000000000000000000000000010"
        "48656c6c6f20576f726c64207633322100000000000000000000000000000000"
        "0000000000000000000000000000000000000000000000000000000000000002"
        "0000000000000000000000000000000000000000000000000000000000000040"
        "0000000000000000000000000000000000000000000000000000000000000140"
        "0000000000000000000000000000000000000000000000000000000000084a0d"
        "0000000000000000000000000000000000000000000000000000000000000080"
        "0000000000000000000000000000000000000000000000000000036aa35f31bc"
        "00000000000000000000000000000000000000000000000000000000000000c0"
        "0000000000000000000000000000000000000000000000000000000000000010"
        "48656c6c6f20576f726c64207631392100000000000000000000000000000000"
        "0000000000000000000000000000000000000000000000000000000000000010"
        "48656c6c6f20576f726c64207632302100000000000000000000000000000000"
        "000000000000000000000000000000000000000000000000000000e5e11e9267"
        "0000000000000000000000000000000000000000000000000000000000000080"
        "000000000000000000000000000000000000000000000000000005f177be8e64"
        "00000000000000000000000000000000000000000000000000000000000000c0"
        "0000000000000000000000000000000000000000000000000000000000000010"
        "48656c6c6f20576f726c64207632312100000000000000000000000000000000"
        "0000000000000000000000000000000000000000000000000000000000000010"
        "48656c6c6f20576f726c64207632322100000000000000000000000000000000"
        "0000000000000000000000000000000000000000000000000000000000000080"
        "0000000000000000000000000000000000000000000000000000000000000320"
        "000000000000000000000000000000000000000000000000000000000053359c"
        "0000000000000000000000000000000000000000000000000000000000000360"
        "0000000000000000000000000000000000000000000000000000000000000002"
        "0000000000000000000000000000000000000000000000000000000000000040"
        "0000000000000000000000000000000000000000000000000000000000000160"
        "000000000000000000000000000000000000000000000000000043d1af405ba5"
        "0000000000000000000000000000000000000000000000000000000000000060"
        "00000000000000000000000000000000000000000000000000000000000000e0"
        "0000000000000000000000000000000000000000000000000000000000000040"
        "0000000000000000000000000000000000000000000000000002016e5a4d17a5"
        "0000000000000000000000000000000000000000000000000000000000000010"
        "48656c6c6f20576f726c64207632332100000000000000000000000000000000"
        "0000000000000000000000000000000000000000000000000000000000000010"
        "48656c6c6f20576f726c64207632342100000000000000000000000000000000"
        "000000000000000000000000000000000000000000000000000000396725fc52"
        "0000000000000000000000000000000000000000000000000000000000000060"
        "00000000000000000000000000000000000000000000000000000000000000e0"
        "0000000000000000000000000000000000000000000000000000000000000040"
        "000000000000000000000000000000000000000000000000000051bb61e4e478"
        "0000000000000000000000000000000000000000000000000000000000000010"
        "48656c6c6f20576f726c64207632352100000000000000000000000000000000"
        "0000000000000000000000000000000000000000000000000000000000000010"
        "48656c6c6f20576f726c64207632362100000000000000000000000000000000"
        "0000000000000000000000000000000000000000000000000000000000000010"
        "48656c6c6f20576f726c64207633312100000000000000000000000000000000"
        "0000000000000000000000000000000000000000000000000000000000000002"
        "0000000000000000000000000000000000000000000000000000000000000040"
        "0000000000000000000000000000000000000000000000000000000000000140"
        "0000000000000000000000000000000000000000000000000002ce4915b71fee"
        "0000000000000000000000000000000000000000000000000000000000000080"
        "0000000000000000000000000000000000000000000000000000c320e0d726e7"
        "00000000000000000000000000000000000000000000000000000000000000c0"
        "0000000000000000000000000000000000000000000000000000000000000010"
        "48656c6c6f20576f726c64207632372100000000000000000000000000000000"
        "0000000000000000000000000000000000000000000000000000000000000010"
        "48656c6c6f20576f726c64207632382100000000000000000000000000000000"
        "0000000000000000000000000000000000000000000000000003319b32675595"
        "0000000000000000000000000000000000000000000000000000000000000080"
        "000000000000000000000000000000000000000000000000000002ecb56662e1"
        "00000000000000000000000000000000000000000000000000000000000000c0"
        "0000000000000000000000000000000000000000000000000000000000000010"
        "48656c6c6f20576f726c64207632392100000000000000000000000000000000"
        "0000000000000000000000000000000000000000000000000000000000000010"
        "48656c6c6f20576f726c64207633302100000000000000000000000000000000"
      ));
    }

    SECTION("Encode std::tuple<std::vector<std::tuple<uint256_t, uint256_t, uint256_t, uint256_t>>, std::string, uint256_t, std::vector<std::tuple<std::string, std::tuple<uint256_t, uint256_t>, std::string>>>") {
    const auto eS = ABI::Encoder::encodeData<
                                    std::tuple<
                                      std::vector<
                                        std::tuple<uint256_t, uint256_t, uint256_t, uint256_t>
                                      >,
                                      std::string,
                                      uint256_t,
                                      std::vector<
                                        std::tuple<std::string, std::tuple<uint256_t, uint256_t>, std::string>>
                                      >
                                    >({
         {
           {56231324512, 756345627234, 752345265276, 5623452352363},
           {76345236, 645164352134, 7653453246734, 564623412351}
         },
         "Hello World v9!",
         1231987517237125123,
         {
           {"Hello World v1!",{ 32984187651723, 82984751723315}, "Hello World v2!"},
           {"Hello World v3!",{ 193568712831546, 5156713223 }, "Hello World v4!"}
         }
      });
      REQUIRE(eS == Hex::toBytes(
        "0000000000000000000000000000000000000000000000000000000000000020"
        "0000000000000000000000000000000000000000000000000000000000000080"
        "00000000000000000000000000000000000000000000000000000000000001a0"
        "0000000000000000000000000000000000000000000000001118e61e9ca26003"
        "00000000000000000000000000000000000000000000000000000000000001e0"
        "0000000000000000000000000000000000000000000000000000000000000002"
        "0000000000000000000000000000000000000000000000000000000d17a5eb60"
        "000000000000000000000000000000000000000000000000000000b019b66262"
        "000000000000000000000000000000000000000000000000000000af2b45b47c"
        "0000000000000000000000000000000000000000000000000000051d4fe13f6b"
        "00000000000000000000000000000000000000000000000000000000048cef94"
        "0000000000000000000000000000000000000000000000000000009636cac286"
        "000000000000000000000000000000000000000000000000000006f5f55cb10e"
        "00000000000000000000000000000000000000000000000000000083762d8c7f"
        "000000000000000000000000000000000000000000000000000000000000000f"
        "48656c6c6f20576f726c64207639210000000000000000000000000000000000"
        "0000000000000000000000000000000000000000000000000000000000000002"
        "0000000000000000000000000000000000000000000000000000000000000040"
        "0000000000000000000000000000000000000000000000000000000000000140"
        "0000000000000000000000000000000000000000000000000000000000000080"
        "00000000000000000000000000000000000000000000000000001dffbac9c68b"
        "00000000000000000000000000000000000000000000000000004b7964a5f333"
        "00000000000000000000000000000000000000000000000000000000000000c0"
        "000000000000000000000000000000000000000000000000000000000000000f"
        "48656c6c6f20576f726c64207631210000000000000000000000000000000000"
        "000000000000000000000000000000000000000000000000000000000000000f"
        "48656c6c6f20576f726c64207632210000000000000000000000000000000000"
        "0000000000000000000000000000000000000000000000000000000000000080"
        "0000000000000000000000000000000000000000000000000000b00cba5e323a"
        "00000000000000000000000000000000000000000000000000000001335d3307"
        "00000000000000000000000000000000000000000000000000000000000000c0"
        "000000000000000000000000000000000000000000000000000000000000000f"
        "48656c6c6f20576f726c64207633210000000000000000000000000000000000"
        "000000000000000000000000000000000000000000000000000000000000000f"
        "48656c6c6f20576f726c64207634210000000000000000000000000000000000"
      ));
    }

    SECTION("Encode std::vector<std::vector<std::tuple<uint256_t,uint256_t,uint256_t>>>") {
      const auto eS = ABI::Encoder::encodeData<
                                                std::vector<
                                                  std::vector<
                                                    std::tuple<uint256_t,uint256_t,uint256_t>
                                                    >
                                                  >
                                                >({
                                                  {{1,2,3},{4,5,6},{7,8,9}},
                                                  {{10,11,12},{13,14,15},{16,17,18}},
                                                  {{19,20,21},{22,23,24},{25,26,27}}
      });

      REQUIRE(eS == Hex::toBytes("0000000000000000000000000000000000000000000000000000000000000020"
                                 "0000000000000000000000000000000000000000000000000000000000000003"
                                 "0000000000000000000000000000000000000000000000000000000000000060"
                                 "00000000000000000000000000000000000000000000000000000000000001a0"
                                 "00000000000000000000000000000000000000000000000000000000000002e0"
                                 "0000000000000000000000000000000000000000000000000000000000000003"
                                 "0000000000000000000000000000000000000000000000000000000000000001"
                                 "0000000000000000000000000000000000000000000000000000000000000002"
                                 "0000000000000000000000000000000000000000000000000000000000000003"
                                 "0000000000000000000000000000000000000000000000000000000000000004"
                                 "0000000000000000000000000000000000000000000000000000000000000005"
                                 "0000000000000000000000000000000000000000000000000000000000000006"
                                 "0000000000000000000000000000000000000000000000000000000000000007"
                                 "0000000000000000000000000000000000000000000000000000000000000008"
                                 "0000000000000000000000000000000000000000000000000000000000000009"
                                 "0000000000000000000000000000000000000000000000000000000000000003"
                                 "000000000000000000000000000000000000000000000000000000000000000a"
                                 "000000000000000000000000000000000000000000000000000000000000000b"
                                 "000000000000000000000000000000000000000000000000000000000000000c"
                                 "000000000000000000000000000000000000000000000000000000000000000d"
                                 "000000000000000000000000000000000000000000000000000000000000000e"
                                 "000000000000000000000000000000000000000000000000000000000000000f"
                                 "0000000000000000000000000000000000000000000000000000000000000010"
                                 "0000000000000000000000000000000000000000000000000000000000000011"
                                 "0000000000000000000000000000000000000000000000000000000000000012"
                                 "0000000000000000000000000000000000000000000000000000000000000003"
                                 "0000000000000000000000000000000000000000000000000000000000000013"
                                 "0000000000000000000000000000000000000000000000000000000000000014"
                                 "0000000000000000000000000000000000000000000000000000000000000015"
                                 "0000000000000000000000000000000000000000000000000000000000000016"
                                 "0000000000000000000000000000000000000000000000000000000000000017"
                                 "0000000000000000000000000000000000000000000000000000000000000018"
                                 "0000000000000000000000000000000000000000000000000000000000000019"
                                 "000000000000000000000000000000000000000000000000000000000000001a"
                                 "000000000000000000000000000000000000000000000000000000000000001b"));
    }

    SECTION("Decode Uint256 (Array)") {
      Bytes ABI = Hex::toBytes("0x"
        "0000000000000000000000000000000000000000000000000000000000000020"
        "0000000000000000000000000000000000000000000000000000000000000003"
        "0000000000000000000000000000000000000000000000002017594d84130397"
        "0000000000000000000000000000000000000000000000000000027cae776d75"
        "00000000000000000000000000000000000000000000000000016201a9fce5dd"
      );

      auto decodedData = ABI::Decoder::decodeData<std::vector<uint256_t>>(ABI);
      std::vector<uint256_t> decodedVector = std::get<0>(decodedData);

      REQUIRE(decodedVector[0] == uint256_t(2312415123141231511));
      REQUIRE(decodedVector[1] == uint256_t(2734526262645));
      REQUIRE(decodedVector[2] == uint256_t(389234263123421));

      // For coverage
      Bytes ABIWrongSize = Hex::toBytes("0x"
        "0000000000000000000000000000000000000000000000000000000000000020"
        "0000000000000000000000000000000000000000000000000000000000000003"
        "0000000000000000000000000000000000000000000000002017594d84130397"
        "0000000000000000000000000000000000000000000000000000027cae776d75"
        "00000000000000000000000000000000000000000000000000016201a9fce5" // missing last "dd"
      );

      REQUIRE_THROWS(ABI::Decoder::decodeData<std::vector<uint256_t>>(ABIWrongSize));
    }

    SECTION("Decode Int256") {
      Bytes ABI = Hex::toBytes("0x"
        "fffffffffffffffffdaf1854f62f44391b682b86c9106cac85300e6931c0f52e"
      );

      auto decodedData = ABI::Decoder::decodeData<int256_t>(ABI);
      int256_t decodedInt = std::get<0>(decodedData);

      REQUIRE(decodedInt == int256_t("-56789012345678901234567890123456789012345678901234567890"));

      // For coverage
      Bytes ABIWrongSize = Hex::toBytes("0x"
        "fffffffffffffffffdaf1854f62f44391b682b86c9106cac85300e6931c0f5" // missing last "2e"
      );

      REQUIRE_THROWS(ABI::Decoder::decodeData<int256_t>(ABIWrongSize));
    }

    SECTION("Decode Int256 (Array)") {
      Bytes ABI = Hex::toBytes("0x"
      "0000000000000000000000000000000000000000000000000000000000000020"
       "0000000000000000000000000000000000000000000000000000000000000004"
       "ffffffffffffffec550afb1b43e19de8c0785bc873c84b6373300e6931c0f52e0000000000000013aaf504e4bc1e62173f87a4378c37b49c8ccff196ce3f0ad2fffffffffffffffffdaf1854f62f44391b682b86c9106cac85300e6931c0f52e00000000000000000250e7ab09d0bbc6e497d47936ef93537acff196ce3f0ad2"
      );

    auto decodedData = ABI::Decoder::decodeData<std::vector<int256_t>>(ABI);
    std::vector<int256_t> decodedVector = std::get<0>(decodedData);

    REQUIRE(decodedVector[0] == int256_t("-123456789012345678901234567890123456789012345678901234567890"));
    REQUIRE(decodedVector[1] == int256_t("123456789012345678901234567890123456789012345678901234567890"));
    REQUIRE(decodedVector[2] == int256_t("-56789012345678901234567890123456789012345678901234567890"));
    REQUIRE(decodedVector[3] == int256_t("56789012345678901234567890123456789012345678901234567890"));
  }

    SECTION("Decode Address (Array)") {
      Bytes ABI = Hex::toBytes("0x"
        "0000000000000000000000000000000000000000000000000000000000000020"
        "0000000000000000000000000000000000000000000000000000000000000003"
        "0000000000000000000000005b38da6a701c568545dcfcb03fcb875f56beddc4"
        "000000000000000000000000ab8483f64d9c6d1ecf9b849ae677dd3315835cb2"
        "0000000000000000000000004b20993bc481177ec7e8f571cecae8a9e22c02db"
      );

    auto decodedData = ABI::Decoder::decodeData<std::vector<Address>>(ABI);
    std::vector<Address> decodedVector = std::get<0>(decodedData);

      REQUIRE(decodedVector[0] == Address(Hex::toBytes("0x5B38Da6a701c568545dCfcB03FcB875f56beddC4")));
      REQUIRE(decodedVector[1] == Address(Hex::toBytes("0xAb8483F64d9C6d1EcF9b849Ae677dD3315835cb2")));
      REQUIRE(decodedVector[2] == Address(Hex::toBytes("0x4B20993Bc481177ec7E8f571ceCaE8A9e22C02db")));
    }

    SECTION("Decode Bool (Array)") {
      Bytes ABI = Hex::toBytes("0x"
        "0000000000000000000000000000000000000000000000000000000000000020"
        "0000000000000000000000000000000000000000000000000000000000000003"
        "0000000000000000000000000000000000000000000000000000000000000001"
        "0000000000000000000000000000000000000000000000000000000000000000"
        "0000000000000000000000000000000000000000000000000000000000000001"
      );

    auto decodedData = ABI::Decoder::decodeData<std::vector<bool>>(ABI);
    std::vector<bool> decodedVector = std::get<0>(decodedData);

    REQUIRE(decodedVector[0] == true);
    REQUIRE(decodedVector[1] == false);
    REQUIRE(decodedVector[2] == true);
  }

    SECTION("Decode Bytes (Single)") {
      Bytes ABI = Hex::toBytes("0x"
        "0000000000000000000000000000000000000000000000000000000000000020"
        "0000000000000000000000000000000000000000000000000000000000000004"
        "0adf1f1a00000000000000000000000000000000000000000000000000000000"
      );

      auto decodedData = ABI::Decoder::decodeData<Bytes>(ABI);
      Bytes decodedBytes = std::get<0>(decodedData);

    REQUIRE(decodedBytes == Hex::toBytes("0x0adf1f1a"));
  }

    SECTION("Decode Bytes (Array)") {
      Bytes ABI = Hex::toBytes("0x"
        "0000000000000000000000000000000000000000000000000000000000000020"
        "0000000000000000000000000000000000000000000000000000000000000004"
        "0000000000000000000000000000000000000000000000000000000000000080"
        "00000000000000000000000000000000000000000000000000000000000000c0"
        "0000000000000000000000000000000000000000000000000000000000000100"
        "0000000000000000000000000000000000000000000000000000000000000140"
        "0000000000000000000000000000000000000000000000000000000000000004"
        "0adf1f1a00000000000000000000000000000000000000000000000000000000"
        "0000000000000000000000000000000000000000000000000000000000000004"
        "fffadcba00000000000000000000000000000000000000000000000000000000"
        "0000000000000000000000000000000000000000000000000000000000000006"
        "0113ffedc2310000000000000000000000000000000000000000000000000000"
        "0000000000000000000000000000000000000000000000000000000000000002"
        "aaaa000000000000000000000000000000000000000000000000000000000000"
      );

      auto decodedData = ABI::Decoder::decodeData<std::vector<Bytes>>(ABI);
      std::vector<Bytes> decodedBytes = std::get<0>(decodedData);

    REQUIRE(decodedBytes[0] == Hex::toBytes("0x0adf1f1a"));
    REQUIRE(decodedBytes[1] == Hex::toBytes("0xfffadcba"));
    REQUIRE(decodedBytes[2] == Hex::toBytes("0x0113ffedc231"));
    REQUIRE(decodedBytes[3] == Hex::toBytes("0xaaaa"));
  }

    SECTION("Decode String (Single)") {
      Bytes ABI = Hex::toBytes("0x"
        "0000000000000000000000000000000000000000000000000000000000000020"
        "000000000000000000000000000000000000000000000000000000000000000e"
        "5468697320697320612074657374000000000000000000000000000000000000"
      );

    auto decodedData = ABI::Decoder::decodeData<std::string>(ABI);
    std::string decodedString = std::get<0>(decodedData);

    REQUIRE(decodedString == "This is a test");
  }

    SECTION("Decode String (Array)") {
      Bytes ABI = Hex::toBytes("0x"
        "0000000000000000000000000000000000000000000000000000000000000020"
        "0000000000000000000000000000000000000000000000000000000000000004"
        "0000000000000000000000000000000000000000000000000000000000000080"
        "00000000000000000000000000000000000000000000000000000000000000c0"
        "0000000000000000000000000000000000000000000000000000000000000100"
        "0000000000000000000000000000000000000000000000000000000000000140"
        "0000000000000000000000000000000000000000000000000000000000000016"
        "5468697320697320746865206669727374207465737400000000000000000000"
        "0000000000000000000000000000000000000000000000000000000000000017"
        "5468697320697320746865207365636f6e642074657374000000000000000000"
        "0000000000000000000000000000000000000000000000000000000000000016"
        "5468697320697320746865207468697264207465737400000000000000000000"
        "0000000000000000000000000000000000000000000000000000000000000016"
        "546869732069732074686520666f727468207465737400000000000000000000"
      );

    auto decodedData = ABI::Decoder::decodeData<std::vector<std::string>>(ABI);
    std::vector<std::string> decodedString = std::get<0>(decodedData);

    REQUIRE(decodedString[0] == "This is the first test");
    REQUIRE(decodedString[1] == "This is the second test");
    REQUIRE(decodedString[2] == "This is the third test");
    REQUIRE(decodedString[3] == "This is the forth test"); // Someone fix this typo for the
                                                // love of the cosmos
  }

    SECTION("Decode Bytes (Array) + String (Array)") {
      Bytes ABI = Hex::toBytes("0x"
                               "0000000000000000000000000000000000000000000000000000000000000040"
                               "00000000000000000000000000000000000000000000000000000000000001e0"
                               "0000000000000000000000000000000000000000000000000000000000000004"
                               "0000000000000000000000000000000000000000000000000000000000000080"
                               "00000000000000000000000000000000000000000000000000000000000000c0"
                               "0000000000000000000000000000000000000000000000000000000000000100"
                               "0000000000000000000000000000000000000000000000000000000000000140"
                               "0000000000000000000000000000000000000000000000000000000000000016"
                               "5468697320697320746865206669727374207465737400000000000000000000"
                               "0000000000000000000000000000000000000000000000000000000000000017"
                               "5468697320697320746865207365636f6e642074657374000000000000000000"
                               "0000000000000000000000000000000000000000000000000000000000000016"
                               "5468697320697320746865207468697264207465737400000000000000000000"
                               "0000000000000000000000000000000000000000000000000000000000000016"
                               "546869732069732074686520666f727468207465737400000000000000000000"
                               "0000000000000000000000000000000000000000000000000000000000000004"
                               "0000000000000000000000000000000000000000000000000000000000000080"
                               "00000000000000000000000000000000000000000000000000000000000000c0"
                               "0000000000000000000000000000000000000000000000000000000000000100"
                               "0000000000000000000000000000000000000000000000000000000000000140"
                               "0000000000000000000000000000000000000000000000000000000000000004"
                               "0adf1f1a00000000000000000000000000000000000000000000000000000000"
                               "0000000000000000000000000000000000000000000000000000000000000004"
                               "fffadcba00000000000000000000000000000000000000000000000000000000"
                               "0000000000000000000000000000000000000000000000000000000000000006"
                               "0113ffedc2310000000000000000000000000000000000000000000000000000"
                               "0000000000000000000000000000000000000000000000000000000000000002"
                               "aaaa000000000000000000000000000000000000000000000000000000000000"
      );

      auto decodedData = ABI::Decoder::decodeData<std::vector<std::string>, std::vector<Bytes>>(ABI);
      std::vector<std::string> decodedString = std::get<0>(decodedData);
      std::vector<Bytes> decodedBytes = std::get<1>(decodedData);

      REQUIRE(decodedString[0] == "This is the first test");
      REQUIRE(decodedString[1] == "This is the second test");
      REQUIRE(decodedString[2] == "This is the third test");
      REQUIRE(decodedString[3] ==
              "This is the forth test");
      REQUIRE(decodedBytes[0] == Hex::toBytes("0x0adf1f1a"));
      REQUIRE(decodedBytes[1] == Hex::toBytes("0xfffadcba"));
      REQUIRE(decodedBytes[2] == Hex::toBytes("0x0113ffedc231"));
      REQUIRE(decodedBytes[3] == Hex::toBytes("0xaaaa"));
    }

    SECTION("Decode Encode std::tuple<uint256_t,uint256_t,uint256_t>") {
      const Bytes ABI = Hex::toBytes("0000000000000000000000000000000000000000000000002017594d84130397"
                                     "0000000000000000000000000000000000000000000000000000027cae776d75"
                                     "00000000000000000000000000000000000000000000000000016201a9fce5dd");

      // Expected = { 2312415123141231511, 2734526262645, 389234263123421 }
      auto decodedData = ABI::Decoder::decodeData<std::tuple<uint256_t, uint256_t, uint256_t>>(ABI);
      // The internal tuple is nested in a std::tuple
      auto decodedTuple = std::get<0>(decodedData);
      REQUIRE(std::get<0>(decodedTuple) == uint256_t(2312415123141231511));
      REQUIRE(std::get<1>(decodedTuple) == uint256_t(2734526262645));
      REQUIRE(std::get<2>(decodedTuple) == uint256_t(389234263123421));
    }

    SECTION("Decode std::tuple<uint256_t, std::tuple<uint256_t, uint256_t>, uint256_t, uint256_t>") {
      const Bytes ABI = Hex::toBytes("0000000000000000000000000000000000000000000000000000117f53a360a5"
                                     "00000000000000000000000000000000000000000000000000000016d8c09adb"
                                     "0000000000000000000000000000000000000000000000000000000248e2806e"
                                     "00000000000000000000000000000000000000000000000000000012ec4c9a30"
                                     "00000000000000000000000000000000000000000000000000009d3080a27f5c");

      // Expected = { 19238561734821, { 98125781723,9812738158 } , 81273854512, 172831642124124}
      auto decodedData = ABI::Decoder::decodeData<std::tuple<uint256_t, std::tuple<uint256_t, uint256_t>, uint256_t, uint256_t>>(ABI);
      // The internal tuple is nested in a std::tuple
      auto decodedTuple = std::get<0>(decodedData);
      REQUIRE(std::get<0>(decodedTuple) == uint256_t(19238561734821));
      REQUIRE(std::get<0>(std::get<1>(decodedTuple)) == uint256_t(98125781723));
      REQUIRE(std::get<1>(std::get<1>(decodedTuple)) == uint256_t(9812738158));
      REQUIRE(std::get<2>(decodedTuple) == uint256_t(81273854512));
      REQUIRE(std::get<3>(decodedTuple) == uint256_t(172831642124124));
    }

    SECTION("Decode std::tuple<uint256_t, std::string, uint256_t, std::string>") {
      const Bytes ABI = Hex::toBytes("0000000000000000000000000000000000000000000000000000000000000020"
                                     "000000000000000000000000000000000000000000000000000049e26567d9a3"
                                     "0000000000000000000000000000000000000000000000000000000000000080"
                                     "00000000000000000000000000000000000000000000000000000012ec4c9a30"
                                     "00000000000000000000000000000000000000000000000000000000000000c0"
                                     "000000000000000000000000000000000000000000000000000000000000000f"
                                     "48656c6c6f20576f726c64207631210000000000000000000000000000000000"
                                     "000000000000000000000000000000000000000000000000000000000000000f"
                                     "48656c6c6f20576f726c64207632210000000000000000000000000000000000");
      // Expected = {81236712741283, "Hello World v1!", 81273854512, "Hello World v2!"}
      auto decodedData = ABI::Decoder::decodeData<std::tuple<uint256_t, std::string, uint256_t, std::string>>(ABI);
      auto decodedTuple = std::get<0>(decodedData);
      REQUIRE(std::get<0>(decodedTuple) == uint256_t(81236712741283));
      REQUIRE(std::get<1>(decodedTuple) == "Hello World v1!");
      REQUIRE(std::get<2>(decodedTuple) == uint256_t(81273854512));
      REQUIRE(std::get<3>(decodedTuple) == "Hello World v2!");
    }

    SECTION("Decode std::vector<std::tuple<uint256_t, std::string, uint256_t, std::string>>") {
      const Bytes ABI = Hex::toBytes("0000000000000000000000000000000000000000000000000000000000000020"
                                     "0000000000000000000000000000000000000000000000000000000000000002"
                                     "0000000000000000000000000000000000000000000000000000000000000040"
                                     "0000000000000000000000000000000000000000000000000000000000000140"
                                     "000000000000000000000000000000000000000000000000000049e26567d9a3"
                                     "0000000000000000000000000000000000000000000000000000000000000080"
                                     "00000000000000000000000000000000000000000000000000000012ec4c9a30"
                                     "00000000000000000000000000000000000000000000000000000000000000c0"
                                     "000000000000000000000000000000000000000000000000000000000000000f"
                                     "48656c6c6f20576f726c64207631210000000000000000000000000000000000"
                                     "000000000000000000000000000000000000000000000000000000000000000f"
                                     "48656c6c6f20576f726c64207632210000000000000000000000000000000000"
                                     "0000000000000000000000000000000000000000000000000000037e5d62cd03"
                                     "0000000000000000000000000000000000000000000000000000000000000080"
                                     "000000000000000000000000000000000000000000000000000004b83eb5817b"
                                     "00000000000000000000000000000000000000000000000000000000000000c0"
                                     "000000000000000000000000000000000000000000000000000000000000000f"
                                     "48656c6c6f20576f726c64207633210000000000000000000000000000000000"
                                     "000000000000000000000000000000000000000000000000000000000000000f"
                                     "48656c6c6f20576f726c64207634210000000000000000000000000000000000");

      // Expected = {
      //              { 81236712741283, "Hello World v1!", 81273854512, "Hello World v2!"},
      //              { 3841267518723, "Hello World v3!", 5189372576123, "Hello World v4!"}
      //            }
      auto decodedData = ABI::Decoder::decodeData<std::vector<std::tuple<uint256_t, std::string, uint256_t, std::string>>>(ABI);
      auto decodedVector = std::get<0>(decodedData);
      REQUIRE(std::get<0>(decodedVector[0]) == uint256_t(81236712741283));
      REQUIRE(std::get<1>(decodedVector[0]) == "Hello World v1!");
      REQUIRE(std::get<2>(decodedVector[0]) == uint256_t(81273854512));
      REQUIRE(std::get<3>(decodedVector[0]) == "Hello World v2!");
      REQUIRE(std::get<0>(decodedVector[1]) == uint256_t(3841267518723));
      REQUIRE(std::get<1>(decodedVector[1]) == "Hello World v3!");
      REQUIRE(std::get<2>(decodedVector[1]) == uint256_t(5189372576123));
      REQUIRE(std::get<3>(decodedVector[1]) == "Hello World v4!");
    }

    SECTION("Decode std::tuple<std::vector<std::tuple<uint256_t, std::string, uint256_t, std::string>>, std::string, uint256_t, std::vector<std::tuple<uint256_t, std::string, uint256_t, std::string>>>") {
      auto ABI = Hex::toBytes("0000000000000000000000000000000000000000000000000000000000000020"
                                       "0000000000000000000000000000000000000000000000000000000000000080"
                                       "00000000000000000000000000000000000000000000000000000000000002e0"
                                       "000000000000000000000000000000000000000000000000000000004d62c5dc"
                                       "0000000000000000000000000000000000000000000000000000000000000320"
                                       "0000000000000000000000000000000000000000000000000000000000000002"
                                       "0000000000000000000000000000000000000000000000000000000000000040"
                                       "0000000000000000000000000000000000000000000000000000000000000140"
                                       "000000000000000000000000000000000000000000000000000049e26567d9a3"
                                       "0000000000000000000000000000000000000000000000000000000000000080"
                                       "00000000000000000000000000000000000000000000000000000012ec4c9a30"
                                       "00000000000000000000000000000000000000000000000000000000000000c0"
                                       "000000000000000000000000000000000000000000000000000000000000000f"
                                       "48656c6c6f20576f726c64207631210000000000000000000000000000000000"
                                       "000000000000000000000000000000000000000000000000000000000000000f"
                                       "48656c6c6f20576f726c64207632210000000000000000000000000000000000"
                                       "0000000000000000000000000000000000000000000000000000037e5d62cd03"
                                       "0000000000000000000000000000000000000000000000000000000000000080"
                                       "000000000000000000000000000000000000000000000000000004b83eb5817b"
                                       "00000000000000000000000000000000000000000000000000000000000000c0"
                                       "000000000000000000000000000000000000000000000000000000000000000f"
                                       "48656c6c6f20576f726c64207633210000000000000000000000000000000000"
                                       "000000000000000000000000000000000000000000000000000000000000000f"
                                       "48656c6c6f20576f726c64207634210000000000000000000000000000000000"
                                       "000000000000000000000000000000000000000000000000000000000000000f"
                                       "48656c6c6f20576f726c64207635210000000000000000000000000000000000"
                                       "0000000000000000000000000000000000000000000000000000000000000002"
                                       "0000000000000000000000000000000000000000000000000000000000000040"
                                       "0000000000000000000000000000000000000000000000000000000000000140"
                                       "000000000000000000000000000000000000000000000000000049e26567d9a3"
                                       "0000000000000000000000000000000000000000000000000000000000000080"
                                       "00000000000000000000000000000000000000000000000000000012ec4c9a30"
                                       "00000000000000000000000000000000000000000000000000000000000000c0"
                                       "000000000000000000000000000000000000000000000000000000000000000f"
                                       "48656c6c6f20576f726c64207631210000000000000000000000000000000000"
                                       "000000000000000000000000000000000000000000000000000000000000000f"
                                       "48656c6c6f20576f726c64207632210000000000000000000000000000000000"
                                       "0000000000000000000000000000000000000000000000000000037e5d62cd03"
                                       "0000000000000000000000000000000000000000000000000000000000000080"
                                       "000000000000000000000000000000000000000000000000000004b83eb5817b"
                                       "00000000000000000000000000000000000000000000000000000000000000c0"
                                       "000000000000000000000000000000000000000000000000000000000000000f"
                                       "48656c6c6f20576f726c64207633210000000000000000000000000000000000"
                                       "000000000000000000000000000000000000000000000000000000000000000f"
                                       "48656c6c6f20576f726c64207634210000000000000000000000000000000000");
      // Expected =      {
      //                     {
      //                       {81236712741283, "Hello World v1!", 81273854512, "Hello World v2!"},
      //                       {3841267518723, "Hello World v3!", 5189372576123, "Hello World v4!"}
      //                     },
      //                     "Hello World v5!",
      //                     1298318812,
      //                     {
      //                       {81236712741283, "Hello World v1!", 81273854512, "Hello World v2!"},
      //                       {3841267518723, "Hello World v3!", 5189372576123, "Hello World v4!"}
      //                     }
      //                 }
      auto decodedData = ABI::Decoder::decodeData<std::tuple<std::vector<std::tuple<uint256_t, std::string, uint256_t, std::string>>, std::string, uint256_t, std::vector<std::tuple<uint256_t, std::string, uint256_t, std::string>>>>(ABI);
      auto decodedTuple = std::get<0>(decodedData);
      auto decodedVector = std::get<0>(decodedTuple);
      REQUIRE(std::get<0>(decodedVector[0]) == uint256_t(81236712741283));
      REQUIRE(std::get<1>(decodedVector[0]) == "Hello World v1!");
      REQUIRE(std::get<2>(decodedVector[0]) == uint256_t(81273854512));
      REQUIRE(std::get<3>(decodedVector[0]) == "Hello World v2!");
      REQUIRE(std::get<0>(decodedVector[1]) == uint256_t(3841267518723));
      REQUIRE(std::get<1>(decodedVector[1]) == "Hello World v3!");
      REQUIRE(std::get<2>(decodedVector[1]) == uint256_t(5189372576123));
      REQUIRE(std::get<3>(decodedVector[1]) == "Hello World v4!");
      REQUIRE(std::get<1>(decodedTuple) == "Hello World v5!");
      REQUIRE(std::get<2>(decodedTuple) == uint256_t(1298318812));
      auto decodedVector2 = std::get<3>(decodedTuple);
      REQUIRE(std::get<0>(decodedVector2[0]) == uint256_t(81236712741283));
      REQUIRE(std::get<1>(decodedVector2[0]) == "Hello World v1!");
      REQUIRE(std::get<2>(decodedVector2[0]) == uint256_t(81273854512));
      REQUIRE(std::get<3>(decodedVector2[0]) == "Hello World v2!");
      REQUIRE(std::get<0>(decodedVector2[1]) == uint256_t(3841267518723));
      REQUIRE(std::get<1>(decodedVector2[1]) == "Hello World v3!");
      REQUIRE(std::get<2>(decodedVector2[1]) == uint256_t(5189372576123));
      REQUIRE(std::get<3>(decodedVector2[1]) == "Hello World v4!");
    }

    SECTION("Decode std::tuple<std::vector<std::tuple<std::vector<std::tuple<uint256_t, std::string, uint256_t, std::string>>,std::string,uint256_t,std::vector<std::tuple<uint256_t, std::tuple<std::string, uint256_t>, std::string>>>>,std::string,std::vector<std::tuple<std::vector<std::tuple<uint256_t, std::tuple<std::string, uint256_t>, std::string>>,std::string,uint256_t,std::vector<std::tuple<uint256_t, std::string, uint256_t, std::string>>>>>")
    {
      auto ABI = Hex::toBytes("0000000000000000000000000000000000000000000000000000000000000020"
                                    "0000000000000000000000000000000000000000000000000000000000000060"
                                    "0000000000000000000000000000000000000000000000000000000000000c40"
                                    "0000000000000000000000000000000000000000000000000000000000000c80"
                                    "0000000000000000000000000000000000000000000000000000000000000002"
                                    "0000000000000000000000000000000000000000000000000000000000000040"
                                    "0000000000000000000000000000000000000000000000000000000000000600"
                                    "0000000000000000000000000000000000000000000000000000000000000080"
                                    "00000000000000000000000000000000000000000000000000000000000002e0"
                                    "000000000000000000000000000000000000000000000000000000004d62c5dc"
                                    "0000000000000000000000000000000000000000000000000000000000000320"
                                    "0000000000000000000000000000000000000000000000000000000000000002"
                                    "0000000000000000000000000000000000000000000000000000000000000040"
                                    "0000000000000000000000000000000000000000000000000000000000000140"
                                    "000000000000000000000000000000000000000000000000000000011011b5fd"
                                    "0000000000000000000000000000000000000000000000000000000000000080"
                                    "0000000000000000000000000000000000000000000000000000000002509166"
                                    "00000000000000000000000000000000000000000000000000000000000000c0"
                                    "000000000000000000000000000000000000000000000000000000000000000f"
                                    "48656c6c6f20576f726c64207631210000000000000000000000000000000000"
                                    "000000000000000000000000000000000000000000000000000000000000000f"
                                    "48656c6c6f20576f726c64207632210000000000000000000000000000000000"
                                    "000000000000000000000000000000000000000000000000000000011011ba36"
                                    "0000000000000000000000000000000000000000000000000000000000000080"
                                    "000000000000000000000000000000000000000000000000000000011016d484"
                                    "00000000000000000000000000000000000000000000000000000000000000c0"
                                    "000000000000000000000000000000000000000000000000000000000000000f"
                                    "48656c6c6f20576f726c64207633210000000000000000000000000000000000"
                                    "000000000000000000000000000000000000000000000000000000000000000f"
                                    "48656c6c6f20576f726c64207634210000000000000000000000000000000000"
                                    "0000000000000000000000000000000000000000000000000000000000000010"
                                    "48656c6c6f20576f726c64207633342100000000000000000000000000000000"
                                    "0000000000000000000000000000000000000000000000000000000000000002"
                                    "0000000000000000000000000000000000000000000000000000000000000040"
                                    "0000000000000000000000000000000000000000000000000000000000000160"
                                    "0000000000000000000000000000000000000000000000000000000013f61ed8"
                                    "0000000000000000000000000000000000000000000000000000000000000060"
                                    "00000000000000000000000000000000000000000000000000000000000000e0"
                                    "0000000000000000000000000000000000000000000000000000000000000040"
                                    "00000000000000000000000000000000000000000000000000000000020eca5e"
                                    "000000000000000000000000000000000000000000000000000000000000000f"
                                    "48656c6c6f20576f726c64207631210000000000000000000000000000000000"
                                    "000000000000000000000000000000000000000000000000000000000000000f"
                                    "48656c6c6f20576f726c64207632210000000000000000000000000000000000"
                                    "000000000000000000000000000000000000000000000000000000000321cb90"
                                    "0000000000000000000000000000000000000000000000000000000000000060"
                                    "00000000000000000000000000000000000000000000000000000000000000e0"
                                    "0000000000000000000000000000000000000000000000000000000000000040"
                                    "00000000000000000000000000000000000000000000000000000000035c4649"
                                    "000000000000000000000000000000000000000000000000000000000000000f"
                                    "48656c6c6f20576f726c64207633210000000000000000000000000000000000"
                                    "000000000000000000000000000000000000000000000000000000000000000f"
                                    "48656c6c6f20576f726c64207634210000000000000000000000000000000000"
                                    "0000000000000000000000000000000000000000000000000000000000000080"
                                    "00000000000000000000000000000000000000000000000000000000000002e0"
                                    "00000000000000000000000000000000000000000000000000000000024c9cef"
                                    "0000000000000000000000000000000000000000000000000000000000000320"
                                    "0000000000000000000000000000000000000000000000000000000000000002"
                                    "0000000000000000000000000000000000000000000000000000000000000040"
                                    "0000000000000000000000000000000000000000000000000000000000000140"
                                    "0000000000000000000000000000000000000000000000000000000049ab62fb"
                                    "0000000000000000000000000000000000000000000000000000000000000080"
                                    "000000000000000000000000000000000000000000000000000000000455bbf1"
                                    "00000000000000000000000000000000000000000000000000000000000000c0"
                                    "000000000000000000000000000000000000000000000000000000000000000f"
                                    "48656c6c6f20576f726c64207637210000000000000000000000000000000000"
                                    "000000000000000000000000000000000000000000000000000000000000000f"
                                    "48656c6c6f20576f726c64207638210000000000000000000000000000000000"
                                    "00000000000000000000000000000000000000000000000000000000155200a4"
                                    "0000000000000000000000000000000000000000000000000000000000000080"
                                    "000000000000000000000000000000000000000000000000000000000096109b"
                                    "00000000000000000000000000000000000000000000000000000000000000c0"
                                    "000000000000000000000000000000000000000000000000000000000000000f"
                                    "48656c6c6f20576f726c64207639210000000000000000000000000000000000"
                                    "0000000000000000000000000000000000000000000000000000000000000010"
                                    "48656c6c6f20576f726c64207631302100000000000000000000000000000000"
                                    "0000000000000000000000000000000000000000000000000000000000000010"
                                    "48656c6c6f20576f726c64207633332100000000000000000000000000000000"
                                    "0000000000000000000000000000000000000000000000000000000000000002"
                                    "0000000000000000000000000000000000000000000000000000000000000040"
                                    "0000000000000000000000000000000000000000000000000000000000000160"
                                    "00000000000000000000000000000000000000000000000000000339d188cffe"
                                    "0000000000000000000000000000000000000000000000000000000000000060"
                                    "00000000000000000000000000000000000000000000000000000000000000e0"
                                    "0000000000000000000000000000000000000000000000000000000000000040"
                                    "00000000000000000000000000000000000000000000000000000000024a2037"
                                    "0000000000000000000000000000000000000000000000000000000000000010"
                                    "48656c6c6f20576f726c64207631312100000000000000000000000000000000"
                                    "0000000000000000000000000000000000000000000000000000000000000010"
                                    "48656c6c6f20576f726c64207631322100000000000000000000000000000000"
                                    "0000000000000000000000000000000000000000000000000000000099c5bdcc"
                                    "0000000000000000000000000000000000000000000000000000000000000060"
                                    "00000000000000000000000000000000000000000000000000000000000000e0"
                                    "0000000000000000000000000000000000000000000000000000000000000040"
                                    "00000000000000000000000000000000000000000000000000000000021d86fb"
                                    "0000000000000000000000000000000000000000000000000000000000000010"
                                    "48656c6c6f20576f726c64207631332100000000000000000000000000000000"
                                    "0000000000000000000000000000000000000000000000000000000000000010"
                                    "48656c6c6f20576f726c64207631342100000000000000000000000000000000"
                                    "000000000000000000000000000000000000000000000000000000000000000d"
                                    "6262626262626262626262626200000000000000000000000000000000000000"
                                    "0000000000000000000000000000000000000000000000000000000000000002"
                                    "0000000000000000000000000000000000000000000000000000000000000040"
                                    "0000000000000000000000000000000000000000000000000000000000000600"
                                    "0000000000000000000000000000000000000000000000000000000000000080"
                                    "0000000000000000000000000000000000000000000000000000000000000320"
                                    "00000000000000000000000000000000000000000000000000000001104376fb"
                                    "0000000000000000000000000000000000000000000000000000000000000360"
                                    "0000000000000000000000000000000000000000000000000000000000000002"
                                    "0000000000000000000000000000000000000000000000000000000000000040"
                                    "0000000000000000000000000000000000000000000000000000000000000160"
                                    "0000000000000000000000000000000000000000000000000000298b76fc8c04"
                                    "0000000000000000000000000000000000000000000000000000000000000060"
                                    "00000000000000000000000000000000000000000000000000000000000000e0"
                                    "0000000000000000000000000000000000000000000000000000000000000040"
                                    "000000000000000000000000000000000000000000000000000000010e7ad169"
                                    "0000000000000000000000000000000000000000000000000000000000000010"
                                    "48656c6c6f20576f726c64207631352100000000000000000000000000000000"
                                    "0000000000000000000000000000000000000000000000000000000000000010"
                                    "48656c6c6f20576f726c64207631362100000000000000000000000000000000"
                                    "00000000000000000000000000000000000000000000000000000000000c0c3c"
                                    "0000000000000000000000000000000000000000000000000000000000000060"
                                    "00000000000000000000000000000000000000000000000000000000000000e0"
                                    "0000000000000000000000000000000000000000000000000000000000000040"
                                    "000000000000000000000000000000000000000000000000000000000361eb7b"
                                    "0000000000000000000000000000000000000000000000000000000000000010"
                                    "48656c6c6f20576f726c64207631372100000000000000000000000000000000"
                                    "0000000000000000000000000000000000000000000000000000000000000010"
                                    "48656c6c6f20576f726c64207631382100000000000000000000000000000000"
                                    "0000000000000000000000000000000000000000000000000000000000000010"
                                    "48656c6c6f20576f726c64207633322100000000000000000000000000000000"
                                    "0000000000000000000000000000000000000000000000000000000000000002"
                                    "0000000000000000000000000000000000000000000000000000000000000040"
                                    "0000000000000000000000000000000000000000000000000000000000000140"
                                    "0000000000000000000000000000000000000000000000000000000000084a0d"
                                    "0000000000000000000000000000000000000000000000000000000000000080"
                                    "0000000000000000000000000000000000000000000000000000036aa35f31bc"
                                    "00000000000000000000000000000000000000000000000000000000000000c0"
                                    "0000000000000000000000000000000000000000000000000000000000000010"
                                    "48656c6c6f20576f726c64207631392100000000000000000000000000000000"
                                    "0000000000000000000000000000000000000000000000000000000000000010"
                                    "48656c6c6f20576f726c64207632302100000000000000000000000000000000"
                                    "000000000000000000000000000000000000000000000000000000e5e11e9267"
                                    "0000000000000000000000000000000000000000000000000000000000000080"
                                    "000000000000000000000000000000000000000000000000000005f177be8e64"
                                    "00000000000000000000000000000000000000000000000000000000000000c0"
                                    "0000000000000000000000000000000000000000000000000000000000000010"
                                    "48656c6c6f20576f726c64207632312100000000000000000000000000000000"
                                    "0000000000000000000000000000000000000000000000000000000000000010"
                                    "48656c6c6f20576f726c64207632322100000000000000000000000000000000"
                                    "0000000000000000000000000000000000000000000000000000000000000080"
                                    "0000000000000000000000000000000000000000000000000000000000000320"
                                    "000000000000000000000000000000000000000000000000000000000053359c"
                                    "0000000000000000000000000000000000000000000000000000000000000360"
                                    "0000000000000000000000000000000000000000000000000000000000000002"
                                    "0000000000000000000000000000000000000000000000000000000000000040"
                                    "0000000000000000000000000000000000000000000000000000000000000160"
                                    "000000000000000000000000000000000000000000000000000043d1af405ba5"
                                    "0000000000000000000000000000000000000000000000000000000000000060"
                                    "00000000000000000000000000000000000000000000000000000000000000e0"
                                    "0000000000000000000000000000000000000000000000000000000000000040"
                                    "0000000000000000000000000000000000000000000000000002016e5a4d17a5"
                                    "0000000000000000000000000000000000000000000000000000000000000010"
                                    "48656c6c6f20576f726c64207632332100000000000000000000000000000000"
                                    "0000000000000000000000000000000000000000000000000000000000000010"
                                    "48656c6c6f20576f726c64207632342100000000000000000000000000000000"
                                    "000000000000000000000000000000000000000000000000000000396725fc52"
                                    "0000000000000000000000000000000000000000000000000000000000000060"
                                    "00000000000000000000000000000000000000000000000000000000000000e0"
                                    "0000000000000000000000000000000000000000000000000000000000000040"
                                    "000000000000000000000000000000000000000000000000000051bb61e4e478"
                                    "0000000000000000000000000000000000000000000000000000000000000010"
                                    "48656c6c6f20576f726c64207632352100000000000000000000000000000000"
                                    "0000000000000000000000000000000000000000000000000000000000000010"
                                    "48656c6c6f20576f726c64207632362100000000000000000000000000000000"
                                    "0000000000000000000000000000000000000000000000000000000000000010"
                                    "48656c6c6f20576f726c64207633312100000000000000000000000000000000"
                                    "0000000000000000000000000000000000000000000000000000000000000002"
                                    "0000000000000000000000000000000000000000000000000000000000000040"
                                    "0000000000000000000000000000000000000000000000000000000000000140"
                                    "0000000000000000000000000000000000000000000000000002ce4915b71fee"
                                    "0000000000000000000000000000000000000000000000000000000000000080"
                                    "0000000000000000000000000000000000000000000000000000c320e0d726e7"
                                    "00000000000000000000000000000000000000000000000000000000000000c0"
                                    "0000000000000000000000000000000000000000000000000000000000000010"
                                    "48656c6c6f20576f726c64207632372100000000000000000000000000000000"
                                    "0000000000000000000000000000000000000000000000000000000000000010"
                                    "48656c6c6f20576f726c64207632382100000000000000000000000000000000"
                                    "0000000000000000000000000000000000000000000000000003319b32675595"
                                    "0000000000000000000000000000000000000000000000000000000000000080"
                                    "000000000000000000000000000000000000000000000000000002ecb56662e1"
                                    "00000000000000000000000000000000000000000000000000000000000000c0"
                                    "0000000000000000000000000000000000000000000000000000000000000010"
                                    "48656c6c6f20576f726c64207632392100000000000000000000000000000000"
                                    "0000000000000000000000000000000000000000000000000000000000000010"
                                    "48656c6c6f20576f726c64207633302100000000000000000000000000000000");
      // Expected = {
      //              {
      //                {
      //                  {
      //                    {4564563453, "Hello World v1!", 38834534, "Hello World v2!"},
      //                    {4564564534, "Hello World v3!", 4564898948 , "Hello World v4!"}
      //                  },
      //                  "Hello World v34!",
      //                  1298318812,
      //                  {
      //                    {334896856, {"Hello World v1!", 34523742}, "Hello World v2!"},
      //                    {52546448, {"Hello World v3!", 56378953}, "Hello World v4!"}
      //                  }
      //                },
      //                {
      //                  {
      //                    {1235968763, "Hello World v7!", 72727537, "Hello World v8!"},
      //                    {357695652, "Hello World v9!", 9834651, "Hello World v10!"}
      //                  },
      //                  "Hello World v33!",
      //                  38575343,
      //                  {
      //                    {3546863423486, {"Hello World v11!", 38412343}, "Hello World v12!"},
      //                    {2579873228, {"Hello World v13!", 35489531}, "Hello World v14!"}
      //                  }
      //                }
      //              },
      //              "bbbbbbbbbbbbb",
      //            {
      //              {
      //                {
      //                  {45678973455364, {"Hello World v15!", 4537897321}, "Hello World v16!"},
      //                  {789564, {"Hello World v17!", 56748923}, "Hello World v18!"}
      //                },
      //                "Hello World v32!",
      //                4567824123,
      //                {
      //                  {543245, "Hello World v19!", 3756542341564 , "Hello World v20!"},
      //                  {987324387943, "Hello World v21!", 6534654234212, "Hello World v22!"}
      //                }
      //              },
      //              {
      //                {
      //                  {74567867456421, {"Hello World v23!", 564523426453413}, "Hello World v24!"},
      //                  {246543678546, {"Hello World v25!", 89865243124856}, "Hello World v26!"}
      //                },
      //                "Hello World v31!",
      //                5453212,
      //                {
      //                  {789763245678574, "Hello World v27!", 214545978566375, "Hello World v28!"},
      //                  {898967565456789, "Hello World v29!", 3215678923489, "Hello World v30!"}
      //                }
      //              }
      //            }
      //
    auto decodedData = ABI::Decoder::decodeData<std::tuple<
                                         std::vector<
                                           std::tuple<
                                             std::vector<
                                               std::tuple<uint256_t, std::string, uint256_t, std::string>
                                             >,
                                             std::string,
                                             uint256_t,
                                             std::vector<
                                               std::tuple<uint256_t, std::tuple<std::string, uint256_t>, std::string>
                                             >
                                           >
                                         >,
                                         std::string,
                                         std::vector<
                                           std::tuple<
                                             std::vector<
                                               std::tuple<uint256_t, std::tuple<std::string, uint256_t>, std::string>
                                             >,
                                             std::string,
                                             uint256_t,
                                             std::vector<
                                               std::tuple<uint256_t, std::string, uint256_t, std::string>
                                             >
                                           >
                                         >
                                        >>(ABI);

      auto decodedTuple = std::get<0>(decodedData);
      auto firstVector = std::get<0>(decodedTuple);
      auto middleString = std::get<1>(decodedTuple);
      auto secondVector = std::get<2>(decodedTuple);
      REQUIRE(middleString == "bbbbbbbbbbbbb");

      REQUIRE(firstVector.size() == 2);
      REQUIRE(secondVector.size() == 2);

      std::vector<std::tuple<uint256_t, std::string, uint256_t, std::string>> firstVectorIndexZeroFirstVector = std::get<0>(firstVector[0]);
      auto firstVectorIndexZeroMiddleString = std::get<1>(firstVector[0]);
      auto firstVectorIndexZeroMiddleUint = std::get<2>(firstVector[0]);
      std::vector<std::tuple<uint256_t, std::tuple<std::string, uint256_t>, std::string>> firstVectorIndexZeroLastVector = std::get<3>(firstVector[0]);
      REQUIRE(firstVectorIndexZeroFirstVector.size() == 2);
      REQUIRE(firstVectorIndexZeroLastVector.size() == 2);

      std::tuple<uint256_t, std::string, uint256_t, std::string> firstVectorFirstVectorFirstTuple = firstVectorIndexZeroFirstVector[0];
      std::tuple<uint256_t, std::string, uint256_t, std::string> firstVectorFirstVectorSecondTuple = firstVectorIndexZeroFirstVector[1];
      std::tuple<uint256_t, std::tuple<std::string, uint256_t>, std::string> firstVectorLastVectorFirstTuple = firstVectorIndexZeroLastVector[0];
      std::tuple<uint256_t, std::tuple<std::string, uint256_t>, std::string> firstVectorLastVectorSecondTuple = firstVectorIndexZeroLastVector[1];
      REQUIRE(std::get<0>(firstVectorFirstVectorFirstTuple) == uint256_t(4564563453));
      REQUIRE(std::get<1>(firstVectorFirstVectorFirstTuple) == "Hello World v1!");
      REQUIRE(std::get<2>(firstVectorFirstVectorFirstTuple) == uint256_t(38834534));
      REQUIRE(std::get<3>(firstVectorFirstVectorFirstTuple) == "Hello World v2!");
      REQUIRE(std::get<0>(firstVectorFirstVectorSecondTuple) == uint256_t(4564564534));
      REQUIRE(std::get<1>(firstVectorFirstVectorSecondTuple) == "Hello World v3!");
      REQUIRE(std::get<2>(firstVectorFirstVectorSecondTuple) == uint256_t(4564898948));
      REQUIRE(std::get<3>(firstVectorFirstVectorSecondTuple) == "Hello World v4!");
      REQUIRE(firstVectorIndexZeroMiddleString == "Hello World v34!");
      REQUIRE(firstVectorIndexZeroMiddleUint == uint256_t(1298318812));
      REQUIRE(std::get<0>(firstVectorLastVectorFirstTuple) == uint256_t(334896856));
      REQUIRE(std::get<0>(std::get<1>(firstVectorLastVectorFirstTuple)) == "Hello World v1!");
      REQUIRE(std::get<1>(std::get<1>(firstVectorLastVectorFirstTuple)) == uint256_t(34523742));
      REQUIRE(std::get<2>(firstVectorLastVectorFirstTuple) == "Hello World v2!");
      REQUIRE(std::get<0>(firstVectorLastVectorSecondTuple) == uint256_t(52546448));
      REQUIRE(std::get<0>(std::get<1>(firstVectorLastVectorSecondTuple)) == "Hello World v3!");
      REQUIRE(std::get<1>(std::get<1>(firstVectorLastVectorSecondTuple)) == uint256_t(56378953));
      REQUIRE(std::get<2>(firstVectorLastVectorSecondTuple) == "Hello World v4!");

      std::vector<std::tuple<uint256_t, std::string, uint256_t, std::string>> firstVectorIndexOneFirstVector = std::get<0>(firstVector[1]);
      auto firstVectorIndexOneMiddleString = std::get<1>(firstVector[1]);
      auto firstVectorIndexOneMiddleUint = std::get<2>(firstVector[1]);
      std::vector<std::tuple<uint256_t, std::tuple<std::string, uint256_t>, std::string>> firstVectorIndexOneLastVector = std::get<3>(firstVector[1]);
      REQUIRE(firstVectorIndexOneFirstVector.size() == 2);
      REQUIRE(firstVectorIndexOneLastVector.size() == 2);

      std::tuple<uint256_t, std::string, uint256_t, std::string> firstVectorIndexOneFirstVectorFirstTuple = firstVectorIndexOneFirstVector[0];
      std::tuple<uint256_t, std::string, uint256_t, std::string> firstVectorIndexOneFirstVectorSecondTuple = firstVectorIndexOneFirstVector[1];
      std::tuple<uint256_t, std::tuple<std::string, uint256_t>, std::string> firstVectorIndexOneLastVectorFirstTuple = firstVectorIndexOneLastVector[0];
      std::tuple<uint256_t, std::tuple<std::string, uint256_t>, std::string> firstVectorIndexOneLastVectorSecondTuple = firstVectorIndexOneLastVector[1];
      REQUIRE(std::get<0>(firstVectorIndexOneFirstVectorFirstTuple) == uint256_t(1235968763));
      REQUIRE(std::get<1>(firstVectorIndexOneFirstVectorFirstTuple) == "Hello World v7!");
      REQUIRE(std::get<2>(firstVectorIndexOneFirstVectorFirstTuple) == uint256_t(72727537));
      REQUIRE(std::get<3>(firstVectorIndexOneFirstVectorFirstTuple) == "Hello World v8!");
      REQUIRE(std::get<0>(firstVectorIndexOneFirstVectorSecondTuple) == uint256_t(357695652));
      REQUIRE(std::get<1>(firstVectorIndexOneFirstVectorSecondTuple) == "Hello World v9!");
      REQUIRE(std::get<2>(firstVectorIndexOneFirstVectorSecondTuple) == uint256_t(9834651));
      REQUIRE(std::get<3>(firstVectorIndexOneFirstVectorSecondTuple) == "Hello World v10!");
      REQUIRE(firstVectorIndexOneMiddleString == "Hello World v33!");
      REQUIRE(firstVectorIndexOneMiddleUint == uint256_t(38575343));
      REQUIRE(std::get<0>(firstVectorIndexOneLastVectorFirstTuple) == uint256_t(3546863423486));
      REQUIRE(std::get<0>(std::get<1>(firstVectorIndexOneLastVectorFirstTuple)) == "Hello World v11!");
      REQUIRE(std::get<1>(std::get<1>(firstVectorIndexOneLastVectorFirstTuple)) == uint256_t(38412343));
      REQUIRE(std::get<2>(firstVectorIndexOneLastVectorFirstTuple) == "Hello World v12!");
      REQUIRE(std::get<0>(firstVectorIndexOneLastVectorSecondTuple) == uint256_t(2579873228));
      REQUIRE(std::get<0>(std::get<1>(firstVectorIndexOneLastVectorSecondTuple)) == "Hello World v13!");
      REQUIRE(std::get<1>(std::get<1>(firstVectorIndexOneLastVectorSecondTuple)) == uint256_t(35489531));
      REQUIRE(std::get<2>(firstVectorIndexOneLastVectorSecondTuple) == "Hello World v14!");

      std::vector<std::tuple<uint256_t, std::tuple<std::string, uint256_t>, std::string>> secondVectorIndexZeroFirstVector = std::get<0>(secondVector[0]);
      auto secondVectorIndexZeroMiddleString = std::get<1>(secondVector[0]);
      auto secondVectorIndexZeroMiddleUint = std::get<2>(secondVector[0]);
      std::vector<std::tuple<uint256_t, std::string, uint256_t, std::string>> secondVectorIndexZeroLastVector = std::get<3>(secondVector[0]);
      REQUIRE(secondVectorIndexZeroFirstVector.size() == 2);
      REQUIRE(secondVectorIndexZeroLastVector.size() == 2);

      std::tuple<uint256_t, std::tuple<std::string, uint256_t>, std::string> secondVectorIndexZeroFirstVectorFirstTuple = secondVectorIndexZeroFirstVector[0];
      std::tuple<uint256_t, std::tuple<std::string, uint256_t>, std::string> secondVectorIndexZeroFirstVectorSecondTuple = secondVectorIndexZeroFirstVector[1];
      std::tuple<uint256_t, std::string, uint256_t, std::string> secondVectorIndexZeroLastVectorFirstTuple = secondVectorIndexZeroLastVector[0];
      std::tuple<uint256_t, std::string, uint256_t, std::string> secondVectorIndexZeroLastVectorSecondTuple = secondVectorIndexZeroLastVector[1];
      REQUIRE(std::get<0>(secondVectorIndexZeroFirstVectorFirstTuple) == uint256_t(45678973455364));
      REQUIRE(std::get<0>(std::get<1>(secondVectorIndexZeroFirstVectorFirstTuple)) == "Hello World v15!");
      REQUIRE(std::get<1>(std::get<1>(secondVectorIndexZeroFirstVectorFirstTuple)) == uint256_t(4537897321));
      REQUIRE(std::get<2>(secondVectorIndexZeroFirstVectorFirstTuple) == "Hello World v16!");
      REQUIRE(std::get<0>(secondVectorIndexZeroFirstVectorSecondTuple) == uint256_t(789564));
      REQUIRE(std::get<0>(std::get<1>(secondVectorIndexZeroFirstVectorSecondTuple)) == "Hello World v17!");
      REQUIRE(std::get<1>(std::get<1>(secondVectorIndexZeroFirstVectorSecondTuple)) == uint256_t(56748923));
      REQUIRE(std::get<2>(secondVectorIndexZeroFirstVectorSecondTuple) == "Hello World v18!");
      REQUIRE(secondVectorIndexZeroMiddleString == "Hello World v32!");
      REQUIRE(secondVectorIndexZeroMiddleUint == uint256_t(4567824123));
      REQUIRE(std::get<0>(secondVectorIndexZeroLastVectorFirstTuple) == uint256_t(543245));
      REQUIRE(std::get<1>(secondVectorIndexZeroLastVectorFirstTuple) == "Hello World v19!");
      REQUIRE(std::get<2>(secondVectorIndexZeroLastVectorFirstTuple) == uint256_t(3756542341564));
      REQUIRE(std::get<3>(secondVectorIndexZeroLastVectorFirstTuple) == "Hello World v20!");
      REQUIRE(std::get<0>(secondVectorIndexZeroLastVectorSecondTuple) == uint256_t(987324387943));
      REQUIRE(std::get<1>(secondVectorIndexZeroLastVectorSecondTuple) == "Hello World v21!");
      REQUIRE(std::get<2>(secondVectorIndexZeroLastVectorSecondTuple) == uint256_t(6534654234212));
      REQUIRE(std::get<3>(secondVectorIndexZeroLastVectorSecondTuple) == "Hello World v22!");

      std::vector<std::tuple<uint256_t, std::tuple<std::string, uint256_t>, std::string>> secondVectorIndexOneFirstVector = std::get<0>(secondVector[1]);
      auto secondVectorIndexOneMiddleString = std::get<1>(secondVector[1]);
      auto secondVectorIndexOneMiddleUint = std::get<2>(secondVector[1]);
      std::vector<std::tuple<uint256_t, std::string, uint256_t, std::string>> secondVectorIndexOneLastVector = std::get<3>(secondVector[1]);
      REQUIRE(secondVectorIndexOneFirstVector.size() == 2);
      REQUIRE(secondVectorIndexOneLastVector.size() == 2);

      std::tuple<uint256_t, std::tuple<std::string, uint256_t>, std::string> secondVectorIndexOneFirstVectorFirstTuple = secondVectorIndexOneFirstVector[0];
      std::tuple<uint256_t, std::tuple<std::string, uint256_t>, std::string> secondVectorIndexOneFirstVectorSecondTuple = secondVectorIndexOneFirstVector[1];
      std::tuple<uint256_t, std::string, uint256_t, std::string> secondVectorIndexOneLastVectorFirstTuple = secondVectorIndexOneLastVector[0];
      std::tuple<uint256_t, std::string, uint256_t, std::string> secondVectorIndexOneLastVectorSecondTuple = secondVectorIndexOneLastVector[1];
      REQUIRE(std::get<0>(secondVectorIndexOneFirstVectorFirstTuple) == uint256_t(74567867456421));
      REQUIRE(std::get<0>(std::get<1>(secondVectorIndexOneFirstVectorFirstTuple)) == "Hello World v23!");
      REQUIRE(std::get<1>(std::get<1>(secondVectorIndexOneFirstVectorFirstTuple)) == uint256_t(564523426453413));
      REQUIRE(std::get<2>(secondVectorIndexOneFirstVectorFirstTuple) == "Hello World v24!");
      REQUIRE(std::get<0>(secondVectorIndexOneFirstVectorSecondTuple) == uint256_t(246543678546));
      REQUIRE(std::get<0>(std::get<1>(secondVectorIndexOneFirstVectorSecondTuple)) == "Hello World v25!");
      REQUIRE(std::get<1>(std::get<1>(secondVectorIndexOneFirstVectorSecondTuple)) == uint256_t(89865243124856));
      REQUIRE(std::get<2>(secondVectorIndexOneFirstVectorSecondTuple) == "Hello World v26!");
      REQUIRE(secondVectorIndexOneMiddleString == "Hello World v31!");
      REQUIRE(secondVectorIndexOneMiddleUint == uint256_t(5453212));
      REQUIRE(std::get<0>(secondVectorIndexOneLastVectorFirstTuple) == uint256_t(789763245678574));
      REQUIRE(std::get<1>(secondVectorIndexOneLastVectorFirstTuple) == "Hello World v27!");
      REQUIRE(std::get<2>(secondVectorIndexOneLastVectorFirstTuple) == uint256_t(214545978566375));
      REQUIRE(std::get<3>(secondVectorIndexOneLastVectorFirstTuple) == "Hello World v28!");
      REQUIRE(std::get<0>(secondVectorIndexOneLastVectorSecondTuple) == uint256_t(898967565456789));
      REQUIRE(std::get<1>(secondVectorIndexOneLastVectorSecondTuple) == "Hello World v29!");
      REQUIRE(std::get<2>(secondVectorIndexOneLastVectorSecondTuple) == uint256_t(3215678923489));
      REQUIRE(std::get<3>(secondVectorIndexOneLastVectorSecondTuple) == "Hello World v30!");
    }

    SECTION("Decode std::tuple<std::vector<std::tuple<uint256_t, uint256_t, uint256_t, uint256_t>>, std::string, uint256_t, std::vector<std::tuple<std::string, std::tuple<uint256_t, uint256_t>, std::string>>>") {
      const Bytes ABI = Hex::toBytes("0000000000000000000000000000000000000000000000000000000000000020"
                                     "0000000000000000000000000000000000000000000000000000000000000080"
                                     "00000000000000000000000000000000000000000000000000000000000001a0"
                                     "0000000000000000000000000000000000000000000000001118e61e9ca26003"
                                     "00000000000000000000000000000000000000000000000000000000000001e0"
                                     "0000000000000000000000000000000000000000000000000000000000000002"
                                     "0000000000000000000000000000000000000000000000000000000d17a5eb60"
                                     "000000000000000000000000000000000000000000000000000000b019b66262"
                                     "000000000000000000000000000000000000000000000000000000af2b45b47c"
                                     "0000000000000000000000000000000000000000000000000000051d4fe13f6b"
                                     "00000000000000000000000000000000000000000000000000000000048cef94"
                                     "0000000000000000000000000000000000000000000000000000009636cac286"
                                     "000000000000000000000000000000000000000000000000000006f5f55cb10e"
                                     "00000000000000000000000000000000000000000000000000000083762d8c7f"
                                     "000000000000000000000000000000000000000000000000000000000000000f"
                                     "48656c6c6f20576f726c64207639210000000000000000000000000000000000"
                                     "0000000000000000000000000000000000000000000000000000000000000002"
                                     "0000000000000000000000000000000000000000000000000000000000000040"
                                     "0000000000000000000000000000000000000000000000000000000000000140"
                                     "0000000000000000000000000000000000000000000000000000000000000080"
                                     "00000000000000000000000000000000000000000000000000001dffbac9c68b"
                                     "00000000000000000000000000000000000000000000000000004b7964a5f333"
                                     "00000000000000000000000000000000000000000000000000000000000000c0"
                                     "000000000000000000000000000000000000000000000000000000000000000f"
                                     "48656c6c6f20576f726c64207631210000000000000000000000000000000000"
                                     "000000000000000000000000000000000000000000000000000000000000000f"
                                     "48656c6c6f20576f726c64207632210000000000000000000000000000000000"
                                     "0000000000000000000000000000000000000000000000000000000000000080"
                                     "0000000000000000000000000000000000000000000000000000b00cba5e323a"
                                     "00000000000000000000000000000000000000000000000000000001335d3307"
                                     "00000000000000000000000000000000000000000000000000000000000000c0"
                                     "000000000000000000000000000000000000000000000000000000000000000f"
                                     "48656c6c6f20576f726c64207633210000000000000000000000000000000000"
                                     "000000000000000000000000000000000000000000000000000000000000000f"
                                     "48656c6c6f20576f726c64207634210000000000000000000000000000000000");
      // Expected = {
      //              {
      //                {56231324512, 756345627234, 752345265276, 5623452352363},
      //                {76345236, 645164352134, 7653453246734, 564623412351}
      //              },
      //              "Hello World v9!",
      //              1231987517237125123,
      //              {
      //                    {"Hello World v1!",{ 32984187651723, 82984751723315}, "Hello World v2!"},
      //                    {"Hello World v3!",{ 193568712831546, 5156713223 }, "Hello World v4!"}
      //              }
      //            }
      auto decodedData = ABI::Decoder::decodeData<std::tuple<
                                         std::vector<
                                           std::tuple<uint256_t, uint256_t, uint256_t, uint256_t>
                                         >,
                                         std::string,
                                         uint256_t,
                                         std::vector<
                                           std::tuple<std::string, std::tuple<uint256_t, uint256_t>, std::string>
                                         >
                                        >>(ABI);

      auto decodedTuple = std::get<0>(decodedData);
      auto firstVector = std::get<0>(decodedTuple);
      auto middleString = std::get<1>(decodedTuple);
      auto middleUint = std::get<2>(decodedTuple);
      auto secondVector = std::get<3>(decodedTuple);

      REQUIRE(middleString == "Hello World v9!");
      REQUIRE(middleUint == uint256_t(1231987517237125123));
      REQUIRE(firstVector.size() == 2);
      REQUIRE(secondVector.size() == 2);
      REQUIRE(std::get<0>(firstVector[0]) == uint256_t(56231324512));
      REQUIRE(std::get<1>(firstVector[0]) == uint256_t(756345627234));
      REQUIRE(std::get<2>(firstVector[0]) == uint256_t(752345265276));
      REQUIRE(std::get<3>(firstVector[0]) == uint256_t(5623452352363));
      REQUIRE(std::get<0>(firstVector[1]) == uint256_t(76345236));
      REQUIRE(std::get<1>(firstVector[1]) == uint256_t(645164352134));
      REQUIRE(std::get<2>(firstVector[1]) == uint256_t(7653453246734));
      REQUIRE(std::get<3>(firstVector[1]) == uint256_t(564623412351));
      REQUIRE(std::get<0>(secondVector[0]) == "Hello World v1!");
      REQUIRE(std::get<0>(std::get<1>(secondVector[0])) == uint256_t(32984187651723));
      REQUIRE(std::get<1>(std::get<1>(secondVector[0])) == uint256_t(82984751723315));
      REQUIRE(std::get<2>(secondVector[0]) == "Hello World v2!");
      REQUIRE(std::get<0>(secondVector[1]) == "Hello World v3!");
      REQUIRE(std::get<0>(std::get<1>(secondVector[1])) == uint256_t(193568712831546));
      REQUIRE(std::get<1>(std::get<1>(secondVector[1])) == uint256_t(5156713223));
      REQUIRE(std::get<2>(secondVector[1]) == "Hello World v4!");
    }

    SECTION("Decode std::vector<std::vector<std::tuple<uint256_t,uint256_t,uint256_t>>>") {
      const Bytes ABI = Hex::toBytes("0000000000000000000000000000000000000000000000000000000000000020"
                                     "0000000000000000000000000000000000000000000000000000000000000003"
                                     "0000000000000000000000000000000000000000000000000000000000000060"
                                     "00000000000000000000000000000000000000000000000000000000000001a0"
                                     "00000000000000000000000000000000000000000000000000000000000002e0"
                                     "0000000000000000000000000000000000000000000000000000000000000003"
                                     "0000000000000000000000000000000000000000000000000000000000000001"
                                     "0000000000000000000000000000000000000000000000000000000000000002"
                                     "0000000000000000000000000000000000000000000000000000000000000003"
                                     "0000000000000000000000000000000000000000000000000000000000000004"
                                     "0000000000000000000000000000000000000000000000000000000000000005"
                                     "0000000000000000000000000000000000000000000000000000000000000006"
                                     "0000000000000000000000000000000000000000000000000000000000000007"
                                     "0000000000000000000000000000000000000000000000000000000000000008"
                                     "0000000000000000000000000000000000000000000000000000000000000009"
                                     "0000000000000000000000000000000000000000000000000000000000000003"
                                     "000000000000000000000000000000000000000000000000000000000000000a"
                                     "000000000000000000000000000000000000000000000000000000000000000b"
                                     "000000000000000000000000000000000000000000000000000000000000000c"
                                     "000000000000000000000000000000000000000000000000000000000000000d"
                                     "000000000000000000000000000000000000000000000000000000000000000e"
                                     "000000000000000000000000000000000000000000000000000000000000000f"
                                     "0000000000000000000000000000000000000000000000000000000000000010"
                                     "0000000000000000000000000000000000000000000000000000000000000011"
                                     "0000000000000000000000000000000000000000000000000000000000000012"
                                     "0000000000000000000000000000000000000000000000000000000000000003"
                                     "0000000000000000000000000000000000000000000000000000000000000013"
                                     "0000000000000000000000000000000000000000000000000000000000000014"
                                     "0000000000000000000000000000000000000000000000000000000000000015"
                                     "0000000000000000000000000000000000000000000000000000000000000016"
                                     "0000000000000000000000000000000000000000000000000000000000000017"
                                     "0000000000000000000000000000000000000000000000000000000000000018"
                                     "0000000000000000000000000000000000000000000000000000000000000019"
                                     "000000000000000000000000000000000000000000000000000000000000001a"
                                     "000000000000000000000000000000000000000000000000000000000000001b");

      // Expected = {
      //              {
      //                {1,2,3},
      //                {4,5,6},
      //                {7,8,9}
      //              },
      //              {
      //                {10,11,12},
      //                {13,14,15},
      //                {16,17,18},
      //                {19,20,21}
      //              }
      //            }
      auto decodedDataTpl = ABI::Decoder::decodeData<std::vector<std::vector<std::tuple<uint256_t,uint256_t,uint256_t>>>>(ABI);
      auto decodedData = std::get<0>(decodedDataTpl);
      std::vector<std::tuple<uint256_t,uint256_t,uint256_t>> vector1 = {{1,2,3},{4,5,6},{7,8,9}};
      std::vector<std::tuple<uint256_t,uint256_t,uint256_t>> vector2 = {{10,11,12},{13,14,15},{16,17,18}};
      std::vector<std::tuple<uint256_t,uint256_t,uint256_t>> vector3 = {{19,20,21},{22,23,24},{25,26,27}};
      REQUIRE(decodedData.size() == 3);
      REQUIRE(decodedData[0] == vector1);
      REQUIRE(decodedData[1] == vector2);
      REQUIRE(decodedData[2] == vector3);
    }
  };
} // namespace TABI

