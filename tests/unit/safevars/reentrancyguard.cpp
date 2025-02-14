/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#include "libs/catch2/catch_amalgamated.hpp"

#include "contract/variables/reentrancyguard.h"

namespace TReentrancyGuard {
  TEST_CASE("ReentrancyGuard class", "[unit][safevars][reentrancyguard]") {
    SECTION("Constructor") {
      bool lock = false;
      ReentrancyGuard guard(lock);
      REQUIRE(lock == true);
      REQUIRE_THROWS(ReentrancyGuard(lock));
      guard.~ReentrancyGuard();
      REQUIRE(lock == false);
    }
  }
}

