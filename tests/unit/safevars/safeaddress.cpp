/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#include "libs/catch2/catch_amalgamated.hpp"

#include "contract/variables/safeaddress.h"

#include "utils/utils.h"

namespace TSafeAddress {
  TEST_CASE("SafeAddress Class", "[unit][safevars][safeaddress]") {
    SECTION("Constructor") {
      Address add(Utils::randBytes(20));
      SafeAddress safeAdd0; // Empty
      SafeAddress safeAdd1(add); // Normal
      SafeAddress safeAdd2(safeAdd1); // Copy
      REQUIRE(safeAdd0 == Address());
      REQUIRE(safeAdd1 == add);
      REQUIRE(safeAdd2 == add);
      REQUIRE(safeAdd2 == safeAdd1);
    }

    SECTION("operator=") {
      Address add(Utils::randBytes(20));
      Address add2(Utils::randBytes(20));
      SafeAddress safeAdd1;
      SafeAddress safeAdd2;
      SafeAddress safeAdd3; // Empty on purpose
      // Set a value and revert - result should be Address() (copy == nullptr)
      safeAdd1 = add;
      safeAdd1.revert();
      REQUIRE(safeAdd1 == Address());
      // Set a value and commit - result should be the set value
      safeAdd1 = add;
      safeAdd1.commit();
      REQUIRE(safeAdd1 == add);
      // Set another value and revert - result should be the previous value
      safeAdd1 = add2;
      safeAdd1.revert();
      REQUIRE(safeAdd1 == add);
      // Set another value and commit - result should be the new value
      safeAdd1 = add2;
      safeAdd1.commit();
      REQUIRE(safeAdd1 == add2);
      // Copy a value and revert - result should be Address() (copy2 == nullptr)
      safeAdd2 = safeAdd1;
      safeAdd2.revert();
      REQUIRE(safeAdd2 == Address());
      // Copy a value and commit - result should be the set value
      safeAdd2 = safeAdd1;
      safeAdd2.commit();
      REQUIRE(safeAdd2 == safeAdd1);
      // Copy another value and revert - result should be the previous value
      safeAdd2 = safeAdd3;
      safeAdd2.revert();
      REQUIRE(safeAdd2 == safeAdd1);
      // Copy another value and commit - result should be Address() (safeAdd3 was never set)
      safeAdd2 = safeAdd3;
      safeAdd2.commit();
      REQUIRE(safeAdd2 == safeAdd3);
    }
  }
}

