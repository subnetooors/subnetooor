/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#include "libs/catch2/catch_amalgamated.hpp"

#include "contract/variables/safearray.h"

namespace TSafeArray {
  TEST_CASE("SafeArray Class", "[unit][safevars][safearray]") {
    SECTION("Constructor") { // also tests most trivial const funcs
      SafeArray<int, 0> emptyArr;
      SafeArray<int, 5> defaultArr;
      SafeArray<int, 5> arr({1,2,3,4,5});
      REQUIRE(emptyArr.empty());
      REQUIRE(emptyArr.size() == 0);
      REQUIRE(emptyArr.max_size() == 0);
      REQUIRE(!defaultArr.empty());
      REQUIRE(defaultArr.size() == 5);
      REQUIRE(defaultArr.max_size() == 5);
      REQUIRE(!arr.empty());
      REQUIRE(arr.size() == 5);
      REQUIRE(arr.max_size() == 5);
      REQUIRE(arr.front() == 1);
      REQUIRE(arr.back() == 5);
      for (std::size_t i = 0; i < 5; i++) {
        REQUIRE(std::as_const(defaultArr).at(i) == 0);
        REQUIRE(std::as_const(arr)[i] == i + 1);
      }
      int ct1 = arr.front(); // = 1
      int ct2 = arr.back(); // = 5
      for (auto it = arr.cbegin(); it != arr.cend(); it++) {
        REQUIRE((*it) == ct1);
        ct1++;
      }
      for (auto it = arr.crbegin(); it != arr.crend(); it++) {
        REQUIRE((*it) == ct2);
        ct2--;
      }
    }

    SECTION("at") {
      SafeArray<std::string, 5> arr({"a", "b", "c", "d", "e"});
      REQUIRE_THROWS(arr.at(5));
      for (std::size_t i = 0; i < arr.size(); i++) arr.at(i) = "x";
      arr.revert();
      REQUIRE(arr.at(0) == "a");
      REQUIRE(arr.at(1) == "b");
      REQUIRE(arr.at(2) == "c");
      REQUIRE(arr.at(3) == "d");
      REQUIRE(arr.at(4) == "e");
      for (std::size_t i = 0; i < arr.size(); i++) arr.at(i) = "x";
      arr.commit();
      REQUIRE(arr.at(0) == "x");
      REQUIRE(arr.at(1) == "x");
      REQUIRE(arr.at(2) == "x");
      REQUIRE(arr.at(3) == "x");
      REQUIRE(arr.at(4) == "x");
    }

    SECTION("operator[]") {
      SafeArray<std::string, 5> arr({"a", "b", "c", "d", "e"});
      for (std::size_t i = 0; i < arr.size(); i++) arr[i] = "x";
      arr.revert();
      REQUIRE(arr[0] == "a");
      REQUIRE(arr[1] == "b");
      REQUIRE(arr[2] == "c");
      REQUIRE(arr[3] == "d");
      REQUIRE(arr[4] == "e");
      for (std::size_t i = 0; i < arr.size(); i++) arr[i] = "x";
      arr.commit();
      REQUIRE(arr[0] == "x");
      REQUIRE(arr[1] == "x");
      REQUIRE(arr[2] == "x");
      REQUIRE(arr[3] == "x");
      REQUIRE(arr[4] == "x");
    }

    SECTION("front and back") {
      SafeArray<std::string, 5> arr({"a", "b", "c", "d", "e"});
      arr.front() = "x";
      arr.revert();
      REQUIRE(arr.front() == "a");
      arr.front() = "x";
      arr.commit();
      REQUIRE(arr.front() == "x");
      arr.back() = "y";
      arr.revert();
      REQUIRE(arr.back() == "e");
      arr.back() = "y";
      arr.commit();
      REQUIRE(arr.back() == "y");
    }

    SECTION("operator== and operator!=") {
      SafeArray<std::string, 5> arr1({"a", "b", "c", "d", "e"});
      SafeArray<std::string, 5> arr2({"a", "b", "c", "d", "e"});
      SafeArray<std::string, 5> arr3({"e", "d", "c", "b", "a"});
      std::array<std::string, 5> arrRaw1({"a", "b", "c", "d", "e"});
      std::array<std::string, 5> arrRaw2({"a", "b", "c", "d", "e"});
      std::array<std::string, 5> arrRaw3({"e", "d", "c", "b", "a"});
      REQUIRE(arr1 == arr2);
      REQUIRE(arr1 != arr3);
      REQUIRE(arr1 == arrRaw1);
      REQUIRE(arr1 != arrRaw3);
    }

    SECTION("fill") {
      SafeArray<int, 5> arrFill({1,2,3,4,5});
      arrFill.fill(100);
      arrFill.revert();
      for (std::size_t i = 0; i < arrFill.size(); i++) REQUIRE(arrFill[i] == i + 1);
      arrFill.fill(100);
      arrFill.commit();
      for (std::size_t i = 0; i < arrFill.size(); i++) REQUIRE(arrFill[i] == 100);
    }

    SECTION("Coverage") {
      SafeArray<int, 5> arrFill({1,2,3,4,5});
      // copy_ != nullptr
      arrFill.fill(200);
      for (std::size_t i = 0; i < arrFill.size(); i++) REQUIRE(arrFill.at(i) == 200);
      for (std::size_t i = 0; i < arrFill.size(); i++) REQUIRE(arrFill[i] == 200);
      REQUIRE(arrFill.front() == 200);
      REQUIRE(arrFill.back() == 200);
      arrFill.fill(300);
      for (std::size_t i = 0; i < arrFill.size(); i++) REQUIRE(arrFill.at(i) == 300);
      for (std::size_t i = 0; i < arrFill.size(); i++) REQUIRE(arrFill[i] == 300);
      REQUIRE(arrFill.front() == 300);
      REQUIRE(arrFill.back() == 300);
    }
  }
}

