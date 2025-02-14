/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#include "libs/catch2/catch_amalgamated.hpp"

#include "contract/variables/safestring.h"

using Catch::Matchers::Equals;

namespace TSafeString {
  TEST_CASE("SafeString class", "[unit][safevars][safestring]") {
    SECTION("Constructor") {
      SafeString emptyStr;
      SafeString str("Hello World");
      std::string strRaw = "Hello Copy";
      SafeString copyStr(strRaw);
      SafeString copyStr2(copyStr);
      REQUIRE(emptyStr.empty());
      REQUIRE(emptyStr.length() == 0);
      REQUIRE(!str.empty());
      REQUIRE(str.size() == 11);
      REQUIRE(!copyStr.empty());
      REQUIRE(copyStr.length() == 10);
      REQUIRE(copyStr == strRaw);
      REQUIRE(!copyStr2.empty());
      REQUIRE(copyStr2.size() == 10);
      REQUIRE(copyStr2 == copyStr);
      REQUIRE(str.get() == "Hello World");
      REQUIRE(str.data()[0] == 'H');
      REQUIRE(str.c_str()[10] == 'd');
    }

    SECTION("assign") {
      SafeString str("000");
      // assign std::string copy
      str.assign(std::string("111"));
      str.revert();
      REQUIRE(str == "000");
      str.assign(std::string("111"));
      str.commit();
      REQUIRE(str == "111");
      // assign std::string move
      std::string mov1 = "222";
      std::string mov2 = "222";
      std::string movA = "222";
      str.assign(std::move(mov1));
      str.assign(std::move(movA)); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "111");
      str.assign(std::move(mov2));
      str.commit();
      REQUIRE(str == "222");
      // assign SafeString copy
      SafeString cpy("333");
      str.assign(cpy);
      str.assign(cpy); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "222");
      str.assign(cpy);
      str.commit();
      REQUIRE(str == "333");
      // assign std::string substring (str, pos, count)
      std::string sub = "aa444aa";
      str.assign(sub, 2, 3);
      str.assign(sub, 2, 3); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "333");
      str.assign(sub, 2, 3);
      str.commit();
      REQUIRE(str == "444");
      // assign SafeString substring (str, pos, count)
      SafeString sub2("bbbbb555bbbbb");
      str.assign(sub2, 5, 3);
      str.assign(sub2, 5, 3); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "444");
      str.assign(sub2, 5, 3);
      str.commit();
      REQUIRE(str == "555");
      // assign number of chars (count, ch)
      str.assign(3, '6');
      str.assign(3, '9'); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "555");
      str.assign(3, '6');
      str.commit();
      REQUIRE(str == "666");
      // assign non-NULL-terminated C-style string (char*, count)
      char c[3] = {'7', '7', '7'};
      str.assign(c, 3);
      str.assign(c, 3); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "666");
      str.assign(c, 3);
      str.commit();
      REQUIRE(str == "777");
      // assign NULL-terminated C-style string (char*)
      char c2[4] = {'8', '8', '8', '\0'};
      str.assign(c2);
      str.assign(c2); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "777");
      str.assign(c2);
      str.commit();
      REQUIRE(str == "888");
      // assign iterators
      std::string iter("cccccccccc999cccccccccc");
      str.assign(iter.cbegin() + 10, iter.cend() - 10);
      str.assign(iter.cbegin() + 10, iter.cend() - 10); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "888");
      str.assign(iter.cbegin() + 10, iter.cend() - 10);
      str.commit();
      REQUIRE(str == "999");
      // assign ilist
      std::initializer_list<char> ilist {'!','!','!'};
      str.assign(ilist);
      str.assign(ilist); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "999");
      str.assign(ilist);
      str.commit();
      REQUIRE(str == "!!!");
    }

    SECTION("at, front and back") {
      SafeString str("Hello");
      // const at
      REQUIRE(std::as_const(str).at(0) == 'H');
      REQUIRE(std::as_const(str).at(1) == 'e');
      REQUIRE(std::as_const(str).at(2) == 'l');
      REQUIRE(std::as_const(str).at(3) == 'l');
      REQUIRE(std::as_const(str).at(4) == 'o');
      REQUIRE_THROWS(std::as_const(str).at(6));
      // non-const at
      str.at(0) = 'W';
      str.at(1) = 'o';
      str.at(2) = 'r';
      str.at(3) = 'l';
      str.at(4) = 'd';
      str.revert();
      REQUIRE(std::as_const(str).at(0) == 'H');
      REQUIRE(std::as_const(str).at(1) == 'e');
      REQUIRE(std::as_const(str).at(2) == 'l');
      REQUIRE(std::as_const(str).at(3) == 'l');
      REQUIRE(std::as_const(str).at(4) == 'o');
      str.at(0) = 'W';
      str.at(1) = 'o';
      str.at(2) = 'r';
      str.at(3) = 'l';
      str.at(4) = 'd';
      str.commit();
      REQUIRE(std::as_const(str).at(0) == 'W');
      REQUIRE(std::as_const(str).at(1) == 'o');
      REQUIRE(std::as_const(str).at(2) == 'r');
      REQUIRE(std::as_const(str).at(3) == 'l');
      REQUIRE(std::as_const(str).at(4) == 'd');
      // front and back
      str.front() = 'H';
      str.revert();
      REQUIRE(std::as_const(str).front() == 'W');
      str.front() = 'H';
      str.commit();
      REQUIRE(std::as_const(str).front() == 'H');
      str.back() = '!';
      str.revert();
      REQUIRE(std::as_const(str).back() == 'd');
      str.back() = '!';
      str.commit();
      REQUIRE(std::as_const(str).back() == '!');
      // For coverage (copy != nullptr)
      REQUIRE(str.at(0) == 'H');
      REQUIRE(str.front() == 'H');
      REQUIRE(str.back() == '!');
    }

    SECTION("begin, end, rbegin, rend") {
      SafeString str("Hello World");
      // alter from begin to end
      for (auto it = str.begin(); it != str.end(); it++) *it = std::toupper(*it);
      str.revert();
      REQUIRE(str == "Hello World");
      for (auto it = str.begin(); it != str.end(); it++) *it = std::toupper(*it);
      str.commit();
      REQUIRE(str == "HELLO WORLD");
      str = "Hello World"; str.commit(); // always reset for next test
      // alter from end to begin (on purpose, end also copies the string)
      for (auto it = str.end() - 1; it != str.begin() - 1; it--) *it = std::toupper(*it);
      str.revert();
      REQUIRE(str == "Hello World");
      for (auto it = str.end() - 1; it != str.begin() - 1; it--) *it = std::toupper(*it);
      str.commit();
      REQUIRE(str == "HELLO WORLD");
      str = "Hello World"; str.commit();
      // alter from rbegin to rend
      for (auto it = str.rbegin(); it != str.rend(); it++) *it = std::toupper(*it);
      str.revert();
      REQUIRE(str == "Hello World");
      for (auto it = str.rbegin(); it != str.rend(); it++) *it = std::toupper(*it);
      str.commit();
      REQUIRE(str == "HELLO WORLD");
      str = "Hello World"; str.commit();
      // alter from rend to rbegin (on purpose, rend also copies the string)
      for (auto it = str.rend() - 1; it != str.rbegin() - 1; it--) *it = std::toupper(*it);
      str.revert();
      REQUIRE(str == "Hello World");
      for (auto it = str.rend() - 1; it != str.rbegin() - 1; it--) *it = std::toupper(*it);
      str.commit();
      REQUIRE(str == "HELLO WORLD");
    }

    SECTION("reserve, capacity and shrink_to_fit") {
      SafeString str("Hello World");
      std::size_t oriCap = str.capacity();
      // reserve
      str.reserve(100);
      str.reserve(200); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str.capacity() <= oriCap);
      str.reserve(100);
      str.commit();
      REQUIRE(str.capacity() > oriCap);
      REQUIRE(str.capacity() <= 100);
      // shrink_to_fit
      str.shrink_to_fit();
      str.shrink_to_fit(); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str.capacity() > oriCap);
      REQUIRE(str.capacity() <= 100);
      str.shrink_to_fit();
      str.commit();
      REQUIRE(str.capacity() <= oriCap);
    }

    SECTION("clear") {
      SafeString str("Hello World");
      str.clear();
      str.clear(); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE((!str.empty() && str == "Hello World"));
      str.clear();
      str.commit();
      REQUIRE((str.empty() && str != "Hello World"));
    }

    SECTION("insert") {
      SafeString str("Hello");
      // insert repeat chars (count, ch)
      str.insert(0, 5, 'a');
      str.insert(0, 5, 'b'); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "Hello");
      str.insert(0, 5, 'a');
      str.commit();
      REQUIRE(str == "aaaaaHello");
      str = "Hello"; str.commit(); // always reset for next test
      // insert NULL-terminated C-style string (char*)
      char c[4] = {'b', 'b', 'b', '\0'};
      str.insert(0, c);
      str.insert(0, c); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "Hello");
      str.insert(0, c);
      str.commit();
      REQUIRE(str == "bbbHello");
      str = "Hello"; str.commit();
      // insert non-NULL-terminated C-style string (char*, count)
      char c2[3] = {'c', 'c', 'c'};
      str.insert(0, c2, 3);
      str.insert(0, c2, 3); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "Hello");
      str.insert(0, c2, 3);
      str.commit();
      REQUIRE(str == "cccHello");
      str = "Hello"; str.commit();
      // insert SafeString
      SafeString str2("World");
      str.insert(0, str2);
      str.insert(0, str2); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "Hello");
      str.insert(0, str2);
      str.commit();
      REQUIRE(str == "WorldHello");
      str = "Hello"; str.commit();
      // insert std::string
      std::string str3("World");
      str.insert(0, str3);
      str.insert(0, str3); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "Hello");
      str.insert(0, str3);
      str.commit();
      REQUIRE(str == "WorldHello");
      str = "Hello"; str.commit();
      // insert SafeString substring (str, idx, count)
      SafeString str4("dddddWorldddddd");
      str.insert(0, str4, 5, 5);
      str.insert(0, str4, 5, 5); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "Hello");
      str.insert(0, str4, 5, 5);
      str.commit();
      REQUIRE(str == "WorldHello");
      str = "Hello"; str.commit();
      // insert std::string substring (str, idx, count)
      std::string str5("eeeeeWorldeeeee");
      str.insert(0, str5, 5, 5);
      str.insert(0, str5, 5, 5); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "Hello");
      str.insert(0, str5, 5, 5);
      str.commit();
      REQUIRE(str == "WorldHello");
      str = "Hello"; str.commit();
      // insert char with iterator (pos, ch)
      str.insert(str.cend(), '!');
      str.insert(str.cend(), '#'); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "Hello");
      str.insert(str.cend(), '!');
      str.commit();
      REQUIRE(str == "Hello!");
      str = "Hello"; str.commit();
      // insert repeat chars with iterator (pos, count, ch)
      str.insert(str.cend(), 3, '!');
      str.insert(str.cend(), 3, '#'); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "Hello");
      str.insert(str.cend(), 3, '!');
      str.commit();
      REQUIRE(str == "Hello!!!");
      str = "Hello"; str.commit();
      // insert with iterators
      std::string iter("ffffffffffWorldffffffffff");
      str.insert(str.cend(), iter.cbegin() + 10, iter.cend() - 10);
      str.insert(str.cend(), iter.cbegin() + 10, iter.cend() - 10); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "Hello");
      str.insert(str.cend(), iter.cbegin() + 10, iter.cend() - 10);
      str.commit();
      REQUIRE(str == "HelloWorld");
      str = "Hello"; str.commit();
      // insert ilist
      std::initializer_list<char> ilist { 'D', 'a', 'r', 'k', 'n', 'e', 's', 's' };
      str.insert(str.cend(), ilist);
      str.insert(str.cend(), ilist); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "Hello");
      str.insert(str.cend(), ilist);
      str.commit();
      REQUIRE(str == "HelloDarkness"); // it's an old friend of mine :)
    }

    SECTION("erase") {
      SafeString str("Hello World");
      // erase a number of chars
      str.erase(2, 6); // "llo Wo"
      str.erase(0, 1); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "Hello World");
      str.erase(2, 6);
      str.commit();
      REQUIRE(str == "Herld");
      str = "Hello World"; str.commit(); // always reset str for next test
      // erase one char
      str.erase(str.cbegin() + 4); // "o"
      str.erase(str.cbegin() + 1); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "Hello World");
      str.erase(str.cbegin() + 4);
      str.commit();
      REQUIRE(str == "Hell World");
      str = "Hello World"; str.commit();
      // erase a range of chars
      str.erase(str.cbegin() + 5, str.cend()); // " World"
      str.erase(str.cbegin(), str.cend()); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "Hello World");
      str.erase(str.cbegin() + 5, str.cend()); // " World"
      str.commit();
      REQUIRE(str == "Hello");
    }

    SECTION("push_back and pop_back") {
      SafeString str("Goodbye");
      str.push_back('!');
      str.revert();
      REQUIRE(str == "Goodbye");
      str.push_back('!');
      str.commit();
      REQUIRE(str == "Goodbye!");
      str.pop_back();
      str.revert();
      REQUIRE(str == "Goodbye!");
      str.pop_back();
      str.commit();
      REQUIRE(str == "Goodbye");
      // For coverage (copy != nullptr)
      str.push_back('!');
      str.pop_back();
      str.push_back('!');
      REQUIRE(str == "Goodbye!");
    }

    SECTION("append") {
      SafeString str("Howdy");
      // append number of chars
      str.append(3, '.');
      str.append(3, '.'); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "Howdy");
      str.append(3, '.');
      str.commit();
      REQUIRE(str == "Howdy...");
      str = "Howdy"; str.commit(); // always reset str for next test
      // append SafeString
      SafeString str2("Pardner");
      str.append(str2);
      str.append(str2); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "Howdy");
      str.append(str2);
      str.commit();
      REQUIRE(str == "HowdyPardner");
      str = "Howdy"; str.commit();
      // append std::string
      std::string str3("Miss");
      str.append(str3);
      str.append(str3); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "Howdy");
      str.append(str3);
      str.commit();
      REQUIRE(str == "HowdyMiss");
      str = "Howdy"; str.commit();
      // append Safestring substring
      SafeString str4("Dat's Mah Horse");
      str.append(str4, 10, 2); // "Ho"
      str.append(str4, 10, 2); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "Howdy");
      str.append(str4, 10, 2);
      str.commit();
      REQUIRE(str == "HowdyHo"); // Mr. Hankey!
      str = "Howdy"; str.commit();
      // append std::string substring
      std::string str5("It's a Champion Breed");
      str.append(str5, 7, 5); // "Champ"
      str.append(str5, 7, 5); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "Howdy");
      str.append(str5, 7, 5);
      str.commit();
      REQUIRE(str == "HowdyChamp");
      str = "Howdy"; str.commit();
      // append non-NULL-terminated C-style string
      char c[6] = {'F','a','m','i','l','y'};
      str.append(c, 3);
      str.append(c, 3); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "Howdy");
      str.append(c, 3);
      str.commit();
      REQUIRE(str == "HowdyFam"); // got ya
      str = "Howdy"; str.commit();
      // append NULL-terminated C-style string
      char c2[4] = {'B','r','o','\0'};
      str.append(c2);
      str.append(c2); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "Howdy");
      str.append(c2);
      str.commit();
      REQUIRE(str == "HowdyBro");
      str = "Howdy"; str.commit();
      // append range of chars (iterator)
      std::string iter("The Pizza Planet Oneiric Experience"); // I'm hungry and sleepy and running out of ideas
      str.append(iter.cbegin() + 10, iter.cbegin() + 16); // "Planet"
      str.append(iter.cbegin() + 10, iter.cbegin() + 16); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "Howdy");
      str.append(iter.cbegin() + 10, iter.cbegin() + 16);
      str.commit();
      REQUIRE(str == "HowdyPlanet");
      str = "Howdy"; str.commit();
      // append ilist
      std::initializer_list<char> ilist {'S','e','e','y','a'};
      str.append(ilist);
      str.append(ilist); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "Howdy");
      str.append(ilist);
      str.commit();
      REQUIRE(str == "HowdySeeya");
    }

    SECTION("compare") {
      SafeString str("Bonjour");
      // compare std::string
      std::string str1L("BonjourMessier");
      std::string str1E("Bonjour");
      std::string str1G("Bonj");
      REQUIRE(str.compare(str1L) < 0);
      REQUIRE(str.compare(str1E) == 0);
      REQUIRE(str.compare(str1G) > 0);
      // compare SafeString
      SafeString str2L("Bonjourrr");
      SafeString str2E("Bonjour");
      SafeString str2G("Bonjou");
      REQUIRE(str.compare(str2L) < 0);
      REQUIRE(str.compare(str2E) == 0);
      REQUIRE(str.compare(str2G) > 0);
      // compare std::string substring
      REQUIRE(str.compare(0, 7, str1L) < 0); // cutting corners here cuz' I need some sleep
      REQUIRE(str.compare(0, 7, str1E) == 0);
      REQUIRE(str.compare(0, 7, str1G) > 0);
      // compare SafeString substring
      REQUIRE(str.compare(0, 7, str2L) < 0);
      REQUIRE(str.compare(0, 7, str2E) == 0);
      REQUIRE(str.compare(0, 7, str2G) > 0);
      // compare substring with std::string substring
      REQUIRE(str.compare(0, 3, str1E, 0, 5) < 0);
      REQUIRE(str.compare(0, 5, str1E, 0, 5) == 0);
      REQUIRE(str.compare(0, 7, str1E, 0, 5) > 0);
      // compare substring with SafeString substring
      REQUIRE(str.compare(0, 3, str2E, 0, 5) < 0);
      REQUIRE(str.compare(0, 5, str2E, 0, 5) == 0);
      REQUIRE(str.compare(0, 7, str2E, 0, 5) > 0);
      // compare C-style string
      const char* cstrL = "BonjourMademoseille";
      const char* cstrE = "Bonjour";
      const char* cstrG = "B";
      REQUIRE(str.compare(cstrL) < 0);
      REQUIRE(str.compare(cstrE) == 0);
      REQUIRE(str.compare(cstrG) > 0);
      // compare C-style substring
      REQUIRE(str.compare(0, 7, cstrL) < 0);
      REQUIRE(str.compare(0, 7, cstrE) == 0);
      REQUIRE(str.compare(0, 7, cstrG) > 0);
      // compare substring with C-style substring
      REQUIRE(str.compare(0, 3, cstrE, 0, 5) < 0);
      REQUIRE(str.compare(0, 5, cstrE, 0, 5) == 0);
      REQUIRE(str.compare(0, 7, cstrE, 0, 5) > 0);
    }

    SECTION("starts_with, ends_with and contains") {
      SafeString str("Hola que tal");
      // starts_with
      const char* cA1 = "Ho";
      const char* cA2 = "Ro";
      REQUIRE(str.starts_with(std::string_view("Hola")));
      REQUIRE(!str.starts_with(std::string_view("Rola")));
      REQUIRE(str.starts_with('H'));
      REQUIRE(!str.starts_with('R'));
      REQUIRE(str.starts_with(cA1));
      REQUIRE(!str.starts_with(cA2));
      // ends_with
      const char* cB1 = "al";
      const char* cB2 = "el";
      REQUIRE(str.ends_with(std::string_view("tal")));
      REQUIRE(!str.ends_with(std::string_view("tell")));
      REQUIRE(str.ends_with('l'));
      REQUIRE(!str.ends_with('t'));
      REQUIRE(str.ends_with(cB1));
      REQUIRE(!str.ends_with(cB2));
      // contains
      const char* cC1 = "que";
      const char* cC2 = "quo";
      REQUIRE(str.contains(std::string_view("la")));
      REQUIRE(!str.contains(std::string_view("lu")));
      REQUIRE(str.contains('a'));
      REQUIRE(!str.contains('i'));
      REQUIRE(str.contains(cC1));
      REQUIRE(!str.contains(cC2));
    }

    SECTION("replace") {
      SafeString str("Alo Brasil");
      // replace SafeString (pos + count)
      SafeString str1("ost");
      str.replace(5, 3, str1);
      str.replace(5, 3, str1); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "Alo Brasil");
      str.replace(5, 3, str1);
      str.commit();
      REQUIRE(str == "Alo Bostil");
      str = "Alo Brasil"; str.commit(); // always reset str for next test
      // replace std::string (pos + count)
      std::string str2("urr");
      str.replace(5, 3, str2);
      str.replace(5, 3, str2); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "Alo Brasil");
      str.replace(5, 3, str2);
      str.commit();
      REQUIRE(str == "Alo Burril");
      str = "Alo Brasil"; str.commit();
      // replace SafeString (iterators)
      SafeString str3("anan");
      str.replace(str.cbegin() + 5, str.cend() - 2, str3);
      str.replace(str.cbegin() + 5, str.cend() - 2, str3); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "Alo Brasil");
      str.replace(str.cbegin() + 5, str.cend() - 2, str3);
      str.commit();
      REQUIRE(str == "Alo Bananil");
      str = "Alo Brasil"; str.commit();
      // replace std::string (iterators)
      std::string str4("eston");
      str.replace(str.cbegin() + 5, str.cend() - 2, str4);
      str.replace(str.cbegin() + 5, str.cend() - 2, str4); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "Alo Brasil");
      str.replace(str.cbegin() + 5, str.cend() - 2, str4);
      str.commit();
      REQUIRE(str == "Alo Bestonil");
      str = "Alo Brasil"; str.commit();
      // replace SafeString substring (pos + count)
      SafeString str5("fundo do poço");
      str.replace(5, 3, str5, 1, 3);
      str.replace(5, 3, str5, 1, 3); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "Alo Brasil");
      str.replace(5, 3, str5, 1, 3);
      str.commit();
      REQUIRE(str == "Alo Bundil");
      str = "Alo Brasil"; str.commit();
      // replace std::string substring (pos + count)
      std::string str6("viva o aldeão da taverna");
      str.replace(5, 3, str6, 7, 3);
      str.replace(5, 3, str6, 7, 3); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "Alo Brasil");
      str.replace(5, 3, str6, 7, 3);
      str.commit();
      REQUIRE(str == "Alo Baldil");
      str = "Alo Brasil"; str.commit();
      // replace std::string substring (iterators)
      std::string str7("todo mundo sabe que latrocinio significa roubo seguido de morte");
      str.replace(str.cbegin() + 4, str.cend() - 2, str7.cbegin() + 20, str7.cend() - 39);
      str.replace(str.cbegin() + 4, str.cend() - 2, str7.cbegin() + 20, str7.cend() - 39); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "Alo Brasil");
      str.replace(str.cbegin() + 4, str.cend() - 2, str7.cbegin() + 20, str7.cend() - 39);
      str.commit();
      REQUIRE(str == "Alo latril");
      str = "Alo Brasil"; str.commit();
      // replace C-style substring (pos + count)
      const char* c = "inutil, a gente somos inutil";
      str.replace(4, 4, c, 4);
      str.replace(4, 4, c, 4); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "Alo Brasil");
      str.replace(4, 4, c, 4);
      str.commit();
      REQUIRE(str == "Alo inutil");
      str = "Alo Brasil"; str.commit();
      // replace C-style substring (iterators)
      const char* c2 = "establishment";
      str.replace(str.cbegin() + 5, str.cend() - 2, c2, 3);
      str.replace(str.cbegin() + 5, str.cend() - 2, c2, 3); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "Alo Brasil");
      str.replace(str.cbegin() + 5, str.cend() - 2, c2, 3);
      str.commit();
      REQUIRE(str == "Alo Bestil");
      str = "Alo Brasil"; str.commit();
      // replace C-style string (pos)
      const char* c3 = "Huelandia";
      str.replace(4, 6, c3);
      str.replace(4, 6, c3); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "Alo Brasil");
      str.replace(4, 6, c3);
      str.commit();
      REQUIRE(str == "Alo Huelandia");
      str = "Alo Brasil"; str.commit();
      // replace C-style string (iterators)
      const char* c4 = "infern";
      str.replace(str.cbegin() + 4, str.cend() - 2, c4);
      str.replace(str.cbegin() + 4, str.cend() - 2, c4); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "Alo Brasil");
      str.replace(str.cbegin() + 4, str.cend() - 2, c4);
      str.commit();
      REQUIRE(str == "Alo infernil");
      str = "Alo Brasil"; str.commit();
      // replace repeat chars (pos + count)
      str.replace(6, 4, 10, 'r');
      str.replace(6, 4, 10, 'r'); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "Alo Brasil");
      str.replace(6, 4, 10, 'r');
      str.commit();
      REQUIRE(str == "Alo Brrrrrrrrrrr");
      str = "Alo Brasil"; str.commit();
      // replace repeat chars (iterators)
      str.replace(str.cbegin() + 3, str.cend(), 10, 'o');
      str.replace(str.cbegin() + 3, str.cend(), 10, 'o'); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "Alo Brasil");
      str.replace(str.cbegin() + 3, str.cend(), 10, 'o');
      str.commit();
      REQUIRE(str == "Alooooooooooo");
      str = "Alo Brasil"; str.commit();
      // replace ilist (iterators)
      std::initializer_list<char> ilist { 'A', 'd', 'e', 'u', 's' };
      str.replace(str.cbegin(), str.cbegin() + 3, ilist);
      str.replace(str.cbegin(), str.cbegin() + 3, ilist); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "Alo Brasil");
      str.replace(str.cbegin(), str.cbegin() + 3, ilist);
      str.commit();
      REQUIRE(str == "Adeus Brasil");
      str = "Alo Brasil"; str.commit();
      // replace std::string_view (pos + count)
      std::string_view sv1("orr");
      str.replace(5, 3, sv1);
      str.replace(5, 3, sv1); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "Alo Brasil");
      str.replace(5, 3, sv1);
      str.commit();
      REQUIRE(str == "Alo Borril");
      str = "Alo Brasil"; str.commit();
      // replace std::string_view (iterators)
      std::string_view sv2("arr");
      str.replace(str.cbegin() + 5, str.cend() - 2, sv2);
      str.replace(str.cbegin() + 5, str.cend() - 2, sv2); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "Alo Brasil");
      str.replace(str.cbegin() + 5, str.cend() - 2, sv2);
      str.commit();
      REQUIRE(str == "Alo Barril");
      str = "Alo Brasil"; str.commit();
      // replace std::string_view substring (pos + count)
      std::string_view sv3("Baronesa da Pisadinha");
      str.replace(4, 4, sv3, 0, 5);
      str.replace(4, 4, sv3, 0, 5); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "Alo Brasil");
      str.replace(4, 4, sv3, 0, 5);
      str.commit();
      REQUIRE(str == "Alo Baronil");
    }

    SECTION("substr and copy") {
      SafeString str("Wilkommen");
      REQUIRE(str.substr(0, 5) == "Wilko");
      REQUIRE(str.substr(6) == "men");
      REQUIRE(str.substr() == "Wilkommen");
      char buf[10];
      str.copy(buf, 4, 3);
      REQUIRE(buf[0] == 'k');
      REQUIRE(buf[1] == 'o');
      REQUIRE(buf[2] == 'm');
      REQUIRE(buf[3] == 'm');
      char buf2[10];
      str.copy(buf2, 6);
      REQUIRE(buf2[0] == 'W');
      REQUIRE(buf2[1] == 'i');
      REQUIRE(buf2[2] == 'l');
      REQUIRE(buf2[3] == 'k');
      REQUIRE(buf2[4] == 'o');
      REQUIRE(buf2[5] == 'm');
    }

    SECTION("resize") {
      SafeString str("aaa");
      // resize bigger, default char ('\0')
      str.resize(5);
      str.resize(10); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str.size() == 3);
      REQUIRE(str == "aaa");
      str.resize(5);
      str.commit();
      REQUIRE(str.size() == 5);
      REQUIRE(str[0] == 'a');
      REQUIRE(str[1] == 'a');
      REQUIRE(str[2] == 'a');
      REQUIRE(str[3] == '\0');
      REQUIRE(str[4] == '\0');
      // resize smaller, default char ('\0')
      str.resize(1);
      str.revert();
      REQUIRE(str.size() == 5);
      REQUIRE(str[0] == 'a');
      REQUIRE(str[1] == 'a');
      REQUIRE(str[2] == 'a');
      REQUIRE(str[3] == '\0');
      REQUIRE(str[4] == '\0');
      str.resize(1);
      str.commit();
      REQUIRE(str.size() == 1);
      REQUIRE(str == "a");
      // resize bigger, custom char
      str.resize(10, 'a');
      str.resize(20, 'a'); // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str.size() == 1);
      REQUIRE(str == "a");
      str.resize(10, 'a');
      str.commit();
      REQUIRE(str.size() == 10);
      REQUIRE(str == "aaaaaaaaaa");
      // resize smaller, custom char
      str.resize(3, 'b');
      str.revert();
      REQUIRE(str.size() == 10);
      REQUIRE(str == "aaaaaaaaaa");
      str.resize(3, 'b');
      str.commit();
      REQUIRE(str.size() == 3);
      REQUIRE(str == "aaa");
    }

    SECTION("swap") {
      SafeString str1("string1");
      SafeString str2("string2");
      std::string strRaw1("string3");
      std::string strRaw2("string4");
      // swap std::string
      str1.swap(strRaw1);
      str1.swap(strRaw1); // For coverage (copy != nullptr)
      str1.revert();
      REQUIRE(str1 == "string1");
      str1.swap(strRaw2);
      str1.commit();
      REQUIRE(str1 == "string4");
      str1 = "string1"; str1.commit(); // reset str1 for next test
      // swap SafeString
      str1.swap(str2);
      str1.swap(str2); // For coverage (copy != nullptr)
      str1.revert();
      str2.revert();
      REQUIRE(str1 == "string1");
      REQUIRE(str2 == "string2");
      str1.swap(str2);
      str1.commit();
      str2.commit();
      REQUIRE(str1 == "string2");
      REQUIRE(str2 == "string1");
    }

    SECTION("find and rfind") {
      SafeString str("Hello Again");
      // find std::string
      std::string str1("Hell");
      REQUIRE(str.find(str1) != std::string::npos);
      REQUIRE(str.find(str1, 6) == std::string::npos);
      // find SafeString
      SafeString str2("Hell");
      REQUIRE(str.find(str2) != std::string::npos);
      REQUIRE(str.find(str2, 6) == std::string::npos);
      // find C-style substring
      const char* str3 = "Heck";
      REQUIRE(str.find(str3, 0, 2) != std::string::npos);
      REQUIRE(str.find(str3, 2, 2) == std::string::npos);
      REQUIRE(str.find(str3, 0, 4) == std::string::npos);
      // find C-style string
      const char* str4 = "He";
      REQUIRE(str.find(str4) != std::string::npos);
      REQUIRE(str.find(str4, 6) == std::string::npos);
      // find char
      REQUIRE(str.find('o') != std::string::npos);
      REQUIRE(str.find('o', 6) == std::string::npos);
      REQUIRE(str.find('W') == std::string::npos);
      // rfind std::string
      std::string str5("gain");
      REQUIRE(str.rfind(str5) != std::string::npos);
      REQUIRE(str.rfind(str5, 6) == std::string::npos);
      // rfind SafeString
      SafeString str6("gain");
      REQUIRE(str.rfind(str6) != std::string::npos);
      REQUIRE(str.rfind(str6, 6) == std::string::npos);
      // rfind C-style substring
      const char* str7 = "Agony";
      REQUIRE(str.rfind(str7, 8, 2) != std::string::npos);
      REQUIRE(str.rfind(str7, 4, 2) == std::string::npos);
      REQUIRE(str.rfind(str7, 8, 4) == std::string::npos);
      // rfind C-style string
      const char* str8 = "Ag";
      REQUIRE(str.rfind(str8) != std::string::npos);
      REQUIRE(str.rfind(str8, 4) == std::string::npos);
      // rfind char
      REQUIRE(str.rfind('n') != std::string::npos);
      REQUIRE(str.rfind('n', 6) == std::string::npos);
      REQUIRE(str.rfind('W') == std::string::npos);
    }

    SECTION("find_first_of and find_first_not_of") {
      SafeString str("abcdefghi");
      // find_first_of SafeString
      SafeString str1("abc");
      REQUIRE(str.find_first_of(str1) != std::string::npos);
      REQUIRE(str.find_first_of(str1, 3) == std::string::npos);
      // find_first_of std::string
      std::string str2("def");
      REQUIRE(str.find_first_of(str2) != std::string::npos);
      REQUIRE(str.find_first_of(str2, 6) == std::string::npos);
      // find_first_of C-style substring
      const char* str3 = "jklabc";
      REQUIRE(str.find_first_of(str3, 0, 3) == std::string::npos);
      REQUIRE(str.find_first_of(str3, 0, 6) != std::string::npos);
      REQUIRE(str.find_first_of(str3, 6, 6) == std::string::npos);
      // find_first_of C-style string
      const char* str4 = "bcd";
      REQUIRE(str.find_first_of(str4) != std::string::npos);
      REQUIRE(str.find_first_of(str4, 6) == std::string::npos);
      // find_first_of char
      REQUIRE(str.find_first_of('e') != std::string::npos);
      REQUIRE(str.find_first_of('e', 6) == std::string::npos);
      REQUIRE(str.find_first_of('z') == std::string::npos);
      // find_first_not_of SafeString
      SafeString str5("defghi");
      REQUIRE(str.find_first_not_of(str5) != std::string::npos);
      REQUIRE(str.find_first_not_of(str5, 3) == std::string::npos);
      // find_first_not_of std::string
      std::string str6("ghi");
      REQUIRE(str.find_first_not_of(str6) != std::string::npos);
      REQUIRE(str.find_first_not_of(str6, 6) == std::string::npos);
      // find_first_not_of C-style substring
      const char* str7 = "defghi";
      REQUIRE(str.find_first_not_of(str7, 0, 3) != std::string::npos);
      REQUIRE(str.find_first_not_of(str7, 3, 6) == std::string::npos);
      REQUIRE(str.find_first_not_of(str7, 6, 3) != std::string::npos);
      // find_first_not_of C-style string
      const char* str8 = "ghi";
      REQUIRE(str.find_first_not_of(str8) != std::string::npos);
      REQUIRE(str.find_first_not_of(str8, 6) == std::string::npos);
      // find_first_not_of char
      REQUIRE(str.find_first_not_of('e') != std::string::npos);
      REQUIRE(str.find_first_not_of('i', 8) == std::string::npos);
      REQUIRE(str.find_first_not_of('z') != std::string::npos);
    }

    SECTION("find_last_of and find_last_not_of") {
      SafeString str("abcdefghi");
      // find_last_of SafeString
      SafeString str1("ghi");
      REQUIRE(str.find_last_of(str1) != std::string::npos);
      REQUIRE(str.find_last_of(str1, 3) == std::string::npos);
      // find_last_of std::string
      std::string str2("def");
      REQUIRE(str.find_last_of(str2) != std::string::npos);
      REQUIRE(str.find_last_of(str2, 2) == std::string::npos);
      // find_last_of C-style substring
      const char* str3 = "defghi";
      REQUIRE(str.find_last_of(str3, 0, 3) == std::string::npos);
      REQUIRE(str.find_last_of(str3, 6, 3) != std::string::npos);
      REQUIRE(str.find_last_of(str3, 9, 6) != std::string::npos);
      // find_last_of C-style string
      const char* str4 = "ghi";
      REQUIRE(str.find_last_of(str4) != std::string::npos);
      REQUIRE(str.find_last_of(str4, 3) == std::string::npos);
      // find_last_of char
      REQUIRE(str.find_last_of('g') != std::string::npos);
      REQUIRE(str.find_last_of('g', 3) == std::string::npos);
      REQUIRE(str.find_last_of('z') == std::string::npos);
      // find_last_not_of SafeString
      SafeString str5("abcdef");
      REQUIRE(str.find_last_not_of(str5) != std::string::npos);
      REQUIRE(str.find_last_not_of(str5, 3) == std::string::npos);
      // find_last_not_of std::string
      std::string str6("abc");
      REQUIRE(str.find_last_not_of(str6) != std::string::npos);
      REQUIRE(str.find_last_not_of(str6, 2) == std::string::npos);
      // find_last_not_of C-style substring
      const char* str7 = "abcdef";
      REQUIRE(str.find_last_not_of(str7, 0, 3) == std::string::npos);
      REQUIRE(str.find_last_not_of(str7, 3, 3) != std::string::npos);
      REQUIRE(str.find_last_not_of(str7, 3, 6) == std::string::npos);
      // find_last_not_of C-style string
      const char* str8 = "abc";
      REQUIRE(str.find_last_not_of(str8) != std::string::npos);
      REQUIRE(str.find_last_not_of(str8, 2) == std::string::npos);
      // find_last_not_of char
      REQUIRE(str.find_last_not_of('e') != std::string::npos);
      REQUIRE(str.find_last_not_of('a', 0) == std::string::npos);
      REQUIRE(str.find_last_not_of('z') != std::string::npos);
    }

    SECTION("operator=") {
      SafeString str("Test0");
      // assign SafeString
      SafeString str1("Test1");
      str = str1;
      str = str1; // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "Test0");
      str = str1;
      str.commit();
      REQUIRE(str == "Test1");
      // assign std::string
      std::string str2("Test2");
      str = str2;
      str = str2; // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "Test1");
      str = str2;
      str.commit();
      REQUIRE(str == "Test2");
      // assign C-style string
      const char* str3 = "Test3";
      str = str3;
      str = str3; // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "Test2");
      str = str3;
      str.commit();
      REQUIRE(str == "Test3");
      // assign char
      char ch = '4';
      str = ch;
      str = ch; // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "Test3");
      str = ch;
      str.commit();
      REQUIRE(str == "4");
      // assign ilist
      std::initializer_list<char> ilist { 'T', 'e', 's', 't', '5' };
      str = ilist;
      str = ilist; // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "4");
      str = ilist;
      str.commit();
      REQUIRE(str == "Test5");
    }

    SECTION("operator+=") {
      SafeString str("Test");
      // assign SafeString
      SafeString str1("111");
      str += str1;
      str += str1; // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "Test");
      str += str1;
      str.commit();
      REQUIRE(str == "Test111");
      // assign std::string
      std::string str2("222");
      str += str2;
      str += str2; // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "Test111");
      str += str2;
      str.commit();
      REQUIRE(str == "Test111222");
      // assign C-style string
      const char* str3 = "333";
      str += str3;
      str += str3; // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "Test111222");
      str += str3;
      str.commit();
      REQUIRE(str == "Test111222333");
      // assign char
      char ch = '4';
      str += ch;
      str += ch; // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "Test111222333");
      str += ch;
      str.commit();
      REQUIRE(str == "Test1112223334");
      // assign ilist
      std::initializer_list<char> ilist { '5', '6', '7', '8', '9' };
      str += ilist;
      str += ilist; // For coverage (copy != nullptr)
      str.revert();
      REQUIRE(str == "Test1112223334");
      str += ilist;
      str.commit();
      REQUIRE(str == "Test111222333456789");
    }

    SECTION("operator[]") {
      SafeString str("Hewwo");
      // const []
      REQUIRE(std::as_const(str)[0] == 'H');
      REQUIRE(std::as_const(str)[1] == 'e');
      REQUIRE(std::as_const(str)[2] == 'w');
      REQUIRE(std::as_const(str)[3] == 'w');
      REQUIRE(std::as_const(str)[4] == 'o');
      // non-const []
      str[0] = 'W';
      str.revert();
      REQUIRE(str == "Hewwo");
      str[4] = 'u';
      str.commit();
      REQUIRE(str == "Hewwu");
    }

    SECTION("operator+") {
      SafeString str1("Test1");
      SafeString str2("Test2");
      std::string str3("Test3");
      const char* str4 = "Test4";
      char ch = '5';
      REQUIRE((str1 + str2) == "Test1Test2");
      REQUIRE((str1 + str3) == "Test1Test3");
      REQUIRE((str1 + str4) == "Test1Test4");
      REQUIRE((str1 + ch) == "Test15");
    }

    SECTION("operator== and operator!=") {
      SafeString strA1("AAAAA");
      SafeString strA2("AAAAA");
      SafeString strB("BBBBB");
      std::string strRawA("AAAAA");
      std::string strRawB("BBBBB");
      const char* cstrA = "AAAAA";
      const char* cstrB = "BBBBB";
      REQUIRE(strA1 == strA2);
      REQUIRE(strA1 != strB);
      REQUIRE(strA1 == strRawA);
      REQUIRE(strA1 != strRawB);
      REQUIRE(strA1 == cstrA);
      REQUIRE(strA1 != cstrB);
    }

    SECTION("operator< and operator>") {
      SafeString strA("AAAAA");
      SafeString strB("BBBBB");
      std::string strRawA("AAAAA");
      std::string strRawB("BBBBB");
      const char* cstrA = "AAAAA";
      const char* cstrB = "BBBBB";
      REQUIRE(strA < strB);
      REQUIRE(strB > strA);
      REQUIRE(!(strA > strB));
      REQUIRE(!(strB < strA));
      REQUIRE(strA < strRawB);
      REQUIRE(strB > strRawA);
      REQUIRE(!(strA > strRawB));
      REQUIRE(!(strB < strRawA));
      REQUIRE(strA < cstrB);
      REQUIRE(strB > cstrA);
      REQUIRE(!(strA > cstrB));
      REQUIRE(!(strB < cstrA));
    }

    SECTION("operator<= and operator>=") {
      SafeString strA1("AAAAA");
      SafeString strA2("AAAAA");
      SafeString strB("BBBBB");
      std::string strRawA("AAAAA");
      std::string strRawB("BBBBB");
      const char* cstrA = "AAAAA";
      const char* cstrB = "BBBBB";
      REQUIRE(strA1 <= strA2);
      REQUIRE(strA1 >= strA2);
      REQUIRE(strA1 <= strB);
      REQUIRE(strB >= strA1);
      REQUIRE(!(strA1 >= strB));
      REQUIRE(!(strB <= strA1));
      REQUIRE(strA1 <= strRawA);
      REQUIRE(strA1 >= strRawA);
      REQUIRE(strA1 <= strRawB);
      REQUIRE(strB >= strRawA);
      REQUIRE(!(strA1 >= strRawB));
      REQUIRE(!(strB <= strRawA));
      REQUIRE(strA1 <= cstrA);
      REQUIRE(strA1 >= cstrA);
      REQUIRE(strA1 <= cstrB);
      REQUIRE(strB >= cstrA);
      REQUIRE(!(strA1 >= cstrB));
      REQUIRE(!(strB <= cstrA));
    }

    SECTION("operator<<") {
      SafeString str("ABCDE");
      std::stringstream ss;
      ss << str;
      REQUIRE_THAT(ss.str(), Equals("ABCDE"));
      str.revert(); // For coverage (copy != nullptr)
    }
  }
}

