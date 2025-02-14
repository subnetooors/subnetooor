/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#include "libs/catch2/catch_amalgamated.hpp"

#include "contract/variables/safeunorderedmap.h" // safehash.h

namespace TSafeUnorderedMap {
  TEST_CASE("SafeUnorderedMap Class", "[unit][safevars][safeunorderedmap]") {
    SECTION("Constructor") {
      Address add1(Utils::randBytes(20));
      Address add2(Utils::randBytes(20));
      Address add3(Utils::randBytes(20));
      uint256_t bal1("19283815712031512");
      uint256_t bal2("96482364197823643");
      uint256_t bal3("29884639238924532");

      SafeUnorderedMap<Address, uint256_t> emptyMap;
      SafeUnorderedMap<Address, uint256_t> map({{add1,bal1},{add2,bal2},{add3,bal3}});
      SafeUnorderedMap<Address, uint256_t> copyMap(map);
      REQUIRE(emptyMap.empty());
      REQUIRE(map.size() == 3);
      REQUIRE(copyMap == map);
      // using count
      REQUIRE(map.count(add1) == 1);
      REQUIRE(map.count(add2) == 1);
      REQUIRE(map.count(add3) == 1);
      // using contains
      REQUIRE(map.contains(add1));
      REQUIRE(map.contains(add2));
      REQUIRE(map.contains(add3));
    }

    SECTION("clear") {
      Address add1(Utils::randBytes(20));
      Address add2(Utils::randBytes(20));
      Address add3(Utils::randBytes(20));
      uint256_t bal1("19283815712031512");
      uint256_t bal2("96482364197823643");
      uint256_t bal3("29884639238924532");
      SafeUnorderedMap<Address, uint256_t> map({{add1,bal1},{add2,bal2},{add3,bal3}});

      map.clear();
      map.revert();
      REQUIRE(map.contains(add1));
      REQUIRE(map.contains(add2));
      REQUIRE(map.contains(add3));
      map.clear();
      map.commit();
      REQUIRE(map.empty());
    }

    SECTION("operator[]") {
      Address add(Utils::randBytes(20));
      uint256_t bal1("19283815712031512");
      uint256_t bal2("96482364197823643");
      SafeUnorderedMap<Address, uint256_t> map;

      // Create an empty key and revert - key should be removed and its value should be empty
      uint256_t emptyVal = map[add];
      map.revert();
      REQUIRE(!map.contains(add));
      REQUIRE(emptyVal == 0);
      // Create a key, assign a value and revert - same as above
      map[add] = bal1;
      map.revert();
      REQUIRE(!map.contains(add));
      // Create a key, assign a value by copy and commit - key and value should remain
      map[add] = bal1;
      map.commit();
      REQUIRE(map.contains(add));
      REQUIRE(map[add] == bal1);
      // Change key value by move and commit - value should be the new one
      uint256_t bal2Ref = bal2;
      map[add] = std::move(bal2);
      map.commit();
      REQUIRE(map[add] == bal2Ref);
    }

    SECTION("at") {
      Address add(Utils::randBytes(20));
      uint256_t bal1("19283815712031512");
      uint256_t bal2("96482364197823643");
      SafeUnorderedMap<Address, uint256_t> map({{add,bal1}});

      // Roughly the same as operator[] but we check for throws if the key doesn't exist
      REQUIRE(std::as_const(map).at(add) == bal1); // const at
      REQUIRE_THROWS(std::as_const(map).at(Address(Utils::randBytes(20))));
      // Assign a value and revert - value should remain the same
      map.at(add) = bal2; // non-const at
      map.revert();
      REQUIRE(std::as_const(map).at(add) == bal1);
      // Assign a value and commit - value should be the new one
      map.at(add) = bal2;
      map.commit();
      REQUIRE(std::as_const(map).at(add) == bal2);
    }

    SECTION("find") {
      Address add(Utils::randBytes(20));
      uint256_t bal("19283815712031512");
      uint256_t bal2("64512342624123513");
      SafeUnorderedMap<Address, uint256_t> map({{add,bal}});

      // Const find: check for existing and non-existing key
      auto found = std::as_const(map).find(add);
      REQUIRE(found != map.end());
      auto notFound = std::as_const(map).find(Address(Utils::randBytes(20)));
      REQUIRE(notFound == map.end());
      // Non-const find: check existing key, assign and revert
      auto found2 = map.find(add);
      found2->second = bal2;
      map.revert();
      REQUIRE(std::as_const(map).find(add)->second == bal);
      // Non-const find: check existing key, assign and commit
      auto found3 = map.find(add);
      found3->second = bal2;
      map.commit();
      REQUIRE(std::as_const(map).find(add)->second == bal2);
      // Non-const find: check for non-existing key
      auto notFound2 = map.find(Address(Utils::randBytes(20)));
      REQUIRE(notFound2 == map.end());
    }

    // TODO: missing tests for begin(), end() cbegin() and cend() due to lack of SafeIterators
    // This is commented on purpose for future reference
    /*
    SECTION("begin, end, cbegin and cend") {
      Address add1(Utils::randBytes(20));
      Address add2(Utils::randBytes(20));
      Address add3(Utils::randBytes(20));
      uint256_t bal1("19283815712031512");
      uint256_t bal2("96482364197823643");
      uint256_t bal3("29884639238924532");
      uint256_t bal0(0);
      SafeUnorderedMap<Address, uint256_t> map({{add1,bal1},{add2,bal2},{add3,bal3}});

      // Const iteration: check that values are not altered
      auto cB = map.cbegin();
      auto cE = map.cend();
      REQUIRE(std::as_const(map).at(add1) == bal1);
      REQUIRE(std::as_const(map).at(add2) == bal2);
      REQUIRE(std::as_const(map).at(add3) == bal3);

      // Non-const iteration: alter all values, then revert
      auto cB2 = map.begin();
      while (cB2 != map.end()) {
        (*cB2).second = bal0;
        std::advance(cB2, 1);
      }
      map.revert();
      REQUIRE(std::as_const(map).at(add1) == bal1);
      REQUIRE(std::as_const(map).at(add2) == bal2);
      REQUIRE(std::as_const(map).at(add3) == bal3);

      // Non-const iteration: alter all values, then commit
      auto cE2 = map.end(); // backwards on purpose - begin() makes copies, end() doesn't
      while (cE2 != map.begin()) {
        std::advance(cE2, -1);
        (*cE2).second = bal0;
      }
      map.commit();
      REQUIRE(std::as_const(map).at(add1) == bal0);
      REQUIRE(std::as_const(map).at(add2) == bal0);
      REQUIRE(std::as_const(map).at(add3) == bal0);
    }
    */

    SECTION("insert (simple)") {
      Address add0(Utils::randBytes(20));
      uint256_t bal0("19283815712031512");
      SafeUnorderedMap<Address, uint256_t> map({{add0,bal0}});

      // Attempt inserting an existing key, then prepare mass insertions
      REQUIRE(map.insert(std::make_pair(add0, bal0)).second == false);
      std::pair<Address, uint256_t> movePair = std::make_pair(add0, bal0);
      REQUIRE(map.insert(std::move(movePair)).second == false);
      std::vector<std::pair<Address, uint256_t>> values;
      for (uint8_t i = 0; i < 100; i++) { // inserting 100 values
        values.emplace_back(std::make_pair(Address(Utils::randBytes(20)), bal0));
      }
      std::vector<std::pair<Address, uint256_t>> valCopy1 = values; // copies for move ops
      std::vector<std::pair<Address, uint256_t>> valCopy2 = values;

      // Mass insert by copy, then revert
      for (std::pair<Address, uint256_t> val : values) map.insert(val);
      map.revert();
      REQUIRE(map.size() == 1);
      REQUIRE(map.at(add0) == bal0);

      // Mass insert by copy, then commit
      for (std::pair<Address, uint256_t> val : values) map.insert(val);
      map.commit();
      REQUIRE(map.size() == 101);
      for (std::pair<Address, uint256_t> val : values) REQUIRE(map.at(val.first) == val.second);

      map.clear(); map.insert(std::make_pair(add0, bal0)); map.commit(); // revert to starting map

      // Mass insert by move, then revert
      for (std::pair<Address, uint256_t> val : valCopy1) map.insert(std::move(val));
      map.revert();
      REQUIRE(map.size() == 1);
      REQUIRE(map.at(add0) == bal0);

      // Mass insert by move, then commit
      for (std::pair<Address, uint256_t> val : valCopy2) map.insert(std::move(val));
      map.commit();
      REQUIRE(map.size() == 101);
      for (std::pair<Address, uint256_t> val : values) REQUIRE(map.at(val.first) == val.second);

      // Force failure on insert (for coverage)
      REQUIRE_FALSE(map.insert(valCopy2[0]).second);
    }

    SECTION("insert (hint)") {
      Address add0(Utils::randBytes(20));
      uint256_t bal0("19283815712031512");
      SafeUnorderedMap<Address, uint256_t> map({{add0,bal0}});

      // Same as insert (simple), but hint is set to cbegin() anyway, we test this just for conscience's sake
      std::vector<std::pair<Address, uint256_t>> values;
      for (uint8_t i = 0; i < 100; i++) { // inserting 100 values
        values.emplace_back(std::make_pair(Address(Utils::randBytes(20)), bal0));
      }
      std::vector<std::pair<Address, uint256_t>> valCopy1 = values; // copies for move ops
      std::vector<std::pair<Address, uint256_t>> valCopy2 = values;

      // Mass insert by copy, then revert
      for (std::pair<Address, uint256_t> val : values) map.insert(map.cbegin(), val);
      map.revert();
      REQUIRE(map.size() == 1);
      REQUIRE(map.at(add0) == bal0);

      // Mass insert by copy, then commit
      for (std::pair<Address, uint256_t> val : values) map.insert(map.cbegin(), val);
      map.commit();
      REQUIRE(map.size() == 101);
      for (std::pair<Address, uint256_t> val : values) REQUIRE(map.at(val.first) == val.second);

      map.clear(); map.insert(std::make_pair(add0, bal0)); map.commit(); // revert to starting map

      // Mass insert by move, then revert
      for (std::pair<Address, uint256_t> val : valCopy1) map.insert(map.cbegin(), std::move(val));
      map.revert();
      REQUIRE(map.size() == 1);
      REQUIRE(map.at(add0) == bal0);

      // Mass insert by move, then commit
      for (std::pair<Address, uint256_t> val : valCopy2) map.insert(map.cbegin(), std::move(val));
      map.commit();
      REQUIRE(map.size() == 101);
      for (std::pair<Address, uint256_t> val : values) REQUIRE(map.at(val.first) == val.second);
    }

    SECTION("insert (range)") {
      Address add0(Utils::randBytes(20));
      uint256_t bal0("19283815712031512");
      SafeUnorderedMap<Address, uint256_t> map({{add0,bal0}});

      // Same as insert (simple), but we use iterators to the vector instead (also an ilist)
      std::vector<std::pair<Address, uint256_t>> values;
      for (uint8_t i = 0; i < 100; i++) { // Inserting 100 values
        values.emplace_back(std::make_pair(Address(Utils::randBytes(20)), bal0));
      }
      std::initializer_list<std::pair<const Address, uint256_t>> ilist { // Creating 10 values
        std::make_pair(Address(Utils::randBytes(20)), bal0),
        std::make_pair(Address(Utils::randBytes(20)), bal0),
        std::make_pair(Address(Utils::randBytes(20)), bal0),
        std::make_pair(Address(Utils::randBytes(20)), bal0),
        std::make_pair(Address(Utils::randBytes(20)), bal0),
        std::make_pair(Address(Utils::randBytes(20)), bal0),
        std::make_pair(Address(Utils::randBytes(20)), bal0),
        std::make_pair(Address(Utils::randBytes(20)), bal0),
        std::make_pair(Address(Utils::randBytes(20)), bal0),
        std::make_pair(Address(Utils::randBytes(20)), bal0)
      };

      // Mass insert by iterators, then revert
      map.insert(values.cbegin(), values.cend());
      map.revert();
      REQUIRE(map.size() == 1);
      REQUIRE(map.at(add0) == bal0);

      // Mass insert by iterators, then commit
      map.insert(values.cbegin(), values.cend());
      map.commit();
      REQUIRE(map.size() == 101);
      for (std::pair<Address, uint256_t> val : values) REQUIRE(map.at(val.first) == val.second);

      map.insert(values.cbegin(), values.cend()); // "Re-insert" for coverage

      map.clear(); map.insert(std::make_pair(add0, bal0)); map.commit(); // Revert to starting map

      // Mass insert by ilist, then revert
      map.insert(ilist);
      map.revert();
      REQUIRE(map.size() == 1);
      REQUIRE(map.at(add0) == bal0);

      // Mass insert by ilist, then commit
      map.insert(ilist);
      map.commit();
      REQUIRE(map.size() == 11);
      for (auto it = ilist.begin(); it != ilist.end(); it++) REQUIRE(map.at(it->first) == it->second);

      map.insert(ilist); // "Re-insert" for coverage
    }

    SECTION("insert_or_assign (copy)") {
      Address add0(Utils::randBytes(20));
      uint256_t bal0("19283815712031512");
      SafeUnorderedMap<Address, uint256_t> map({{add0,bal0}});

      // Two rounds: one for insert, another for assign
      uint256_t oldBal("93847329875983254");
      uint256_t newBal("38975489237598433");
      std::vector<std::pair<Address, uint256_t>> oldVals;
      std::vector<std::pair<Address, uint256_t>> newVals;
      for (uint8_t i = 0; i < 100; i++) { // inserting 100 values
        Address newAdd(Utils::randBytes(20));
        oldVals.emplace_back(std::make_pair(newAdd, oldBal));
        newVals.emplace_back(std::make_pair(newAdd, newBal));
      }

      // Insert and revert
      for (std::pair<Address, uint256_t> val : oldVals) map.insert_or_assign(val.first, val.second);
      map.revert();
      REQUIRE(map.size() == 1);
      REQUIRE(map.at(add0) == bal0);

      // Insert and commit
      for (std::pair<Address, uint256_t> val : oldVals) map.insert_or_assign(val.first, val.second);
      map.commit();
      REQUIRE(map.size() == 101);
      for (std::pair<Address, uint256_t> val : oldVals) REQUIRE(map.at(val.first) == val.second);

      // Assign and revert
      for (std::pair<Address, uint256_t> val : newVals) map.insert_or_assign(val.first, val.second);
      map.revert();
      REQUIRE(map.size() == 101);
      for (std::pair<Address, uint256_t> val : oldVals) REQUIRE(map.at(val.first) == val.second);

      // Assign and commit
      for (std::pair<Address, uint256_t> val : newVals) map.insert_or_assign(val.first, val.second);
      map.commit();
      REQUIRE(map.size() == 101);
      for (std::pair<Address, uint256_t> val : newVals) REQUIRE(map.at(val.first) == val.second);
    }

    SECTION("insert_or_assign (move)") {
      Address add0(Utils::randBytes(20));
      uint256_t bal0("19283815712031512");
      SafeUnorderedMap<Address, uint256_t> map({{add0,bal0}});

      // Same logic, but calls the move function instead
      uint256_t oldBal("93847329875983254");
      uint256_t newBal("38975489237598433");
      std::vector<std::pair<Address, uint256_t>> oldVals;
      std::vector<std::pair<Address, uint256_t>> newVals;
      for (uint8_t i = 0; i < 100; i++) { // inserting 100 values
        Address newAdd(Utils::randBytes(20));
        oldVals.emplace_back(std::make_pair(newAdd, oldBal));
        newVals.emplace_back(std::make_pair(newAdd, newBal));
      }
      // We make copies because it'll all be moved, but we still need a reference to compare to
      std::vector<std::pair<Address, uint256_t>> oldVals1 = oldVals;
      std::vector<std::pair<Address, uint256_t>> oldVals2 = oldVals;
      std::vector<std::pair<Address, uint256_t>> newVals1 = newVals;
      std::vector<std::pair<Address, uint256_t>> newVals2 = newVals;

      // Insert and revert
      for (std::pair<Address, uint256_t> val : oldVals1) {
        map.insert_or_assign(std::move(val.first), std::move(val.second));
      }
      map.revert();
      REQUIRE(map.size() == 1);
      REQUIRE(map.at(add0) == bal0);

      // Insert and commit
      for (std::pair<Address, uint256_t> val : oldVals2) {
        map.insert_or_assign(std::move(val.first), std::move(val.second));
      }
      map.commit();
      REQUIRE(map.size() == 101);
      for (std::pair<Address, uint256_t> val : oldVals) REQUIRE(map.at(val.first) == val.second);

      // Assign and revert
      for (std::pair<Address, uint256_t> val : newVals1) {
        map.insert_or_assign(std::move(val.first), std::move(val.second));
      }
      map.revert();
      REQUIRE(map.size() == 101);
      for (std::pair<Address, uint256_t> val : oldVals) REQUIRE(map.at(val.first) == val.second);

      // Assign and commit
      for (std::pair<Address, uint256_t> val : newVals2) {
        map.insert_or_assign(std::move(val.first), std::move(val.second));
      }
      map.commit();
      REQUIRE(map.size() == 101);
      for (std::pair<Address, uint256_t> val : newVals) REQUIRE(map.at(val.first) == val.second);
    }

    SECTION("insert_or_assign (copy+hint)") {
      Address add0(Utils::randBytes(20));
      uint256_t bal0("19283815712031512");
      SafeUnorderedMap<Address, uint256_t> map({{add0,bal0}});

      // Same as insert(hint), again, for conscience's sake
      uint256_t oldBal("93847329875983254");
      uint256_t newBal("38975489237598433");
      std::vector<std::pair<Address, uint256_t>> oldVals;
      std::vector<std::pair<Address, uint256_t>> newVals;
      for (uint8_t i = 0; i < 100; i++) { // inserting 100 values
        Address newAdd(Utils::randBytes(20));
        oldVals.emplace_back(std::make_pair(newAdd, oldBal));
        newVals.emplace_back(std::make_pair(newAdd, newBal));
      }

      // Insert and revert
      for (std::pair<Address, uint256_t> val : oldVals) map.insert_or_assign(map.cbegin(), val.first, val.second);
      map.revert();
      REQUIRE(map.size() == 1);
      REQUIRE(map.at(add0) == bal0);

      // Insert and commit
      for (std::pair<Address, uint256_t> val : oldVals) map.insert_or_assign(map.cbegin(), val.first, val.second);
      map.commit();
      REQUIRE(map.size() == 101);
      for (std::pair<Address, uint256_t> val : oldVals) REQUIRE(map.at(val.first) == val.second);

      // Assign and revert
      for (std::pair<Address, uint256_t> val : newVals) map.insert_or_assign(map.cbegin(), val.first, val.second);
      map.revert();
      REQUIRE(map.size() == 101);
      for (std::pair<Address, uint256_t> val : oldVals) REQUIRE(map.at(val.first) == val.second);

      // Assign and commit
      for (std::pair<Address, uint256_t> val : newVals) map.insert_or_assign(map.cbegin(), val.first, val.second);
      map.commit();
      REQUIRE(map.size() == 101);
      for (std::pair<Address, uint256_t> val : newVals) REQUIRE(map.at(val.first) == val.second);
    }

    SECTION("insert_or_assign (move+hint)") {
      Address add0(Utils::randBytes(20));
      uint256_t bal0("19283815712031512");
      SafeUnorderedMap<Address, uint256_t> map({{add0,bal0}});

      // Ditto (I know this is dumb but hey testing is testing)
      uint256_t oldBal("93847329875983254");
      uint256_t newBal("38975489237598433");
      std::vector<std::pair<Address, uint256_t>> oldVals;
      std::vector<std::pair<Address, uint256_t>> newVals;
      for (uint8_t i = 0; i < 100; i++) { // inserting 100 values
        Address newAdd(Utils::randBytes(20));
        oldVals.emplace_back(std::make_pair(newAdd, oldBal));
        newVals.emplace_back(std::make_pair(newAdd, newBal));
      }
      // We make copies because it'll all be moved, but we still need a reference to compare to
      std::vector<std::pair<Address, uint256_t>> oldVals1 = oldVals;
      std::vector<std::pair<Address, uint256_t>> oldVals2 = oldVals;
      std::vector<std::pair<Address, uint256_t>> newVals1 = newVals;
      std::vector<std::pair<Address, uint256_t>> newVals2 = newVals;

      // Insert and revert
      for (std::pair<Address, uint256_t> val : oldVals1) {
        map.insert_or_assign(map.cbegin(), std::move(val.first), std::move(val.second));
      }
      map.revert();
      REQUIRE(map.size() == 1);
      REQUIRE(map.at(add0) == bal0);

      // Insert and commit
      for (std::pair<Address, uint256_t> val : oldVals2) {
        map.insert_or_assign(map.cbegin(), std::move(val.first), std::move(val.second));
      }
      map.commit();
      REQUIRE(map.size() == 101);
      for (std::pair<Address, uint256_t> val : oldVals) REQUIRE(map.at(val.first) == val.second);

      // Assign and revert
      for (std::pair<Address, uint256_t> val : newVals1) {
        map.insert_or_assign(map.cbegin(), std::move(val.first), std::move(val.second));
      }
      map.revert();
      REQUIRE(map.size() == 101);
      for (std::pair<Address, uint256_t> val : oldVals) REQUIRE(map.at(val.first) == val.second);

      // Assign and commit
      for (std::pair<Address, uint256_t> val : newVals2) {
        map.insert_or_assign(map.cbegin(), std::move(val.first), std::move(val.second));
      }
      map.commit();
      REQUIRE(map.size() == 101);
      for (std::pair<Address, uint256_t> val : newVals) REQUIRE(map.at(val.first) == val.second);
    }

    SECTION("emplace + emplace_hint") {
      Address add0(Utils::randBytes(20));
      uint256_t bal0("19283815712031512");
      SafeUnorderedMap<Address, uint256_t> map({{add0,bal0}});

      // Same as insert but it's emplace... yadda yadda yadda...
      REQUIRE(map.emplace(std::make_pair(add0, bal0)).second == false);
      std::pair<Address, uint256_t> movePair = std::make_pair(add0, bal0);
      REQUIRE(map.emplace(std::move(movePair)).second == false);
      std::vector<std::pair<Address, uint256_t>> values;
      for (uint8_t i = 0; i < 100; i++) { // emplacing 100 values
        values.emplace_back(std::make_pair(Address(Utils::randBytes(20)), bal0));
      }

      // Mass emplace, then revert
      for (std::pair<Address, uint256_t> val : values) map.emplace(val);
      map.revert();
      REQUIRE(map.size() == 1);
      REQUIRE(map.at(add0) == bal0);

      // Mass emplace, then commit
      for (std::pair<Address, uint256_t> val : values) map.emplace(val);
      map.commit();
      REQUIRE(map.size() == 101);
      for (std::pair<Address, uint256_t> val : values) REQUIRE(map.at(val.first) == val.second);

      map.clear(); map.insert(std::make_pair(add0, bal0)); map.commit(); // revert to starting map

      // Mass emplace with hint, then revert
      for (std::pair<Address, uint256_t> val : values) map.emplace_hint(map.cbegin(), val);
      map.revert();
      REQUIRE(map.size() == 1);
      REQUIRE(map.at(add0) == bal0);

      // Mass emplace with hint, then commit
      for (std::pair<Address, uint256_t> val : values) map.emplace_hint(map.cbegin(), val);
      map.commit();
      REQUIRE(map.size() == 101);
      for (std::pair<Address, uint256_t> val : values) REQUIRE(map.at(val.first) == val.second);
    }

    SECTION("try_emplace (copy key + move key)") {
      Address add0(Utils::randBytes(20));
      uint256_t bal0("19283815712031512");
      SafeUnorderedMap<Address, uint256_t> map({{add0,bal0}});

      // ...yadda yadda yadda but there's one extra step to test the
      // "do nothing if key exists" part ok I think we all get it by now
      std::vector<std::pair<Address, uint256_t>> values;
      for (uint8_t i = 0; i < 100; i++) { // emplacing 100 values
        values.emplace_back(std::make_pair(Address(Utils::randBytes(20)), bal0));
      }
      std::vector<std::pair<Address, uint256_t>> values1 = values; // copies for move ops
      std::vector<std::pair<Address, uint256_t>> values2 = values;
      std::vector<std::pair<Address, uint256_t>> values3 = values;

      // Mass emplace by key copy, then revert
      for (std::pair<Address, uint256_t> val : values) map.try_emplace(val.first, val.second);
      map.revert();
      REQUIRE(map.size() == 1);
      REQUIRE(map.at(add0) == bal0);

      // Mass emplace by key copy, then commit
      for (std::pair<Address, uint256_t> val : values) map.try_emplace(val.first, val.second);
      map.commit();
      REQUIRE(map.size() == 101);
      for (std::pair<Address, uint256_t> val : values) REQUIRE(map.at(val.first) == val.second);

      // Mass emplace again, but confirm it's doing nothing
      for (std::pair<Address, uint256_t> val : values) {
        REQUIRE(map.try_emplace(val.first, val.second).second == false);
      }

      map.clear(); map.insert(std::make_pair(add0, bal0)); map.commit(); // revert to starting map

      // Mass emplace by key move, then revert
      for (std::pair<Address, uint256_t> val : values1) map.try_emplace(std::move(val.first), val.second);
      map.revert();
      REQUIRE(map.size() == 1);
      REQUIRE(map.at(add0) == bal0);

      // Mass emplace by key move, then commit
      for (std::pair<Address, uint256_t> val : values2) map.try_emplace(std::move(val.first), val.second);
      map.commit();
      REQUIRE(map.size() == 101);
      for (std::pair<Address, uint256_t> val : values) REQUIRE(map.at(val.first) == val.second);

      // Mass emplace again, but confirm it's doing nothing
      for (std::pair<Address, uint256_t> val : values3) {
        REQUIRE(map.try_emplace(val.first, val.second).second == false);
      }
    }

    SECTION("try_emplace (copy key hint + move key hint)") {
      Address add0(Utils::randBytes(20));
      uint256_t bal0("19283815712031512");
      SafeUnorderedMap<Address, uint256_t> map({{add0,bal0}});

      // literally feeling like youtube id ILoWMEWd7Yk rn
      std::vector<std::pair<Address, uint256_t>> values;
      for (uint8_t i = 0; i < 100; i++) { // emplacing 100 values
        values.emplace_back(std::make_pair(Address(Utils::randBytes(20)), bal0));
      }
      std::vector<std::pair<Address, uint256_t>> values1 = values; // copies for move ops
      std::vector<std::pair<Address, uint256_t>> values2 = values;

      // Mass emplace by key copy, then revert
      for (std::pair<Address, uint256_t> val : values) map.try_emplace(map.cbegin(), val.first, val.second);
      map.revert();
      REQUIRE(map.size() == 1);
      REQUIRE(map.at(add0) == bal0);

      // Mass emplace by key copy, then commit
      for (std::pair<Address, uint256_t> val : values) map.try_emplace(map.cbegin(), val.first, val.second);
      map.commit();
      REQUIRE(map.size() == 101);
      for (std::pair<Address, uint256_t> val : values) REQUIRE(map.at(val.first) == val.second);

      map.clear(); map.insert(std::make_pair(add0, bal0)); map.commit(); // revert to starting map

      // Mass emplace by key move, then revert
      for (std::pair<Address, uint256_t> val : values1) map.try_emplace(map.cbegin(), std::move(val.first), val.second);
      map.revert();
      REQUIRE(map.size() == 1);
      REQUIRE(map.at(add0) == bal0);

      // Mass emplace by key move, then commit
      for (std::pair<Address, uint256_t> val : values2) map.try_emplace(map.cbegin(), std::move(val.first), val.second);
      map.commit();
      REQUIRE(map.size() == 101);
      for (std::pair<Address, uint256_t> val : values) REQUIRE(map.at(val.first) == val.second);
    }

    SECTION("erase") {
      Address add0(Utils::randBytes(20));
      uint256_t bal0("19283815712031512");
      SafeUnorderedMap<Address, uint256_t> map({{add0,bal0}});
      std::vector<std::pair<Address, uint256_t>> values;
      for (uint8_t i = 0; i < 100; i++) { // emplacing 100 values
        values.emplace_back(std::make_pair(Address(Utils::randBytes(20)), bal0));
      }
      map.insert(values.cbegin(), values.cend());
      map.commit();
      REQUIRE(map.size() == 101);

      // Erase a single key using an iterator
      std::pair<Address, uint256_t> firstVal = (*map.cbegin());
      map.erase(map.cbegin());
      map.revert();
      REQUIRE(map.size() == 101);
      REQUIRE((map.cbegin()->first == firstVal.first && map.cbegin()->second == firstVal.second));
      map.erase(map.cbegin());
      map.commit();
      REQUIRE(map.size() == 100);
      REQUIRE(!map.contains(firstVal.first));
      map[firstVal.first] = firstVal.second; map.commit(); REQUIRE(map.size() == 101); // Re-add the key for next test

      // Erase a single key using a value
      map.erase(add0);
      map.revert();
      REQUIRE(map.size() == 101);
      REQUIRE(map[add0] == bal0);
      map.erase(add0);
      map.commit();
      REQUIRE(map.size() == 100);
      REQUIRE(!map.contains(add0));

      // Erase a range of keys using iterators
      auto itB = map.cbegin();
      auto itE = map.cbegin();
      std::advance(itE, map.size() / 2); // Thanos snap, half the map is gone
      map.erase(itB, itE);
      map.revert();
      REQUIRE(map.size() == 100);
      itB = map.cbegin(); // Refresh iterators just in case
      itE = map.cbegin();
      std::advance(itE, map.size() / 2);
      map.erase(itB, itE);
      map.commit();
      REQUIRE(map.size() == 50);
    }
  }
}

