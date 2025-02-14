/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#include "libs/catch2/catch_amalgamated.hpp"

#include "contract/variables/safeuint.h"

#include "utils/uintconv.h" // UnderlyingType

// Helper struct and templates for testing multiple types
template <int Size> struct UnderlyingType;

// C++ native types
template <> struct UnderlyingType<8> { using type = uint8_t; };
template <> struct UnderlyingType<16> { using type = uint16_t; };
template <> struct UnderlyingType<32> { using type = uint32_t; };
template <> struct UnderlyingType<64> { using type = uint64_t; };

// Boost types
template<> struct UnderlyingType<24> { using type = uint24_t; };
template<> struct UnderlyingType<40> { using type = uint40_t; };
template<> struct UnderlyingType<48> { using type = uint48_t; };
template<> struct UnderlyingType<56> { using type = uint56_t; };
template<> struct UnderlyingType<72> { using type = uint72_t; };
template<> struct UnderlyingType<80> { using type = uint80_t; };
template<> struct UnderlyingType<88> { using type = uint88_t; };
template<> struct UnderlyingType<96> { using type = uint96_t; };
template<> struct UnderlyingType<104> { using type = uint104_t; };
template<> struct UnderlyingType<112> { using type = uint112_t; };
template<> struct UnderlyingType<120> { using type = uint120_t; };
template<> struct UnderlyingType<128> { using type = uint128_t; };
template<> struct UnderlyingType<136> { using type = uint136_t; };
template<> struct UnderlyingType<144> { using type = uint144_t; };
template<> struct UnderlyingType<152> { using type = uint152_t; };
template<> struct UnderlyingType<160> { using type = uint160_t; };
template<> struct UnderlyingType<168> { using type = uint168_t; };
template<> struct UnderlyingType<176> { using type = uint176_t; };
template<> struct UnderlyingType<184> { using type = uint184_t; };
template<> struct UnderlyingType<192> { using type = uint192_t; };
template<> struct UnderlyingType<200> { using type = uint200_t; };
template<> struct UnderlyingType<208> { using type = uint208_t; };
template<> struct UnderlyingType<216> { using type = uint216_t; };
template<> struct UnderlyingType<224> { using type = uint224_t; };
template<> struct UnderlyingType<232> { using type = uint232_t; };
template<> struct UnderlyingType<240> { using type = uint240_t; };
template<> struct UnderlyingType<248> { using type = uint248_t; };
template<> struct UnderlyingType<256> { using type = uint256_t; };

// Helper template for testing all types
template <int Size> struct SafeUintTester {
  using SafeUint = SafeUint_t<Size>;
  using UnderlyingType = typename UnderlyingType<Size>::type;

  void operator()() const {
    SECTION(std::string("SafeUint_t<") + std::to_string(Size) + "> underlying type") {
      SafeUint val;
      REQUIRE(std::is_same_v<std::decay_t<decltype(val.get())>, std::decay_t<UnderlyingType>>);
    }

    SECTION(std::string("SafeUint_t<") + std::to_string(Size) + "> constructor") {
      SafeUint val(UnderlyingType(42));
      SafeUint copyVal(val);
      REQUIRE(val == UnderlyingType(42));
      REQUIRE(copyVal == val);
    }

    SECTION(std::string("SafeUint_t<") + std::to_string(Size) + "> operator+") {
      SafeUint val(UnderlyingType(42));
      SafeUint valOver(std::numeric_limits<UnderlyingType>::max());
      SafeUint valUnder(std::numeric_limits<UnderlyingType>::min());
      bool hadOver1 = false;
      bool hadOver2 = false;
      bool hadOver3 = false;
      bool hadUnder = false;
      // catch over/underflow
      try { valOver = valOver + SafeUint(UnderlyingType(1)); } catch (std::overflow_error& e) { hadOver1 = true; }
      try { valOver = valOver + UnderlyingType(1); } catch (std::overflow_error& e) { hadOver2 = true; }
      try { valOver = valOver + int(1); } catch (std::overflow_error& e) { hadOver3 = true; }
      try { valUnder = valUnder + int(-1); } catch (std::underflow_error& e) { hadUnder = true; }
      REQUIRE(hadOver1);
      REQUIRE(hadOver2);
      REQUIRE(hadOver3);
      REQUIRE(hadUnder);
      // operate with uint
      val = val + UnderlyingType(5);
      val.revert();
      REQUIRE(val == UnderlyingType(42));
      val = val + UnderlyingType(5);
      val.commit();
      REQUIRE(val == UnderlyingType(47));
      // operate with int
      val = val + int(-5);
      val.revert();
      REQUIRE(val == UnderlyingType(47));
      val = val + int(-5);
      val.commit();
      REQUIRE(val == UnderlyingType(42));
      // operate with SafeUint
      SafeUint sum(UnderlyingType(10));
      val = val + sum;
      val.revert();
      REQUIRE(val == UnderlyingType(42));
      val = val + sum;
      val.commit();
      REQUIRE(val == UnderlyingType(52));
    }

    SECTION(std::string("SafeUint_t<") + std::to_string(Size) + "> operator-") {
      SafeUint val(UnderlyingType(42));
      SafeUint valOver(std::numeric_limits<UnderlyingType>::max());
      SafeUint valUnder(std::numeric_limits<UnderlyingType>::min());
      bool hadUnder1 = false;
      bool hadUnder2 = false;
      bool hadUnder3 = false;
      bool hadOver = false;
      // catch over/underflow
      try { valUnder = valUnder - SafeUint(UnderlyingType(1)); } catch (std::underflow_error& e) { hadUnder1 = true; }
      try { valUnder = valUnder - UnderlyingType(1); } catch (std::underflow_error& e) { hadUnder2 = true; }
      try { valUnder = valUnder - int(1); } catch (std::underflow_error& e) { hadUnder3 = true; }
      try { valOver = valOver - int(-1); } catch (std::overflow_error& e) { hadOver = true; }
      REQUIRE(hadUnder1);
      REQUIRE(hadUnder2);
      REQUIRE(hadUnder3);
      REQUIRE(hadOver);
      // operate with uint
      val = val - UnderlyingType(5);
      val.revert();
      REQUIRE(val == UnderlyingType(42));
      val = val - UnderlyingType(5);
      val.commit();
      REQUIRE(val == UnderlyingType(37));
      // operate with int
      val = val - int(-5);
      val.revert();
      REQUIRE(val == UnderlyingType(37));
      val = val - int(-5);
      val.commit();
      REQUIRE(val == UnderlyingType(42));
      // operate with SafeUint
      SafeUint sub(UnderlyingType(10));
      val = val - sub;
      val.revert();
      REQUIRE(val == UnderlyingType(42));
      val = val - sub;
      val.commit();
      REQUIRE(val == UnderlyingType(32));
    }

    SECTION(std::string("SafeUint_t<") + std::to_string(Size) + "> operator*") {
      SafeUint val(UnderlyingType(42));
      SafeUint valZero1(UnderlyingType(42));
      SafeUint valZero2(UnderlyingType(0));
      SafeUint valOver(std::numeric_limits<UnderlyingType>::max());
      SafeUint valUnder(1);
      bool hadZero1 = false;
      bool hadZero2 = false;
      bool hadZero3 = false;
      bool hadZero4 = false;
      bool hadOver1 = false;
      bool hadOver2 = false;
      bool hadUnder = false;
      // catch over/underflow and mul by zero
      try { valZero1 = valZero1 * UnderlyingType(0); } catch (std::domain_error& e) { hadZero1 = true; }
      try { valZero2 = valZero2 * UnderlyingType(10); } catch (std::domain_error& e) { hadZero2 = true; }
      try { valZero2 = valZero2 * UnderlyingType(0); } catch (std::domain_error& e) { hadZero3 = true; }
      try { valZero2 = valZero2 * int(0); } catch (std::domain_error& e) { hadZero4 = true; }
      try { valOver = valOver * UnderlyingType(2); } catch (std::overflow_error& e) { hadOver1 = true; }
      try { valOver = valOver * valOver; } catch (std::overflow_error& e) { hadOver2 = true; }
      try { valUnder = valUnder * int(-1); } catch (std::underflow_error& e) { hadUnder = true; }
      REQUIRE(hadZero1);
      REQUIRE(hadZero2);
      REQUIRE(hadZero3);
      REQUIRE(hadZero4);
      REQUIRE(hadOver1);
      REQUIRE(hadOver2);
      REQUIRE(hadUnder);
      // operate with uint
      val = val * UnderlyingType(2);
      val.revert();
      REQUIRE(val == UnderlyingType(42));
      val = val * UnderlyingType(2);
      val.commit();
      REQUIRE(val == UnderlyingType(84));
      val = UnderlyingType(42); val.commit(); // reset to ensure fit into minimum (SafeUint<8>)
      // operate with int
      val = val * int(2);
      val.revert();
      REQUIRE(val == UnderlyingType(42));
      val = val * int(2);
      val.commit();
      REQUIRE(val == UnderlyingType(84));
      val = UnderlyingType(42); val.commit(); // reset to ensure fit into minimum (SafeUint<8>)
      // operate with SafeUint
      SafeUint mul(UnderlyingType(2));
      val = val * mul;
      val.revert();
      REQUIRE(val == UnderlyingType(42));
      val = val * mul;
      val.commit();
      REQUIRE(val == UnderlyingType(84));
    }

    SECTION(std::string("SafeUint_t<") + std::to_string(Size) + "> operator/") {
      SafeUint val(UnderlyingType(42));
      SafeUint valZero1(UnderlyingType(42));
      SafeUint valZero2(UnderlyingType(0));
      SafeUint valUnder(1);
      bool hadZero1 = false;
      bool hadZero2 = false;
      bool hadZero3 = false;
      bool hadZero4 = false;
      bool hadZero5 = false;
      bool hadUnder = false;
      // catch underflow and div by zero
      try { valZero1 = valZero1 / UnderlyingType(0); } catch (std::domain_error& e) { hadZero1 = true; }
      try { valZero2 = valZero2 / UnderlyingType(2); } catch (std::domain_error& e) { hadZero2 = true; }
      try { valZero1 = valZero1 / valZero2; } catch (std::domain_error& e) { hadZero3 = true; }
      try { valZero2 = valZero2 / valZero1; } catch (std::domain_error& e) { hadZero4 = true; }
      try { valZero1 = valZero1 / int(0); } catch (std::domain_error& e) { hadZero5 = true; }
      try { valUnder = valUnder / int(-1); } catch (std::domain_error& e) { hadUnder = true; }
      REQUIRE(hadZero1);
      REQUIRE(hadZero2);
      REQUIRE(hadZero3);
      REQUIRE(hadZero4);
      REQUIRE(hadZero5);
      REQUIRE(hadUnder);
      // operate with uint
      val = val / UnderlyingType(2);
      val.revert();
      REQUIRE(val == UnderlyingType(42));
      val = val / UnderlyingType(2);
      val.commit();
      REQUIRE(val == UnderlyingType(21));
      // operate with int
      val = val / int(3);
      val.revert();
      REQUIRE(val == UnderlyingType(21));
      val = val / int(3);
      val.commit();
      REQUIRE(val == UnderlyingType(7));
      // operate with SafeUint
      SafeUint div(UnderlyingType(3));
      val = val / div;
      val.revert();
      REQUIRE(val == UnderlyingType(7));
      val = val / div;
      val.commit();
      REQUIRE(val == UnderlyingType(2)); // % 1
    }

    SECTION(std::string("SafeUint_t<") + std::to_string(Size) + "> operator%") {
      SafeUint val(UnderlyingType(42));
      SafeUint valZero1(UnderlyingType(42));
      SafeUint valZero2(UnderlyingType(0));
      bool hadZero1 = false;
      bool hadZero2 = false;
      bool hadZero3 = false;
      bool hadZero4 = false;
      bool hadZero5 = false;
      bool hadZero6 = false;
      // catch mod by zero
      try { valZero1 = valZero1 % UnderlyingType(0); } catch (std::domain_error& e) { hadZero1 = true; }
      try { valZero2 = valZero2 % UnderlyingType(2); } catch (std::domain_error& e) { hadZero2 = true; }
      try { valZero1 = valZero1 % valZero2; } catch (std::domain_error& e) { hadZero3 = true; }
      try { valZero2 = valZero2 % valZero1; } catch (std::domain_error& e) { hadZero4 = true; }
      try { valZero1 = valZero1 % int(0); } catch (std::domain_error& e) { hadZero5 = true; }
      try { valZero2 = valZero2 % int(2); } catch (std::domain_error& e) { hadZero6 = true; }
      REQUIRE(hadZero1);
      REQUIRE(hadZero2);
      REQUIRE(hadZero3);
      REQUIRE(hadZero4);
      REQUIRE(hadZero5);
      REQUIRE(hadZero6);
      // operate with uint
      val = val % UnderlyingType(15);
      val.revert();
      REQUIRE(val == UnderlyingType(42));
      val = val % UnderlyingType(15);
      val.commit();
      REQUIRE(val == UnderlyingType(12));
      // operate with int
      val = val % int(8);
      val.revert();
      REQUIRE(val == UnderlyingType(12));
      val = val % int(8);
      val.commit();
      REQUIRE(val == UnderlyingType(4));
      // operate with SafeUint
      SafeUint mod(UnderlyingType(3));
      val = val % mod;
      val.revert();
      REQUIRE(val == UnderlyingType(4));
      val = val % mod;
      val.commit();
      REQUIRE(val == UnderlyingType(1));
    }

    SECTION(std::string("SafeUint_t<") + std::to_string(Size) + "> operator&, | and ^ (uint)") {
      SafeUint val1(UnderlyingType(0b10101010));
      SafeUint val2(UnderlyingType(0b10101010));
      SafeUint val3(UnderlyingType(0b10101010));
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

    SECTION(std::string("SafeUint_t<") + std::to_string(Size) + "> operator&, | and ^ (int)") {
      SafeUint val1(UnderlyingType(0b10101010));
      SafeUint val2(UnderlyingType(0b10101010));
      SafeUint val3(UnderlyingType(0b10101010));
      SafeUint valNeg1(UnderlyingType(0b10101010));
      SafeUint valNeg2(UnderlyingType(0b10101010));
      SafeUint valNeg3(UnderlyingType(0b10101010));
      bool hadNeg1 = false;
      bool hadNeg2 = false;
      bool hadNeg3 = false;
      // check for negative bitwise
      try { valNeg1 = valNeg1 & int(-1); } catch (std::domain_error& e) { hadNeg1 = true; }
      try { valNeg2 = valNeg2 | int(-1); } catch (std::domain_error& e) { hadNeg2 = true; }
      try { valNeg3 = valNeg3 ^ int(-1); } catch (std::domain_error& e) { hadNeg3 = true; }
      REQUIRE(hadNeg1);
      REQUIRE(hadNeg2);
      REQUIRE(hadNeg3);
      // bitwise AND
      val1 = val1 & int(0b11110000);
      val1.revert();
      REQUIRE(val1 == UnderlyingType(0b10101010));
      val1 = val1 & int(0b11110000);
      val1.commit();
      REQUIRE(val1 == UnderlyingType(0b10100000));
      // bitwise OR
      val2 = val2 | int(0b11110000);
      val2.revert();
      REQUIRE(val2 == UnderlyingType(0b10101010));
      val2 = val2 | int(0b11110000);
      val2.commit();
      REQUIRE(val2 == UnderlyingType(0b11111010));
      // bitwise XOR
      val3 = val3 ^ int(0b11110000);
      val3.revert();
      REQUIRE(val3 == UnderlyingType(0b10101010));
      val3 = val3 ^ int(0b11110000);
      val3.commit();
      REQUIRE(val3 == UnderlyingType(0b01011010));
    }

    SECTION(std::string("SafeUint_t<") + std::to_string(Size) + "> operator&, | and ^ (SafeUint)") {
      SafeUint val1(UnderlyingType(0b10101010));
      SafeUint val2(UnderlyingType(0b10101010));
      SafeUint val3(UnderlyingType(0b10101010));
      SafeUint val4(UnderlyingType(0b10101010));
      SafeUint val5(UnderlyingType(0b10101010));
      SafeUint valOp(UnderlyingType(0b11110000));
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

    SECTION(std::string("SafeUint_t<") + std::to_string(Size) + "> operator<< and >>") {
      SafeUint val1(UnderlyingType(0b10101010));
      SafeUint val2(UnderlyingType(0b10101010));
      // bitwise left shift
      val1 = val1 << 2;
      val1.revert();
      REQUIRE(val1 == UnderlyingType(0b10101010));
      val1 = val1 << 2;
      val1.commit();
      REQUIRE(val1 == UnderlyingType(0b1010101000));
      // bitwise right shift
      val2 = val2 >> 2;
      val2.revert();
      REQUIRE(val2 == UnderlyingType(0b10101010));
      val2 = val2 >> 2;
      val2.commit();
      REQUIRE(val2 == UnderlyingType(0b00101010));
    }

    SECTION(std::string("SafeUint_t<") + std::to_string(Size) + "> operator!, && and ||") {
      SafeUint val(UnderlyingType(0));
      // logical NOT
      val = !val;
      val.revert();
      REQUIRE(val == UnderlyingType(0));
      val = !val;
      val.commit();
      REQUIRE(val == UnderlyingType(1));
      // logical AND (uint)
      val = val && UnderlyingType(0);
      val.revert();
      REQUIRE(val == UnderlyingType(1));
      val = val && UnderlyingType(0);
      val.commit();
      REQUIRE(val == UnderlyingType(0));
      // logical OR (uint)
      val = val || UnderlyingType(1);
      val.revert();
      REQUIRE(val == UnderlyingType(0));
      val = val || UnderlyingType(1);
      val.commit();
      REQUIRE(val == UnderlyingType(1));
      // logical AND (SafeUint)
      val = val && SafeUint(UnderlyingType(0));
      val.revert();
      REQUIRE(val == UnderlyingType(1));
      val = val && SafeUint(UnderlyingType(0));
      val.commit();
      REQUIRE(val == UnderlyingType(0));
      // logical OR (SafeUint)
      val = val || SafeUint(UnderlyingType(1));
      val.revert();
      REQUIRE(val == UnderlyingType(0));
      val = val || SafeUint(UnderlyingType(1));
      val.commit();
      REQUIRE(val == UnderlyingType(1));
      // For coverage
      SafeUint val0(UnderlyingType(0));
      SafeUint val1(UnderlyingType(1));
      REQUIRE_FALSE((val0 && val0));
      REQUIRE_FALSE((val0 && val1));
      REQUIRE_FALSE((val1 && val0));
      REQUIRE((val1 && val1));
      REQUIRE_FALSE((val0 && UnderlyingType(0)));
      REQUIRE_FALSE((val0 && UnderlyingType(1)));
      REQUIRE_FALSE((val1 && UnderlyingType(0)));
      REQUIRE((val1 && UnderlyingType(1)));
      REQUIRE_FALSE((val0 || val0));
      REQUIRE((val0 || val1));
      REQUIRE((val1 || val0));
      REQUIRE((val1 || val1));
      REQUIRE_FALSE((val0 || UnderlyingType(0)));
      REQUIRE((val0 || UnderlyingType(1)));
      REQUIRE((val1 || UnderlyingType(0)));
      REQUIRE((val1 || UnderlyingType(1)));
    }

    SECTION(std::string("SafeUint_t<") + std::to_string(Size) + "> operator== and !=") {
      SafeUint valA1(UnderlyingType(42));
      SafeUint valA2(UnderlyingType(42));
      SafeUint valB1(UnderlyingType(24));
      SafeUint valB2(UnderlyingType(24));
      int valIntA1(42);
      int valIntA2(-42);
      int valIntB1(24);
      int valIntB2(-24);
      // compare uint
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
      // compare int
      REQUIRE(valA1 == valIntA1);
      REQUIRE(!(valA1 == valIntA2));
      REQUIRE(valA1 != valB1);
      REQUIRE(valA1 != valB2);
      REQUIRE(valA2 == valIntA1);
      REQUIRE(!(valA2 == valIntA2));
      REQUIRE(valA2 != valB1);
      REQUIRE(valA2 != valB2);
      REQUIRE(valB1 != valA1);
      REQUIRE(valB1 != valA2);
      REQUIRE(valB1 == valIntB1);
      REQUIRE(!(valB1 == valIntB2));
      REQUIRE(valB2 != valA1);
      REQUIRE(valB2 != valA2);
      REQUIRE(valB2 == valIntB1);
      REQUIRE(!(valB2 == valIntB2));
      // compare SafeUint
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

    SECTION(std::string("SafeUint_t<") + std::to_string(Size) + "> operator<, >, <= and >=") {
      SafeUint valA1(UnderlyingType(42));
      SafeUint valA2(UnderlyingType(42));
      SafeUint valB1(UnderlyingType(43));
      SafeUint valB2(UnderlyingType(43));
      int valIntA1(42);
      int valIntA2(-42);
      int valIntB1(43);
      int valIntB2(-43);
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
      // compare int
      REQUIRE(!(valA1 < valIntA1));
      REQUIRE(valA1 <= valIntA1);
      REQUIRE(valA1 >= valIntA1);
      REQUIRE(!(valA1 > valIntA1));
      REQUIRE(!(valA1 < valIntA2));
      REQUIRE(!(valA1 <= valIntA2));
      REQUIRE(valA1 >= valIntA2);
      REQUIRE(valA1 > valIntA2);
      REQUIRE(!(valB1 < valIntB1));
      REQUIRE(valB1 <= valIntB1);
      REQUIRE(valB1 >= valIntB1);
      REQUIRE(!(valB1 > valIntB1));
      REQUIRE(!(valB1 < valIntB2));
      REQUIRE(!(valB1 <= valIntB2));
      REQUIRE(valB1 >= valIntB2);
      REQUIRE(valB1 > valIntB2);
      // compare SafeUint
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

    SECTION(std::string("SafeUint_t<") + std::to_string(Size) + "> operator=") {
      SafeUint val(UnderlyingType(42));
      SafeUint valNeg(UnderlyingType(42));
      bool hadNeg = false;
      // check for negative assign
      try { valNeg = int(-1); } catch (std::domain_error& e) { hadNeg = true; }
      REQUIRE(hadNeg);
      // assign uint
      val = UnderlyingType(24);
      val.revert();
      REQUIRE(val == UnderlyingType(42));
      val = UnderlyingType(24);
      val.commit();
      REQUIRE(val == UnderlyingType(24));
      // assign int
      val = int(42);
      val.revert();
      REQUIRE(val == UnderlyingType(24));
      val = int(42);
      val.commit();
      REQUIRE(val == UnderlyingType(42));
      // assign SafeUint
      SafeUint val2(UnderlyingType(24));
      val = val2;
      val.revert();
      REQUIRE(val == UnderlyingType(42));
      val = val2;
      val.commit();
      REQUIRE(val == UnderlyingType(24));
    }

    SECTION(std::string("SafeUint_t<") + std::to_string(Size) + "> operator+=") {
      SafeUint val(UnderlyingType(42));
      SafeUint valOver(std::numeric_limits<UnderlyingType>::max());
      SafeUint valUnder(std::numeric_limits<UnderlyingType>::min());
      bool hadOver1 = false;
      bool hadOver2 = false;
      bool hadOver3 = false;
      bool hadUnder = false;
      // catch over/underflow
      try { valOver += SafeUint(UnderlyingType(1)); } catch (std::overflow_error& e) { hadOver1 = true; }
      try { valOver += UnderlyingType(1); } catch (std::overflow_error& e) { hadOver2 = true; }
      try { valOver += int(1); } catch (std::overflow_error& e) { hadOver3 = true; }
      try { valUnder += int(-1); } catch (std::underflow_error& e) { hadUnder = true; }
      REQUIRE(hadOver1);
      REQUIRE(hadOver2);
      REQUIRE(hadOver3);
      REQUIRE(hadUnder);
      // operate with uint
      val += UnderlyingType(5);
      val.revert();
      REQUIRE(val == UnderlyingType(42));
      val += UnderlyingType(5);
      val.commit();
      REQUIRE(val == UnderlyingType(47));
      // operate with int
      val += int(-5);
      val.revert();
      REQUIRE(val == UnderlyingType(47));
      val += int(-5);
      val.commit();
      REQUIRE(val == UnderlyingType(42));
      // operate with SafeUint
      SafeUint sum(UnderlyingType(10));
      val += sum;
      val.revert();
      REQUIRE(val == UnderlyingType(42));
      val += sum;
      val.commit();
      REQUIRE(val == UnderlyingType(52));
    }

    SECTION(std::string("SafeUint_t<") + std::to_string(Size) + "> operator-=") {
      SafeUint val(UnderlyingType(42));
      SafeUint valOver(std::numeric_limits<UnderlyingType>::max());
      SafeUint valUnder(std::numeric_limits<UnderlyingType>::min());
      bool hadUnder1 = false;
      bool hadUnder2 = false;
      bool hadUnder3 = false;
      bool hadOver = false;
      // catch over/underflow
      try { valUnder -= SafeUint(UnderlyingType(1)); } catch (std::underflow_error& e) { hadUnder1 = true; }
      try { valUnder -= UnderlyingType(1); } catch (std::underflow_error& e) { hadUnder2 = true; }
      try { valUnder -= int(1); } catch (std::underflow_error& e) { hadUnder3 = true; }
      try { valOver -= int(-1); } catch (std::overflow_error& e) { hadOver = true; }
      REQUIRE(hadUnder1);
      REQUIRE(hadUnder2);
      REQUIRE(hadUnder3);
      REQUIRE(hadOver);
      // operate with uint
      val -= UnderlyingType(5);
      val.revert();
      REQUIRE(val == UnderlyingType(42));
      val -= UnderlyingType(5);
      val.commit();
      REQUIRE(val == UnderlyingType(37));
      // operate with int
      val -= int(-5);
      val.revert();
      REQUIRE(val == UnderlyingType(37));
      val -= int(-5);
      val.commit();
      REQUIRE(val == UnderlyingType(42));
      // operate with SafeUint
      SafeUint sub(UnderlyingType(10));
      val -= sub;
      val.revert();
      REQUIRE(val == UnderlyingType(42));
      val -= sub;
      val.commit();
      REQUIRE(val == UnderlyingType(32));
    }

    SECTION(std::string("SafeUint_t<") + std::to_string(Size) + "> operator*=") {
      SafeUint val(UnderlyingType(42));
      SafeUint valZero1(UnderlyingType(42));
      SafeUint valZero2(UnderlyingType(0));
      SafeUint valOver(std::numeric_limits<UnderlyingType>::max());
      SafeUint valUnder(1);
      bool hadZero1 = false;
      bool hadZero2 = false;
      bool hadZero3 = false;
      bool hadZero4 = false;
      bool hadOver1 = false;
      bool hadOver2 = false;
      bool hadUnder = false;
      // catch over/underflow and mul by zero
      try { valZero1 *= UnderlyingType(0); } catch (std::domain_error& e) { hadZero1 = true; }
      try { valZero2 *= UnderlyingType(10); } catch (std::domain_error& e) { hadZero2 = true; }
      try { valZero2 *= UnderlyingType(0); } catch (std::domain_error& e) { hadZero3 = true; }
      try { valZero2 *= int(0); } catch (std::domain_error& e) { hadZero4 = true; }
      try { valOver *= UnderlyingType(2); } catch (std::overflow_error& e) { hadOver1 = true; }
      try { valOver *= valOver; } catch (std::overflow_error& e) { hadOver2 = true; }
      try { valUnder *= int(-1); } catch (std::underflow_error& e) { hadUnder = true; }
      REQUIRE(hadZero1);
      REQUIRE(hadZero2);
      REQUIRE(hadZero3);
      REQUIRE(hadZero4);
      REQUIRE(hadOver1);
      REQUIRE(hadOver2);
      REQUIRE(hadUnder);
      // operate with uint
      val *= UnderlyingType(2);
      val.revert();
      REQUIRE(val == UnderlyingType(42));
      val *= UnderlyingType(2);
      val.commit();
      REQUIRE(val == UnderlyingType(84));
      val = UnderlyingType(42); val.commit(); // reset to ensure fit into minimum (SafeUint<8>)
      // operate with int
      val *= int(2);
      val.revert();
      REQUIRE(val == UnderlyingType(42));
      val *= int(2);
      val.commit();
      REQUIRE(val == UnderlyingType(84));
      val = UnderlyingType(42); val.commit(); // reset to ensure fit into minimum (SafeUint<8>)
      // operate with SafeUint
      SafeUint mul(UnderlyingType(2));
      val *= mul;
      val.revert();
      REQUIRE(val == UnderlyingType(42));
      val *= mul;
      val.commit();
      REQUIRE(val == UnderlyingType(84));
    }

    SECTION(std::string("SafeUint_t<") + std::to_string(Size) + "> operator/=") {
      SafeUint val(UnderlyingType(42));
      SafeUint valZero1(UnderlyingType(42));
      SafeUint valZero2(UnderlyingType(0));
      SafeUint valUnder(1);
      bool hadZero1 = false;
      bool hadZero2 = false;
      bool hadZero3 = false;
      bool hadZero4 = false;
      bool hadZero5 = false;
      bool hadUnder = false;
      // catch underflow and div by zero
      try { valZero1 /= UnderlyingType(0); } catch (std::domain_error& e) { hadZero1 = true; }
      try { valZero2 /= UnderlyingType(2); } catch (std::domain_error& e) { hadZero2 = true; }
      try { valZero1 /= valZero2; } catch (std::domain_error& e) { hadZero3 = true; }
      try { valZero2 /= valZero1; } catch (std::domain_error& e) { hadZero4 = true; }
      try { valZero1 /= int(0); } catch (std::domain_error& e) { hadZero5 = true; }
      try { valUnder /= int(-1); } catch (std::domain_error& e) { hadUnder = true; }
      REQUIRE(hadZero1);
      REQUIRE(hadZero2);
      REQUIRE(hadZero3);
      REQUIRE(hadZero4);
      REQUIRE(hadZero5);
      REQUIRE(hadUnder);
      // operate with uint
      val /= UnderlyingType(2);
      val.revert();
      REQUIRE(val == UnderlyingType(42));
      val /= UnderlyingType(2);
      val.commit();
      REQUIRE(val == UnderlyingType(21));
      // operate with int
      val /= int(3);
      val.revert();
      REQUIRE(val == UnderlyingType(21));
      val /= int(3);
      val.commit();
      REQUIRE(val == UnderlyingType(7));
      // operate with SafeUint
      SafeUint div(UnderlyingType(3));
      val /= div;
      val.revert();
      REQUIRE(val == UnderlyingType(7));
      val /= div;
      val.commit();
      REQUIRE(val == UnderlyingType(2)); // % 1
    }

    SECTION(std::string("SafeUint_t<") + std::to_string(Size) + "> operator%=") {
      SafeUint val(UnderlyingType(42));
      SafeUint valZero1(UnderlyingType(42));
      SafeUint valZero2(UnderlyingType(0));
      bool hadZero1 = false;
      bool hadZero2 = false;
      bool hadZero3 = false;
      bool hadZero4 = false;
      bool hadZero5 = false;
      bool hadZero6 = false;
      // catch mod by zero
      try { valZero1 %= UnderlyingType(0); } catch (std::domain_error& e) { hadZero1 = true; }
      try { valZero2 %= UnderlyingType(2); } catch (std::domain_error& e) { hadZero2 = true; }
      try { valZero1 %= valZero2; } catch (std::domain_error& e) { hadZero3 = true; }
      try { valZero2 %= valZero1; } catch (std::domain_error& e) { hadZero4 = true; }
      try { valZero1 %= int(0); } catch (std::domain_error& e) { hadZero5 = true; }
      try { valZero2 %= int(2); } catch (std::domain_error& e) { hadZero6 = true; }
      REQUIRE(hadZero1);
      REQUIRE(hadZero2);
      REQUIRE(hadZero3);
      REQUIRE(hadZero4);
      REQUIRE(hadZero5);
      REQUIRE(hadZero6);
      // operate with uint
      val %= UnderlyingType(15);
      val.revert();
      REQUIRE(val == UnderlyingType(42));
      val %= UnderlyingType(15);
      val.commit();
      REQUIRE(val == UnderlyingType(12));
      // operate with int
      val %= int(8);
      val.revert();
      REQUIRE(val == UnderlyingType(12));
      val %= int(8);
      val.commit();
      REQUIRE(val == UnderlyingType(4));
      // operate with SafeUint
      SafeUint mod(UnderlyingType(3));
      val %= mod;
      val.revert();
      REQUIRE(val == UnderlyingType(4));
      val %= mod;
      val.commit();
      REQUIRE(val == UnderlyingType(1));
    }

    SECTION(std::string("SafeUint_t<") + std::to_string(Size) + "> operator&=, |= and ^= (uint)") {
      SafeUint val1(UnderlyingType(0b10101010));
      SafeUint val2(UnderlyingType(0b10101010));
      SafeUint val3(UnderlyingType(0b10101010));
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
    }

    SECTION(std::string("SafeUint_t<") + std::to_string(Size) + "> operator&=, |= and ^= (int)") {
      SafeUint val1(UnderlyingType(0b10101010));
      SafeUint val2(UnderlyingType(0b10101010));
      SafeUint val3(UnderlyingType(0b10101010));
      SafeUint valNeg1(UnderlyingType(0b10101010));
      SafeUint valNeg2(UnderlyingType(0b10101010));
      SafeUint valNeg3(UnderlyingType(0b10101010));
      bool hadNeg1 = false;
      bool hadNeg2 = false;
      bool hadNeg3 = false;
      // check for negative bitwise
      try { valNeg1 &= int(-1); } catch (std::domain_error& e) { hadNeg1 = true; }
      try { valNeg2 |= int(-1); } catch (std::domain_error& e) { hadNeg2 = true; }
      try { valNeg3 ^= int(-1); } catch (std::domain_error& e) { hadNeg3 = true; }
      REQUIRE(hadNeg1);
      REQUIRE(hadNeg2);
      REQUIRE(hadNeg3);
      // bitwise AND
      val1 &= int(0b11110000);
      val1.revert();
      REQUIRE(val1 == UnderlyingType(0b10101010));
      val1 &= int(0b11110000);
      val1.commit();
      REQUIRE(val1 == UnderlyingType(0b10100000));
      // bitwise OR
      val2 |= int(0b11110000);
      val2.revert();
      REQUIRE(val2 == UnderlyingType(0b10101010));
      val2 |= int(0b11110000);
      val2.commit();
      REQUIRE(val2 == UnderlyingType(0b11111010));
      // bitwise XOR
      val3 ^= int(0b11110000);
      val3.revert();
      REQUIRE(val3 == UnderlyingType(0b10101010));
      val3 ^= int(0b11110000);
      val3.commit();
      REQUIRE(val3 == UnderlyingType(0b01011010));
    }

    SECTION(std::string("SafeUint_t<") + std::to_string(Size) + "> operator&=, |= and ^= (SafeUint)") {
      SafeUint val1(UnderlyingType(0b10101010));
      SafeUint val2(UnderlyingType(0b10101010));
      SafeUint val3(UnderlyingType(0b10101010));
      SafeUint val4(UnderlyingType(0b10101010));
      SafeUint val5(UnderlyingType(0b10101010));
      SafeUint valOp(UnderlyingType(0b11110000));
      // bitwise AND
      val1 &= valOp;
      val1.revert();
      REQUIRE(val1 == UnderlyingType(0b10101010));
      val1 &= valOp;
      val1.commit();
      REQUIRE(val1 == UnderlyingType(0b10100000));
      // bitwise OR
      val2 |= valOp;
      val2.revert();
      REQUIRE(val2 == UnderlyingType(0b10101010));
      val2 |= valOp;
      val2.commit();
      REQUIRE(val2 == UnderlyingType(0b11111010));
      // bitwise XOR
      val3 ^= valOp;
      val3.revert();
      REQUIRE(val3 == UnderlyingType(0b10101010));
      val3 ^= valOp;
      val3.commit();
      REQUIRE(val3 == UnderlyingType(0b01011010));
    }

    SECTION(std::string("SafeUint_t<") + std::to_string(Size) + "> operator<<= and >>=") {
      SafeUint val1(UnderlyingType(0b10101010));
      SafeUint val2(UnderlyingType(0b10101010));
      // bitwise left shift
      val1 <<= 2;
      val1.revert();
      REQUIRE(val1 == UnderlyingType(0b10101010));
      val1 <<= 2;
      val1.commit();
      REQUIRE(val1 == UnderlyingType(0b1010101000));
      // bitwise right shift
      val2 >>= 2;
      val2.revert();
      REQUIRE(val2 == UnderlyingType(0b10101010));
      val2 >>= 2;
      val2.commit();
      REQUIRE(val2 == UnderlyingType(0b00101010));
    }

    SECTION(std::string("SafeUint_t<") + std::to_string(Size) + "> operator++ and -- (pre and post)") {
      SafeUint val(UnderlyingType(42));
      SafeUint valOver1(std::numeric_limits<UnderlyingType>::max());
      SafeUint valOver2(std::numeric_limits<UnderlyingType>::max());
      SafeUint valUnder1(std::numeric_limits<UnderlyingType>::min());
      SafeUint valUnder2(std::numeric_limits<UnderlyingType>::min());
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
      REQUIRE(++val == UnderlyingType(43));
      val.revert();
      REQUIRE(val == UnderlyingType(42));
      REQUIRE(++val == UnderlyingType(43));
      val.commit();
      REQUIRE(val == UnderlyingType(43));
      // increment postfix
      REQUIRE(val++ == UnderlyingType(43));
      val.revert();
      REQUIRE(val == UnderlyingType(43));
      REQUIRE(val++ == UnderlyingType(43));
      val.commit();
      REQUIRE(val == UnderlyingType(44));
      // decrement prefix
      REQUIRE(--val == UnderlyingType(43));
      val.revert();
      REQUIRE(val == UnderlyingType(44));
      REQUIRE(--val == UnderlyingType(43));
      val.commit();
      REQUIRE(val == UnderlyingType(43));
      // decrement postfix
      REQUIRE(val-- == UnderlyingType(43));
      val.revert();
      REQUIRE(val == UnderlyingType(43));
      REQUIRE(val-- == UnderlyingType(43));
      val.commit();
      REQUIRE(val == UnderlyingType(42));
    }
  }
};

TEST_CASE("SafeUint tests", "[unit][safevars][safeuint]") {
  SafeUintTester<8>()();
  SafeUintTester<16>()();
  SafeUintTester<32>()();
  SafeUintTester<64>()();

  SafeUintTester<24>()();
  SafeUintTester<40>()();
  SafeUintTester<48>()();
  SafeUintTester<56>()();
  SafeUintTester<72>()();
  SafeUintTester<80>()();
  SafeUintTester<88>()();
  SafeUintTester<96>()();
  SafeUintTester<104>()();
  SafeUintTester<112>()();
  SafeUintTester<120>()();
  SafeUintTester<128>()();
  SafeUintTester<136>()();
  SafeUintTester<144>()();
  SafeUintTester<152>()();
  SafeUintTester<160>()();
  SafeUintTester<168>()();
  SafeUintTester<176>()();
  SafeUintTester<184>()();
  SafeUintTester<192>()();
  SafeUintTester<200>()();
  SafeUintTester<208>()();
  SafeUintTester<216>()();
  SafeUintTester<224>()();
  SafeUintTester<232>()();
  SafeUintTester<240>()();
  SafeUintTester<248>()();
  SafeUintTester<256>()();
}

