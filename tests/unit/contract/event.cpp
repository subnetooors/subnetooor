/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#include "libs/catch2/catch_amalgamated.hpp"

#include "contract/event.h"

#include "bytes/view.h"
#include "bytes/random.h"
#include "bytes/hex.h"

namespace TEvent {
  TEST_CASE("Event Class", "[unit][contract][event]") {
    SECTION("EVM Event Constructor") {
      Hash txHash = bytes::random();
      Hash blockHash = bytes::random();
      std::vector<Hash> topics = {bytes::random(), bytes::random(), bytes::random(), bytes::random(), bytes::random()};
      Address add(bytes::hex("0x1234567890123456789012345678901234567890"));
      Bytes data{0xDE, 0xAD, 0xBE, 0xEF};

      Event e("myEvent", 0, txHash, 1, blockHash, 2, add, data, topics, false);
      REQUIRE(e.getName() == "myEvent");
      REQUIRE(e.getLogIndex() == 0);
      REQUIRE(e.getTxHash() == txHash);
      REQUIRE(e.getTxIndex() == 1);
      REQUIRE(e.getBlockHash() == blockHash);
      REQUIRE(e.getBlockIndex() == 2);
      REQUIRE(e.getAddress() == add);
      REQUIRE(e.getData() == data);
      REQUIRE(e.getTopics().size() == 5);
      REQUIRE(e.getTopics()[0] == topics[0]);
      REQUIRE(e.getTopics()[1] == topics[1]);
      REQUIRE(e.getTopics()[2] == topics[2]);
      REQUIRE(e.getTopics()[3] == topics[3]);
      REQUIRE(e.getTopics()[4] == topics[4]);
      REQUIRE(e.isAnonymous() == false);
      REQUIRE(e.getSelector() == topics[0]);
    }

    SECTION("CPP Event Constructor") {
      Hash txHash = bytes::random();
      Hash blockHash = bytes::random();
      Address add(bytes::hex("0x1234567890123456789012345678901234567890"));

      // Anonymous event
      Event e1("myEvent", 0, txHash, 1, blockHash, 2, add, std::make_tuple(
        EventParam<std::string, true>("p1"),
        EventParam<std::string, true>("p2"),
        EventParam<std::string, true>("p3"),
        EventParam<std::string, true>("p4"),
        EventParam<std::string, true>("p5")
      ), true);
      REQUIRE(e1.getName() == "myEvent");
      REQUIRE(e1.getLogIndex() == 0);
      REQUIRE(e1.getTxHash() == txHash);
      REQUIRE(e1.getTxIndex() == 1);
      REQUIRE(e1.getBlockHash() == blockHash);
      REQUIRE(e1.getBlockIndex() == 2);
      REQUIRE(e1.getAddress() == add);
      REQUIRE(e1.getData() == Bytes());
      REQUIRE(e1.getTopics().size() == 4); // topics only goes up to 4 even though we put 5
      REQUIRE(e1.isAnonymous() == true);
      REQUIRE(e1.getSelector() == Hash());

      // Non-anonymous event
      Event e2("myEvent", 0, txHash, 1, blockHash, 2, add, std::make_tuple(
        EventParam<std::string, true>("p1"),
        EventParam<std::string, true>("p2"),
        EventParam<std::string, true>("p3"),
        EventParam<std::string, true>("p4"),
        EventParam<std::string, true>("p5")
      ), false);
      REQUIRE(e2.getName() == "myEvent");
      REQUIRE(e2.getLogIndex() == 0);
      REQUIRE(e2.getTxHash() == txHash);
      REQUIRE(e2.getTxIndex() == 1);
      REQUIRE(e2.getBlockHash() == blockHash);
      REQUIRE(e2.getBlockIndex() == 2);
      REQUIRE(e2.getAddress() == add);
      REQUIRE(e2.getData() == Bytes());
      REQUIRE(e2.getTopics().size() == 4); // topics only goes up to 4 (event signature + 3) even though we put 5
      REQUIRE(e2.isAnonymous() == false);
      REQUIRE(e2.getSelector() != Hash());
    }

    SECTION("JSON String Constructor") {
      // Copied from above
      std::string e1Str = "{\"name\":\"myEvent\",\"logIndex\":0,\"txHash\":\"0x05846d60d5b92b068c28a9017831e29827243bd4f642734977dcb111ccd40425\",\"txIndex\":1,\"blockHash\":\"0xec313458e29969850621411a36ab7b93c2b494ffb1ca77b5e62ea5b42100674d\",\"blockIndex\":2,\"address\":\"0x1234567890123456789012345678901234567890\",\"data\":[],\"topics\":[\"0x260e065801cba6ca065f28640c3d94ef235f67db5431448aae1a51af7214efaf\",\"0xc30a3ae685bfcb917dceb41e4afed5342f332572d5b3a8212679077685c494cb\",\"0xff05acda0d6ef15409d713cb0f124d2c3a3fd95b33af096109172229d5c8671a\",\"0x0c38459a0b5ed2a98afa2a407dc31ff1744c1c3159dbbbd3aef8736778a0e063\"],\"anonymous\":true}";
      std::string e2Str = "{\"name\":\"myEvent\",\"logIndex\":0,\"txHash\":\"0x05846d60d5b92b068c28a9017831e29827243bd4f642734977dcb111ccd40425\",\"txIndex\":1,\"blockHash\":\"0xec313458e29969850621411a36ab7b93c2b494ffb1ca77b5e62ea5b42100674d\",\"blockIndex\":2,\"address\":\"0x1234567890123456789012345678901234567890\",\"data\":[],\"topics\":[\"0x386cc2513b9e8b9e78a0792c33d6f69798774e0fa5424d3042fdd0fe7647420b\",\"0x260e065801cba6ca065f28640c3d94ef235f67db5431448aae1a51af7214efaf\",\"0xc30a3ae685bfcb917dceb41e4afed5342f332572d5b3a8212679077685c494cb\",\"0xff05acda0d6ef15409d713cb0f124d2c3a3fd95b33af096109172229d5c8671a\"],\"anonymous\":false}";

      Event e1(e1Str);
      Event e2(e2Str);

      REQUIRE(e1.getName() == "myEvent");
      REQUIRE(e1.getLogIndex() == 0);
      REQUIRE(e1.getTxHash().hex(true).get() == "0x05846d60d5b92b068c28a9017831e29827243bd4f642734977dcb111ccd40425");
      REQUIRE(e1.getTxIndex() == 1);
      REQUIRE(e1.getBlockHash().hex(true).get() == "0xec313458e29969850621411a36ab7b93c2b494ffb1ca77b5e62ea5b42100674d");
      REQUIRE(e1.getBlockIndex() == 2);
      REQUIRE(e1.getAddress().hex(true).get() == "0x1234567890123456789012345678901234567890");
      REQUIRE(e1.getData() == Bytes());
      REQUIRE(e1.getTopics().size() == 4);
      REQUIRE(e1.getTopics()[0].hex(true).get() == "0x260e065801cba6ca065f28640c3d94ef235f67db5431448aae1a51af7214efaf");
      REQUIRE(e1.getTopics()[1].hex(true).get() == "0xc30a3ae685bfcb917dceb41e4afed5342f332572d5b3a8212679077685c494cb");
      REQUIRE(e1.getTopics()[2].hex(true).get() == "0xff05acda0d6ef15409d713cb0f124d2c3a3fd95b33af096109172229d5c8671a");
      REQUIRE(e1.getTopics()[3].hex(true).get() == "0x0c38459a0b5ed2a98afa2a407dc31ff1744c1c3159dbbbd3aef8736778a0e063");
      REQUIRE(e1.isAnonymous() == true);
      REQUIRE(e1.getSelector() == Hash());

      REQUIRE(e2.getName() == "myEvent");
      REQUIRE(e2.getLogIndex() == 0);
      REQUIRE(e2.getTxHash().hex(true).get() == "0x05846d60d5b92b068c28a9017831e29827243bd4f642734977dcb111ccd40425");
      REQUIRE(e2.getTxIndex() == 1);
      REQUIRE(e2.getBlockHash().hex(true).get() == "0xec313458e29969850621411a36ab7b93c2b494ffb1ca77b5e62ea5b42100674d");
      REQUIRE(e2.getBlockIndex() == 2);
      REQUIRE(e2.getAddress().hex(true).get() == "0x1234567890123456789012345678901234567890");
      REQUIRE(e2.getData() == Bytes());
      REQUIRE(e2.getTopics().size() == 4);
      REQUIRE(e2.getTopics()[0].hex(true).get() == "0x386cc2513b9e8b9e78a0792c33d6f69798774e0fa5424d3042fdd0fe7647420b");
      REQUIRE(e2.getTopics()[1].hex(true).get() == "0x260e065801cba6ca065f28640c3d94ef235f67db5431448aae1a51af7214efaf");
      REQUIRE(e2.getTopics()[2].hex(true).get() == "0xc30a3ae685bfcb917dceb41e4afed5342f332572d5b3a8212679077685c494cb");
      REQUIRE(e2.getTopics()[3].hex(true).get() == "0xff05acda0d6ef15409d713cb0f124d2c3a3fd95b33af096109172229d5c8671a");
      REQUIRE(e2.isAnonymous() == false);
      REQUIRE(e2.getSelector() != Hash());
    }

    SECTION("Serialization (Normal + RPC)") {
      Hash txHash(Hex::toBytes("0x53472c61f1db8612fcdd17f24b78986bfa111ea3e323522456b1a78560f2215a"));
      Hash blockHash(Hex::toBytes("0x2b9b8644330d50ffb90c5fea02b73b562dfc550ec7f8c85f643b20391a972d5f"));
      Address add(bytes::hex("0x1234567890123456789012345678901234567890"));
      Event e1("myEvent", 0, txHash, 1, blockHash, 2, add, std::make_tuple(
        EventParam<std::string, true>("p1"),
        EventParam<std::string, true>("p2"),
        EventParam<std::string, true>("p3"),
        EventParam<std::string, true>("p4"),
        EventParam<std::string, true>("p5")
      ), false);

      std::string e1Str = e1.serializeToJson();
      json e1Json = e1.serializeForRPC();

      REQUIRE(e1Str == "{\"name\":\"myEvent\",\"logIndex\":0,\"txHash\":\"0x53472c61f1db8612fcdd17f24b78986bfa111ea3e323522456b1a78560f2215a\",\"txIndex\":1,\"blockHash\":\"0x2b9b8644330d50ffb90c5fea02b73b562dfc550ec7f8c85f643b20391a972d5f\",\"blockIndex\":2,\"address\":\"0x1234567890123456789012345678901234567890\",\"data\":[],\"topics\":[\"0x386cc2513b9e8b9e78a0792c33d6f69798774e0fa5424d3042fdd0fe7647420b\",\"0x260e065801cba6ca065f28640c3d94ef235f67db5431448aae1a51af7214efaf\",\"0xc30a3ae685bfcb917dceb41e4afed5342f332572d5b3a8212679077685c494cb\",\"0xff05acda0d6ef15409d713cb0f124d2c3a3fd95b33af096109172229d5c8671a\"],\"anonymous\":false}");
      REQUIRE(e1Json.dump() == "{\"address\":\"0x1234567890123456789012345678901234567890\",\"blockHash\":\"0x2b9b8644330d50ffb90c5fea02b73b562dfc550ec7f8c85f643b20391a972d5f\",\"blockNumber\":\"0x0000000000000002\",\"data\":\"0x\",\"logIndex\":\"0x0000000000000000\",\"removed\":false,\"topics\":[\"0x386cc2513b9e8b9e78a0792c33d6f69798774e0fa5424d3042fdd0fe7647420b\",\"0x260e065801cba6ca065f28640c3d94ef235f67db5431448aae1a51af7214efaf\",\"0xc30a3ae685bfcb917dceb41e4afed5342f332572d5b3a8212679077685c494cb\",\"0xff05acda0d6ef15409d713cb0f124d2c3a3fd95b33af096109172229d5c8671a\"],\"transactionHash\":\"0x53472c61f1db8612fcdd17f24b78986bfa111ea3e323522456b1a78560f2215a\",\"transactionIndex\":\"0x0000000000000001\"}");

      Event e2(e1Str);
      REQUIRE(e2.getName() == "myEvent");
      REQUIRE(e2.getLogIndex() == 0);
      REQUIRE(e2.getTxHash() == txHash);
      REQUIRE(e2.getTxIndex() == 1);
      REQUIRE(e2.getBlockHash() == blockHash);
      REQUIRE(e2.getBlockIndex() == 2);
      REQUIRE(e2.getAddress() == add);
      REQUIRE(e2.getData() == Bytes());
      REQUIRE(e2.getTopics().size() == 4);
      REQUIRE(e2.getTopics()[0].hex(true).get() == "0x386cc2513b9e8b9e78a0792c33d6f69798774e0fa5424d3042fdd0fe7647420b");
      REQUIRE(e2.getTopics()[1].hex(true).get() == "0x260e065801cba6ca065f28640c3d94ef235f67db5431448aae1a51af7214efaf");
      REQUIRE(e2.getTopics()[2].hex(true).get() == "0xc30a3ae685bfcb917dceb41e4afed5342f332572d5b3a8212679077685c494cb");
      REQUIRE(e2.getTopics()[3].hex(true).get() == "0xff05acda0d6ef15409d713cb0f124d2c3a3fd95b33af096109172229d5c8671a");
      REQUIRE(e2.isAnonymous() == false);
      REQUIRE(e2.getSelector() != Hash());
    }
  }
}

