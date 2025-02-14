/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#include "libs/catch2/catch_amalgamated.hpp"

#include "utils/evmcconv.h"
#include "utils/uintconv.h"
#include "utils/strconv.h"

namespace TJsonAbi {
  TEST_CASE("EVMCConv Namespace", "[unit][utils][evmcconv]") {
    SECTION("uint256 <-> evmcUint256") {
      uint256_t i = 12345678;
      evmc::uint256be resEVMC = EVMCConv::uint256ToEvmcUint256(i);
      REQUIRE(UintConv::bytesToUint256(View<Bytes>(resEVMC.bytes, 32)) == i);
      uint256_t resUINT = EVMCConv::evmcUint256ToUint256(resEVMC);
      REQUIRE(resUINT == i);
    }

    SECTION("bytes <-> evmcUint256") {
      uint256_t i = 12345678;
      evmc::uint256be iEVMC = EVMCConv::uint256ToEvmcUint256(i);
      BytesArr<32> resBYTES = EVMCConv::evmcUint256ToBytes(iEVMC);
      REQUIRE(UintConv::bytesToUint256(resBYTES) == i);
      evmc::uint256be resEVMC = EVMCConv::bytesToEvmcUint256(resBYTES);
      REQUIRE(UintConv::bytesToUint256(View<Bytes>(resEVMC.bytes, 32)) == i);
    }

    SECTION("getFunctor") {
      evmc_message msg1;
      evmc_message msg2;
      Bytes msg1Data = Bytes{0x00, 0x00};
      Bytes msg2Data = Bytes{0x01, 0x02, 0x03, 0x04, 0xab, 0xcd, 0xef, 0xff};
      msg1.input_size = 2;
      msg1.input_data = msg1Data.data();
      msg2.input_size = 8;
      msg2.input_data = msg2Data.data();
      REQUIRE(EVMCConv::getFunctor(msg1) == Functor());
      REQUIRE(EVMCConv::getFunctor(msg2) != Functor());
      REQUIRE(Hex::fromBytes(UintConv::uint32ToBytes(EVMCConv::getFunctor(msg2).value)).get() == "01020304");
    }

    SECTION("getFunctionArgs") {
      evmc_message msg1;
      evmc_message msg2;
      Bytes msg1Data = Bytes{0x00, 0x00};
      Bytes msg2Data = Bytes{0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
      msg1.input_size = 2;
      msg1.input_data = msg1Data.data();
      msg2.input_size = 16;
      msg2.input_data = msg2Data.data();
      View<Bytes> get1 = EVMCConv::getFunctionArgs(msg1);
      View<Bytes> get2 = EVMCConv::getFunctionArgs(msg2);
      REQUIRE(Hex::fromBytes(get1).get() == "");
      REQUIRE(Hex::fromBytes(get2).get() == "0405060708090a0b0c0d0e0f");
    }
  }
}

