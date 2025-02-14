/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#include "../../src/libs/catch2/catch_amalgamated.hpp"

#include "../../src/contract/templates/dexv2/dexv2factory.h"
#include "../../src/contract/templates/dexv2/dexv2router02.h"
#include "../../src/contract/templates/nativewrapper.h"

#include "../sdktestsuite.hpp"

// TODO: test events if/when implemented

namespace TDEXV2 {
  TEST_CASE("DEXV2 Benchmark", "[benchmark][dexv2]") {
    SECTION("CPP DEXV2 Swap Benchmark") {
      SDKTestSuite sdk = SDKTestSuite::createNewEnvironment("testDEXV2LiqTokenTokenPair");
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
      // on behalf of "owner" (which already has the tokens)
      Hash approveATx = sdk.callFunction(tokenA, &ERC20::approve, router, uint256_t("10000000000000000000000"));
      REQUIRE(approveATx != Hash());
      Hash approveBTx = sdk.callFunction(tokenB, &ERC20::approve, router, uint256_t("10000000000000000000000"));
      REQUIRE(approveBTx != Hash());
      REQUIRE(sdk.callViewFunction(tokenA, &ERC20::allowance, owner, router) == uint256_t("10000000000000000000000"));
      REQUIRE(sdk.callViewFunction(tokenB, &ERC20::allowance, owner, router) == uint256_t("10000000000000000000000"));
      REQUIRE(sdk.callViewFunction(tokenA, &ERC20::balanceOf, owner) == uint256_t("10000000000000000000000"));
      REQUIRE(sdk.callViewFunction(tokenB, &ERC20::balanceOf, owner) == uint256_t("10000000000000000000000"));

      // Add liquidity of 100 from A and 250 from B
      uint256_t deadline = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::system_clock::now().time_since_epoch()
      ).count() + 60000000000;  // 60000 seconds
      // tokenA, tokenB, amountADesired, amountBDesired, amountAMin, amountBMin, to, deadline
      Hash addLiqudityTx = sdk.callFunction(router, &DEXV2Router02::addLiquidity,
        tokenA, tokenB, uint256_t("100000000000000000000"), uint256_t("250000000000000000000"),
        uint256_t(0), uint256_t(0), owner, deadline
      );
      REQUIRE (addLiqudityTx != Hash());

      // Create and do a simple swap transaction
      // tokenA, tokenB, amountIn, amountOutMin, to, deadline
      // (We will use the transaction for benchmarking later)
      std::shared_ptr<const FinalizedBlock> blockBefore = sdk.latest();
      Hash swapTx = sdk.callFunction(router, &DEXV2Router02::swapExactTokensForTokens,
        uint256_t("10000"), uint256_t(0), std::vector<Address>({ tokenA, tokenB }), owner, deadline
      );
      std::shared_ptr<const FinalizedBlock> blockAfter = sdk.latest();
      REQUIRE(blockAfter != blockBefore);
      REQUIRE(sdk.latest()->getTxs().size() == 1);
      REQUIRE(swapTx != Hash());
      TxBlock tx = (*sdk.latest()->getTxs()[0]);

      uint64_t iterations = 250000;
      auto start = std::chrono::high_resolution_clock::now();
      for (uint64_t i = 0; i < iterations; i++) sdk.benchCall(tx);
      auto end = std::chrono::high_resolution_clock::now();

      long double durationInMicroseconds = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
      long double microSecsPerCall = durationInMicroseconds / iterations;
      std::cout << "DEXV2 Swap Token to Token " << microSecsPerCall << " microseconds per call" << std::endl;
      std::cout << "CPP Total Time: " << durationInMicroseconds / 1000000 << " seconds" << std::endl;
    }
  }
}

