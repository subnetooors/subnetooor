/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#include "libs/catch2/catch_amalgamated.hpp"

#include "contract/variables/safebool.h"

namespace TSafeBool {
  TEST_CASE("SafeBool class", "[unit][safevars][safebool]") {
    SECTION("Constructor") {
      SafeBool bT(true);
      SafeBool bTc(bT);
      SafeBool bF; // false by default
      SafeBool bF2(false);
      SafeBool bFc(bF);
      // operator bool()
      REQUIRE(bT);
      REQUIRE(bTc);
      REQUIRE(!bF);
      REQUIRE(!bF2);
      REQUIRE(!bFc);
      // operator==
      REQUIRE(bT == true);
      REQUIRE(bTc == true);
      REQUIRE(bF == false);
      REQUIRE(bF2 == false);
      REQUIRE(bFc == false);
      REQUIRE(bTc == bT);
      REQUIRE(bFc == bF);
    }

    SECTION("operator=") {
      SafeBool sb1;
      SafeBool sb2;
      SafeBool sb3;
      // Set a value and revert - result should be default value at construction (false)
      sb1 = true;
      sb1.revert();
      REQUIRE(!sb1);
      // Set a value and commit - result should be the set value
      sb1 = true;
      sb1.commit();
      REQUIRE(sb1);
      // Set another value and revert - result should be the previous value
      sb1 = false;
      sb1.revert();
      REQUIRE(sb1);
      // Copy a value and revert - result should be default value at construction (false)
      sb2 = sb1;
      sb2.revert();
      REQUIRE(!sb2);
      // Copy a value and commit - result should be the set value
      sb2 = sb1;
      sb2.commit();
      REQUIRE(sb2);
      // Copy another value and revert - result should be the previous value
      sb2 = sb3;
      sb2.revert();
      REQUIRE(sb2);
    }
  }
}

