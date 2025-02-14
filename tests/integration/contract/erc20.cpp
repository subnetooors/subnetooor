/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#include "libs/catch2/catch_amalgamated.hpp"

#include "contract/templates/erc20.h"

#include "bytes/hex.h"

#include "../../sdktestsuite.hpp"

namespace TERC20 {
  TEST_CASE("ERC20 Class", "[integration][contract][erc20]") {
    SECTION("ERC20 creation") {
      SDKTestSuite sdk = SDKTestSuite::createNewEnvironment("testERC20Creation");
      REQUIRE(sdk.getState().getContractsSize() == 1);
      Address erc20 = sdk.deployContract<ERC20>(
        std::string("TestToken"), std::string("TST"), uint8_t(18), uint256_t("1000000000000000000")
      );
      REQUIRE(sdk.getState().getContractsSize() == 2);
      Address owner = sdk.getChainOwnerAccount().address;
      REQUIRE(sdk.callViewFunction(erc20, &ERC20::name) == "TestToken");
      REQUIRE(sdk.callViewFunction(erc20, &ERC20::symbol) == "TST");
      REQUIRE(sdk.callViewFunction(erc20, &ERC20::decimals) == 18);
      REQUIRE(sdk.callViewFunction(erc20, &ERC20::totalSupply) == uint256_t("1000000000000000000"));
      REQUIRE(sdk.callViewFunction(erc20, &ERC20::balanceOf, owner) == uint256_t("1000000000000000000"));
    }

    SECTION("ERC20 transfer()") {
      SDKTestSuite sdk = SDKTestSuite::createNewEnvironment("testERC20Transfer");
      Address erc20 = sdk.deployContract<ERC20>(
        std::string("TestToken"), std::string("TST"), uint8_t(18), uint256_t("1000000000000000000")
      );
      Address owner = sdk.getChainOwnerAccount().address;
      Address to(Utils::randBytes(20));

      uint256_t balanceMe = sdk.callViewFunction(erc20, &ERC20::balanceOf, owner);
      uint256_t balanceTo = sdk.callViewFunction(erc20, &ERC20::balanceOf, to);
      REQUIRE(balanceMe == uint256_t("1000000000000000000")); // 1 TST
      REQUIRE(balanceTo == 0);

      Hash transferTx = sdk.callFunction(erc20, &ERC20::transfer, to, uint256_t("500000000000000000")); // 0.5 TST
      balanceMe = sdk.callViewFunction(erc20, &ERC20::balanceOf, owner);
      balanceTo = sdk.callViewFunction(erc20, &ERC20::balanceOf, to);
      REQUIRE(balanceMe == uint256_t("500000000000000000"));
      REQUIRE(balanceTo == uint256_t("500000000000000000"));
      auto transferEvents = sdk.getEventsEmittedByTx(transferTx, &ERC20::Transfer);
      REQUIRE(transferEvents.size() == 1);
      REQUIRE(std::get<0>(ABI::Decoder::decodeData<Address>(transferEvents[0].getTopics()[1].asBytes())) == owner);
      REQUIRE(std::get<0>(ABI::Decoder::decodeData<Address>(transferEvents[0].getTopics()[2].asBytes())) == to);
      REQUIRE(std::get<0>(ABI::Decoder::decodeData<uint256_t>(transferEvents[0].getData())) == uint256_t("500000000000000000"));

      // "owner" doesn't have enough balance, this should throw and balances should stay intact
      REQUIRE_THROWS(sdk.callFunction(erc20, &ERC20::transfer, to, uint256_t("1000000000000000000")));
    }

    SECTION("ERC20 approve()") {
      SDKTestSuite sdk = SDKTestSuite::createNewEnvironment("testERC20Approve");
      Address erc20 = sdk.deployContract<ERC20>(
        std::string("TestToken"), std::string("TST"), uint8_t(18), uint256_t("1000000000000000000")
      );
      Address owner = sdk.getChainOwnerAccount().address;
      Address to(Utils::randBytes(20));

      uint256_t allowance = sdk.callViewFunction(erc20, &ERC20::allowance, owner, to);
      REQUIRE(allowance == 0); // "to" is not allowed (yet) to spend anything on behalf of "erc20"
      Hash approveTx = sdk.callFunction(erc20, &ERC20::approve, to, uint256_t("500000000000000000"));
      allowance = sdk.callViewFunction(erc20, &ERC20::allowance, owner, to);
      REQUIRE(allowance == uint256_t("500000000000000000")); // "to" can now spend 0.5 TST

      auto approveEvents = sdk.getEventsEmittedByTx(approveTx, &ERC20::Approval);
      REQUIRE(approveEvents.size() == 1);
      REQUIRE(std::get<0>(ABI::Decoder::decodeData<Address>(approveEvents[0].getTopics()[1].asBytes())) == owner);
      REQUIRE(std::get<0>(ABI::Decoder::decodeData<Address>(approveEvents[0].getTopics()[2].asBytes())) == to);
      REQUIRE(std::get<0>(ABI::Decoder::decodeData<uint256_t>(approveEvents[0].getData())) == uint256_t("500000000000000000"));

      // Search for a non-existing spender (for coverage)
      Address ghost(bytes::hex("0x1234567890123456789012345678901234567890"));
      REQUIRE(sdk.callViewFunction(erc20, &ERC20::allowance, owner, ghost) == uint256_t(0));
    }

    SECTION("ERC20 transferFrom()") {
      SDKTestSuite sdk = SDKTestSuite::createNewEnvironment("testERC20TransferFrom");
      Address erc20 = sdk.deployContract<ERC20>(
        std::string("TestToken"), std::string("TST"), uint8_t(18), uint256_t("1000000000000000000")
      );
      Address owner = sdk.getChainOwnerAccount().address;
      Address to(Utils::randBytes(20));

      // Check "owner" has 1 TST and "to" has 0 TST
      uint256_t balanceMe = sdk.callViewFunction(erc20, &ERC20::balanceOf, owner);
      uint256_t balanceTo = sdk.callViewFunction(erc20, &ERC20::balanceOf, to);
      REQUIRE(balanceMe == uint256_t("1000000000000000000")); // 1 TST
      REQUIRE(balanceTo == 0);

      // Approve "owner" first so it can spend up to 3 TST
      // (it doesn't have that much, but it's on purpose so we can test invalid balances)
      Hash approveTx = sdk.callFunction(erc20, &ERC20::approve, owner, uint256_t("3000000000000000000"));
      REQUIRE(sdk.callViewFunction(erc20, &ERC20::allowance, owner, owner) == uint256_t("3000000000000000000"));

      // Transfer 0.5 TST specifically from "owner" to "to"
      Hash transferTx = sdk.callFunction(erc20, &ERC20::transferFrom, owner, to, uint256_t("500000000000000000"));
      balanceMe = sdk.callViewFunction(erc20, &ERC20::balanceOf, owner);
      balanceTo = sdk.callViewFunction(erc20, &ERC20::balanceOf, to);
      REQUIRE(balanceMe == uint256_t("500000000000000000"));
      REQUIRE(balanceTo == uint256_t("500000000000000000"));
      auto transferEvents = sdk.getEventsEmittedByTx(transferTx, &ERC20::Transfer);
      REQUIRE(transferEvents.size() == 1);
      REQUIRE(std::get<0>(ABI::Decoder::decodeData<Address>(transferEvents[0].getTopics()[1].asBytes())) == owner);
      REQUIRE(std::get<0>(ABI::Decoder::decodeData<Address>(transferEvents[0].getTopics()[2].asBytes())) == to);
      REQUIRE(std::get<0>(ABI::Decoder::decodeData<uint256_t>(transferEvents[0].getData())) == uint256_t("500000000000000000"));

      // "owner" doesn't have enough balance, this should throw and balances should stay intact
      REQUIRE_THROWS(sdk.callFunction(erc20, &ERC20::transferFrom, owner, to, uint256_t("1000000000000000000")));
      balanceMe = sdk.callViewFunction(erc20, &ERC20::balanceOf, owner);
      balanceTo = sdk.callViewFunction(erc20, &ERC20::balanceOf, to);
      REQUIRE(balanceMe == uint256_t("500000000000000000"));
      REQUIRE(balanceTo == uint256_t("500000000000000000"));
    }
  }
}

