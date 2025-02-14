/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#include "libs/catch2/catch_amalgamated.hpp"

#include "contract/variables/safeint.h"

#include "utils/intconv.h" // UnderlyingType

// Helper struct and templates for testing multiple types
template <int Size> struct UnderlyingType;

// C++ native types
template <> struct UnderlyingType<8> { using type = int8_t; };
template <> struct UnderlyingType<16> { using type = int16_t; };
template <> struct UnderlyingType<32> { using type = int32_t; };
template <> struct UnderlyingType<64> { using type = int64_t; };

// Boost types
template<> struct UnderlyingType<24> { using type = int24_t; };
template<> struct UnderlyingType<40> { using type = int40_t; };
template<> struct UnderlyingType<48> { using type = int48_t; };
template<> struct UnderlyingType<56> { using type = int56_t; };
template<> struct UnderlyingType<72> { using type = int72_t; };
template<> struct UnderlyingType<80> { using type = int80_t; };
template<> struct UnderlyingType<88> { using type = int88_t; };
template<> struct UnderlyingType<96> { using type = int96_t; };
template<> struct UnderlyingType<104> { using type = int104_t; };
template<> struct UnderlyingType<112> { using type = int112_t; };
template<> struct UnderlyingType<120> { using type = int120_t; };
template<> struct UnderlyingType<128> { using type = int128_t; };
template<> struct UnderlyingType<136> { using type = int136_t; };
template<> struct UnderlyingType<144> { using type = int144_t; };
template<> struct UnderlyingType<152> { using type = int152_t; };
template<> struct UnderlyingType<160> { using type = int160_t; };
template<> struct UnderlyingType<168> { using type = int168_t; };
template<> struct UnderlyingType<176> { using type = int176_t; };
template<> struct UnderlyingType<184> { using type = int184_t; };
template<> struct UnderlyingType<192> { using type = int192_t; };
template<> struct UnderlyingType<200> { using type = int200_t; };
template<> struct UnderlyingType<208> { using type = int208_t; };
template<> struct UnderlyingType<216> { using type = int216_t; };
template<> struct UnderlyingType<224> { using type = int224_t; };
template<> struct UnderlyingType<232> { using type = int232_t; };
template<> struct UnderlyingType<240> { using type = int240_t; };
template<> struct UnderlyingType<248> { using type = int248_t; };
template<> struct UnderlyingType<256> { using type = int256_t; };

// Helper template for testing all types
template <int Size> struct SafeIntTester {
  using SafeInt = SafeInt_t<Size>;
  using UnderlyingType = typename UnderlyingType<Size>::type;

  void operator()() const {
    SECTION(std::string("SafeInt_t<") + std::to_string(Size) + "> underlying type") {
      SafeInt val;
      REQUIRE(std::is_same_v<std::decay_t<decltype(val.get())>, std::decay_t<UnderlyingType>>);
    }

    SECTION(std::string("SafeInt_t<") + std::to_string(Size) + "> constructor") {
      SafeInt val(UnderlyingType(-42));
      SafeInt copyVal(val);
      REQUIRE(val == UnderlyingType(-42));
      REQUIRE(copyVal == val);
    }

    SECTION(std::string("SafeInt_t<") + std::to_string(Size) + "> operator+") {
      SafeInt valPos(UnderlyingType(42));
      SafeInt valMin(UnderlyingType(-42));
      SafeInt valOver(std::numeric_limits<UnderlyingType>::max());
      SafeInt valUnder(std::numeric_limits<UnderlyingType>::min());
      bool hadOver1 = false;
      bool hadOver2 = false;
      bool hadUnder1 = false;
      bool hadUnder2 = false;
      // catch over/underflow
      try { valOver = valOver + valPos; } catch (std::overflow_error& e) { hadOver1 = true; }
      try { valOver = valOver + UnderlyingType(1); } catch (std::overflow_error& e) { hadOver2 = true; }
      try { valUnder = valUnder + valMin; } catch (std::underflow_error& e) { hadUnder1 = true; }
      try { valUnder = valUnder + UnderlyingType(-1); } catch (std::underflow_error& e) { hadUnder2 = true; }
      REQUIRE(hadOver1);
      REQUIRE(hadOver2);
      REQUIRE(hadUnder1);
      REQUIRE(hadUnder2);
      // operate with int
      SafeInt val(UnderlyingType(-42));
      val = val + UnderlyingType(5);
      val.revert();
      REQUIRE(val == UnderlyingType(-42));
      val = val + UnderlyingType(5);
      val.commit();
      REQUIRE(val == UnderlyingType(-37));
      // operate with SafeInt
      SafeInt sum(UnderlyingType(10));
      val = val + sum;
      val.revert();
      REQUIRE(val == UnderlyingType(-37));
      val = val + sum;
      val.commit();
      REQUIRE(val == UnderlyingType(-27));
      // For coverage (operate with 0)
      REQUIRE(val + UnderlyingType(0) == val);
      REQUIRE(val + SafeInt(UnderlyingType(0)) == val);
    }

    SECTION(std::string("SafeInt_t<") + std::to_string(Size) + "> operator-") {
      SafeInt valPos(UnderlyingType(1));
      SafeInt valMin(UnderlyingType(-1));
      SafeInt valOver(std::numeric_limits<UnderlyingType>::max());
      SafeInt valUnder(std::numeric_limits<UnderlyingType>::min());
      bool hadOver1 = false;
      bool hadOver2 = false;
      bool hadUnder1 = false;
      bool hadUnder2 = false;
      // catch over/underflow
      try { valOver = valOver - valMin; } catch (std::overflow_error& e) { hadOver1 = true; }
      try { valOver = valOver - UnderlyingType(-1); } catch (std::overflow_error& e) { hadOver2 = true; }
      try { valUnder = valUnder - valPos; } catch (std::underflow_error& e) { hadUnder1 = true; }
      try { valUnder = valUnder - UnderlyingType(1); } catch (std::underflow_error& e) { hadUnder2 = true; }
      REQUIRE(hadOver1);
      REQUIRE(hadOver2);
      REQUIRE(hadUnder1);
      REQUIRE(hadUnder2);
      // operate with int
      SafeInt val(UnderlyingType(-42));
      val = val - UnderlyingType(5);
      val.revert();
      REQUIRE(val == UnderlyingType(-42));
      val = val - UnderlyingType(5);
      val.commit();
      REQUIRE(val == UnderlyingType(-47));
      // operate with SafeInt
      SafeInt sub(UnderlyingType(10));
      val = val - sub;
      val.revert();
      REQUIRE(val == UnderlyingType(-47));
      val = val - sub;
      val.commit();
      REQUIRE(val == UnderlyingType(-57));
      // For coverage (operate with 0)
      REQUIRE(val - UnderlyingType(0) == val);
      REQUIRE(val - SafeInt(UnderlyingType(0)) == val);
    }

    SECTION(std::string("SafeInt_t<") + std::to_string(Size) + "> operator*") {
      SafeInt valPos(UnderlyingType(42));
      SafeInt valZero(UnderlyingType(0));
      SafeInt valOver(std::numeric_limits<UnderlyingType>::max());
      SafeInt valUnder(std::numeric_limits<UnderlyingType>::min());
      bool hadZero1 = false;
      bool hadZero2 = false;
      bool hadZero3 = false;
      bool hadZero4 = false;
      bool hadOver1 = false;
      bool hadOver2 = false;
      bool hadUnder1 = false;
      bool hadUnder2 = false;
      // catch over/underflow and mul by zero
      try { valPos = valPos * valZero; } catch (std::domain_error& e) { hadZero1 = true; }
      try { valPos = valPos * UnderlyingType(0); } catch (std::domain_error& e) { hadZero2 = true; }
      try { valZero = valZero * valPos; } catch (std::domain_error& e) { hadZero3 = true; }
      try { valZero = valZero * UnderlyingType(42); } catch (std::domain_error& e) { hadZero4 = true; }
      try { valOver = valOver * valPos; } catch (std::overflow_error& e) { hadOver1 = true; }
      try { valOver = valOver * UnderlyingType(2); } catch (std::overflow_error& e) { hadOver2 = true; }
      try { valUnder = valUnder * valPos; } catch (std::underflow_error& e) { hadUnder1 = true; }
      try { valUnder = valUnder * UnderlyingType(2); } catch (std::underflow_error& e) { hadUnder2 = true; }
      REQUIRE(hadZero1);
      REQUIRE(hadZero2);
      REQUIRE(hadZero3);
      REQUIRE(hadZero4);
      REQUIRE(hadOver1);
      REQUIRE(hadOver2);
      REQUIRE(hadUnder1);
      REQUIRE(hadUnder2);
      // operate with int
      SafeInt val(UnderlyingType(-42));
      val = val * UnderlyingType(2);
      val.revert();
      REQUIRE(val == UnderlyingType(-42));
      val = val * UnderlyingType(2);
      val.commit();
      REQUIRE(val == UnderlyingType(-84));
      val = UnderlyingType(-42); val.commit(); // reset to ensure fit into minimum (SafeInt<8>)
      // operate with SafeInt
      SafeInt mul(UnderlyingType(2));
      val = val * mul;
      val.revert();
      REQUIRE(val == UnderlyingType(-42));
      val = val * mul;
      val.commit();
      REQUIRE(val == UnderlyingType(-84));
    }

    SECTION(std::string("SafeInt_t<") + std::to_string(Size) + "> operator/") {
      SafeInt valPos(UnderlyingType(42));
      SafeInt valMin(UnderlyingType(-1));
      SafeInt valZero(UnderlyingType(0));
      SafeInt valOver(std::numeric_limits<UnderlyingType>::min());
      bool hadZero1 = false;
      bool hadZero2 = false;
      bool hadOver1 = false;
      bool hadOver2 = false;
      // catch overflow and div by zero
      try { valPos = valPos / valZero; } catch (std::domain_error& e) { hadZero1 = true; }
      try { valPos = valPos / UnderlyingType(0); } catch (std::domain_error& e) { hadZero2 = true; }
      try { valOver = valOver / valMin; } catch (std::overflow_error& e) { hadOver1 = true; }
      try { valOver = valOver / UnderlyingType(-1); } catch (std::overflow_error& e) { hadOver2 = true; }
      REQUIRE(hadZero1);
      REQUIRE(hadZero2);
      REQUIRE(hadOver1);
      REQUIRE(hadOver2);
      // operate with int
      SafeInt val(UnderlyingType(-42));
      val = val / UnderlyingType(2);
      val.revert();
      REQUIRE(val == UnderlyingType(-42));
      val = val / UnderlyingType(2);
      val.commit();
      REQUIRE(val == UnderlyingType(-21));
      // operate with SafeInt
      SafeInt div(UnderlyingType(3));
      val = val / div;
      val.revert();
      REQUIRE(val == UnderlyingType(-21));
      val = val / div;
      val.commit();
      REQUIRE(val == UnderlyingType(-7));
      // For coverage
      SafeInt valOne(UnderlyingType(1));
      REQUIRE(valOver / valOne == valOver);
      REQUIRE(valOver / UnderlyingType(1) == valOver);
    }

    SECTION(std::string("SafeInt_t<") + std::to_string(Size) + "> operator%") {
      SafeInt val(UnderlyingType(-42));
      SafeInt valZero(UnderlyingType(0));
      bool hadZero1 = false;
      bool hadZero2 = false;
      // catch mod by zero
      try { val = val % valZero; } catch (std::domain_error& e) { hadZero1 = true; }
      try { val = val % UnderlyingType(0); } catch (std::domain_error& e) { hadZero2 = true; }
      REQUIRE(hadZero1);
      REQUIRE(hadZero2);
      // operate with int
      val = val % UnderlyingType(9);
      val.revert();
      REQUIRE(val == UnderlyingType(-42));
      val = val % UnderlyingType(9);
      val.commit();
      REQUIRE(val == UnderlyingType(-6));
      // operate with SafeInt
      SafeInt mod(UnderlyingType(4));
      val = val % mod;
      val.revert();
      REQUIRE(val == UnderlyingType(-6));
      val = val % mod;
      val.commit();
      REQUIRE(val == UnderlyingType(-2));
    }

    SECTION(std::string("SafeInt_t<") + std::to_string(Size) + "> operator&, | and ^ (int)") {
      SafeInt val1(UnderlyingType(0b10101010));
      SafeInt val2(UnderlyingType(0b10101010));
      SafeInt val3(UnderlyingType(0b10101010));
      // bitwise AND
      val1 = val1 & UnderlyingType(0b11110000);
      val1.revert();
      REQUIRE(val1 == UnderlyingType(0b10101010));
      val1 = val1 & UnderlyingType(0b11110000);
      val1.commit();
      REQUIRE(val1 == UnderlyingType(0b10100000));
      // bitwise OR
      val2 = val2 | UnderlyingType(0b11110000);
      val2.revert();
      REQUIRE(val2 == UnderlyingType(0b10101010));
      val2 = val2 | UnderlyingType(0b11110000);
      val2.commit();
      REQUIRE(val2 == UnderlyingType(0b11111010));
      // bitwise XOR
      val3 = val3 ^ UnderlyingType(0b11110000);
      val3.revert();
      REQUIRE(val3 == UnderlyingType(0b10101010));
      val3 = val3 ^ UnderlyingType(0b11110000);
      val3.commit();
      REQUIRE(val3 == UnderlyingType(0b01011010));
    }

    SECTION(std::string("SafeInt_t<") + std::to_string(Size) + "> operator&, | and ^ (SafeInt)") {
      SafeInt val1(UnderlyingType(0b10101010));
      SafeInt val2(UnderlyingType(0b10101010));
      SafeInt val3(UnderlyingType(0b10101010));
      SafeInt val4(UnderlyingType(0b10101010));
      SafeInt val5(UnderlyingType(0b10101010));
      SafeInt valOp(UnderlyingType(0b11110000));
      // bitwise AND
      val1 = val1 & valOp;
      val1.revert();
      REQUIRE(val1 == UnderlyingType(0b10101010));
      val1 = val1 & valOp;
      val1.commit();
      REQUIRE(val1 == UnderlyingType(0b10100000));
      // bitwise OR
      val2 = val2 | valOp;
      val2.revert();
      REQUIRE(val2 == UnderlyingType(0b10101010));
      val2 = val2 | valOp;
      val2.commit();
      REQUIRE(val2 == UnderlyingType(0b11111010));
      // bitwise XOR
      val3 = val3 ^ valOp;
      val3.revert();
      REQUIRE(val3 == UnderlyingType(0b10101010));
      val3 = val3 ^ valOp;
      val3.commit();
      REQUIRE(val3 == UnderlyingType(0b01011010));
    }

    SECTION(std::string("SafeInt_t<") + std::to_string(Size) + "> operator<< and >>") {
      SafeInt val4(UnderlyingType(0b10101010));
      SafeInt val5(UnderlyingType(0b10101010));
      // bitwise left shift
      val4 = val4 << 2;
      val4.revert();
      REQUIRE(val4 == UnderlyingType(0b10101010));
      val4 = val4 << 2;
      val4.commit();
      REQUIRE(val4 == UnderlyingType(0b1010101000));
      // bitwise right shift
      val5 = val5 >> 2;
      val5.revert();
      REQUIRE(val5 == UnderlyingType(0b10101010));
      val5 = val5 >> 2;
      val5.commit();
      REQUIRE(val5 == UnderlyingType((Size == 8) ? 0b11101010 : 0b00101010)); // https://stackoverflow.com/a/22734721
    }

    SECTION(std::string("SafeInt_t<") + std::to_string(Size) + "> operator!, && and ||") {
      SafeInt val(UnderlyingType(0));
      // logical NOT
      val = !val;
      val.revert();
      REQUIRE(val == UnderlyingType(0));
      val = !val;
      val.commit();
      REQUIRE(val == UnderlyingType(1));
      // logical AND (int)
      val = val && UnderlyingType(0);
      val.revert();
      REQUIRE(val == UnderlyingType(1));
      val = val && UnderlyingType(0);
      val.commit();
      REQUIRE(val == UnderlyingType(0));
      // logical OR (int)
      val = val || UnderlyingType(1);
      val.revert();
      REQUIRE(val == UnderlyingType(0));
      val = val || UnderlyingType(1);
      val.commit();
      REQUIRE(val == UnderlyingType(1));
      // logical AND (SafeInt)
      val = val && SafeInt(UnderlyingType(0));
      val.revert();
      REQUIRE(val == UnderlyingType(1));
      val = val && SafeInt(UnderlyingType(0));
      val.commit();
      REQUIRE(val == UnderlyingType(0));
      // logical OR (SafeInt)
      val = val || SafeInt(UnderlyingType(1));
      val.revert();
      REQUIRE(val == UnderlyingType(0));
      val = val || SafeInt(UnderlyingType(1));
      val.commit();
      REQUIRE(val == UnderlyingType(1));
    }

    SECTION(std::string("SafeInt_t<") + std::to_string(Size) + "> operator== and !=") {
      SafeInt valA1(UnderlyingType(-42));
      SafeInt valA2(UnderlyingType(-42));
      SafeInt valB1(UnderlyingType(-24));
      SafeInt valB2(UnderlyingType(-24));
      // compare int
      REQUIRE(valA1 == valA2.get());
      REQUIRE(valA1 != valB1.get());
      REQUIRE(valA1 != valB2.get());
      REQUIRE(valA2 == valA1.get());
      REQUIRE(valA2 != valB1.get());
      REQUIRE(valA2 != valB2.get());
      REQUIRE(valB1 != valA1.get());
      REQUIRE(valB1 != valA2.get());
      REQUIRE(valB1 == valB2.get());
      REQUIRE(valB2 != valA1.get());
      REQUIRE(valB2 != valA2.get());
      REQUIRE(valB2 == valB1.get());
      // compare SafeInt
      REQUIRE(valA1 == valA2);
      REQUIRE(valA1 != valB1);
      REQUIRE(valA1 != valB2);
      REQUIRE(valA2 == valA1);
      REQUIRE(valA2 != valB1);
      REQUIRE(valA2 != valB2);
      REQUIRE(valB1 != valA1);
      REQUIRE(valB1 != valA2);
      REQUIRE(valB1 == valB2);
      REQUIRE(valB2 != valA1);
      REQUIRE(valB2 != valA2);
      REQUIRE(valB2 == valB1);
    }

    SECTION(std::string("SafeInt_t<") + std::to_string(Size) + "> operator<, >, <= and >=") {
      SafeInt valA1(UnderlyingType(-42));
      SafeInt valA2(UnderlyingType(-42));
      SafeInt valB1(UnderlyingType(-41));
      SafeInt valB2(UnderlyingType(-41));
      // compare int
      REQUIRE(!(valA1 < valA2.get()));
      REQUIRE(valA1 <= valA2.get());
      REQUIRE(valA1 >= valA2.get());
      REQUIRE(!(valA1 > valA2.get()));
      REQUIRE(valA1 < valB1.get());
      REQUIRE(valA1 <= valB1.get());
      REQUIRE(!(valA1 >= valB1.get()));
      REQUIRE(!(valA1 > valB1.get()));
      REQUIRE(!(valB1 < valA1.get()));
      REQUIRE(!(valB1 <= valA1.get()));
      REQUIRE(valB1 >= valA1.get());
      REQUIRE(valB1 > valA1.get());
      REQUIRE(!(valB1 < valB2.get()));
      REQUIRE(valB1 <= valB2.get());
      REQUIRE(valB1 >= valB2.get());
      REQUIRE(!(valB1 > valB2.get()));
      // compare SafeInt
      REQUIRE(!(valA1 < valA2));
      REQUIRE(valA1 <= valA2);
      REQUIRE(valA1 >= valA2);
      REQUIRE(!(valA1 > valA2));
      REQUIRE(valA1 < valB1);
      REQUIRE(valA1 <= valB1);
      REQUIRE(!(valA1 >= valB1));
      REQUIRE(!(valA1 > valB1));
      REQUIRE(!(valB1 < valA1));
      REQUIRE(!(valB1 <= valA1));
      REQUIRE(valB1 >= valA1);
      REQUIRE(valB1 > valA1);
      REQUIRE(!(valB1 < valB2));
      REQUIRE(valB1 <= valB2);
      REQUIRE(valB1 >= valB2);
      REQUIRE(!(valB1 > valB2));
    }

    SECTION(std::string("SafeInt_t<") + std::to_string(Size) + "> operator=") {
      SafeInt val(UnderlyingType(-42));
      // assign int
      val = UnderlyingType(-24);
      val.revert();
      REQUIRE(val == UnderlyingType(-42));
      val = UnderlyingType(-24);
      val.commit();
      REQUIRE(val == UnderlyingType(-24));
      // assign SafeInt
      SafeInt val2(UnderlyingType(-42));
      val = val2;
      val.revert();
      REQUIRE(val == UnderlyingType(-24));
      val = val2;
      val.commit();
      REQUIRE(val == UnderlyingType(-42));
    }

    SECTION(std::string("SafeInt_t<") + std::to_string(Size) + "> operator+=") {
      SafeInt valPos(UnderlyingType(42));
      SafeInt valMin(UnderlyingType(-42));
      SafeInt valOver(std::numeric_limits<UnderlyingType>::max());
      SafeInt valUnder(std::numeric_limits<UnderlyingType>::min());
      bool hadOver1 = false;
      bool hadOver2 = false;
      bool hadUnder1 = false;
      bool hadUnder2 = false;
      // catch over/underflow
      try { valOver += valPos; } catch (std::overflow_error& e) { hadOver1 = true; }
      try { valOver += UnderlyingType(1); } catch (std::overflow_error& e) { hadOver2 = true; }
      try { valUnder += valMin; } catch (std::underflow_error& e) { hadUnder1 = true; }
      try { valUnder += UnderlyingType(-1); } catch (std::underflow_error& e) { hadUnder2 = true; }
      REQUIRE(hadOver1);
      REQUIRE(hadOver2);
      REQUIRE(hadUnder1);
      REQUIRE(hadUnder2);
      // operate with int
      SafeInt val(UnderlyingType(-42));
      val += UnderlyingType(5);
      val.revert();
      REQUIRE(val == UnderlyingType(-42));
      val += UnderlyingType(5);
      val.commit();
      REQUIRE(val == UnderlyingType(-37));
      // operate with SafeInt
      SafeInt sum(UnderlyingType(10));
      val += sum;
      val.revert();
      REQUIRE(val == UnderlyingType(-37));
      val += sum;
      val.commit();
      REQUIRE(val == UnderlyingType(-27));
      // For coverage (operate with 0)
      val += UnderlyingType(0);
      REQUIRE(val == UnderlyingType(-27));
      val += SafeInt(UnderlyingType(0));
      REQUIRE(val == UnderlyingType(-27));
    }

    SECTION(std::string("SafeInt_t<") + std::to_string(Size) + "> operator-=") {
      SafeInt valPos(UnderlyingType(1));
      SafeInt valMin(UnderlyingType(-1));
      SafeInt valOver(std::numeric_limits<UnderlyingType>::max());
      SafeInt valUnder(std::numeric_limits<UnderlyingType>::min());
      bool hadOver1 = false;
      bool hadOver2 = false;
      bool hadUnder1 = false;
      bool hadUnder2 = false;
      // catch over/underflow
      try { valOver -= valMin; } catch (std::overflow_error& e) { hadOver1 = true; }
      try { valOver -= UnderlyingType(-1); } catch (std::overflow_error& e) { hadOver2 = true; }
      try { valUnder -= valPos; } catch (std::underflow_error& e) { hadUnder1 = true; }
      try { valUnder -= UnderlyingType(1); } catch (std::underflow_error& e) { hadUnder2 = true; }
      REQUIRE(hadOver1);
      REQUIRE(hadOver2);
      REQUIRE(hadUnder1);
      REQUIRE(hadUnder2);
      // operate with int
      SafeInt val(UnderlyingType(-42));
      val -= UnderlyingType(5);
      val.revert();
      REQUIRE(val == UnderlyingType(-42));
      val -= UnderlyingType(5);
      val.commit();
      REQUIRE(val == UnderlyingType(-47));
      // operate with SafeInt
      SafeInt sub(UnderlyingType(10));
      val -= sub;
      val.revert();
      REQUIRE(val == UnderlyingType(-47));
      val -= sub;
      val.commit();
      REQUIRE(val == UnderlyingType(-57));
      // For coverage (operate with 0)
      REQUIRE(val - UnderlyingType(0) == val);
      REQUIRE(val - SafeInt(UnderlyingType(0)) == val);
      val -= UnderlyingType(0);
      REQUIRE(val == UnderlyingType(-57));
      val -= SafeInt(UnderlyingType(0));
      REQUIRE(val == UnderlyingType(-57));
    }

    SECTION(std::string("SafeInt_t<") + std::to_string(Size) + "> operator*=") {
      SafeInt valPos(UnderlyingType(42));
      SafeInt valZero(UnderlyingType(0));
      SafeInt valOver(std::numeric_limits<UnderlyingType>::max());
      SafeInt valUnder(std::numeric_limits<UnderlyingType>::min());
      bool hadZero1 = false;
      bool hadZero2 = false;
      bool hadZero3 = false;
      bool hadZero4 = false;
      bool hadOver1 = false;
      bool hadOver2 = false;
      bool hadUnder1 = false;
      bool hadUnder2 = false;
      // catch over/underflow and mul by zero
      try { valPos *= valZero; } catch (std::domain_error& e) { hadZero1 = true; }
      try { valPos *= UnderlyingType(0); } catch (std::domain_error& e) { hadZero2 = true; }
      try { valZero *= valPos; } catch (std::domain_error& e) { hadZero3 = true; }
      try { valZero *= UnderlyingType(42); } catch (std::domain_error& e) { hadZero4 = true; }
      try { valOver *= valPos; } catch (std::overflow_error& e) { hadOver1 = true; }
      try { valOver *= UnderlyingType(2); } catch (std::overflow_error& e) { hadOver2 = true; }
      try { valUnder *= valPos; } catch (std::underflow_error& e) { hadUnder1 = true; }
      try { valUnder *= UnderlyingType(2); } catch (std::underflow_error& e) { hadUnder2 = true; }
      REQUIRE(hadZero1);
      REQUIRE(hadZero2);
      REQUIRE(hadZero3);
      REQUIRE(hadZero4);
      REQUIRE(hadOver1);
      REQUIRE(hadOver2);
      REQUIRE(hadUnder1);
      REQUIRE(hadUnder2);
      // operate with int
      SafeInt val(UnderlyingType(-42));
      val *= UnderlyingType(2);
      val.revert();
      REQUIRE(val == UnderlyingType(-42));
      val *= UnderlyingType(2);
      val.commit();
      REQUIRE(val == UnderlyingType(-84));
      val = UnderlyingType(-42); val.commit(); // reset to ensure fit into minimum (SafeInt<8>)
      // operate with SafeInt
      SafeInt mul(UnderlyingType(2));
      val *= mul;
      val.revert();
      REQUIRE(val == UnderlyingType(-42));
      val *= mul;
      val.commit();
      REQUIRE(val == UnderlyingType(-84));
    }

    SECTION(std::string("SafeInt_t<") + std::to_string(Size) + "> operator/=") {
      SafeInt valPos(UnderlyingType(42));
      SafeInt valMin(UnderlyingType(-1));
      SafeInt valZero(UnderlyingType(0));
      SafeInt valOver(std::numeric_limits<UnderlyingType>::min());
      bool hadZero1 = false;
      bool hadZero2 = false;
      bool hadOver1 = false;
      bool hadOver2 = false;
      // catch overflow and div by zero
      try { valPos /= valZero; } catch (std::domain_error& e) { hadZero1 = true; }
      try { valPos /= UnderlyingType(0); } catch (std::domain_error& e) { hadZero2 = true; }
      try { valOver /= valMin; } catch (std::overflow_error& e) { hadOver1 = true; }
      try { valOver /= UnderlyingType(-1); } catch (std::overflow_error& e) { hadOver2 = true; }
      REQUIRE(hadZero1);
      REQUIRE(hadZero2);
      REQUIRE(hadOver1);
      REQUIRE(hadOver2);
      // operate with int
      SafeInt val(UnderlyingType(-42));
      val /= UnderlyingType(2);
      val.revert();
      REQUIRE(val == UnderlyingType(-42));
      val /= UnderlyingType(2);
      val.commit();
      REQUIRE(val == UnderlyingType(-21));
      // operate with SafeInt
      SafeInt div(UnderlyingType(3));
      val /= div;
      val.revert();
      REQUIRE(val == UnderlyingType(-21));
      val /= div;
      val.commit();
      REQUIRE(val == UnderlyingType(-7));
      // For coverage
      SafeInt valOne(UnderlyingType(1));
      val /= valOne;
      REQUIRE(val == UnderlyingType(-7));
      val /= UnderlyingType(1);
      REQUIRE(val == UnderlyingType(-7));
    }

    SECTION(std::string("SafeInt_t<") + std::to_string(Size) + "> operator%=") {
      SafeInt val(UnderlyingType(-42));
      SafeInt valZero(UnderlyingType(0));
      bool hadZero1 = false;
      bool hadZero2 = false;
      // catch mod by zero
      try { val %= valZero; } catch (std::domain_error& e) { hadZero1 = true; }
      try { val %= UnderlyingType(0); } catch (std::domain_error& e) { hadZero2 = true; }
      REQUIRE(hadZero1);
      REQUIRE(hadZero2);
      // operate with int
      val %= UnderlyingType(9);
      val.revert();
      REQUIRE(val == UnderlyingType(-42));
      val %= UnderlyingType(9);
      val.commit();
      REQUIRE(val == UnderlyingType(-6));
      // operate with SafeInt
      SafeInt mod(UnderlyingType(4));
      val %= mod;
      val.revert();
      REQUIRE(val == UnderlyingType(-6));
      val %= mod;
      val.commit();
      REQUIRE(val == UnderlyingType(-2));
    }

    SECTION(std::string("SafeInt_t<") + std::to_string(Size) + "> operator&=, |=, ^=, <<= and >>= (int)") {
      SafeInt val1(UnderlyingType(0b10101010));
      SafeInt val2(UnderlyingType(0b10101010));
      SafeInt val3(UnderlyingType(0b10101010));
      SafeInt val4(UnderlyingType(0b10101010));
      SafeInt val5(UnderlyingType(0b10101010));
      // bitwise AND
      val1 &= UnderlyingType(0b11110000);
      val1.revert();
      REQUIRE(val1 == UnderlyingType(0b10101010));
      val1 &= UnderlyingType(0b11110000);
      val1.commit();
      REQUIRE(val1 == UnderlyingType(0b10100000));
      // bitwise OR
      val2 |= UnderlyingType(0b11110000);
      val2.revert();
      REQUIRE(val2 == UnderlyingType(0b10101010));
      val2 |= UnderlyingType(0b11110000);
      val2.commit();
      REQUIRE(val2 == UnderlyingType(0b11111010));
      // bitwise XOR
      val3 ^= UnderlyingType(0b11110000);
      val3.revert();
      REQUIRE(val3 == UnderlyingType(0b10101010));
      val3 ^= UnderlyingType(0b11110000);
      val3.commit();
      REQUIRE(val3 == UnderlyingType(0b01011010));
      // bitwise left shift
      val4 <<= 2;
      val4.revert();
      REQUIRE(val4 == UnderlyingType(0b10101010));
      val4 <<= 2;
      val4.commit();
      REQUIRE(val4 == UnderlyingType(0b1010101000));
      // bitwise right shift
      val5 >>= 2;
      val5.revert();
      REQUIRE(val5 == UnderlyingType(0b10101010));
      val5 >>= 2;
      val5.commit();
      REQUIRE(val5 == UnderlyingType((Size == 8) ? 0b11101010 : 0b00101010)); // https://stackoverflow.com/a/22734721
    }

    SECTION(std::string("SafeInt_t<") + std::to_string(Size) + "> operator&=, |=, ^=, <<= and >>= (SafeInt)") {
      SafeInt val1(UnderlyingType(0b10101010));
      SafeInt val2(UnderlyingType(0b10101010));
      SafeInt val3(UnderlyingType(0b10101010));
      SafeInt val4(UnderlyingType(0b10101010));
      SafeInt val5(UnderlyingType(0b10101010));
      SafeInt valOp1(UnderlyingType(0b11110000));
      SafeInt valOp2(UnderlyingType(2));
      // bitwise AND
      val1 &= valOp1;
      val1.revert();
      REQUIRE(val1 == UnderlyingType(0b10101010));
      val1 &= valOp1;
      val1.commit();
      REQUIRE(val1 == UnderlyingType(0b10100000));
      // bitwise OR
      val2 |= valOp1;
      val2.revert();
      REQUIRE(val2 == UnderlyingType(0b10101010));
      val2 |= valOp1;
      val2.commit();
      REQUIRE(val2 == UnderlyingType(0b11111010));
      // bitwise XOR
      val3 ^= valOp1;
      val3.revert();
      REQUIRE(val3 == UnderlyingType(0b10101010));
      val3 ^= valOp1;
      val3.commit();
      REQUIRE(val3 == UnderlyingType(0b01011010));
    }

    SECTION(std::string("SafeInt_t<") + std::to_string(Size) + "> operator++ and -- (pre and post)") {
      SafeInt val(UnderlyingType(-42));
      SafeInt valOver1(std::numeric_limits<UnderlyingType>::max());
      SafeInt valOver2(std::numeric_limits<UnderlyingType>::max());
      SafeInt valUnder1(std::numeric_limits<UnderlyingType>::min());
      SafeInt valUnder2(std::numeric_limits<UnderlyingType>::min());
      bool hadOver1 = false;
      bool hadOver2 = false;
      bool hadUnder1 = false;
      bool hadUnder2 = false;
      // catch over/underflow
      try { ++valOver1; } catch (std::overflow_error& e) { hadOver1 = true; }
      try { valOver2++; } catch (std::overflow_error& e) { hadOver2 = true; }
      try { --valUnder1; } catch (std::underflow_error& e) { hadUnder1 = true; }
      try { valUnder2--; } catch (std::underflow_error& e) { hadUnder2 = true; }
      REQUIRE(hadOver1);
      REQUIRE(hadOver2);
      REQUIRE(hadUnder1);
      REQUIRE(hadUnder2);
      // increment prefix
      REQUIRE(++val == UnderlyingType(-41));
      val.revert();
      REQUIRE(val == UnderlyingType(-42));
      REQUIRE(++val == UnderlyingType(-41));
      val.commit();
      REQUIRE(val == UnderlyingType(-41));
      // increment postfix
      REQUIRE(val++ == UnderlyingType(-41));
      val.revert();
      REQUIRE(val == UnderlyingType(-41));
      REQUIRE(val++ == UnderlyingType(-41));
      val.commit();
      REQUIRE(val == UnderlyingType(-40));
      // decrement prefix
      REQUIRE(--val == UnderlyingType(-41));
      val.revert();
      REQUIRE(val == UnderlyingType(-40));
      REQUIRE(--val == UnderlyingType(-41));
      val.commit();
      REQUIRE(val == UnderlyingType(-41));
      // decrement postfix
      REQUIRE(val-- == UnderlyingType(-41));
      val.revert();
      REQUIRE(val == UnderlyingType(-41));
      REQUIRE(val-- == UnderlyingType(-41));
      val.commit();
      REQUIRE(val == UnderlyingType(-42));
    }
  }
};

TEST_CASE("SafeInt tests", "[unit][safevars][safeint]") {
  SafeIntTester<8>()();
  SafeIntTester<16>()();
  SafeIntTester<32>()();
  SafeIntTester<64>()();

  SafeIntTester<24>()();
  SafeIntTester<40>()();
  SafeIntTester<48>()();
  SafeIntTester<56>()();
  SafeIntTester<72>()();
  SafeIntTester<80>()();
  SafeIntTester<88>()();
  SafeIntTester<96>()();
  SafeIntTester<104>()();
  SafeIntTester<112>()();
  SafeIntTester<120>()();
  SafeIntTester<128>()();
  SafeIntTester<136>()();
  SafeIntTester<144>()();
  SafeIntTester<152>()();
  SafeIntTester<160>()();
  SafeIntTester<168>()();
  SafeIntTester<176>()();
  SafeIntTester<184>()();
  SafeIntTester<192>()();
  SafeIntTester<200>()();
  SafeIntTester<208>()();
  SafeIntTester<216>()();
  SafeIntTester<224>()();
  SafeIntTester<232>()();
  SafeIntTester<240>()();
  SafeIntTester<248>()();
  SafeIntTester<256>()();
}

