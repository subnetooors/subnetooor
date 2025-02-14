/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#include "libs/catch2/catch_amalgamated.hpp"

#include "contract/variables/safebytes.h"

namespace TSafeBytes {
  TEST_CASE("SafeBytes Class", "[unit][safevars][safebytes]") {
    SECTION("Constructor") { // also tests some common const funcs
      SafeBytes emptyVec;
      SafeBytes vec({0x01,0x02,0x03,0x04,0x05});
      SafeBytes repeatVec(5, 0x50);
      SafeBytes emptyRepeatVec(5);
      SafeBytes iterVec(vec.cbegin(), vec.cend() - 2);
      std::initializer_list<uint8_t> ilist {0x10,0x20,0x30,0x40,0x50};
      SafeBytes ilistVec(ilist);
      SafeBytes copyVec(vec);

      REQUIRE((emptyVec.empty() && emptyVec.size() == 0));
      REQUIRE((!vec.empty() && vec.size() == 5));
      REQUIRE((!repeatVec.empty() && repeatVec.size() == 5));
      REQUIRE((!emptyRepeatVec.empty() && emptyRepeatVec.size() == 5));
      REQUIRE((!iterVec.empty() && iterVec.size() == 3));
      REQUIRE((!ilistVec.empty() && ilistVec.size() == 5));
      REQUIRE((!copyVec.empty() && copyVec.size() == 5 && copyVec == vec));
      REQUIRE(vec.front() == 0x01);
      REQUIRE(vec.back() == 0x05);
      REQUIRE(vec.at(2) == 0x03);
      REQUIRE(vec[3] == 0x04);

      REQUIRE(vec[0] == 0x01);
      REQUIRE(vec[1] == 0x02);
      REQUIRE(vec[2] == 0x03);
      REQUIRE(vec[3] == 0x04);
      REQUIRE(vec[4] == 0x05);
      for (std::size_t i = 0; i < repeatVec.size(); i++) REQUIRE(repeatVec[i] == 0x50);
      for (std::size_t i = 0; i < emptyRepeatVec.size(); i++) REQUIRE(emptyRepeatVec[i] == 0x00);
      REQUIRE(iterVec[0] == 0x01);
      REQUIRE(iterVec[1] == 0x02);
      REQUIRE(iterVec[2] == 0x03);
      REQUIRE(ilistVec[0] == 0x10);
      REQUIRE(ilistVec[1] == 0x20);
      REQUIRE(ilistVec[2] == 0x30);
      REQUIRE(ilistVec[3] == 0x40);
      REQUIRE(ilistVec[4] == 0x50);
      REQUIRE(copyVec[0] == 0x01);
      REQUIRE(copyVec[1] == 0x02);
      REQUIRE(copyVec[2] == 0x03);
      REQUIRE(copyVec[3] == 0x04);
      REQUIRE(copyVec[4] == 0x05);
    }

    SECTION("assign") {
      SafeBytes vec;
      SafeBytes vec2;
      SafeBytes vec3;
      std::initializer_list<uint8_t> ilist { 0xFF, 0xFF, 0xFF, 0xFF };
      // assign with repeating value
      vec.assign(5, 0xFF);
      vec.revert();
      REQUIRE(vec.empty());
      vec.assign(5, 0xFF);
      vec.commit();
      REQUIRE((!vec.empty() && vec.size() == 5));
      for (std::size_t i = 0; i < vec.size(); i++) REQUIRE(vec[i] == 0xFF);
      // assign with iterators
      vec2.assign(vec.cbegin(), vec.cend() - 2);
      vec2.assign(vec.cbegin(), vec.cend() - 2); // extra call for coverage (copy != nullptr)
      vec2.revert();
      REQUIRE(vec2.empty());
      vec2.assign(vec.cbegin(), vec.cend() - 2);
      vec2.commit();
      REQUIRE((!vec2.empty() && vec2.size() == 3));
      for (std::size_t i = 0; i < vec2.size(); i++) REQUIRE(vec2[i] == 0xFF);
      // assign with ilist
      vec3.assign(ilist);
      vec3.assign(ilist); // extra call for coverage (copy != nullptr)
      vec3.revert();
      REQUIRE(vec3.empty());
      vec3.assign(ilist);
      vec3.commit();
      REQUIRE((!vec3.empty() && vec3.size() == 4));
      for (std::size_t i = 0; i < vec3.size(); i++) REQUIRE(vec3[i] == 0xFF);
    }

    SECTION("at") {
      SafeBytes vec({0x01, 0x02, 0x03, 0x04, 0x05});
      REQUIRE_THROWS(vec.at(5));
      for (std::size_t i = 0; i < vec.size(); i++) vec.at(i) = 0xFF;
      vec.revert();
      REQUIRE(vec.at(0) == 0x01);
      REQUIRE(vec.at(1) == 0x02);
      REQUIRE(vec.at(2) == 0x03);
      REQUIRE(vec.at(3) == 0x04);
      REQUIRE(vec.at(4) == 0x05);
      for (std::size_t i = 0; i < vec.size(); i++) vec.at(i) = 0xFF;
      vec.commit();
      REQUIRE(vec.at(0) == 0xFF);
      REQUIRE(vec.at(1) == 0xFF);
      REQUIRE(vec.at(2) == 0xFF);
      REQUIRE(vec.at(3) == 0xFF);
      REQUIRE(vec.at(4) == 0xFF);
      // For coverage (copy != nullptr)
      vec.assign(5, 0xFF);
      REQUIRE(vec.at(0) == 0xFF);
      REQUIRE(vec.at(1) == 0xFF);
      REQUIRE(vec.at(2) == 0xFF);
      REQUIRE(vec.at(3) == 0xFF);
      REQUIRE(vec.at(4) == 0xFF);
    }

    SECTION("operator[]") {
      SafeBytes vec({0x01, 0x02, 0x03, 0x04, 0x05});
      for (std::size_t i = 0; i < vec.size(); i++) vec[i] = 0xFF;
      vec.revert();
      REQUIRE(vec[0] == 0x01);
      REQUIRE(vec[1] == 0x02);
      REQUIRE(vec[2] == 0x03);
      REQUIRE(vec[3] == 0x04);
      REQUIRE(vec[4] == 0x05);
      for (std::size_t i = 0; i < vec.size(); i++) vec[i] = 0xFF;
      vec.commit();
      REQUIRE(vec[0] == 0xFF);
      REQUIRE(vec[1] == 0xFF);
      REQUIRE(vec[2] == 0xFF);
      REQUIRE(vec[3] == 0xFF);
      REQUIRE(vec[4] == 0xFF);
      // For coverage (copy != nullptr)
      vec.assign(5, 0xFF);
      REQUIRE(vec[0] == 0xFF);
      REQUIRE(vec[1] == 0xFF);
      REQUIRE(vec[2] == 0xFF);
      REQUIRE(vec[3] == 0xFF);
      REQUIRE(vec[4] == 0xFF);
    }

    SECTION("front and back") {
      SafeBytes vec({0x01, 0x02, 0x03, 0x04, 0x05});
      vec.front() = 0xF0;
      vec.revert();
      REQUIRE(vec.front() == 0x01);
      vec.front() = 0xF0;
      vec.commit();
      REQUIRE(vec.front() == 0xF0);
      vec.back() = 0xFF;
      vec.revert();
      REQUIRE(vec.back() == 0x05);
      vec.back() = 0xFF;
      vec.commit();
      REQUIRE(vec.back() == 0xFF);
      // For coverage (copy != nullptr)
      vec.assign(5, 0xAA);
      REQUIRE(vec.front() == 0xAA);
      REQUIRE(vec.back() == 0xAA);
    }

    // TODO: missing tests for cbegin, cend, crbegin and crend - check SafeUnorderedMap for more info

    SECTION("clear") {
      SafeBytes vec({0x01, 0x02, 0x03});
      vec.clear();
      vec.revert();
      REQUIRE(!vec.empty());
      REQUIRE(vec.size() == 3);
      REQUIRE(vec[0] == 0x01);
      REQUIRE(vec[1] == 0x02);
      REQUIRE(vec[2] == 0x03);
      vec.clear();
      vec.commit();
      REQUIRE(vec.empty());
      REQUIRE(vec.size() == 0);
      // For coverage (copy != nullptr)
      vec.assign(5, 0xFF);
      vec.clear();
      REQUIRE(vec.empty());
      REQUIRE(vec.size() == 0);
    }

    SECTION("insert") {
      SafeBytes vec({0x01,0x02,0x03,0x04,0x05});
      // insert by copy (pos and value)
      vec.insert(vec.cbegin(), 0x00);
      vec.revert();
      REQUIRE(vec.size() == 5);
      REQUIRE(*(vec.cbegin()) == 0x01);
      vec.insert(vec.cbegin(), 0x00);
      vec.commit();
      REQUIRE(vec.size() == 6);
      REQUIRE(*(vec.cbegin()) == 0x00);  // vec = {0x00,0x01,0x02,0x03,0x04,0x05}
      // insert by move (pos and value)
      uint8_t n = 0x06;
      uint8_t n2 = 0x06;
      vec.insert(vec.cend(), std::move(n));
      vec.revert();
      REQUIRE(vec.size() == 6);
      REQUIRE(*(vec.cend() - 1) == 0x05);
      vec.insert(vec.cend(), std::move(n2));
      vec.commit();
      REQUIRE(vec.size() == 7);
      REQUIRE(*(vec.cend() - 1) == 0x06);  // vec = {0x00,0x01,0x02,0x03,0x04,0x05,0x06}
      // insert with repeat (pos count and value)
      vec.insert(vec.cbegin() + 2, 3, 0x07);
      vec.revert();
      REQUIRE(vec.size() == 7);
      REQUIRE(*(vec.cbegin() + 2) == 0x02);
      vec.insert(vec.cbegin() + 2, 3, 0x07);
      vec.commit();
      REQUIRE(vec.size() == 10);
      REQUIRE(*(vec.cbegin() + 2) == 0x07);
      REQUIRE(*(vec.cbegin() + 3) == 0x07);
      REQUIRE(*(vec.cbegin() + 4) == 0x07);
      REQUIRE(*(vec.cbegin() + 5) == 0x02);  // vec = {0x00,0x01,0x07,0x07,0x07,0x02,0x03,0x04,0x05,0x06}
      // insert with iterators
      std::vector<uint8_t> vec2({0x10,0x20,0x30});
      vec.insert(vec.cbegin(), vec2.cbegin(), vec2.cend());
      vec.revert();
      REQUIRE(vec.size() == 10);
      REQUIRE(*(vec.cbegin()) == 0x00);
      vec.insert(vec.cbegin(), vec2.cbegin(), vec2.cend());
      vec.commit();
      REQUIRE(vec.size() == 13);
      REQUIRE(*(vec.cbegin()) == 0x10);
      REQUIRE(*(vec.cbegin() + 1) == 0x20);
      REQUIRE(*(vec.cbegin() + 2) == 0x30);
      REQUIRE(*(vec.cbegin() + 3) == 0x00); // vec = {0x10,0x20,0x30,0x00,0x01,0x07,0x07,0x07,0x02,0x03,0x04,0x05,0x06}
      // insert with pos and ilist
      std::initializer_list<uint8_t> ilist {0xA0, 0xB0, 0xC0};
      vec.insert(vec.cend(), ilist);
      vec.revert();
      REQUIRE(vec.size() == 13);
      REQUIRE(*(vec.cend() - 1) == 0x06);
      vec.insert(vec.cend(), ilist);
      vec.commit();
      REQUIRE(vec.size() == 16);
      REQUIRE(*(vec.cend() - 4) == 0x06);
      REQUIRE(*(vec.cend() - 3) == 0xA0);
      REQUIRE(*(vec.cend() - 2) == 0xB0);
      REQUIRE(*(vec.cend() - 1) == 0xC0);
      // For coverage (undo != nullptr)
      vec.clear();
      vec.assign(1, 0x02);
      vec.commit();
      vec.insert(vec.cbegin(), 0x01);
      uint8_t mv = 0x00;
      vec.insert(vec.cbegin(), std::move(mv));
      vec.insert(vec.cend(), 1, 0x03);
      std::vector<uint8_t> vecIt({0x04,0x05,0x06});
      vec.insert(vec.cend(), vecIt.cbegin(), vecIt.cend());
      std::initializer_list<uint8_t> ilistCov {0x07, 0x08, 0x09};
      vec.insert(vec.cend(), ilistCov);
      // For coverage (copy != nullptr)
      vec.clear();
      vec.commit();
      vec.assign(1, 0x02);
      vec.insert(vec.cbegin(), 0x01);
      uint8_t mv2 = 0x00;
      vec.insert(vec.cbegin(), std::move(mv2));
      vec.insert(vec.cend(), 1, 0x03);
      std::vector<uint8_t> vecIt2({0x04,0x05,0x06});
      vec.insert(vec.cend(), vecIt2.cbegin(), vecIt2.cend());
      std::initializer_list<uint8_t> ilistCov2 {0x07, 0x08, 0x09};
      vec.insert(vec.cend(), ilistCov2);
    }

    SECTION("emplace") {
      // Same as insert, but there's only one overload to care about
      SafeBytes vec({0x01,0x02,0x03,0x04,0x05});
      vec.emplace(vec.cbegin(), 0x00);
      vec.revert();
      REQUIRE(vec.size() == 5);
      REQUIRE(*(vec.cbegin()) == 0x01);
      vec.emplace(vec.cbegin(), 0x00);
      vec.commit();
      REQUIRE(vec.size() == 6);
      REQUIRE(*(vec.cbegin()) == 0x00);
      // For coverage (undo != nullptr)
      SafeBytes vec2({0x01,0x02,0x03,0x04,0x05});
      vec2.emplace(vec2.cbegin(), 0xFF);
      vec2.emplace(vec2.cend(), 0xFF);
      // For coverage (copy != nullptr)
      vec2.clear();
      vec2.emplace(vec2.cbegin(), 0xFF);
    }

    SECTION("erase") {
      SafeBytes vec({0x00,0x01,0x02,0x03,0x04,0x05});
      // erase a single value (pos)
      vec.erase(vec.cbegin());
      vec.revert();
      REQUIRE(vec.size() == 6);
      REQUIRE(*(vec.cbegin()) == 0x00);
      vec.erase(vec.cbegin());
      vec.commit();
      REQUIRE(vec.size() == 5);
      REQUIRE(*(vec.cbegin()) == 0x01);
      // erase a range of values (iterator)
      vec.erase(vec.cbegin() + 1, vec.cend() - 1); // {0x02,0x03,0x04}
      vec.revert();
      REQUIRE(vec.size() == 5);
      REQUIRE(vec[0] == 0x01);
      REQUIRE(vec[1] == 0x02);
      REQUIRE(vec[2] == 0x03);
      REQUIRE(vec[3] == 0x04);
      REQUIRE(vec[4] == 0x05);
      vec.erase(vec.cbegin() + 1, vec.cend() - 1);
      vec.commit();
      REQUIRE(vec.size() == 2);
      REQUIRE(vec[0] == 0x01);
      REQUIRE(vec[1] == 0x05);
      // For coverage (undo != nullptr)
      SafeBytes vec2({0x00,0x01,0x02,0x03,0x04,0x05});
      vec2.erase(vec2.cbegin());
      vec2.erase(vec2.cbegin());
      vec2.erase(vec2.cbegin(), vec2.cend());
      // For coverage (copy != nullptr)
      vec2.commit();
      vec2.assign(5, 0xFF);
      vec2.erase(vec2.cbegin());
      vec2.erase(vec2.cbegin(), vec2.cend());
    }

    SECTION("push_back, emplace_back and pop_back") {
      SafeBytes vec({0x01, 0x02, 0x03});
      // push back by copy
      vec.push_back(0x04);
      vec.revert();
      REQUIRE((vec.size() == 3 && vec.back() == 0x03));
      vec.push_back(0x04);
      vec.commit();
      REQUIRE((vec.size() == 4 && vec.back() == 0x04)); // vec = {0x01,0x02,0x03,0x04}
      // push back by move
      uint8_t mv1 = 0x05;
      uint8_t mv2 = 0x05;
      vec.push_back(std::move(mv1));
      vec.revert();
      REQUIRE((vec.size() == 4 && vec.back() == 0x04));
      vec.push_back(std::move(mv2));
      vec.commit();
      REQUIRE((vec.size() == 5 && vec.back() == 0x05)); // vec = {0x01,0x02,0x03,0x04,0x05}
      // emplace back
      vec.emplace_back(0x06);
      vec.revert();
      REQUIRE((vec.size() == 5 && vec.back() == 0x05));
      vec.emplace_back(0x06);
      vec.commit();
      REQUIRE((vec.size() == 6 && vec.back() == 0x06)); // vec = {0x01,0x02,0x03,0x04,0x05,0x06}
      // pop back
      for (int i = 0; i < 5; i++) vec.pop_back();
      vec.revert();
      REQUIRE((vec.size() == 6 && vec.back() == 0x06));
      for (int i = 0; i < 5; i++) vec.pop_back();
      vec.commit();
      REQUIRE((vec.size() == 1 && vec.back() == 0x01)); // vec = {0x01}
      // For coverage (undo != nullptr)
      SafeBytes vec2({0x01, 0x02, 0x03});
      vec2.push_back(0x00);
      vec2.push_back(0xAA);
      uint8_t mv = 0xBB;
      vec2.push_back(std::move(mv));
      vec2.emplace_back(0xCC);
      vec2.pop_back();
      // For coverage (copy != nullptr)
      vec2.clear();
      vec2.push_back(0x00);
      vec2.push_back(0xAA);
      uint8_t mv3 = 0xBB;
      vec2.push_back(std::move(mv3));
      vec2.emplace_back(0xCC);
      vec2.pop_back();
    }

    SECTION("resize") {
      SafeBytes vec({0x01,0x02,0x03,0x04,0x05});
      // resize to a bigger size, with default elements
      vec.resize(10);
      vec.revert();
      REQUIRE(vec.size() == 5);
      REQUIRE(vec[0] == 0x01);
      REQUIRE(vec[1] == 0x02);
      REQUIRE(vec[2] == 0x03);
      REQUIRE(vec[3] == 0x04);
      REQUIRE(vec[4] == 0x05);
      vec.resize(10);
      vec.commit();
      REQUIRE(vec.size() == 10);
      REQUIRE(vec[0] == 0x01);
      REQUIRE(vec[1] == 0x02);
      REQUIRE(vec[2] == 0x03);
      REQUIRE(vec[3] == 0x04);
      REQUIRE(vec[4] == 0x05);
      for (std::size_t i = 5; i < 10; i++) REQUIRE(vec[i] == 0x00); // vec = {0x01,0x02,0x03,0x04,0x05,0x00,0x00,0x00,0x00,0x00}
      // resize to a smaller size, with default elements
      vec.resize(3);
      vec.revert();
      REQUIRE(vec.size() == 10);
      REQUIRE(vec[0] == 0x01);
      REQUIRE(vec[1] == 0x02);
      REQUIRE(vec[2] == 0x03);
      REQUIRE(vec[3] == 0x04);
      REQUIRE(vec[4] == 0x05);
      for (std::size_t i = 5; i < 10; i++) REQUIRE(vec[i] == 0x00);
      vec.resize(3);
      vec.commit();
      REQUIRE(vec.size() == 3);
      REQUIRE(vec[0] == 0x01);
      REQUIRE(vec[1] == 0x02);
      REQUIRE(vec[2] == 0x03);
      // resize to a bigger size, with repeated elements
      vec.resize(6, 0x10);
      vec.revert();
      REQUIRE(vec.size() == 3);
      REQUIRE(vec[0] == 0x01);
      REQUIRE(vec[1] == 0x02);
      REQUIRE(vec[2] == 0x03);
      vec.resize(6, 0x10);
      vec.commit();
      REQUIRE(vec.size() == 6);
      REQUIRE(vec[0] == 0x01);
      REQUIRE(vec[1] == 0x02);
      REQUIRE(vec[2] == 0x03);
      for (std::size_t i = 3; i < 6; i++) REQUIRE(vec[i] == 0x10);
      // resize to a smaller size, with repeated elements
      vec.resize(3, 0x20);
      vec.revert();
      REQUIRE(vec.size() == 6);
      REQUIRE(vec[0] == 0x01);
      REQUIRE(vec[1] == 0x02);
      REQUIRE(vec[2] == 0x03);
      for (std::size_t i = 3; i < 6; i++) REQUIRE(vec[i] == 0x10);
      vec.resize(3, 0x20);
      vec.commit();
      REQUIRE(vec.size() == 3);
      REQUIRE(vec[0] == 0x01);
      REQUIRE(vec[1] == 0x02);
      REQUIRE(vec[2] == 0x03);
      // resize to the same size (do nothing basically)
      vec.resize(3);
      REQUIRE(vec.size() == 3);
      REQUIRE(vec[0] == 0x01);
      REQUIRE(vec[1] == 0x02);
      REQUIRE(vec[2] == 0x03);
      // resize to 0 with both overloads
      vec.resize(0);
      vec.revert();
      REQUIRE(vec.size() == 3);
      REQUIRE(vec[0] == 0x01);
      REQUIRE(vec[1] == 0x02);
      REQUIRE(vec[2] == 0x03);
      vec.resize(0, 0x30);
      vec.revert();
      REQUIRE(vec.size() == 3);
      REQUIRE(vec[0] == 0x01);
      REQUIRE(vec[1] == 0x02);
      REQUIRE(vec[2] == 0x03);
      vec.resize(0);
      vec.commit();
      REQUIRE(vec.empty());
      vec.resize(5, 0xF0); // temporarily fill the vector for the other overload
      vec.commit();
      for (std::size_t i = 0; i < 5; i++) REQUIRE(vec[i] == 0xF0);
      vec.resize(0, 0xFF);
      vec.commit();
      REQUIRE(vec.empty());
      // For coverage (undo != nullptr)
      vec.resize(2);
      vec.resize(5);
      vec.resize(1);
      // For coverage (copy != nullptr)
      vec.clear();
      vec.resize(10);
      // Same but it's the other overload
      vec.clear();
      vec.commit();
      vec.resize(2, 0x00);
      vec.resize(5, 0x01);
      vec.resize(1, 0x02);
      vec.clear();
      vec.resize(10, 0x03);
    }
  }
}

