#ifndef BDK_MESSAGES_GAS_H
#define BDK_MESSAGES_GAS_H

#include "utils/evmcconv.h"
#include "outofgas.h"

class Gas {
public:
  constexpr explicit Gas(uint256_t value = 0) : value_(value) {}

  constexpr void use(const uint256_t& amount) {
    if (amount > value_) {
      value_ = 0;
      throw OutOfGas();
    }

    value_ -= amount;
  }

  constexpr void refund(const uint256_t& amount) {
    value_ += amount;
  }

  template<typename T>
    requires std::constructible_from<T, uint256_t>
  explicit constexpr operator T() const {
    return static_cast<T>(value_);
  }

private:
  uint256_t value_;
};

#endif // BDK_MESSAGES_GAS_H
