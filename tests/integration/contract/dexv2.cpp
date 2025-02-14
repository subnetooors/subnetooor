/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#include "libs/catch2/catch_amalgamated.hpp"

#include "contract/contractmanager.h"
#include "contract/templates/dexv2/dexv2pair.h"
#include "contract/templates/dexv2/dexv2factory.h"
#include "contract/templates/dexv2/dexv2router02.h"
#include "contract/templates/nativewrapper.h"
#include "contract/abi.h"

#include "utils/db.h"
#include "utils/options.h"

#include "core/state.h"

#include "../../sdktestsuite.hpp"

#include <utility>

// TODO: test events if/when implemented
// TODO: implement the rest of the coverage:
// swapExactTokensForTokens, swapTokensForExactTokens,
// swapExactNativeForTokens, swapTokensForExactNative,
// swapExactTokensForNative, swapNativeForExactTokens
// also whatever's left of DEXV2Pair if applicable

namespace TUQ112x112 {
  TEST_CASE("UQ112x112 Namespace Test", "[integration][contract][dexv2][uq112x112]") {
    SECTION("UQ112x112 Coverage") {
      // Q112 = 5192296858534827628530496329220096
      uint224_t enc = UQ112x112::encode(1024);
      REQUIRE(enc == uint224_t("5316911983139663491615228241121378304"));
      uint224_t div = UQ112x112::uqdiv(uint224_t("123456789000"), uint112_t("1234567890"));
      REQUIRE(div == uint224_t(100));
    }
  }
}

namespace TDEXV2 {
  TEST_CASE("DEXV2 Pair Test", "[integration][contract][dexv2][dexv2pair]") {
    SECTION("Deploy + Dump DEXV2Pair") {
      Address pair;
      Address tokenA;
      Address tokenB;
      Address chainOwner(bytes::hex("0x00dead00665771855a34155f5e7405489df2c3c6"));
      std::unique_ptr<Options> options;
      {
        SDKTestSuite sdk = SDKTestSuite::createNewEnvironment("testDEXV2Pair");
        pair = sdk.deployContract<DEXV2Pair>();
        tokenA = sdk.deployContract<ERC20>(
          std::string("TestTokenA"), std::string("TSTA"), uint8_t(18), uint256_t("1000000000000000000")
        );
        tokenB = sdk.deployContract<ERC20>(
          std::string("TestTokenB"), std::string("TSTB"), uint8_t(18), uint256_t("1000000000000000000")
        );
        sdk.callFunction(pair, &DEXV2Pair::initialize, tokenA, tokenB);
        REQUIRE(sdk.callViewFunction(pair, &DEXV2Pair::factory) == chainOwner);
        REQUIRE(sdk.callViewFunction(pair, &DEXV2Pair::token0) == tokenA);
        REQUIRE(sdk.callViewFunction(pair, &DEXV2Pair::token1) == tokenB);
        REQUIRE(sdk.callViewFunction(pair, &DEXV2Pair::price0CumulativeLast) == 0);
        REQUIRE(sdk.callViewFunction(pair, &DEXV2Pair::price1CumulativeLast) == 0);
        REQUIRE(sdk.callViewFunction(pair, &DEXV2Pair::kLast) == 0);
        std::tuple<uint256_t, uint256_t, uint256_t> reservesOutput = sdk.callViewFunction(pair, &DEXV2Pair::getReserves);
        REQUIRE(std::get<0>(reservesOutput) == 0);
        REQUIRE(std::get<1>(reservesOutput) == 0);
        REQUIRE(std::get<2>(reservesOutput) == 0);

        // Dump to database
        options = std::make_unique<Options>(sdk.getOptions());
        sdk.saveSnapshot();
      }

      // SDKTestSuite should automatically load the state from the DB if we construct it with an Options object
      // (The createNewEnvironment DELETES the DB if any is found)
      SDKTestSuite sdk(*options);
      REQUIRE(sdk.callViewFunction(pair, &DEXV2Pair::factory) == chainOwner);
      REQUIRE(sdk.callViewFunction(pair, &DEXV2Pair::token0) == tokenA);
      REQUIRE(sdk.callViewFunction(pair, &DEXV2Pair::token1) == tokenB);
      REQUIRE(sdk.callViewFunction(pair, &DEXV2Pair::price0CumulativeLast) == 0);
      REQUIRE(sdk.callViewFunction(pair, &DEXV2Pair::price1CumulativeLast) == 0);
      REQUIRE(sdk.callViewFunction(pair, &DEXV2Pair::kLast) == 0);
      std::tuple<uint256_t, uint256_t, uint256_t> reservesOutput = sdk.callViewFunction(pair, &DEXV2Pair::getReserves);
      REQUIRE(std::get<0>(reservesOutput) == 0);
      REQUIRE(std::get<1>(reservesOutput) == 0);
      REQUIRE(std::get<2>(reservesOutput) == 0);
    }
  }

  TEST_CASE("DEXV2 Router Test", "[integration][contract][dexv2][dexv2router]") {
    SECTION("Deploy + Dump DEXV2Router/Factory with a single pair") {
      Address tokenA;
      Address tokenB;
      Address wrapped;
      Address factory;
      Address router;
      Address pair;
      std::unique_ptr<Options> options;
      {
        SDKTestSuite sdk = SDKTestSuite::createNewEnvironment("testDEXV2RouterSinglePair");
        tokenA = sdk.deployContract<ERC20>(std::string("TokenA"), std::string("TKNA"), uint8_t(18), uint256_t("10000000000000000000000"));
        tokenB = sdk.deployContract<ERC20>(std::string("TokenB"), std::string("TKNB"), uint8_t(18), uint256_t("10000000000000000000000"));
        wrapped = sdk.deployContract<NativeWrapper>(std::string("WSPARQ"), std::string("WSPARQ"), uint8_t(18));
        factory = sdk.deployContract<DEXV2Factory>(Address());
        router = sdk.deployContract<DEXV2Router02>(factory, wrapped);
        sdk.callFunction(factory, &DEXV2Factory::createPair, tokenA, tokenB);
        pair = sdk.callViewFunction(factory, &DEXV2Factory::getPairByIndex, uint64_t(0));
        for (const auto& contract : sdk.getState().getCppContracts()) {
          if (contract.first == "TokenA") REQUIRE(contract.second == tokenA);
          if (contract.first == "TokenB") REQUIRE(contract.second == tokenB);
          if (contract.first == "NativeWrapper") REQUIRE(contract.second == wrapped);
          if (contract.first == "DEXV2Factory")  REQUIRE(contract.second == factory);
          if (contract.first == "DEXV2Router02") REQUIRE(contract.second == router);
        }
        // Dump to database
        options = std::make_unique<Options>(sdk.getOptions());
        sdk.saveSnapshot();
      }

      // SDKTestSuite should automatically load the state from the DB if we construct it with an Options object
      // (The createNewEnvironment DELETES the DB if any is found)
      SDKTestSuite sdk(*options);
      for (const auto& contract : sdk.getState().getCppContracts()) {
        if (contract.first == "TokenA") REQUIRE(contract.second == tokenA);
        if (contract.first == "TokenB") REQUIRE(contract.second == tokenB);
        if (contract.first == "NativeWrapper") REQUIRE(contract.second == wrapped);
        if (contract.first == "DEXV2Factory")  REQUIRE(contract.second == factory);
        if (contract.first == "DEXV2Router02") REQUIRE(contract.second == router);
      }

      // For coverage
      REQUIRE(sdk.callViewFunction(router, &DEXV2Router02::factory) == factory);
      REQUIRE(sdk.callViewFunction(router, &DEXV2Router02::wrappedNative) == wrapped);
      REQUIRE(sdk.callViewFunction(factory, &DEXV2Factory::feeTo) == Address());
      REQUIRE(sdk.callViewFunction(factory, &DEXV2Factory::feeToSetter) == Address());
      REQUIRE(sdk.callViewFunction(factory, &DEXV2Factory::allPairsLength) == 1);
      std::vector<Address> allPairs = sdk.callViewFunction(factory, &DEXV2Factory::allPairs);
      REQUIRE(allPairs.size() == 1);
      REQUIRE(allPairs[0] == pair);
      Address add(bytes::hex("0x1234567890123456789012345678901234567890"));
      sdk.callFunction(factory, &DEXV2Factory::setFeeTo, add);
      sdk.callFunction(factory, &DEXV2Factory::setFeeToSetter, add);
      REQUIRE(sdk.callViewFunction(factory, &DEXV2Factory::feeTo) == add);
      REQUIRE(sdk.callViewFunction(factory, &DEXV2Factory::feeToSetter) == add);
      REQUIRE(sdk.callViewFunction(factory, &DEXV2Factory::getPair, tokenA, factory) == Address());

      // For coverage (createPair)
      REQUIRE_THROWS(sdk.callFunction(factory, &DEXV2Factory::createPair, pair, pair)); // Identical addresses
      REQUIRE_THROWS(sdk.callFunction(factory, &DEXV2Factory::createPair, Address(), pair)); // Zero address
      REQUIRE_THROWS(sdk.callFunction(factory, &DEXV2Factory::createPair, tokenA, tokenB)); // Pair exists
    }

    SECTION("Deploy DEXV2 and add/remove liquidity to token/token pair") {
      SDKTestSuite sdk = SDKTestSuite::createNewEnvironment("testDEXV2RouterLiqTokenTokenPair");
      Address tokenA = sdk.deployContract<ERC20>(std::string("TokenA"), std::string("TKNA"), uint8_t(18), uint256_t("10000000000000000000000"));
      Address tokenB = sdk.deployContract<ERC20>(std::string("TokenB"), std::string("TKNB"), uint8_t(18), uint256_t("10000000000000000000000"));
      Address wrapped = sdk.deployContract<NativeWrapper>(std::string("WSPARQ"), std::string("WSPARQ"), uint8_t(18));
      Address factory = sdk.deployContract<DEXV2Factory>(Address());
      Address router = sdk.deployContract<DEXV2Router02>(factory, wrapped);
      Address owner = sdk.getChainOwnerAccount().address;
      for (const auto& contract : sdk.getState().getCppContracts()) {
        if (contract.first == "NativeWrapper") REQUIRE(contract.second == wrapped);
        if (contract.first == "DEXV2Factory")  REQUIRE(contract.second == factory);
        if (contract.first == "DEXV2Router02") REQUIRE(contract.second == router);
      }

      // Approve "router" so it can spend up to 10000 tokens from both sides
      // on behalf of "owner" (which already has the tokens).
      Hash approveATx = sdk.callFunction(tokenA, &ERC20::approve, router, uint256_t("10000000000000000000000"));
      Hash approveBTx = sdk.callFunction(tokenB, &ERC20::approve, router, uint256_t("10000000000000000000000"));
      REQUIRE(sdk.callViewFunction(tokenA, &ERC20::allowance, owner, router) == uint256_t("10000000000000000000000"));
      REQUIRE(sdk.callViewFunction(tokenB, &ERC20::allowance, owner, router) == uint256_t("10000000000000000000000"));
      REQUIRE(sdk.callViewFunction(tokenA, &ERC20::balanceOf, owner) == uint256_t("10000000000000000000000"));
      REQUIRE(sdk.callViewFunction(tokenB, &ERC20::balanceOf, owner) == uint256_t("10000000000000000000000"));

      // Add liquidity of 100 from A and 250 from B
      uint256_t deadline = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::system_clock::now().time_since_epoch()
      ).count() + 60000000;  // 60 seconds
      // tokenA, tokenB, amountADesired, amountBDesired, amountAMin, amountBMin, to, deadline
      Hash addLiquidityTx = sdk.callFunction(router, &DEXV2Router02::addLiquidity,
        tokenA, tokenB, uint256_t("100000000000000000000"), uint256_t("250000000000000000000"),
        uint256_t(0), uint256_t(0), owner, deadline
      );

      // Check if operation worked sucessfully
      Address pair = sdk.callViewFunction(factory, &DEXV2Factory::getPair, tokenA, tokenB);
      uint256_t ownerTknA = sdk.callViewFunction(tokenA, &ERC20::balanceOf, owner);
      uint256_t ownerTknB = sdk.callViewFunction(tokenB, &ERC20::balanceOf, owner);
      uint256_t pairTknA = sdk.callViewFunction(tokenA, &ERC20::balanceOf, pair);
      uint256_t pairTknB = sdk.callViewFunction(tokenB, &ERC20::balanceOf, pair);
      REQUIRE(ownerTknA == uint256_t("9900000000000000000000"));
      REQUIRE(ownerTknB == uint256_t("9750000000000000000000"));
      REQUIRE(pairTknA == uint256_t("100000000000000000000"));
      REQUIRE(pairTknB == uint256_t("250000000000000000000"));

      // Approve "pair" so it can allow up to 10000 liquidity tokens to be
      // withdrawn by the "owner" (which has much less than that)
      Hash approvePairTx = sdk.callFunction(pair, &ERC20::approve, router, uint256_t("10000000000000000000000"));
      REQUIRE(sdk.callViewFunction(pair, &ERC20::allowance, owner, router) == uint256_t("10000000000000000000000"));
      REQUIRE(sdk.callViewFunction(pair, &ERC20::balanceOf, owner) == uint256_t("158113883008418965599"));

      // Remove 50 liquidity tokens from the pair
      deadline = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::system_clock::now().time_since_epoch()
      ).count() + 60000000;  // 60 seconds
      // tokenA, tokenB, liquidity, amountAMin, amountBMin, to, deadline
      Hash removeLiquidityTx = sdk.callFunction(router, &DEXV2Router02::removeLiquidity,
        tokenA, tokenB, uint256_t("50000000000000000000"), uint256_t(0), uint256_t(0), owner, deadline
      );

      // Check if operation worked successfully
      pair = sdk.callViewFunction(factory, &DEXV2Factory::getPair, tokenA, tokenB);
      ownerTknA = sdk.callViewFunction(tokenA, &ERC20::balanceOf, owner);
      ownerTknB = sdk.callViewFunction(tokenB, &ERC20::balanceOf, owner);
      pairTknA = sdk.callViewFunction(tokenA, &ERC20::balanceOf, pair);
      pairTknB = sdk.callViewFunction(tokenB, &ERC20::balanceOf, pair);
      REQUIRE(ownerTknA == uint256_t("9931622776601683793320"));
      REQUIRE(ownerTknB == uint256_t("9829056941504209483300"));
      REQUIRE(pairTknA == uint256_t("68377223398316206680"));
      REQUIRE(pairTknB == uint256_t("170943058495790516700"));

      // For coverage (ensure() and throws on removeLiquidity())
      REQUIRE_THROWS(sdk.callFunction(router, &DEXV2Router02::removeLiquidity,
        tokenA, tokenB, uint256_t("5000000000000000000"),
        uint256_t(0), uint256_t(0), owner, uint256_t(0) // deadline always expired
      ));
      REQUIRE_THROWS(sdk.callFunction(router, &DEXV2Router02::removeLiquidity,
        tokenA, tokenB, uint256_t("5000000000000000000"),
        uint256_t("500000000000000000000"), uint256_t(0), owner, deadline // insufficient amountA (500)
      ));
      REQUIRE_THROWS(sdk.callFunction(router, &DEXV2Router02::removeLiquidity,
        tokenA, tokenB, uint256_t("5000000000000000000"),
        uint256_t(0), uint256_t("500000000000000000000"), owner, deadline // insufficient amountB (500)
      ));
      // For coverage (sync and skim)
      sdk.callFunction(pair, &DEXV2Pair::sync);
      sdk.callFunction(pair, &DEXV2Pair::skim, owner);
    }

    SECTION("Deploy DEXV2 and add/remove liquidity to token/native pair") {
      SDKTestSuite sdk = SDKTestSuite::createNewEnvironment("testDEXV2RouterLiqTokenNativePair");
      Address tokenA = sdk.deployContract<ERC20>(std::string("TokenA"), std::string("TKNA"), uint8_t(18), uint256_t("10000000000000000000000"));
      Address wrapped = sdk.deployContract<NativeWrapper>(std::string("WSPARQ"), std::string("WSPARQ"), uint8_t(18));
      Address factory = sdk.deployContract<DEXV2Factory>(Address());
      Address router = sdk.deployContract<DEXV2Router02>(factory, wrapped);
      Address owner = sdk.getChainOwnerAccount().address;
      for (const auto& contract : sdk.getState().getCppContracts()) {
        if (contract.first == "NativeWrapper") REQUIRE(contract.second == wrapped);
        if (contract.first == "DEXV2Factory")  REQUIRE(contract.second == factory);
        if (contract.first == "DEXV2Router02") REQUIRE(contract.second == router);
      }

      // Approve "router" so it can spend up to 10000 TKNA on behalf of "owner"
      Hash approveATx = sdk.callFunction(tokenA, &ERC20::approve, router, uint256_t("10000000000000000000000"));
      REQUIRE(sdk.callViewFunction(tokenA, &ERC20::allowance, owner, router) == uint256_t("10000000000000000000000"));
      REQUIRE(sdk.callViewFunction(tokenA, &ERC20::balanceOf, owner) == uint256_t("10000000000000000000000"));

      uint256_t ownerNativeBeforeAddLiq = sdk.getNativeBalance(owner);
      // Add liquidity of 100 WSPARQ and 100 TKNA
      uint256_t deadline = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::system_clock::now().time_since_epoch()
      ).count() + 60000000;  // 60 seconds
      // token, amountTokenDesired, amountTokenMin, amountNativeMin, to, deadline
      Hash addLiquidityTx = sdk.callFunction(
        router, uint256_t("100000000000000000000"), &DEXV2Router02::addLiquidityNative,
        tokenA, uint256_t("100000000000000000000"), uint256_t("100000000000000000000"),
        uint256_t("100000000000000000000"), owner, deadline
      );

      // Check if operation worked successfully
      Address pair = sdk.callViewFunction(factory, &DEXV2Factory::getPair, tokenA, wrapped);
      uint256_t ownerTknA = sdk.callViewFunction(tokenA, &ERC20::balanceOf, owner);
      uint256_t ownerNative = sdk.getNativeBalance(owner);
      uint256_t pairTknA = sdk.callViewFunction(tokenA, &ERC20::balanceOf, pair);
      uint256_t wrappedNative = sdk.getNativeBalance(wrapped);
      uint256_t pairNativeWrapped = sdk.callViewFunction(wrapped, &ERC20::balanceOf, pair);
      REQUIRE(ownerTknA == uint256_t("9900000000000000000000"));
      REQUIRE(ownerNative <= ownerNativeBeforeAddLiq - uint256_t("100000000000000000000") - (uint256_t(1000000000) * 21000));
      REQUIRE(pairTknA == uint256_t("100000000000000000000"));
      REQUIRE(wrappedNative == uint256_t("100000000000000000000"));
      REQUIRE(pairNativeWrapped == uint256_t("100000000000000000000"));

      // Approve "pair" so it can allow up to 10000 liquidity tokens to be
      // withdrawn by the "owner" (which has much less than that)
      Hash approvePairTx = sdk.callFunction(pair, &ERC20::approve, router, uint256_t("10000000000000000000000"));
      REQUIRE(sdk.callViewFunction(pair, &ERC20::allowance, owner, router) == uint256_t("10000000000000000000000"));
      REQUIRE(sdk.callViewFunction(pair, &ERC20::balanceOf, owner) == uint256_t("99999999999999999000"));

      uint256_t ownerNativeBeforeSubLiq = sdk.getNativeBalance(owner);
      // Remove 50 liquidity tokens
      deadline = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::system_clock::now().time_since_epoch()
      ).count() + 60000000;  // 60 seconds
      // token, liquidity, amountTokenMin, amountNativeMin, to, deadline
      Hash removeLiquidityTx = sdk.callFunction(
        router, uint256_t("100000000000000000000"), &DEXV2Router02::removeLiquidityNative,
        tokenA, uint256_t("50000000000000000000"), uint256_t("10000000000000000000"),
        uint256_t("10000000000000000000"), owner, deadline
      );

      // Check if operation worked successfully
      pair = sdk.callViewFunction(factory, &DEXV2Factory::getPair, tokenA, wrapped);
      ownerTknA = sdk.callViewFunction(tokenA, &ERC20::balanceOf, owner);
      ownerNative = sdk.getNativeBalance(owner);
      pairTknA = sdk.callViewFunction(tokenA, &ERC20::balanceOf, pair);
      wrappedNative = sdk.getNativeBalance(wrapped);
      pairNativeWrapped = sdk.callViewFunction(wrapped, &ERC20::balanceOf, pair);
      REQUIRE(ownerTknA == uint256_t("9950000000000000000000"));
      REQUIRE(ownerNative >= ownerNativeBeforeSubLiq - uint256_t("100000000000000000000") - (uint256_t(1000000000) * 21000));
      REQUIRE(pairTknA == uint256_t("50000000000000000000"));
      REQUIRE(wrappedNative == uint256_t("50000000000000000000"));
      REQUIRE(pairNativeWrapped == uint256_t("50000000000000000000"));
    }
  }
}

