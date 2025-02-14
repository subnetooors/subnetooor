/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#include "libs/catch2/catch_amalgamated.hpp"

#include "contract/variables/safetuple.h"

namespace TSafeTuple {
  TEST_CASE("SafeTuple Class", "[unit][safevars][safetuple]") {
    SECTION("Constructor") {
      SafeTuple<int, double, std::string> emptyTup;
      SafeTuple<int, double, std::string> tup(std::make_tuple(10, 1.0, "aaa"));
      std::tuple<int, double, std::string> tupRaw(std::make_tuple(20, 2.0, "bbb"));
      SafeTuple<int, double, std::string> tup2(std::move(tupRaw));
      SafeTuple<int, double, std::string> copyTup(tup);
      SafeTuple<int, double, std::string> copyTup2(tup);
      SafeTuple<int, double, std::string> moveTup(std::move(copyTup2));
      SafeTuple<int, double> pairTup(std::make_pair(1000, 100.0));
      REQUIRE(copyTup == tup);
      REQUIRE(moveTup == tup);
      REQUIRE(get<0>(tup) == 10);
      REQUIRE(get<1>(tup) == 1.0);
      REQUIRE(get<2>(tup) == "aaa");
      REQUIRE(get<0>(tup2) == 20);
      REQUIRE(get<1>(tup2) == 2.0);
      REQUIRE(get<2>(tup2) == "bbb");
      REQUIRE(get<0>(pairTup) == 1000);
      REQUIRE(get<1>(pairTup) == 100.0);
    }

    SECTION("get") {
      SafeTuple<int, double, std::string> tup(std::make_tuple(10, 1.0, "aaa"));
      // non-member const get (get<i>(std::as_const(tup)))
      int i = get<0>(std::as_const(tup));
      REQUIRE((i == 10 && get<0>(std::as_const(tup)) == 10));
      // non-member non-const get (get<i>(tup))
      get<0>(tup) = 20;
      get<1>(tup) = 2.0;
      get<2>(tup) = "bbb";
      tup.revert();
      REQUIRE(get<0>(std::as_const(tup)) == 10);
      REQUIRE(get<1>(std::as_const(tup)) == 1.0);
      REQUIRE(get<2>(std::as_const(tup)) == "aaa");
      get<0>(tup) = 20;
      get<1>(tup) = 2.0;
      get<2>(tup) = "bbb";
      tup.commit();
      REQUIRE(get<0>(std::as_const(tup)) == 20);
      REQUIRE(get<1>(std::as_const(tup)) == 2.0);
      REQUIRE(get<2>(std::as_const(tup)) == "bbb");
      // member non-const get (tup.get<i>())
      tup.get<0>() = 30;
      tup.get<1>() = 3.0;
      tup.get<2>() = "ccc";
      tup.revert();
      REQUIRE(get<0>(std::as_const(tup)) == 20);
      REQUIRE(get<1>(std::as_const(tup)) == 2.0);
      REQUIRE(get<2>(std::as_const(tup)) == "bbb");
      tup.get<0>() = 30;
      tup.get<1>() = 3.0;
      tup.get<2>() = "ccc";
      tup.commit();
      REQUIRE(get<0>(std::as_const(tup)) == 30);
      REQUIRE(get<1>(std::as_const(tup)) == 3.0);
      REQUIRE(get<2>(std::as_const(tup)) == "ccc");
    }

    SECTION("raw") {
      SafeTuple<int, double, std::string> tup(std::make_tuple(10, 1.0, "aaa"));
      std::tuple<int, double, std::string> tupRaw = tup.raw();
      REQUIRE(std::get<0>(tupRaw) == 10);
      REQUIRE(std::get<1>(tupRaw) == 1.0);
      REQUIRE(std::get<2>(tupRaw) == "aaa");
    }

    SECTION("operator=") {
      SafeTuple<int, double, std::string> tup(std::make_tuple(10, 1.0, "aaa"));
      // assign by copy
      SafeTuple<int, double, std::string> tup2(std::make_tuple(20, 2.0, "bbb"));
      tup = tup2;
      tup.revert();
      REQUIRE(get<0>(std::as_const(tup)) == 10);
      REQUIRE(get<1>(std::as_const(tup)) == 1.0);
      REQUIRE(get<2>(std::as_const(tup)) == "aaa");
      tup = tup2;
      tup.commit();
      REQUIRE(get<0>(std::as_const(tup)) == 20);
      REQUIRE(get<1>(std::as_const(tup)) == 2.0);
      REQUIRE(get<2>(std::as_const(tup)) == "bbb");
      // assign by move
      SafeTuple<int, double, std::string> tup3A(std::make_tuple(30, 3.0, "ccc"));
      SafeTuple<int, double, std::string> tup3B(std::make_tuple(30, 3.0, "ccc"));
      tup = std::move(tup3A);
      tup.revert();
      REQUIRE(get<0>(std::as_const(tup)) == 20);
      REQUIRE(get<1>(std::as_const(tup)) == 2.0);
      REQUIRE(get<2>(std::as_const(tup)) == "bbb");
      tup = std::move(tup3B);
      tup.commit();
      REQUIRE(get<0>(std::as_const(tup)) == 30);
      REQUIRE(get<1>(std::as_const(tup)) == 3.0);
      REQUIRE(get<2>(std::as_const(tup)) == "ccc");
      // assign by implicit conversion
      SafeTuple<short, float, const char*> tup3(std::make_tuple(40, 4.0, "ddd"));
      tup = tup3;
      tup.revert();
      REQUIRE(get<0>(std::as_const(tup)) == 30);
      REQUIRE(get<1>(std::as_const(tup)) == 3.0);
      REQUIRE(get<2>(std::as_const(tup)) == "ccc");
      tup = tup3;
      tup.commit();
      REQUIRE(get<0>(std::as_const(tup)) == 40);
      REQUIRE(get<1>(std::as_const(tup)) == 4.0);
      REQUIRE(get<2>(std::as_const(tup)) == "ddd");
      // assign by pair
      SafeTuple<int, double> pairTup(std::make_pair(100, 100.0));
      std::pair<int, double> pair = std::make_pair(200, 200.0);
      pairTup = pair;
      pairTup.revert();
      REQUIRE(get<0>(std::as_const(pairTup)) == 100);
      REQUIRE(get<1>(std::as_const(pairTup)) == 100.0);
      pairTup = pair;
      pairTup.commit();
      REQUIRE(get<0>(std::as_const(pairTup)) == 200);
      REQUIRE(get<1>(std::as_const(pairTup)) == 200.0);
    }

    SECTION("swap") {
      SafeTuple<int, double, std::string> tupA1(std::make_tuple(10, 1.0, "aaa"));
      SafeTuple<int, double, std::string> tupA2(std::make_tuple(20, 2.0, "bbb"));
      SafeTuple<int, double, std::string> tupB1(std::make_tuple(30, 3.0, "ccc"));
      SafeTuple<int, double, std::string> tupB2(std::make_tuple(40, 4.0, "ddd"));
      // member func (tup.swap())
      tupA1.swap(tupA2);
      tupA1.revert();
      tupA2.revert();
      REQUIRE(get<0>(tupA1) == 10);
      REQUIRE(get<1>(tupA1) == 1.0);
      REQUIRE(get<2>(tupA1) == "aaa");
      REQUIRE(get<0>(tupA2) == 20);
      REQUIRE(get<1>(tupA2) == 2.0);
      REQUIRE(get<2>(tupA2) == "bbb");
      tupA1.swap(tupA2);
      tupA1.commit();
      tupA2.commit();
      REQUIRE(get<0>(tupA1) == 20);
      REQUIRE(get<1>(tupA1) == 2.0);
      REQUIRE(get<2>(tupA1) == "bbb");
      REQUIRE(get<0>(tupA2) == 10);
      REQUIRE(get<1>(tupA2) == 1.0);
      REQUIRE(get<2>(tupA2) == "aaa");
      // non-member func (swap(tup1, tup2))
      swap(tupB1, tupB2);
      tupB1.revert();
      tupB2.revert();
      REQUIRE(get<0>(tupB1) == 30);
      REQUIRE(get<1>(tupB1) == 3.0);
      REQUIRE(get<2>(tupB1) == "ccc");
      REQUIRE(get<0>(tupB2) == 40);
      REQUIRE(get<1>(tupB2) == 4.0);
      REQUIRE(get<2>(tupB2) == "ddd");
      swap(tupB1, tupB2);
      tupB1.commit();
      tupB2.commit();
      REQUIRE(get<0>(tupB1) == 40);
      REQUIRE(get<1>(tupB1) == 4.0);
      REQUIRE(get<2>(tupB1) == "ddd");
      REQUIRE(get<0>(tupB2) == 30);
      REQUIRE(get<1>(tupB2) == 3.0);
      REQUIRE(get<2>(tupB2) == "ccc");
    }

    SECTION("operator== and operator!=") {
      SafeTuple<int, double, std::string> tup1A(std::make_tuple(10, 1.0, "aaa"));
      SafeTuple<int, double, std::string> tup1B(std::make_tuple(10, 1.0, "aaa"));
      SafeTuple<int, double, std::string> tup2(std::make_tuple(20, 2.0, "aaa")); // "aaa" on purpose (not really but I decided to keep it that way)
      REQUIRE(tup1A == tup1B);
      REQUIRE(tup1A != tup2);
      REQUIRE(tup1B != tup2);
    }

    SECTION("operator< and operator>") {
      SafeTuple<int, double, std::string> tup(std::make_tuple(10, 1.0, "aaa"));
      SafeTuple<int, double, std::string> tupA(std::make_tuple(20, 1.0, "aaa"));
      SafeTuple<int, double, std::string> tupB(std::make_tuple(10, 2.0, "aaa"));
      SafeTuple<int, double, std::string> tupC(std::make_tuple(10, 1.0, "bbb"));
      SafeTuple<int, double, std::string> tupAB(std::make_tuple(20, 2.0, "aaa"));
      SafeTuple<int, double, std::string> tupBC(std::make_tuple(10, 2.0, "bbb"));
      SafeTuple<int, double, std::string> tupABC(std::make_tuple(20, 2.0, "bbb"));
      REQUIRE((tup < tupA && tup < tupB && tup < tupC && tup < tupAB && tup < tupBC && tup < tupABC));
      REQUIRE((tupA > tup && tupA > tupB && tupA > tupC && tupA < tupAB && tupA > tupBC && tupA < tupABC));
      REQUIRE((tupB > tup && tupB < tupA && tupB > tupC && tupB < tupAB && tupB < tupBC && tupB < tupABC));
      REQUIRE((tupC > tup && tupC < tupA && tupC < tupB && tupC < tupAB && tupC < tupBC && tupC < tupABC));
      REQUIRE((tupAB > tup && tupAB > tupA && tupAB > tupB && tupAB > tupC && tupAB > tupBC && tupAB < tupABC));
      REQUIRE((tupBC > tup && tupBC < tupA && tupBC > tupB && tupBC > tupC && tupBC < tupAB && tupBC < tupABC));
      REQUIRE((tupABC > tup && tupABC > tupA && tupABC > tupB && tupABC > tupC && tupABC > tupAB && tupABC > tupBC));
    }

    SECTION("operator<= and operator>=") {
      SafeTuple<int, double, std::string> tupA1(std::make_tuple(10, 1.0, "aaa"));
      SafeTuple<int, double, std::string> tupA2(std::make_tuple(10, 1.0, "aaa"));
      SafeTuple<int, double, std::string> tupB1(std::make_tuple(20, 1.0, "aaa"));
      SafeTuple<int, double, std::string> tupB2(std::make_tuple(20, 1.0, "aaa"));
      REQUIRE(tupA1 <= tupA2);
      REQUIRE(tupA2 <= tupB1);
      REQUIRE(tupB1 <= tupB2);
      REQUIRE(tupB2 >= tupB1);
      REQUIRE(tupB1 >= tupA2);
      REQUIRE(tupA2 >= tupA1);
    }
  }
}

