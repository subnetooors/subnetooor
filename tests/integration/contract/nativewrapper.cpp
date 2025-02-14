/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#include "libs/catch2/catch_amalgamated.hpp"

#include "contract/templates/erc20.h"
#include "contract/templates/nativewrapper.h"
#include "contract/abi.h"
#include "contract/contractmanager.h"

#include "utils/db.h"
#include "utils/options.h"

#include "core/storage.h"
#include "core/state.h"

#include "../../sdktestsuite.hpp"

// TODO: test events if/when implemented

namespace TNativeWrapper {
  TEST_CASE("NativeWrapper tests", "[integration][contract][nativewrapper]") {
    SECTION("NativeWrapper creation") {
      SDKTestSuite sdk = SDKTestSuite::createNewEnvironment("testNativeWrapperCreation");
      Address nativeWrapper = sdk.deployContract<NativeWrapper>(
        std::string("WrappedToken"), std::string("WTKN"), uint8_t(18)
      );
      Address owner = sdk.getChainOwnerAccount().address;
      REQUIRE(sdk.callViewFunction(nativeWrapper, &NativeWrapper::name) == "WrappedToken");
      REQUIRE(sdk.callViewFunction(nativeWrapper, &NativeWrapper::symbol) == "WTKN");
      REQUIRE(sdk.callViewFunction(nativeWrapper, &NativeWrapper::decimals) == 18);
      REQUIRE(sdk.callViewFunction(nativeWrapper, &NativeWrapper::totalSupply) == 0);
    }

    SECTION("NativeWrapper deposit() and withdraw()") {
      SDKTestSuite sdk = SDKTestSuite::createNewEnvironment("testNativeWrapperDepositAndWithdraw");

      const uint256_t gasPrice = 1'000'000'000;

      Address nativeWrapper = sdk.deployContract<NativeWrapper>(
        std::string("WrappedToken"), std::string("WTKN"), uint8_t(18)
      );
      Address owner = sdk.getChainOwnerAccount().address;
      uint256_t amountToTransfer = uint256_t("192838158112259");
      uint256_t amountToWithdraw = amountToTransfer / 3;

      Hash depositTx = sdk.callFunction(nativeWrapper, &NativeWrapper::deposit, amountToTransfer);

      REQUIRE(sdk.getNativeBalance(nativeWrapper) == amountToTransfer);

      uint256_t expectedGasUsed = gasPrice * (uint256_t(CONTRACT_EXECUTION_COST) * 2 + CPP_CONTRACT_CREATION_COST + CPP_CONTRACT_CALL_COST);

      REQUIRE(sdk.getNativeBalance(owner) == uint256_t("1000000000000000000000") - amountToTransfer - expectedGasUsed);
      REQUIRE(sdk.callViewFunction(nativeWrapper, &NativeWrapper::balanceOf, owner) == amountToTransfer);

      Hash withdrawTx = sdk.callFunction(nativeWrapper, &NativeWrapper::withdraw, amountToWithdraw);
      expectedGasUsed += gasPrice * (CONTRACT_EXECUTION_COST + CPP_CONTRACT_CALL_COST);

      REQUIRE(sdk.getNativeBalance(nativeWrapper) == amountToTransfer - amountToWithdraw);
      REQUIRE(sdk.getNativeBalance(owner) == uint256_t("1000000000000000000000") - amountToTransfer + amountToWithdraw - expectedGasUsed);
      REQUIRE(sdk.callViewFunction(nativeWrapper, &NativeWrapper::balanceOf, owner) == amountToTransfer - amountToWithdraw);
    }
  }
}
