/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#include "libs/catch2/catch_amalgamated.hpp"

#include "contract/templates/throwtestA.h"
#include "contract/templates/throwtestB.h"
#include "contract/templates/throwtestC.h"

#include "../../sdktestsuite.hpp"

namespace TThrowTest {
  TEST_CASE("ThrowTest Contracts", "[integration][contract][throwtest]") {
    SECTION("ThrowTest Coverage") {
      // Everything in one section because yes
      Address throwA;
      Address throwB;
      Address throwC;
      std::unique_ptr<Options> options;
      {
        SDKTestSuite sdk = SDKTestSuite::createNewEnvironment("testThrowTestCreation");
        throwA = sdk.deployContract<ThrowTestA>();
        throwB = sdk.deployContract<ThrowTestB>();
        throwC = sdk.deployContract<ThrowTestC>();
        REQUIRE(sdk.callViewFunction(throwA, &ThrowTestA::getNumA) == 0);
        REQUIRE(sdk.callViewFunction(throwB, &ThrowTestB::getNumB) == 0);
        REQUIRE(sdk.callViewFunction(throwC, &ThrowTestC::getNumC) == 0);

        // A and B always throw, C is the only one that keeps the value
        REQUIRE_THROWS(sdk.callFunction(throwA, &ThrowTestA::setNumA, uint8_t(1), throwB, uint8_t(2), throwC, uint8_t(3)));
        REQUIRE(sdk.callViewFunction(throwA, &ThrowTestA::getNumA) == 0);
        REQUIRE(sdk.callViewFunction(throwB, &ThrowTestB::getNumB) == 0);
        REQUIRE(sdk.callViewFunction(throwC, &ThrowTestC::getNumC) == 0);
        REQUIRE_THROWS(sdk.callFunction(throwB, &ThrowTestB::setNumB, uint8_t(4), throwC, uint8_t(5)));
        REQUIRE(sdk.callViewFunction(throwA, &ThrowTestA::getNumA) == 0);
        REQUIRE(sdk.callViewFunction(throwB, &ThrowTestB::getNumB) == 0);
        REQUIRE(sdk.callViewFunction(throwC, &ThrowTestC::getNumC) == 0);
        sdk.callFunction(throwC, &ThrowTestC::setNumC, uint8_t(6));
        REQUIRE(sdk.callViewFunction(throwA, &ThrowTestA::getNumA) == 0);
        REQUIRE(sdk.callViewFunction(throwB, &ThrowTestB::getNumB) == 0);
        REQUIRE(sdk.callViewFunction(throwC, &ThrowTestC::getNumC) == 6);

        // Dump to database
        options = std::make_unique<Options>(sdk.getOptions());
        sdk.saveSnapshot();
      }

      // SDKTestSuite should automatically load the state from the DB if we construct it with an Options object
      // (The createNewEnvironment DELETES the DB if any is found)
      SDKTestSuite sdk(*options);
      REQUIRE(sdk.callViewFunction(throwA, &ThrowTestA::getNumA) == 0);
      REQUIRE(sdk.callViewFunction(throwB, &ThrowTestB::getNumB) == 0);
      REQUIRE(sdk.callViewFunction(throwC, &ThrowTestC::getNumC) == 6);
    }
  }
}

