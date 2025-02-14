/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#include "libs/catch2/catch_amalgamated.hpp"

#include "contract/templates/simplecontract.h"

#include "../../sdktestsuite.hpp"

namespace TSimpleContract {
  TEST_CASE("SimpleContract class", "[integration][contract][simplecontract]") {
    SECTION("SimpleContract creation and dump") {
      Address simpleContract;
      std::unique_ptr<Options> options;
      std::tuple<std::string, uint256_t> tuple;
      {
        SDKTestSuite sdk = SDKTestSuite::createNewEnvironment("testSimpleContractCreation");
        simpleContract = sdk.deployContract<SimpleContract>(
          std::string("TestName"), uint256_t(19283187581),
          std::make_tuple(std::string("TupleName"), uint256_t(987654321))
        );
        REQUIRE(sdk.callViewFunction(simpleContract, &SimpleContract::getName) == "TestName");
        REQUIRE(sdk.callViewFunction(simpleContract, &SimpleContract::getNumber) == 19283187581);
        tuple = sdk.callViewFunction(simpleContract, &SimpleContract::getTuple);
        REQUIRE(std::get<0>(tuple) == "TupleName");
        REQUIRE(std::get<1>(tuple) == 987654321);
        // Dump to database
        options = std::make_unique<Options>(sdk.getOptions());
        sdk.saveSnapshot();
      }
      // SDKTestSuite should automatically load the state from the DB if we construct it with an Options object
      // (The createNewEnvironment DELETES the DB if any is found)
      SDKTestSuite sdk(*options);
      REQUIRE(sdk.callViewFunction(simpleContract, &SimpleContract::getName) == "TestName");
      REQUIRE(sdk.callViewFunction(simpleContract, &SimpleContract::getNumber) == 19283187581);
      tuple = sdk.callViewFunction(simpleContract, &SimpleContract::getTuple);
      REQUIRE(std::get<0>(tuple) == "TupleName");
      REQUIRE(std::get<1>(tuple) == 987654321);
    }

    SECTION("SimpleContract setName, setNumber and setTuple") {
      SDKTestSuite sdk = SDKTestSuite::createNewEnvironment("testSimpleContractSetNameNumberAndTuple");
      Address simpleContract = sdk.deployContract<SimpleContract>(
        std::string("TestName"), uint256_t(19283187581),
        std::make_tuple(std::string("TupleName"), uint256_t(987654321))
      );

      REQUIRE(sdk.callViewFunction(simpleContract, &SimpleContract::getName) == "TestName");
      REQUIRE(sdk.callViewFunction(simpleContract, &SimpleContract::getNumber) == 19283187581);
      REQUIRE(sdk.callViewFunction(simpleContract, static_cast<uint256_t(SimpleContract::*)(const uint256_t&) const>(&SimpleContract::getNumber), uint256_t(1)) == 19283187582);
      std::tuple<std::string, uint256_t> tuple = sdk.callViewFunction(simpleContract, &SimpleContract::getTuple);
      REQUIRE(std::get<0>(tuple) == "TupleName");
      REQUIRE(std::get<1>(tuple) == 987654321);

      Hash nameTx = sdk.callFunction(simpleContract, &SimpleContract::setName, std::string("TryThisName"));
      Hash numberTx = sdk.callFunction(simpleContract, &SimpleContract::setNumber, uint256_t("918258172319061203818967178162134821351"));
      Hash tupleTx = sdk.callFunction(simpleContract, &SimpleContract::setTuple, std::make_tuple(std::string("AnotherName"), uint256_t(999999999)));
      REQUIRE(sdk.callViewFunction(simpleContract, &SimpleContract::getName) == "TryThisName");
      REQUIRE(sdk.callViewFunction(simpleContract, &SimpleContract::getNumber) == uint256_t("918258172319061203818967178162134821351"));
      tuple = sdk.callViewFunction(simpleContract, &SimpleContract::getTuple);
      REQUIRE(std::get<0>(tuple) == "AnotherName");
      REQUIRE(std::get<1>(tuple) == 999999999);

      auto nameEvent = sdk.getEventsEmittedByTx(nameTx, &SimpleContract::nameChanged,
        std::make_tuple(EventParam<std::string, true>("TryThisName"))
      );
      auto numberEvent = sdk.getEventsEmittedByTx(numberTx, &SimpleContract::numberChanged,
        std::make_tuple(EventParam<uint256_t, false>(uint256_t("918258172319061203818967178162134821351")))
      );
      auto tupleEvent = sdk.getEventsEmittedByTx(tupleTx, &SimpleContract::tupleChanged,
        std::make_tuple(EventParam<std::tuple<std::string, uint256_t>, true>(std::make_tuple("AnotherName", uint256_t(999999999))))
      );
      REQUIRE(nameEvent.size() == 1);
      REQUIRE(numberEvent.size() == 1);
      REQUIRE(tupleEvent.size() == 1);
    }

    SECTION("SimpleContract setNames and setNumbers") {
      SDKTestSuite sdk = SDKTestSuite::createNewEnvironment("testSimpleContractSetNamesAndSetNumbers");
      Address simpleContract = sdk.deployContract<SimpleContract>(
        std::string("TestName"), uint256_t(19283187581),
        std::make_tuple(std::string("TupleName"), uint256_t(987654321))
      );

      REQUIRE(sdk.callViewFunction(simpleContract, &SimpleContract::getName) == "TestName");
      REQUIRE(sdk.callViewFunction(simpleContract, &SimpleContract::getNumber) == 19283187581);
      std::tuple<std::string, uint256_t> tuple = sdk.callViewFunction(simpleContract, &SimpleContract::getTuple);
      REQUIRE(std::get<0>(tuple) == "TupleName");
      REQUIRE(std::get<1>(tuple) == 987654321);

      Hash namesTx = sdk.callFunction(simpleContract, &SimpleContract::setNames, {std::string("Try"), std::string("This"), std::string("Name")});
      Hash numbersTx = sdk.callFunction(simpleContract, &SimpleContract::setNumbers, {uint256_t(111000000), uint256_t(222000), uint256_t(333)});
      std::vector<std::string> names = sdk.callViewFunction(simpleContract, &SimpleContract::getNames, uint256_t(3));
      std::vector<uint256_t> numbers = sdk.callViewFunction(simpleContract, &SimpleContract::getNumbers, uint256_t(3));
      REQUIRE(names.size() == 3);
      REQUIRE(numbers.size() == 3);
      for (int i = 0; i < 3; i++) {
        REQUIRE(names[i] == "TryThisName");
        REQUIRE(numbers[i] == uint256_t(111222333));
      }

      auto nameEvent = sdk.getEventsEmittedByTx(namesTx, &SimpleContract::nameChanged,
        std::make_tuple(EventParam<std::string, true>("TryThisName"))
      );
      auto numberEvent = sdk.getEventsEmittedByTx(numbersTx, &SimpleContract::numberChanged,
        std::make_tuple(EventParam<uint256_t, false>(uint256_t(111222333)))
      );
      REQUIRE(nameEvent.size() == 1);
      REQUIRE(numberEvent.size() == 1);
    }

    SECTION("SimpleContract setNamesAndNumbers") {
      SDKTestSuite sdk = SDKTestSuite::createNewEnvironment("testSimpleContractSetNamesAndNumbers");
      Address simpleContract = sdk.deployContract<SimpleContract>(
        std::string("TestName"), uint256_t(19283187581),
        std::make_tuple(std::string("TupleName"), uint256_t(987654321))
      );

      REQUIRE(sdk.callViewFunction(simpleContract, &SimpleContract::getName) == "TestName");
      REQUIRE(sdk.callViewFunction(simpleContract, &SimpleContract::getNumber) == 19283187581);
      std::tuple<std::string, uint256_t> tuple = sdk.callViewFunction(simpleContract, &SimpleContract::getTuple);
      REQUIRE(std::get<0>(tuple) == "TupleName");
      REQUIRE(std::get<1>(tuple) == 987654321);

      std::vector<std::string> names = {"111", "222", "333"};
      std::vector<uint256_t> numbers = {400, 300, 200, 100};
      Hash namesAndNumbersTx = sdk.callFunction(simpleContract, &SimpleContract::setNamesAndNumbers, names, numbers);
      std::tuple<std::string, uint256_t> nameAndNumber = sdk.callViewFunction(simpleContract, &SimpleContract::getNameAndNumber);
      std::tuple<std::vector<std::string>, std::vector<uint256_t>> namesAndNumbers = sdk.callViewFunction(simpleContract, &SimpleContract::getNamesAndNumbers, uint256_t(3));
      REQUIRE(std::get<0>(nameAndNumber) == "111222333");
      REQUIRE(std::get<1>(nameAndNumber) == 1000);
      REQUIRE(std::get<0>(namesAndNumbers).size() == 3);
      REQUIRE(std::get<1>(namesAndNumbers).size() == 3);
      for (int i = 0; i < 3; i++) {
        REQUIRE(std::get<0>(namesAndNumbers)[i] == "111222333");
        REQUIRE(std::get<1>(namesAndNumbers)[i] == 1000);
      }
    }

    SECTION("SimpleContract setNamesAndNumbersInTuple") {
      SDKTestSuite sdk = SDKTestSuite::createNewEnvironment("testSimpleContractSetNamesAndNumbersInTuple");
      Address simpleContract = sdk.deployContract<SimpleContract>(
        std::string("TestName"), uint256_t(19283187581),
        std::make_tuple(std::string("TupleName"), uint256_t(987654321))
      );

      REQUIRE(sdk.callViewFunction(simpleContract, &SimpleContract::getName) == "TestName");
      REQUIRE(sdk.callViewFunction(simpleContract, &SimpleContract::getNumber) == 19283187581);
      std::tuple<std::string, uint256_t> tuple = sdk.callViewFunction(simpleContract, &SimpleContract::getTuple);
      REQUIRE(std::get<0>(tuple) == "TupleName");
      REQUIRE(std::get<1>(tuple) == 987654321);

      std::vector<std::tuple<std::string, uint256_t>> list = {
        {"555", 1000}, {"444", 500}, {"333", 250}, {"222", 150}, {"111", 100}
      };
      Hash namesAndNumbersInTupleTx = sdk.callFunction(simpleContract, &SimpleContract::setNamesAndNumbersInTuple, list);
      std::vector<std::tuple<std::string, uint256_t>> namesAndNumbers = sdk.callViewFunction(simpleContract, &SimpleContract::getNamesAndNumbersInTuple, uint256_t(3));
      REQUIRE(namesAndNumbers.size() == 3);
      for (int i = 0; i < 3; i++) {
        REQUIRE(std::get<0>(namesAndNumbers[i]) == "555444333222111");
        REQUIRE(std::get<1>(namesAndNumbers[i]) == 2000);
      }
    }

    SECTION("SimpleContract setNamesAndNumbersInArrayOfArrays") {
      SDKTestSuite sdk = SDKTestSuite::createNewEnvironment("testSimpleContractSetNamesAndNumbersInArrayOfArrays");
      Address simpleContract = sdk.deployContract<SimpleContract>(
        std::string("TestName"), uint256_t(19283187581),
        std::make_tuple(std::string("TupleName"), uint256_t(987654321))
      );

      REQUIRE(sdk.callViewFunction(simpleContract, &SimpleContract::getName) == "TestName");
      REQUIRE(sdk.callViewFunction(simpleContract, &SimpleContract::getNumber) == 19283187581);
      std::tuple<std::string, uint256_t> tuple = sdk.callViewFunction(simpleContract, &SimpleContract::getTuple);
      REQUIRE(std::get<0>(tuple) == "TupleName");
      REQUIRE(std::get<1>(tuple) == 987654321);

      std::vector<std::vector<std::tuple<std::string, uint256_t>>> namesAndNumbers = {
        {std::make_tuple("a", uint256_t(1)), std::make_tuple("b", uint256_t(2)), std::make_tuple("c", uint256_t(3))},
        {std::make_tuple("d", uint256_t(4)), std::make_tuple("e", uint256_t(5)), std::make_tuple("f", uint256_t(6))}
      };
      Hash tx = sdk.callFunction(simpleContract, &SimpleContract::setNamesAndNumbersInArrayOfArrays, namesAndNumbers);
      std::vector<std::vector<std::tuple<std::string, uint256_t>>> arr = sdk.callViewFunction(simpleContract, &SimpleContract::getNamesAndNumbersInArrayOfArrays, uint256_t(3));
      REQUIRE(arr.size() == 3);
      for (int i = 0; i < 3; i++) {
        REQUIRE(arr[i].size() == 3);
        for (int j = 0; j < 3; j++) {
          REQUIRE(std::get<0>(arr[i][j]) == "abcdef");
          REQUIRE(std::get<1>(arr[i][j]) == 21);
        }
      }
    }

    SECTION("SimpleContract wrong caller (coverage)") {
      TestAccount acc = TestAccount::newRandomAccount();
      SDKTestSuite sdk = SDKTestSuite::createNewEnvironment("testSimpleContractWrongCaller", {acc});
      Address simpleContract = sdk.deployContract<SimpleContract>(
        std::string("TestName"), uint256_t(19283187581),
        std::make_tuple(std::string("TupleName"), uint256_t(987654321))
      );

      REQUIRE_THROWS(sdk.callFunction(simpleContract, acc, &SimpleContract::setName, std::string("aaa")));
      REQUIRE_THROWS(sdk.callFunction(simpleContract, acc, &SimpleContract::setNumber, uint256_t(123)));
      REQUIRE_THROWS(sdk.callFunction(simpleContract, acc, &SimpleContract::setTuple, std::make_tuple(std::string("bbb"), uint256_t(456))));
      REQUIRE_THROWS(sdk.callFunction(simpleContract, acc, &SimpleContract::setNames, {std::string("ccc"), std::string("ddd")}));
      REQUIRE_THROWS(sdk.callFunction(simpleContract, acc, &SimpleContract::setNumbers, {uint256_t(789), uint256_t(987)}));
      REQUIRE_THROWS(sdk.callFunction(simpleContract, acc, &SimpleContract::setNamesAndNumbers,
        {std::string("eee"), std::string("fff")}, {uint256_t(654), uint256_t(321)}
      ));
      REQUIRE_THROWS(sdk.callFunction(simpleContract, acc, &SimpleContract::setNamesAndNumbersInTuple, {
        std::make_tuple(std::string("gggg"), uint256_t(1234)), std::make_tuple(std::string("hhhh"), uint256_t(5678))
      }));
      REQUIRE_THROWS(sdk.callFunction(simpleContract, acc, &SimpleContract::setNamesAndNumbersInArrayOfArrays, {
        {std::make_tuple(std::string("hhhh"), uint256_t(9999)), std::make_tuple(std::string("iiii"), uint256_t(1111))},
        {std::make_tuple(std::string("jjjj"), uint256_t(1000)), std::make_tuple(std::string("kkkk"), uint256_t(9000))}
      }));
    }
  }
}
