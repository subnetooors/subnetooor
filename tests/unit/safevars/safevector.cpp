/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#include "libs/catch2/catch_amalgamated.hpp"

#include "contract/variables/safevector.h"

namespace TSafeVector {
  TEST_CASE("SafeVector Class", "[unit][safevars][safevector]") {
    SECTION("Constructor") { // also tests some common const funcs
      SafeVector<int> emptyVec;
      SafeVector<int> vec({1,2,3,4,5});
      SafeVector<int> repeatVec(5, 50);
      SafeVector<int> emptyRepeatVec(5);
      SafeVector<int> iterVec(vec.cbegin(), vec.cend() - 2);
      std::initializer_list<int> ilist {100,200,300,400,500};
      SafeVector<int> ilistVec(ilist);
      SafeVector<int> copyVec(vec);
      REQUIRE((emptyVec.empty() && emptyVec.size() == 0));
      REQUIRE((!vec.empty() && vec.size() == 5));
      REQUIRE((!repeatVec.empty() && repeatVec.size() == 5));
      REQUIRE((!emptyRepeatVec.empty() && emptyRepeatVec.size() == 5));
      REQUIRE((!iterVec.empty() && iterVec.size() == 3));
      REQUIRE((!ilistVec.empty() && ilistVec.size() == 5));
      REQUIRE((!copyVec.empty() && copyVec.size() == 5 && copyVec == vec));
      REQUIRE(vec.front() == 1);
      REQUIRE(vec.back() == 5);
      REQUIRE(vec.at(2) == 3);
      REQUIRE(vec[3] == 4);
      for (std::size_t i = 0; i < vec.size(); i++) REQUIRE(vec[i] == i + 1);
      for (std::size_t i = 0; i < repeatVec.size(); i++) REQUIRE(repeatVec[i] == 50);
      for (std::size_t i = 0; i < emptyRepeatVec.size(); i++) REQUIRE(emptyRepeatVec[i] == 0);
      for (std::size_t i = 0; i < iterVec.size(); i++) REQUIRE(iterVec[i] == i + 1);
      for (std::size_t i = 0; i < ilistVec.size(); i++) REQUIRE(ilistVec[i] == (i + 1) * 100);
      for (std::size_t i = 0; i < copyVec.size(); i++) REQUIRE(copyVec[i] == i + 1);
    }

    SECTION("assign") {
      SafeVector<std::string> vec;
      SafeVector<std::string> vec2;
      SafeVector<std::string> vec3;
      std::initializer_list<std::string> ilist { "AAAAA", "AAAAA", "AAAAA", "AAAAA" };
      // assign with repeating value
      vec.assign(5, "AAAAA");
      vec.assign(5, "BBBBB"); // For coverage (copy != nullptr)
      vec.revert();
      REQUIRE(vec.empty());
      vec.assign(5, "AAAAA");
      vec.commit();
      REQUIRE((!vec.empty() && vec.size() == 5));
      for (std::size_t i = 0; i < vec.size(); i++) REQUIRE(vec[i] == "AAAAA");
      // assign with iterators
      vec2.assign(vec.cbegin(), vec.cend() - 2);
      vec2.assign(vec.cbegin(), vec.cend() - 2); // For coverage (copy != nullptr)
      vec2.revert();
      REQUIRE(vec2.empty());
      vec2.assign(vec.cbegin(), vec.cend() - 2);
      vec2.commit();
      REQUIRE((!vec2.empty() && vec2.size() == 3));
      for (std::size_t i = 0; i < vec2.size(); i++) REQUIRE(vec2[i] == "AAAAA");
      // assign with ilist
      vec3.assign(ilist);
      vec3.assign(ilist); // For coverage (copy != nullptr)
      vec3.revert();
      REQUIRE(vec3.empty());
      vec3.assign(ilist);
      vec3.commit();
      REQUIRE((!vec3.empty() && vec3.size() == 4));
      for (std::size_t i = 0; i < vec3.size(); i++) REQUIRE(vec3[i] == "AAAAA");
    }

    SECTION("at") {
      SafeVector<std::string> vec({"a", "b", "c", "d", "e"});
      REQUIRE_THROWS(vec.at(5));
      for (std::size_t i = 0; i < vec.size(); i++) vec.at(i) = "x";
      vec.revert();
      REQUIRE(vec.at(0) == "a");
      REQUIRE(vec.at(1) == "b");
      REQUIRE(vec.at(2) == "c");
      REQUIRE(vec.at(3) == "d");
      REQUIRE(vec.at(4) == "e");
      for (std::size_t i = 0; i < vec.size(); i++) vec.at(i) = "x";
      vec.commit();
      REQUIRE(vec.at(0) == "x");
      REQUIRE(vec.at(1) == "x");
      REQUIRE(vec.at(2) == "x");
      REQUIRE(vec.at(3) == "x");
      REQUIRE(vec.at(4) == "x");
      // For coverage
      const std::string str = std::as_const(vec)[0];
      REQUIRE(str == "x");
      vec.assign(5, "y"); // copy != nullptr
      REQUIRE(vec.at(0) == "y");
    }

    SECTION("operator[]") {
      SafeVector<std::string> vec({"a", "b", "c", "d", "e"});
      for (std::size_t i = 0; i < vec.size(); i++) vec[i] = "x";
      vec.revert();
      REQUIRE(vec[0] == "a");
      REQUIRE(vec[1] == "b");
      REQUIRE(vec[2] == "c");
      REQUIRE(vec[3] == "d");
      REQUIRE(vec[4] == "e");
      for (std::size_t i = 0; i < vec.size(); i++) vec[i] = "x";
      vec.commit();
      REQUIRE(vec[0] == "x");
      REQUIRE(vec[1] == "x");
      REQUIRE(vec[2] == "x");
      REQUIRE(vec[3] == "x");
      REQUIRE(vec[4] == "x");
      // For coverage
      const std::string str = std::as_const(vec)[0];
      REQUIRE(str == "x");
      vec.assign(5, "y"); // copy != nullptr
      REQUIRE(vec[0] == "y");
    }

    SECTION("front and back") {
      SafeVector<std::string> vec({"a", "b", "c", "d", "e"});
      vec.front() = "x";
      vec.revert();
      REQUIRE(vec.front() == "a");
      vec.front() = "x";
      vec.commit();
      REQUIRE(vec.front() == "x");
      vec.back() = "y";
      vec.revert();
      REQUIRE(vec.back() == "e");
      vec.back() = "y";
      vec.commit();
      REQUIRE(vec.back() == "y");
      // For coverage
      vec.front();  // undo != nullptr
      vec.back();
      vec.front();
      vec.assign(5, "y"); // copy != nullptr
      REQUIRE(vec.front() == "y");
      REQUIRE(vec.back() == "y");
    }

    // TODO: missing tests for cbegin, cend, crbegin and crend - check SafeUnorderedMap for more info

    SECTION("clear") {
      SafeVector<std::string> vec({"a", "b", "c"});
      vec.clear();
      vec.revert();
      REQUIRE(!vec.empty());
      REQUIRE(vec.size() == 3);
      REQUIRE(vec[0] == "a");
      REQUIRE(vec[1] == "b");
      REQUIRE(vec[2] == "c");
      vec.clear();
      vec.commit();
      REQUIRE(vec.empty());
      REQUIRE(vec.size() == 0);
      // For coverage (copy != nullptr)
      vec.clear();
      vec.clear();
    }

    SECTION("insert") {
      SafeVector<int> vec({1,2,3,4,5});
      // insert by copy (pos and value)
      vec.insert(vec.cbegin(), 0);
      vec.insert(vec.cbegin(), 0); // For coverage (undo != nullptr)
      vec.revert();
      REQUIRE(vec.size() == 5);
      REQUIRE(*(vec.cbegin()) == 1);
      vec.insert(vec.cbegin(), 0);
      vec.commit();
      REQUIRE(vec.size() == 6);
      REQUIRE(*(vec.cbegin()) == 0);  // vec = {0,1,2,3,4,5}
      // insert by move (pos and value)
      int n = 6;
      int n2 = 6;
      int nC = 6;
      vec.insert(vec.cend(), std::move(n));
      vec.insert(vec.cend(), std::move(nC)); // For coverage (undo != nullptr)
      vec.revert();
      REQUIRE(vec.size() == 6);
      REQUIRE(*(vec.cend() - 1) == 5);
      vec.insert(vec.cend(), std::move(n2));
      vec.commit();
      REQUIRE(vec.size() == 7);
      REQUIRE(*(vec.cend() - 1) == 6);  // vec = {0,1,2,3,4,5,6}
      // insert with repeat (pos count and value)
      vec.insert(vec.cbegin() + 2, 3, 7);
      vec.insert(vec.cbegin() + 2, 3, 7); // For coverage (undo != nullptr)
      vec.revert();
      REQUIRE(vec.size() == 7);
      REQUIRE(*(vec.cbegin() + 2) == 2);
      vec.insert(vec.cbegin() + 2, 3, 7);
      vec.commit();
      REQUIRE(vec.size() == 10);
      REQUIRE(*(vec.cbegin() + 2) == 7);
      REQUIRE(*(vec.cbegin() + 3) == 7);
      REQUIRE(*(vec.cbegin() + 4) == 7);
      REQUIRE(*(vec.cbegin() + 5) == 2);  // vec = {0,1,7,7,7,2,3,4,5,6}
      // insert with iterators
      std::vector<int> vec2({10,20,30});
      vec.insert(vec.cbegin(), vec2.cbegin(), vec2.cend());
      vec.insert(vec.cbegin(), vec2.cbegin(), vec2.cend()); // For coverage (undo != nullptr)
      vec.revert();
      REQUIRE(vec.size() == 10);
      REQUIRE(*(vec.cbegin()) == 0);
      vec.insert(vec.cbegin(), vec2.cbegin(), vec2.cend());
      vec.commit();
      REQUIRE(vec.size() == 13);
      REQUIRE(*(vec.cbegin()) == 10);
      REQUIRE(*(vec.cbegin() + 1) == 20);
      REQUIRE(*(vec.cbegin() + 2) == 30);
      REQUIRE(*(vec.cbegin() + 3) == 0); // vec = {10,20,30,0,1,7,7,7,2,3,4,5,6}
      // insert with pos and ilist
      std::initializer_list<int> ilist {1000, 2000, 3000};
      vec.insert(vec.cend(), ilist);
      vec.insert(vec.cend(), ilist); // For coverage (undo != nullptr)
      vec.revert();
      REQUIRE(vec.size() == 13);
      REQUIRE(*(vec.cend() - 1) == 6);
      vec.insert(vec.cend(), ilist);
      vec.commit();
      REQUIRE(vec.size() == 16);
      REQUIRE(*(vec.cend() - 4) == 6);
      REQUIRE(*(vec.cend() - 3) == 1000);
      REQUIRE(*(vec.cend() - 2) == 2000);
      REQUIRE(*(vec.cend() - 1) == 3000);
      // For coverage (copy != nullptr)
      vec.clear();
      int nC2 = 6;
      vec.insert(vec.cbegin(), 0);
      vec.insert(vec.cend(), std::move(nC2));
      vec.insert(vec.cbegin() + 2, 3, 7);
      vec.insert(vec.cbegin(), vec2.cbegin(), vec2.cend());
      vec.insert(vec.cend(), ilist);
    }

    SECTION("emplace") {
      // Same as insert, but there's only one overload to care about
      SafeVector<int> vec({1,2,3,4,5});
      vec.emplace(vec.cbegin(), 0);
      vec.emplace(vec.cbegin(), 0); // For coverage (undo != nullptr)
      vec.revert();
      REQUIRE(vec.size() == 5);
      REQUIRE(*(vec.cbegin()) == 1);
      vec.emplace(vec.cbegin(), 0);
      vec.commit();
      REQUIRE(vec.size() == 6);
      REQUIRE(*(vec.cbegin()) == 0);
      // For coverage (copy != nullptr)
      vec.clear();
      vec.emplace(vec.cbegin(), 0);
      REQUIRE(vec[0] == 0);
    }

    SECTION("erase") {
      SafeVector<int> vec({0,1,2,3,4,5});
      // erase a single value (pos)
      vec.erase(vec.cbegin());
      vec.erase(vec.cbegin()); // For coverage (undo != nullptr)
      vec.revert();
      REQUIRE(vec.size() == 6);
      REQUIRE(*(vec.cbegin()) == 0);
      vec.erase(vec.cbegin());
      vec.commit();
      REQUIRE(vec.size() == 5);
      REQUIRE(*(vec.cbegin()) == 1);
      // erase a range of values (iterator)
      vec.erase(vec.cbegin() + 1, vec.cend() - 1); // {2,3,4}
      vec.erase(vec.cbegin(), vec.cend()); // For coverage (undo != nullptr)
      vec.revert();
      REQUIRE(vec.size() == 5);
      REQUIRE(vec[0] == 1);
      REQUIRE(vec[1] == 2);
      REQUIRE(vec[2] == 3);
      REQUIRE(vec[3] == 4);
      REQUIRE(vec[4] == 5);
      vec.erase(vec.cbegin() + 1, vec.cend() - 1);
      vec.commit();
      REQUIRE(vec.size() == 2);
      REQUIRE(vec[0] == 1);
      REQUIRE(vec[1] == 5);
      // For coverage (copy != nullptr)
      vec.assign(5, 10);
      vec.erase(vec.cbegin());
      vec.erase(vec.cbegin(), vec.cend() - 1);
      REQUIRE(vec[0] == 10);
    }

    SECTION("push_back, emplace_back and pop_back") {
      SafeVector<std::string> vec({"a", "b", "c"});
      // push back by copy
      vec.push_back("d");
      vec.push_back("d"); // For coverage (undo != nullptr)
      vec.revert();
      REQUIRE((vec.size() == 3 && vec.back() == "c"));
      vec.push_back("d");
      vec.commit();
      REQUIRE((vec.size() == 4 && vec.back() == "d")); // vec = {a,b,c,d}
      // push back by move
      std::string mv1 = "e";
      std::string mv2 = "e";
      std::string mvC = "e";
      vec.push_back(std::move(mv1));
      vec.push_back(std::move(mvC)); // For coverage (undo != nullptr)
      vec.revert();
      REQUIRE((vec.size() == 4 && vec.back() == "d"));
      vec.push_back(std::move(mv2));
      vec.commit();
      REQUIRE((vec.size() == 5 && vec.back() == "e")); // vec = {a,b,c,d,e}
      // emplace back
      vec.emplace_back("f");
      vec.emplace_back("f"); // For coverage (undo != nullptr)
      vec.revert();
      REQUIRE((vec.size() == 5 && vec.back() == "e"));
      vec.emplace_back("f");
      vec.commit();
      REQUIRE((vec.size() == 6 && vec.back() == "f")); // vec = {a,b,c,d,e,f}
      // pop back
      for (int i = 0; i < 5; i++) vec.pop_back();
      vec.pop_back(); // For coverage (undo != nullptr)
      vec.revert();
      REQUIRE((vec.size() == 6 && vec.back() == "f"));
      for (int i = 0; i < 5; i++) vec.pop_back();
      vec.commit();
      REQUIRE((vec.size() == 1 && vec.back() == "a")); // vec = {a}
      // For coverage (copy != nullptr)
      vec.assign(5, "x");
      vec.push_back("y");
      std::string mvC2 = "z";
      vec.push_back(std::move(mvC2));
      vec.emplace_back("w");
      vec.pop_back();
    }

    SECTION("resize") {
      SafeVector<int> vec({1,2,3,4,5});
      // resize to a bigger size, with default elements
      vec.resize(10);
      vec.resize(20); // For coverage (undo != nullptr)
      vec.resize(5);
      vec.revert();
      REQUIRE(vec.size() == 5);
      for (std::size_t i = 0; i < 5; i++) REQUIRE(vec[i] == i + 1);
      vec.resize(10);
      vec.commit();
      REQUIRE(vec.size() == 10);
      for (std::size_t i = 0; i < 5; i++) REQUIRE(vec[i] == i + 1);
      for (std::size_t i = 5; i < 10; i++) REQUIRE(vec[i] == 0); // vec = {1,2,3,4,5,0,0,0,0,0}
      // resize to a smaller size, with default elements
      vec.resize(3);
      vec.revert();
      REQUIRE(vec.size() == 10);
      for (std::size_t i = 0; i < 5; i++) REQUIRE(vec[i] == i + 1);
      for (std::size_t i = 5; i < 10; i++) REQUIRE(vec[i] == 0);
      vec.resize(3);
      vec.commit();
      REQUIRE(vec.size() == 3);
      for (std::size_t i = 0; i < 3; i++) REQUIRE(vec[i] == i + 1);
      // resize to a bigger size, with repeated elements
      vec.resize(6, 100);
      vec.resize(10, 100); // For coverage (undo != nullptr)
      vec.resize(2, 100);
      vec.revert();
      REQUIRE(vec.size() == 3);
      for (std::size_t i = 0; i < 3; i++) REQUIRE(vec[i] == i + 1);
      vec.resize(6, 100);
      vec.commit();
      REQUIRE(vec.size() == 6);
      for (std::size_t i = 0; i < 3; i++) REQUIRE(vec[i] == i + 1);
      for (std::size_t i = 3; i < 6; i++) REQUIRE(vec[i] == 100);
      // resize to a smaller size, with repeated elements
      vec.resize(3, 100);
      vec.revert();
      REQUIRE(vec.size() == 6);
      for (std::size_t i = 0; i < 3; i++) REQUIRE(vec[i] == i + 1);
      for (std::size_t i = 3; i < 6; i++) REQUIRE(vec[i] == 100);
      vec.resize(3, 100);
      vec.commit();
      REQUIRE(vec.size() == 3);
      for (std::size_t i = 0; i < 3; i++) REQUIRE(vec[i] == i + 1);
      // resize to the same size (do nothing basically)
      vec.resize(3);
      REQUIRE(vec.size() == 3);
      for (std::size_t i = 0; i < 3; i++) REQUIRE(vec[i] == i + 1);
      // resize to 0 with both overloads
      vec.resize(0);
      vec.revert();
      REQUIRE(vec.size() == 3);
      for (std::size_t i = 0; i < 3; i++) REQUIRE(vec[i] == i + 1);
      vec.resize(0, 100);
      vec.revert();
      REQUIRE(vec.size() == 3);
      for (std::size_t i = 0; i < 3; i++) REQUIRE(vec[i] == i + 1);
      vec.resize(0);
      vec.commit();
      REQUIRE(vec.empty());
      vec.resize(5, 10); // temporarily fill the vector for the other overload
      vec.commit();
      for (std::size_t i = 0; i < 5; i++) REQUIRE(vec[i] == 10);
      vec.resize(0, 100);
      vec.commit();
      REQUIRE(vec.empty());
      // For coverage (copy != nullptr)
      vec.assign(5, 100);
      vec.resize(10);
      vec.resize(20, 200);
    }

    SECTION("operator=") {
      SafeVector<std::string> vec({"a", "b", "c"});
      SafeVector<std::string> vec2({"1", "2", "3"});
      SafeVector<std::string> vec3({"X", "Y", "Z"});
      vec = vec2;
      vec = vec2; // For coverage (copy != nullptr)
      vec.revert();
      REQUIRE(vec[0] == "a");
      REQUIRE(vec[1] == "b");
      REQUIRE(vec[2] == "c");
      vec = vec2;
      vec.commit();
      REQUIRE(vec[0] == "1");
      REQUIRE(vec[1] == "2");
      REQUIRE(vec[2] == "3");
      vec = vec3.get();
      vec = vec3.get(); // For coverage (copy != nullptr)
      vec.revert();
      REQUIRE(vec[0] == "1");
      REQUIRE(vec[1] == "2");
      REQUIRE(vec[2] == "3");
      vec = vec3.get();
      vec.commit();
      REQUIRE(vec[0] == "X");
      REQUIRE(vec[1] == "Y");
      REQUIRE(vec[2] == "Z");
    }
  }
}

