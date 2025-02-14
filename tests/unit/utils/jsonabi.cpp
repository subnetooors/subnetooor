/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#include "libs/catch2/catch_amalgamated.hpp"

#include "utils/jsonabi.h"

namespace TJsonAbi {
  TEST_CASE("JsonAbi Namespace", "[unit][utils][jsonabi]") {
    SECTION("isArray") {
      std::string type1 = "int[]";
      std::string type2 = "int";
      REQUIRE(JsonAbi::isArray(type1));
      REQUIRE_FALSE(JsonAbi::isArray(type2));
      // For coverage
      REQUIRE_FALSE(JsonAbi::isArray("int()")); // "()" != "[]"
      REQUIRE_FALSE(JsonAbi::isArray("int[)")); // "[)" != "[]"
    }

    SECTION("isTuple") {
      std::string type1 = "(int, double, float)";
      std::string type2 = "(int, double)[]";
      std::string type3 = "int";
      REQUIRE(JsonAbi::isTuple(type1));
      REQUIRE(JsonAbi::isTuple(type2));
      REQUIRE_FALSE(JsonAbi::isTuple(type3));
      // For coverage
      REQUIRE_FALSE(JsonAbi::isTuple("{int, double, float}"));  // "{}" != "()"
      REQUIRE_FALSE(JsonAbi::isTuple("{int, double}[]"));
      REQUIRE_FALSE(JsonAbi::isTuple("{int, double, float)")); // "{)" != "()"
      REQUIRE_FALSE(JsonAbi::isTuple("{int, double)[]"));
    }

    SECTION("countTupleArrays") {
      std::string type0 = "(int, double)";
      std::string type1 = "(int, double)[]";
      std::string type2 = "(int, double)[][]";
      std::string type3 = "(int, double)[][][]";
      REQUIRE(JsonAbi::countTupleArrays(type0) == 0);
      REQUIRE(JsonAbi::countTupleArrays(type1) == 1);
      REQUIRE(JsonAbi::countTupleArrays(type2) == 2);
      REQUIRE(JsonAbi::countTupleArrays(type3) == 3);
      // For coverage
      REQUIRE(JsonAbi::countTupleArrays("(int, double){}") == 0); // "{}" != "[]"
      REQUIRE(JsonAbi::countTupleArrays("(int, double){]") == 0); // "{]" != "[]"
    }

    SECTION("getTupleTypes") {
      std::string type1 = "(int, double)";
      std::string type2 = "(bytes, string, address)[]";
      std::vector<std::string> get1 = JsonAbi::getTupleTypes(type1);
      std::vector<std::string> get2 = JsonAbi::getTupleTypes(type2);
      REQUIRE(get1.size() == 2);
      REQUIRE(get2.size() == 3);
      REQUIRE(get1[0] == "int");
      REQUIRE(get1[1] == "double");
      REQUIRE(get2[0] == "bytes");
      REQUIRE(get2[1] == "string");
      REQUIRE(get2[2] == "address");
    }

    SECTION("handleTupleComponents") {
      std::vector<std::string> comps = {"int", "(int, double)", "(bytes, string, address)[]"};
      json compsJson = JsonAbi::handleTupleComponents(comps);
      json obj1 = compsJson[0];
      json obj2 = compsJson[1];
      json obj3 = compsJson[2];
      REQUIRE(obj1["internalType"] == "int");
      REQUIRE(obj1["type"] == "int");
      REQUIRE(obj2["components"].is_array());
      REQUIRE(obj2["type"] == "tuple");
      REQUIRE(obj2["components"][0]["internalType"] == "int");
      REQUIRE(obj2["components"][0]["type"] == "int");
      REQUIRE(obj2["components"][1]["internalType"] == "double");
      REQUIRE(obj2["components"][1]["type"] == "double");
      REQUIRE(obj3["components"].is_array());
      REQUIRE(obj3["type"] == "tuple[]");
      REQUIRE(obj3["components"][0]["internalType"] == "bytes");
      REQUIRE(obj3["components"][0]["type"] == "bytes");
      REQUIRE(obj3["components"][1]["internalType"] == "string");
      REQUIRE(obj3["components"][1]["type"] == "string");
      REQUIRE(obj3["components"][2]["internalType"] == "address");
      REQUIRE(obj3["components"][2]["type"] == "address");
    }

    SECTION("parseMethodInput") {
      std::vector<std::pair<std::string, std::string>> inputs = {
        {"int", "var1"}, {"(int, double)", "var2"}, {"(bytes, string, address)[]", "var3"}
      };
      json inputsJson = JsonAbi::parseMethodInput(inputs);
      json input1 = inputsJson[0];
      json input2 = inputsJson[1];
      json input3 = inputsJson[2];
      REQUIRE(input1["internalType"] == "int");
      REQUIRE(input1["name"] == "var1");
      REQUIRE(input1["type"] == "int");
      REQUIRE(input2["components"].is_array());
      REQUIRE(input2["name"] == "var2");
      REQUIRE(input2["type"] == "tuple");
      REQUIRE(input2["components"][0]["internalType"] == "int");
      REQUIRE(input2["components"][0]["type"] == "int");
      REQUIRE(input2["components"][1]["internalType"] == "double");
      REQUIRE(input2["components"][1]["type"] == "double");
      REQUIRE(input3["components"].is_array());
      REQUIRE(input3["name"] == "var3");
      REQUIRE(input3["type"] == "tuple[]");
      REQUIRE(input3["components"][0]["internalType"] == "bytes");
      REQUIRE(input3["components"][0]["type"] == "bytes");
      REQUIRE(input3["components"][1]["internalType"] == "string");
      REQUIRE(input3["components"][1]["type"] == "string");
      REQUIRE(input3["components"][2]["internalType"] == "address");
      REQUIRE(input3["components"][2]["type"] == "address");
    }

    SECTION("parseMethodOutput") {
      std::vector<std::string> outputs = {"int", "(int, double)", "(bytes, string, address)[]"};
      json outputsJson = JsonAbi::parseMethodOutput(outputs);
      json output1 = outputsJson[0];
      json output2 = outputsJson[1];
      json output3 = outputsJson[2];
      REQUIRE(output1["internalType"] == "int");
      REQUIRE(output1["type"] == "int");
      REQUIRE(output2["components"].is_array());
      REQUIRE(output2["type"] == "tuple");
      REQUIRE(output2["components"][0]["internalType"] == "int");
      REQUIRE(output2["components"][0]["type"] == "int");
      REQUIRE(output2["components"][1]["internalType"] == "double");
      REQUIRE(output2["components"][1]["type"] == "double");
      REQUIRE(output3["components"].is_array());
      REQUIRE(output3["type"] == "tuple[]");
      REQUIRE(output3["components"][0]["internalType"] == "bytes");
      REQUIRE(output3["components"][0]["type"] == "bytes");
      REQUIRE(output3["components"][1]["internalType"] == "string");
      REQUIRE(output3["components"][1]["type"] == "string");
      REQUIRE(output3["components"][2]["internalType"] == "address");
      REQUIRE(output3["components"][2]["type"] == "address");
    }

    SECTION("parseEventArgs") {
      std::vector<std::tuple<std::string, std::string, bool>> events = {
        {"int", "arg1", true}, {"(int, double)", "arg2", false}, {"(bytes, string, address)[]", "arg3", true}
      };
      json eventsJson = JsonAbi::parseEventArgs(events);
      json event1 = eventsJson[0];
      json event2 = eventsJson[1];
      json event3 = eventsJson[2];
      REQUIRE(event1["indexed"] == true);
      REQUIRE(event1["internalType"] == "int");
      REQUIRE(event1["name"] == "arg1");
      REQUIRE(event1["type"] == "int");
      REQUIRE(event2["components"].is_array());
      REQUIRE(event2["indexed"] == false);
      REQUIRE(event2["name"] == "arg2");
      REQUIRE(event2["type"] == "tuple");
      REQUIRE(event2["components"][0]["internalType"] == "int");
      REQUIRE(event2["components"][0]["type"] == "int");
      REQUIRE(event2["components"][1]["internalType"] == "double");
      REQUIRE(event2["components"][1]["type"] == "double");
      REQUIRE(event3["components"].is_array());
      REQUIRE(event3["indexed"] == true);
      REQUIRE(event3["name"] == "arg3");
      REQUIRE(event3["type"] == "tuple[]");
      REQUIRE(event3["components"][0]["internalType"] == "bytes");
      REQUIRE(event3["components"][0]["type"] == "bytes");
      REQUIRE(event3["components"][1]["internalType"] == "string");
      REQUIRE(event3["components"][1]["type"] == "string");
      REQUIRE(event3["components"][2]["internalType"] == "address");
      REQUIRE(event3["components"][2]["type"] == "address");
    }

    SECTION("methodToJSON") {
      ABI::MethodDescription desc;
      desc.name = "transfer";
      desc.inputs = {{"uint256", "amount"}, {"address", "to"}};
      desc.outputs = {"bool"};
      desc.stateMutability = FunctionTypes::NonPayable;
      desc.type = "function";
      json descObj = JsonAbi::methodToJSON(desc);
      REQUIRE(descObj["name"] == "transfer");
      REQUIRE(descObj["inputs"][0]["internalType"] == "uint256");
      REQUIRE(descObj["inputs"][0]["type"] == "uint256");
      REQUIRE(descObj["inputs"][1]["internalType"] == "address");
      REQUIRE(descObj["inputs"][1]["type"] == "address");
      REQUIRE(descObj["outputs"][0]["internalType"] == "bool");
      REQUIRE(descObj["outputs"][0]["type"] == "bool");
      REQUIRE(descObj["stateMutability"] == "nonpayable");
      REQUIRE(descObj["type"] == "function");
    }

    SECTION("eventToJSON") {
      ABI::EventDescription desc;
      desc.name = "transferred";
      desc.args = {{"uint256", "amount", true}, {"address", "to", false}};
      desc.anonymous = false;
      json descObj = JsonAbi::eventToJSON(desc);
      REQUIRE(descObj["anonymous"] == false);
      REQUIRE(descObj["inputs"][0]["indexed"] == true);
      REQUIRE(descObj["inputs"][0]["internalType"] == "uint256");
      REQUIRE(descObj["inputs"][0]["name"] == "amount");
      REQUIRE(descObj["inputs"][0]["type"] == "uint256");
      REQUIRE(descObj["inputs"][1]["indexed"] == false);
      REQUIRE(descObj["inputs"][1]["internalType"] == "address");
      REQUIRE(descObj["inputs"][1]["name"] == "to");
      REQUIRE(descObj["inputs"][1]["type"] == "address");
      REQUIRE(descObj["name"] == "transferred");
      REQUIRE(descObj["type"] == "event");
    }
  }
}

