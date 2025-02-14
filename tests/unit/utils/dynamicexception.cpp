/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#include "libs/catch2/catch_amalgamated.hpp"

#include "utils/dynamicexception.h"

namespace TDynamicException {
  class CustomObject {
    public:
      int value;
      CustomObject(int v) : value(v) {}
      friend std::ostream& operator<<(std::ostream& os, const CustomObject& obj) {
        return os << "CustomObject(value=" << obj.value << ")";
      }
  };

  TEST_CASE("DynamicException Class", "[unit][utils][dynamicexception]") {
    SECTION("Constructor with message only") {
      const std::string filename = "test.cpp";
      const int line = 42;
      const std::string function = "testFunction";
      const std::string message = "Test message";
      DynamicException exception("Function " + function + " failed: " + message + " at " + filename + ":", line);

      try {
        throw exception;
      } catch (const DynamicException& e) {
        REQUIRE(e.what() == "Function " + function + " failed: " + message + " at " + filename + ":" + std::to_string(line));
        REQUIRE(e.getFile() == "");
        REQUIRE(e.getLine() == 0);
        REQUIRE(e.getFunction() == "");
      }
    }

    SECTION("Constructor with message and stacktrace information") {
      const std::string filename = "test.cpp";
      const int line = 42;
      const std::string function = "testFunction";
      const std::string message = "Error in file " + filename + " at line " + std::to_string(line) + " in function " + function;
      DynamicException exception(message, filename, line, function);

      try {
        throw exception;
      } catch (const DynamicException& e) {
        REQUIRE(e.what() == message);
        REQUIRE(e.getFile() == filename);
        REQUIRE(e.getLine() == line);
        REQUIRE(e.getFunction() == function);
      }
    }

    SECTION("Check timestamp format is correct") {
      const std::string message = "Error with timestamp";
      DynamicException exception(message);

      // Timestamp format should be "XXXX-YY-ZZ AA:BB:CC"
      try {
        throw exception;
      } catch (const DynamicException& e) {
        std::string timestamp = e.getTimestamp();
        REQUIRE(!timestamp.empty());
        REQUIRE(timestamp.length() == 19);
        REQUIRE(timestamp[4] == '-');
        REQUIRE(timestamp[7] == '-');
        REQUIRE(timestamp[10] == ' ');
        REQUIRE(timestamp[13] == ':');
        REQUIRE(timestamp[16] == ':');
      }
    }

    SECTION("Exception with single message") {
      const std::string message = "Error with single string message";
      DynamicException exception(message);

      try {
        throw exception;
      } catch (const DynamicException& e) {
        REQUIRE(e.what() == message);
        REQUIRE(e.getFile() == "");
        REQUIRE(e.getLine() == 0);
        REQUIRE(e.getFunction() == "");
      }
    }

    SECTION("Exception with multiple messages") {
      int a = 5;
      int b = 10;
      const std::string message = "Error with multiple messages: " + std::to_string(a) + " and " + std::to_string(b);
      DynamicException exception("Error with multiple messages: ", a, " and ", b);

      try {
        throw exception;
      } catch (const DynamicException& e) {
        REQUIRE(e.what() == message);
        REQUIRE(e.getFile() == "");
        REQUIRE(e.getLine() == 0);
        REQUIRE(e.getFunction() == "");
      }
    }

    SECTION("Exception with various basic types") {
      int intValue = 42;
      double doubleValue = 3.14;
      const std::string message = "Error occurred with values: ";
      DynamicException exception(message, intValue, " and ", doubleValue);

      try {
        throw exception;
      } catch (const DynamicException& e) {
        REQUIRE_THAT(e.what(), Catch::Matchers::Equals("Error occurred with values: 42 and 3.14"));
      }
    }

    SECTION("Exception with strings and literals") {
      const std::string part1 = "Error: ";
      const char* part2 = "Invalid operation";
      DynamicException exception(part1, part2);

      try {
        throw exception;
      } catch (const DynamicException& e) {
        REQUIRE(e.what() == std::string("Error: Invalid operation"));
      }
    }

    SECTION("Exception with custom objects") {
      CustomObject obj(100);
      DynamicException exception("Encountered an issue with ", obj);

      try {
        throw exception;
      } catch (const DynamicException& e) {
        REQUIRE_THAT(e.what(), Catch::Matchers::Equals("Encountered an issue with CustomObject(value=100)"));
      }
    }
  }
}

