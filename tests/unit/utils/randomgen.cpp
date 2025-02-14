/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#include "libs/catch2/catch_amalgamated.hpp"

#include "utils/randomgen.h"

#include "bytes/random.h"

using Catch::Matchers::Equals;

namespace TRandomGen {
  TEST_CASE("RandomGen Class", "[unit][utils][randomgen]") {
    SECTION("Constructor") {
      Bytes bytes(32, 0x00);
      Hash seed(bytes);
      RandomGen generator(seed);

      uint256_t firstRandom = generator();
      uint256_t secondRandom = generator();
      uint256_t thirdRandom = generator();

      REQUIRE(firstRandom == uint256_t("18569430475105882587588266137607568536673111973893317399460219858819262702947"));
      REQUIRE(secondRandom == uint256_t("36662618139459487608036673114889889725324975350372714220936135061884814320089"));
      REQUIRE(thirdRandom == uint256_t("24167556515869808906615918001412365693631812133744141770956806246558790984502"));
    }

    SECTION("getSeed") {
      Hash seed(Hex::toBytes("a62a86472e5c224aa0a784eccaf794abb60302e2073d52ae0d095ac5d16f03a6"));
      RandomGen generator(seed);
      REQUIRE(generator.getSeed() == seed);
      auto newSeed = generator();
      REQUIRE(static_cast<uint256_t>(generator.getSeed()) == newSeed);
      newSeed = generator();
      REQUIRE(static_cast<uint256_t>(generator.getSeed()) == newSeed);
    }

    SECTION("min and max") {
      REQUIRE(RandomGen::min() == uint256_t("0"));
      REQUIRE(RandomGen::max() == uint256_t("115792089237316195423570985008687907853269984665640564039457584007913129639935"));
    }

    SECTION("shuffle") {
      std::vector<std::string> vector = {
        "First String", "Second String", "Third String", "Fourth String",
        "Fifth String", "Sixth String", "Seventh String", "Eighth String",
        "Ninth String", "Tenth String"
      };

      Hash seed(Hex::toBytes("a4dd40261fbabe977ab6ff77a7ea9f76cd3b286aa66290b0d62bdf4303f4382b"));
      RandomGen generator(seed);
      generator.shuffle(vector);

      REQUIRE_THAT(vector[0], Equals("Eighth String"));
      REQUIRE_THAT(vector[1], Equals("Fifth String"));
      REQUIRE_THAT(vector[2], Equals("Second String"));
      REQUIRE_THAT(vector[3], Equals("Tenth String"));
      REQUIRE_THAT(vector[4], Equals("Seventh String"));
      REQUIRE_THAT(vector[5], Equals("Third String"));
      REQUIRE_THAT(vector[6], Equals("Fourth String"));
      REQUIRE_THAT(vector[7], Equals("Ninth String"));
      REQUIRE_THAT(vector[8], Equals("First String"));
      REQUIRE_THAT(vector[9], Equals("Sixth String"));
    }

    SECTION("Bulk randomness generation") {
      Hash seed = bytes::random();
      RandomGen generator(seed);

      std::vector<uint256_t> randoms;
      for (int i = 0; i < 10000; i++) randoms.push_back(generator());
      std::sort(randoms.begin(), randoms.end()); // Sort for easy comparison

      // Check if duplicate values exist
      REQUIRE(std::adjacent_find(randoms.begin(), randoms.end()) == randoms.end());
    }
  }
}

