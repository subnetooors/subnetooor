/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#include "libs/catch2/catch_amalgamated.hpp"

#include "utils/db.h" // utils.h -> strings.h, libs/json.hpp -> (filesystem, string)
#include "utils/strconv.h"

#include "bytes/random.h"

using Catch::Matchers::Equals;

namespace TDB {
  TEST_CASE("DB Tests", "[unit][utils][db]") {
    SECTION("DBBatch Manipulation") {
      DBBatch batch;
      batch.push_back(StrConv::stringToBytes("aaaa"), StrConv::stringToBytes("1234"), StrConv::stringToBytes("0000"));
      batch.push_back(DBEntry(Bytes{0xbb, 0xbb}, Bytes{0x56, 0x78}));
      batch.delete_key(StrConv::stringToBytes("aaaa"), StrConv::stringToBytes("0000"));
      batch.delete_key(Bytes{0xbb, 0xbb});
      auto puts = batch.getPuts();
      auto dels = batch.getDels();
      REQUIRE(puts[0].key == StrConv::stringToBytes("0000aaaa"));
      REQUIRE(puts[0].value == StrConv::stringToBytes("1234"));
      REQUIRE(puts[1].key == Bytes{0xbb, 0xbb});
      REQUIRE(puts[1].value == Bytes{0x56, 0x78});
      REQUIRE(dels[0] == StrConv::stringToBytes("0000aaaa"));
      REQUIRE(dels[1] == Bytes{0xbb, 0xbb});
    }

    SECTION("Open and Close DB (w/ throw)") {
      bool catched = false;
      DB db("testDB");
      REQUIRE(std::filesystem::exists(std::filesystem::current_path().string() + "/testDB"));
      try { DB db2("testDB"); } catch (std::exception& e) { catched = true; }
      REQUIRE(catched == true);
      REQUIRE(db.close());
    }

    SECTION("Simple CRUD (Create + Read + Update + Delete)") {
      // Open
      DB db("testDB");
      std::string key = "d41472b71899ccc0cf16c09ac97af95e";
      std::string value = "5ea04e91c96033ae312af0bb22ec3e370c7789dc28858ea0135966ee2966a616";
      Bytes pfx{0x00, 0x01};

      // Create
      REQUIRE(db.put(key, value, pfx));
      REQUIRE(db.has(key, pfx));

      // Read
      REQUIRE(StrConv::bytesToString(db.get(key, pfx)) == value);

      // Update
      std::string newValue = "f5ea6cbe8cddc3f73bc40e156ced5ef0f80d75bd6794ba18a457c46edaeee6a4";
      REQUIRE(db.put(key, newValue, pfx));
      REQUIRE(db.has(key, pfx));
      REQUIRE(StrConv::bytesToString(db.get(key, pfx)) == newValue);

      // Delete
      REQUIRE(db.del(key, pfx));
      REQUIRE(!db.has(key, pfx));

      // Close
      REQUIRE(db.close());
    }

    SECTION("Batched CRUD (Create + Read + Update + Delete)") {
      // Open
      DB db("testDB");
      Bytes pfx{0x00, 0x00}; // 0000 - "test prefix", doesn't actually exist
      DBBatch batchP;
      DBBatch batchD;
      for (int i = 0; i < 32; i++) {
        batchP.push_back(Utils::makeBytes(bytes::random(32)), Utils::makeBytes(bytes::random(32)), pfx);
        batchD.delete_key(batchP.getPuts()[i].key, pfx);
      }
      std::vector<Bytes> keys; // Reference vector for read checks
      for (const DBEntry& e : batchP.getPuts()) keys.push_back(e.key);
      for (Bytes& k : keys) k.erase(k.begin(), k.begin() + 2); // Remove prefixes (2 bytes)
      std::sort(keys.begin(), keys.end(), [&](Bytes a, Bytes b){ return a < b; }); // Sort the vector for querying key range

      // Create
      std::cout << "BatchPuts: " << batchP.getPuts().size() << std::endl;
      REQUIRE(db.putBatch(batchP));
      for (const DBEntry& entry : batchP.getPuts()) {
        REQUIRE(db.has(entry.key)); // No need to pass prefix as entry.key already contains it
      }

      // Read (all)
      std::vector<DBEntry> getB = db.getBatch(pfx);
      REQUIRE(!getB.empty());
      for (const DBEntry& getE : getB) {
        for (const DBEntry& putE : batchP.getPuts()) {
          if (getE.key == putE.key) REQUIRE(getE.value == putE.value);
        }
      }

      // Read (specific, for coverage)
      std::vector<Bytes> keysToSearch = {keys[0], keys[8], keys[16], keys[24]};
      std::vector<DBEntry> getBS = db.getBatch(pfx, keysToSearch);
      REQUIRE(!getBS.empty());
      REQUIRE(getBS.size() == 4);
      for (const DBEntry& getES : getBS) {
        REQUIRE(std::find(keys.begin(), keys.end(), getES.key) != keys.end());
      }

      // Read (getKeys without limits, for coverage)
      std::vector<Bytes> getBKAll = db.getKeys(pfx);
      for (const Bytes& b : getBKAll) {
        REQUIRE(std::find(keys.begin(), keys.end(), b) != keys.end());
      }

      // Read (getKeys with limits, for coverage)
      std::vector<Bytes> getBK = db.getKeys(pfx, keys[0], keys[7]);
      for (const Bytes& b : getBK) {
        REQUIRE(std::find(keys.begin(), keys.end(), b) != keys.end());
      }

      // Read (getLastByPrefix, for coverage)
      Bytes getLast = db.getLastByPrefix(pfx);
      bool found = false;
      for (const DBEntry& getE : getB) {
        if (getE.value == getLast) found = true;
      }
      REQUIRE(found);

      // Update
      DBBatch newPutB;
      for (int i = 0; i < 32; i++) {
        newPutB.push_back(batchP.getPuts()[i].key, Utils::makeBytes(bytes::random(32)), pfx);
      }
      REQUIRE(db.putBatch(newPutB));
      // No need to pass prefix as entry.key already contains it
      for (const DBEntry& entry : newPutB.getPuts()) REQUIRE(db.has(entry.key));
      std::vector<DBEntry> newGetB = db.getBatch(pfx);
      REQUIRE(!newGetB.empty());
      for (const DBEntry& newGetE : newGetB) {
        for (const DBEntry& newPutE : newPutB.getPuts()) {
          if (newGetE.key == newPutE.key) {
            REQUIRE(newGetE.value == newPutE.value);
          }
        }
      }

      // Delete
      REQUIRE(db.putBatch(batchD));
      // No need to pass prefix as key already contains it
      for (const Bytes& key : batchD.getDels()) REQUIRE(!db.has(key));

      // Close
      REQUIRE(db.close());
    }

    SECTION("Throws/Errors") {
      DB db("testDB");
      REQUIRE(!db.has(StrConv::stringToBytes("dummy")));
      REQUIRE(db.get(StrConv::stringToBytes("dummy")).empty());
      REQUIRE(db.getBatch(StrConv::stringToBytes("0001"), {StrConv::stringToBytes(("dummy"))}).empty());
      REQUIRE(db.close());
    }

    // Clean up last test so DB creation can be properly tested next time
    std::filesystem::remove_all(std::filesystem::current_path().string() + "/testDB");
  }
}

