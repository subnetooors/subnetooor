/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#ifndef SAFEUINT_T_H
#define SAFEUINT_T_H

#include <memory>
#include <boost/multiprecision/cpp_int.hpp>
#include "safebase.h"

/**
 * Template for the type of a uint with the given size.
 * @tparam Size The size of the uint.
 */
template <int Size> struct UintType {
  /// The type of the uint with the given size.
  using type = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<
    Size, Size, boost::multiprecision::unsigned_magnitude, boost::multiprecision::cpp_int_check_type::checked, void
  >>;
};

/// Specialization for the type of a uint with 8 bits.
template <> struct UintType<8> {
  using type = uint8_t; ///< Type of the uint with 8 bits.
};

/// Specialization for the type of a uint with 16 bits.
template <> struct UintType<16> {
  using type = uint16_t; ///< Type of the uint with 16 bits.
};

/// Specialization for the type of a uint with 32 bits.
template <> struct UintType<32> {
  using type = uint32_t; ///< Type of the uint with 32 bits.
};

/// Specialization for the type of a uint with 64 bits.
template <> struct UintType<64> {
  using type = uint64_t; ///< Type of the uint with 64 bits.
};

/**
 * Safe wrapper for a uint_t variable.
 * @tparam Size The size of the uint.
 */
template <int Size> class SafeUint_t : public SafeBase {
  private:
    using uint_t = typename UintType<Size>::type; ///< Type of the uint.
    uint_t value_; ///< Current ("original") value.
    std::unique_ptr<uint_t> copy_; ///< Previous ("temporary") value.

  public:
    static_assert(Size >= 8 && Size <= 256 && Size % 8 == 0, "Size must be between 8 and 256 and a multiple of 8.");

    /**
     * Constructor.
     * @param value The initial value of the variable. Defaults to 0.
     */
    explicit SafeUint_t(const uint_t& value = 0) : SafeBase(nullptr), value_(value), copy_(nullptr) {}

    /**
     * Constructor with owner.
     * @param owner The DynamicContract that owns this variable.
     * @param value The initial value of the variable. Defaults to 0.
     */
    SafeUint_t(DynamicContract* owner, const uint_t& value = 0) : SafeBase(owner), value_(value), copy_(nullptr) {}

    /// Copy constructor. Only copies the CURRENT value.
    SafeUint_t(const SafeUint_t<Size>& other) : SafeBase(nullptr) : value_(other.value_), copy_(nullptr) {}

    /// Getter for the temporary value.
    inline const uint_t& get() const { return this->value_; }

    ///@{
    /**
     * Addition operator.
     * @param other The integer to add.
     * @throw std::overflow_error if an overflow happens.
     * @throw std::underflow_error if an underflow happens.
     * @return A new SafeUint_t with the result of the addition.
     */
    inline SafeUint_t<Size> operator+(const SafeUint_t<Size>& other) const {
      if (this->value_ > std::numeric_limits<uint_t>::max() - other.get()) {
        throw std::overflow_error("Overflow in addition operation.");
      }
      return SafeUint_t<Size>(this->value_ + other.get());
    }
    inline SafeUint_t<Size> operator+(const uint_t& other) const {
      if (this->value_ > std::numeric_limits<uint_t>::max() - other) {
        throw std::overflow_error("Overflow in addition operation.");
      }
      return SafeUint_t<Size>(this->value_ + other);
    }
    inline SafeUint_t<Size> operator+(const int& other) const {
      if (other < 0) {
        if (this->value_ < static_cast<uint_t>(-other)) {
          throw std::underflow_error("Underflow in addition operation.");
        }
      } else {
        if (this->value_ > std::numeric_limits<uint_t>::max() - other) {
          throw std::overflow_error("Overflow in addition operation.");
        }
      }
      return SafeUint_t<Size>(this->value_ + other);
    }
    template<typename T = uint_t>
    requires (!std::is_same<T, uint64_t>::value)
    SafeUint_t<Size> operator+(const uint_t& other) const {
      if (this->value_ > std::numeric_limits<uint_t>::max() - other) {
        throw std::overflow_error("Overflow in addition operation.");
      }
      return SafeUint_t<Size>(this->value_ + other);
    }
    ///@}

    ///@{
    /**
     * Subtraction operator.
     * @param other The integer to subtract.
     * @throw std::overflow_error if an overflow happens.
     * @throw std::underflow_error if an underflow happens.
     * @return A new SafeUint_t with the result of the subtraction.
     */
    inline SafeUint_t<Size> operator-(const SafeUint_t<Size>& other) const {
      if (this->value_ < other.get()) throw std::underflow_error("Underflow in subtraction operation.");
      return SafeUint_t<Size>(this->value_ - other.get());
    }
    inline SafeUint_t<Size> operator-(const uint_t& other) const {
      if (this->value_ < other) throw std::underflow_error("Underflow in subtraction operation.");
      return SafeUint_t<Size>(this->value_ - other);
    }
    template<typename T = uint_t>
    requires (!std::is_same<T, uint64_t>::value)
    SafeUint_t<Size> operator-(const uint_t& other) const {
      if (this->value_ < other) throw std::underflow_error("Underflow in subtraction operation.");
      return SafeUint_t<Size>(this->value_ - other);
    }
    inline SafeUint_t<Size> operator-(const int& other) const {
      if (other > 0) {
        if (this->value_ < static_cast<uint_t>(other)) throw std::underflow_error("Underflow in subtraction operation.");
      } else {
        if (this->value_ > std::numeric_limits<uint_t>::max() + other) {
          throw std::overflow_error("Overflow in subtraction operation.");
        }
      }
      return SafeUint_t<Size>(this->value_ - other);
    }
    ///@}

    ///@{
    /**
     * Multiplication operator.
     * @param other The integer to multiply.
     * @throw std::overflow_error if an overflow happens.
     * @throw std::underflow_error if an underflow happens.
     * @throw std::domain_error if the other value is zero.
     * @return A new SafeUint_t with the result of the multiplication.
     */
    inline SafeUint_t<Size> operator*(const SafeUint_t<Size>& other) const {
      if (other.get() == 0 || this->value_ == 0) throw std::domain_error("Multiplication by zero");
      if (this->value_ > std::numeric_limits<uint_t>::max() / other.get()) {
        throw std::overflow_error("Overflow in multiplication operation.");
      }
      return SafeUint_t<Size>(this->value_ * other.get());
    }
    inline SafeUint_t<Size> operator*(const uint_t& other) const {
      if (other == 0 || this->value_ == 0) throw std::domain_error("Multiplication by zero");
      if (this->value_ > std::numeric_limits<uint_t>::max() / other) {
        throw std::overflow_error("Overflow in multiplication operation.");
      }
      return SafeUint_t<Size>(this->value_ * other);
    }
    template<typename T = uint_t>
    requires (!std::is_same<T, uint64_t>::value)
    SafeUint_t<Size> operator*(const uint_t& other) const {
      if (other == 0 || this->value_ == 0) throw std::domain_error("Multiplication by zero");
      if (this->value_ > std::numeric_limits<uint_t>::max() / other) {
        throw std::overflow_error("Overflow in multiplication operation.");
      }
      return SafeUint_t<Size>(this->value_ * other);
    }
    inline SafeUint_t<Size> operator*(const int& other) const {
      if (other == 0 || this->value_ == 0) throw std::domain_error("Multiplication by zero");
      if (other < 0) {
        throw std::underflow_error("Underflow in multiplication operation.");
      } else {
        if (this->value_ > std::numeric_limits<uint_t>::max() / other) {
          throw std::overflow_error("Overflow in multiplication operation.");
        }
      }
      return SafeUint_t<Size>(this->value_ * other);
    }
    ///@}

    ///@{
    /**
     * Division operator.
     * @param other The integer to divide.
     * @throw std::domain_error if the other value is zero, or if the division results in a negative number.
     * @return A new SafeUint_t with the result of the division.
     */
    inline SafeUint_t<Size> operator/(const SafeUint_t<Size>& other) const {
      if (this->value_ == 0 || other.get() == 0) throw std::domain_error("Division by zero");
      return SafeUint_t<Size>(this->value_ / other.get());
    }
    inline SafeUint_t<Size> operator/(const uint_t& other) const {
      if (this->value_ == 0 || other == 0) throw std::domain_error("Division by zero");
      return SafeUint_t<Size>(this->value_ / other);
    }
    template<typename T = uint_t>
    requires (!std::is_same<T, uint64_t>::value)
    SafeUint_t<Size> operator/(const uint_t& other) const {
      if (this->value_ == 0 || other == 0) throw std::domain_error("Division by zero");
      return SafeUint_t<Size>(this->value_ / other);
    }
    inline SafeUint_t<Size> operator/(const int& other) const {
      if (other == 0) throw std::domain_error("Division by zero");
      // Division by a negative number results in a negative result, which cannot be represented in an unsigned integer.
      if (other < 0) throw std::domain_error("Division by a negative number");
      return SafeUint_t<Size>(this->value_ / other);
    }
    ///@}

    ///@{
    /**
     * Modulus operator.
     * @param other The integer to take the modulo of.
     * @throw std::domain_error if the other value is zero.
     * @return A new SafeUint_t with the result of the modulus.
     */
    inline SafeUint_t<Size> operator%(const SafeUint_t<Size>& other) const {
      if (this->value_ == 0 || other.get() == 0) throw std::domain_error("Modulus by zero");
      return SafeUint_t<Size>(this->value_ % other.get());
    }
    inline SafeUint_t<Size> operator%(const uint_t& other) const {
      if (this->value_ == 0 || other == 0) throw std::domain_error("Modulus by zero");
      return SafeUint_t<Size>(this->value_ % other);
    }
    template<typename T = uint_t>
    requires (!std::is_same<T, uint64_t>::value)
    SafeUint_t<Size> operator%(const uint64_t& other) const {
      if (this->value_ == 0 || other == 0) throw std::domain_error("Modulus by zero");
      return SafeUint_t<Size>(this->value_ % other);
    }
    inline SafeUint_t<Size> operator%(const int& other) const {
      if (this->value_ == 0 || other == 0) throw std::domain_error("Modulus by zero");
      return SafeUint_t<Size>(this->value_ % static_cast<uint_t>(other));
    }
    ///@}

    ///@{
    /**
     * Bitwise AND operator.
     * @param other The integer to apply AND.
     * @return A new SafeUint_t with the result of the AND.
     * @throw std::domain_error if AND is done with a negative number.
     */
    inline SafeUint_t<Size> operator&(const SafeUint_t<Size>& other) const {
      return SafeUint_t<Size>(this->value_ & other.get());
    }
    inline SafeUint_t<Size> operator&(const uint_t& other) const {
      return SafeUint_t<Size>(this->value_ & other);
    }
    template<typename T = uint_t>
    requires (!std::is_same<T, uint64_t>::value)
    SafeUint_t<Size> operator&(const uint64_t& other) const {
      return SafeUint_t<Size>(this->value_ & other);
    }
    inline SafeUint_t<Size> operator&(const int& other) const {
      if (other < 0) throw std::domain_error("Bitwise AND with a negative number");
      return SafeUint_t<Size>(this->value_ & static_cast<uint_t>(other));
    }
    ///@}

    ///@{
    /**
     * Bitwise OR operator.
     * @param other The integer to apply OR.
     * @return A new SafeUint_t with the result of the OR.
     * @throw std::domain_error if OR is done with a negative number.
     */
    inline SafeUint_t<Size> operator|(const SafeUint_t<Size>& other) const {
      return SafeUint_t<Size>(this->value_ | other.get());
    }
    inline SafeUint_t<Size> operator|(const uint_t& other) const {
      return SafeUint_t<Size>(this->value_ | other);
    }
    template<typename T = uint_t>
    requires (!std::is_same<T, uint64_t>::value)
    SafeUint_t<Size> operator|(const uint64_t& other) const {
      return SafeUint_t<Size>(this->value_ | other);
    }
    inline SafeUint_t<Size> operator|(const int& other) const {
      if (other < 0) throw std::domain_error("Bitwise OR with a negative number");
      return SafeUint_t<Size>(this->value_ | static_cast<uint_t>(other));
    }
    ///@}

    ///@{
    /**
     * Bitwise XOR operator.
     * @param other The integer to apply XOR.
     * @return A new SafeUint_t with the result of the XOR.
     * @throw std::domain_error if XOR is done with a negative number.
     */
    inline SafeUint_t<Size> operator^(const SafeUint_t<Size>& other) const {
      return SafeUint_t<Size>(this->value_ ^ other.get());
    }
    inline SafeUint_t<Size> operator^(const uint_t& other) const {
      return SafeUint_t<Size>(this->value_ ^ other);
    }
    template<typename T = uint_t>
    requires (!std::is_same<T, uint64_t>::value)
    SafeUint_t<Size> operator^(const uint64_t& other) const {
      return SafeUint_t<Size>(this->value_ ^ other);
    }
    inline SafeUint_t<Size> operator^(const int& other) const {
      if (other < 0) throw std::domain_error("Bitwise XOR with a negative number");
      return SafeUint_t<Size>(this->value_ ^ static_cast<uint_t>(other));
    }
    ///@}

    ///@{
    /**
     * Left shift operator.
     * @param other The integer to shift.
     * @return A new SafeUint_t with the result of the shift.
     * @throw std::domain_error if shift is done with a negative number.
     */
    inline SafeUint_t<Size> operator<<(const SafeUint_t<Size>& other) const {
      return SafeUint_t<Size>(this->value_ << other.get());
    }
    inline SafeUint_t<Size> operator<<(const uint_t& other) const {
      return SafeUint_t<Size>(this->value_ << other);
    }
    template<typename T = uint_t>
    requires (!std::is_same<T, uint64_t>::value)
    SafeUint_t<Size> operator<<(const uint64_t& other) const {
      return SafeUint_t<Size>(this->value_ << other);
    }
    inline SafeUint_t<Size> operator<<(const int& other) const {
      if (other < 0) throw std::domain_error("Bitwise left shift with a negative number");
      return SafeUint_t<Size>(this->value_ << other);
    }
    ///@}

    ///@{
    /**
     * Right shift operator.
     * @param other The SafeUint_t to shift.
     * @return A new SafeUint_t with the result of the shift.
     * @throw std::domain_error if shift is done with a negative number.
     */
    inline SafeUint_t<Size> operator>>(const SafeUint_t<Size>& other) const {
      return SafeUint_t<Size>(this->value_ >> other.get());
    }
    template<typename T = uint_t>
    requires (!std::is_same<T, uint64_t>::value)
    SafeUint_t<Size> operator>>(const uint_t& other) const {
      return SafeUint_t<Size>(this->value_ >> other);
    }
    inline SafeUint_t<Size> operator>>(const uint64_t& other) const {
      return SafeUint_t<Size>(this->value_ >> other);
    }
    inline SafeUint_t<Size> operator>>(const int& other) const {
      if (other < 0) throw std::domain_error("Bitwise right shift with a negative number");
      return SafeUint_t<Size>(this->value_ >> other);
    }
    ///@}

    /**
     * Logical NOT operator.
     * @return `true` if the value is zero, `false` otherwise.
     */
    inline bool operator!() const { return !(this->value_); }

    ///@{
    /**
     * Logical AND operator.
     * @param other The integer to apply AND.
     * @return `true` if both values are not zero, `false` otherwise.
     */
    inline bool operator&&(const SafeUint_t<Size>& other) const { return this->value_ && other.get(); }
    inline bool operator&&(const uint_t& other) const { return this->value_ && other; }
    template<typename T = uint_t>
    requires (!std::is_same<T, uint64_t>::value)
    SafeUint_t<Size> operator&&(const uint_t& other) const { return this->value_ && other; }
    ///@}

    ///@{
    /**
     * Logical OR operator.
     * @param other The integer to apply OR.
     * @return `true` if at least one value is not zero, `false` otherwise.
     */
    inline bool operator||(const SafeUint_t<Size>& other) const { return this->value_ || other.get(); }
    inline bool operator||(const uint_t& other) const { return this->value_ || other; }
    template<typename T = uint_t>
    requires (!std::is_same<T, uint64_t>::value)
    SafeUint_t<Size> operator||(const uint64_t& other) const { return this->value_ || other; }
    ///@}

    ///@{
    /**
     * Equality operator.
     * @param other The integer to compare.
     * @return `true` if both values are equal, `false` otherwise.
     */
    inline bool operator==(const SafeUint_t<Size>& other) const { return this->value_ == other.get(); }
    inline bool operator==(const uint_t& other) const { return this->value_ == other; }
    template<typename T = uint_t>
    requires (!std::is_same<T, uint64_t>::value)
    bool operator==(const uint64_t& other) const { return this->value_ == other; }
    inline bool operator==(const int& other) const {
      if (other < 0) return false;  // Unsigned value can never be negative
      return this->value_ == static_cast<uint_t>(other);
    }
    ///@}

    ///@{
    /**
     * Inequality operator.
     * @param other The integer to compare.
     * @return `true` if both values are not equal, `false` otherwise.
     */
    inline bool operator!=(const uint_t& other) const { return this->value_ != other; }
    template<typename T = uint_t>
    requires (!std::is_same<T, uint64_t>::value)
    SafeUint_t<Size> operator!=(const uint64_t& other) const { return this->value_ != other; }
    ///@}

    ///@{
    /**
     * Less than operator.
     * @param other The integer to compare.
     * @return `true` if the value is less than the other value, `false` otherwise.
     */
    inline bool operator<(const SafeUint_t<Size>& other) const { return this->value_ < other.get(); }
    inline bool operator<(const uint_t& other) const { return this->value_ < other; }
    template<typename T = uint_t>
    requires (!std::is_same<T, uint64_t>::value)
    SafeUint_t<Size> operator<(const uint64_t& other) const { return this->value_ < other; }
    ///@}

    ///@{
    /**
     * Less than or equal operator.
     * @param other The integer to compare.
     * @return `true` if the value is less than or equal to the other value, `false` otherwise.
     */
    inline bool operator<=(const SafeUint_t<Size>& other) const { return this->value_ <= other.get(); }
    template<typename T = uint_t>
    requires (!std::is_same<T, uint64_t>::value)
    bool operator<=(const uint_t& other) const { return this->value_ <= other; }
    inline bool operator<=(const uint64_t& other) const { return this->value_ <= other; }
    ///@}

    ///@{
    /**
     * Greater than operator.
     * @param other The integer to compare.
     * @return `true` if the value is greater than the other value, `false` otherwise.
     */
    inline bool operator>(const SafeUint_t<Size>& other) const { return this->value_ > other.get(); }
    template<typename T = uint_t>
    requires (!std::is_same<T, uint64_t>::value)
    bool operator>(const uint_t& other) const { return this->value_ > other; }
    inline bool operator>(const uint64_t& other) const { return this->value_ > other; }
    ///@}

    ///@{
    /**
     * Greater than or equal operator.
     * @param other The integer to compare.
     * @return `true` if the value is greater than or equal to the other value, `false` otherwise.
     */
    inline bool operator>=(const SafeUint_t<Size>& other) const { return this->value_ >= other.get(); }
    inline bool operator>=(const uint_t& other) const { return this->value_ >= other; }
    template<typename T = uint_t>
    requires (!std::is_same<T, uint64_t>::value)
    bool operator>=(const uint64_t& other) const { return this->value_ >= other; }
    ///@}

    ///@{
    /**
     * Assignment operator.
     * @param other The integer to assign.
     * @return A reference to this SafeUint_t.
     * @throw std::domain_error if a negative value is assigned.
     */
    inline SafeUint_t<Size>& operator=(const SafeUint_t<Size>& other) {
      if (this->copy_ == nullptr) this->copy_ = std::make_unique<uint_t>(this->value_);
      markAsUsed(); this->value_ = other.get(); return *this;
    }
    inline SafeUint_t<Size>& operator=(const uint_t& other) {
      if (this->copy_ == nullptr) this->copy_ = std::make_unique<uint_t>(this->value_);
      markAsUsed(); this->value_ = other; return *this;
    }
    template<typename T = uint_t>
    requires (!std::is_same<T, uint64_t>::value)
    SafeUint_t<Size> operator=(const uint_t& other) {
      if (this->copy_ == nullptr) this->copy_ = std::make_unique<uint_t>(this->value_);
      markAsUsed(); this->value_ = other; return *this;
    }
    inline SafeUint_t<Size>& operator=(const int& other) {
      if (other < 0) throw std::domain_error("Cannot assign negative value to SafeUint_t");
      if (this->copy_ == nullptr) this->copy_ = std::make_unique<uint_t>(this->value_);
      markAsUsed(); this->value_ = static_cast<uint_t>(other); return *this;
    }
    ///@}

    ///@{
    /**
     * Addition assignment operator.
     * @param other The integer to add.
     * @throw std::overflow_error if an overflow happens.
     * @return A reference to this SafeUint_t.
     */
    inline SafeUint_t<Size>& operator+=(const SafeUint_t<Size>& other) {
      if (this->value_ > std::numeric_limits<uint_t>::max() - other.get()) {
        throw std::overflow_error("Overflow in addition assignment operation.");
      }
      if (this->copy_ == nullptr) this->copy_ = std::make_unique<uint_t>(this->value_);
      markAsUsed(); this->value_ += other.get(); return *this;
    }
    inline SafeUint_t<Size>& operator+=(const uint_t& other) {
      if (this->value_ > std::numeric_limits<uint_t>::max() - other) {
        throw std::overflow_error("Overflow in addition assignment operation.");
      }
      if (this->copy_ == nullptr) this->copy_ = std::make_unique<uint_t>(this->value_);
      markAsUsed(); this->value_ += other; return *this;
    }
    template<typename T = uint_t>
    requires (!std::is_same<T, uint64_t>::value)
    SafeUint_t<Size> operator+=(const uint64_t& other) {
      if (this->value_ > std::numeric_limits<uint_t>::max() - other) {
        throw std::overflow_error("Overflow in addition assignment operation.");
      }
      if (this->copy_ == nullptr) this->copy_ = std::make_unique<uint_t>(this->value_);
      markAsUsed(); this->value_ += other; return *this;
    }
    inline SafeUint_t<Size>& operator+=(const int& other) {
      if (other < 0 || static_cast<uint64_t>(other) > std::numeric_limits<uint_t>::max() - this->value_) {
        throw std::overflow_error("Overflow in addition assignment operation.");
      }
      if (this->copy_ == nullptr) this->copy_ = std::make_unique<uint_t>(this->value_);
      markAsUsed(); this->value_ += static_cast<uint_t>(other); return *this;
    }
    ///@}

    ///@{
    /**
     * Subtraction assignment operator.
     * @param other The integer to subtract.
     * @return A reference to this SafeUint_t.
     * @throw std::underflow_error if an underflow happens.
     * @throw std::invalid_argument if a negative value is subtracted.
     */
    inline SafeUint_t<Size>& operator-=(const SafeUint_t<Size>& other) {
      if (this->value_ < other.get()) throw std::underflow_error("Underflow in subtraction assignment operation.");
      if (this->copy_ == nullptr) this->copy_ = std::make_unique<uint_t>(this->value_);
      markAsUsed(); this->value_ -= other.get(); return *this;
    }
    inline SafeUint_t<Size>& operator-=(const uint_t& other) {
      if (this->value_ < other) throw std::underflow_error("Underflow in subtraction assignment operation.");
      if (this->copy_ == nullptr) this->copy_ = std::make_unique<uint_t>(this->value_);
      markAsUsed(); this->value_ -= other; return *this;
    }
    template<typename T = uint_t>
    requires (!std::is_same<T, uint64_t>::value)
    SafeUint_t<Size> operator-=(const uint64_t& other) {
      if (this->value_ < other) throw std::underflow_error("Underflow in subtraction assignment operation.");
      if (this->copy_ == nullptr) this->copy_ = std::make_unique<uint_t>(this->value_);
      markAsUsed(); this->value_ -= other; return *this;
    }
    inline SafeUint_t<Size>& operator-=(const int& other) {
      if (other < 0) throw std::invalid_argument("Cannot subtract a negative value.");
      auto other_uint = static_cast<uint_t>(other);
      if (this->value_ < other_uint) throw std::underflow_error("Underflow in subtraction assignment operation.");
      if (this->copy_ == nullptr) this->copy_ = std::make_unique<uint_t>(this->value_);
      markAsUsed(); this->value_ -= other_uint; return *this;
    }
    ///@}

    ///@{
    /**
     * Multiplication assignment operator.
     * @param other The integer to multiply.
     * @return A reference to this SafeUint_t.
     * @throw std::domain_error if the other value is zero.
     * @throw std::overflow_error if an overflow happens.
     * @throw std::invalid_argument if the other value is negative.
     */
    inline SafeUint_t<Size>& operator*=(const SafeUint_t<Size>& other) {
      if (other.get() == 0 || this->value_ == 0) throw std::domain_error("Multiplication assignment by zero");
      if (this->value_ > std::numeric_limits<uint_t>::max() / other.get()) {
        throw std::overflow_error("Overflow in multiplication assignment operation.");
      }
      if (this->copy_ == nullptr) this->copy_ = std::make_unique<uint_t>(this->value_);
      markAsUsed(); this->value_ *= other.get(); return *this;
    }
    inline SafeUint_t<Size>& operator*=(const uint_t& other) {
      if (other == 0 || this->value_ == 0) throw std::domain_error("Multiplication assignment by zero");
      if (this->value_ > std::numeric_limits<uint_t>::max() / other) {
        throw std::overflow_error("Overflow in multiplication assignment operation.");
      }
      if (this->copy_ == nullptr) this->copy_ = std::make_unique<uint_t>(this->value_);
      markAsUsed(); this->value_ *= other; return *this;
    }
    inline SafeUint_t<Size>& operator*=(const int& other) {
      if (other < 0) throw std::invalid_argument("Cannot multiply by a negative value.");
      if (other == 0 || this->value_ == 0) throw std::domain_error("Multiplication assignment by zero");
      auto other_uint = static_cast<uint_t>(other);
      if (this->value_ > std::numeric_limits<uint_t>::max() / other_uint) {
        throw std::overflow_error("Overflow in multiplication assignment operation.");
      }
      if (this->copy_ == nullptr) this->copy_ = std::make_unique<uint_t>(this->value_);
      markAsUsed(); this->value_ *= other_uint; return *this;
    }
    ///@}

    ///@{
    /**
     * Division assignment operator.
     * @param other The integer to divide.
     * @return A reference to this SafeUint_t.
     * @throw std::domain_error if the other value is zero.
     * @throw std::invalid_argument if the other value is negative.
     */
    inline SafeUint_t<Size>& operator/=(const SafeUint_t<Size>& other) {
      if (this->value_ == 0 || other.get() == 0) throw std::domain_error("Division assignment by zero");
      if (this->copy_ == nullptr) this->copy_ = std::make_unique<uint_t>(this->value_);
      markAsUsed(); this->value_ /= other.get(); return *this;
    }
    inline SafeUint_t<Size>& operator/=(const uint_t& other) {
      if (this->value_ == 0 || other == 0) throw std::domain_error("Division assignment by zero");
      if (this->copy_ == nullptr) this->copy_ = std::make_unique<uint_t>(this->value_);
      markAsUsed(); this->value_ /= other; return *this;
    }
    template<typename T = uint_t>
    requires (!std::is_same<T, uint64_t>::value)
    SafeUint_t<Size> operator/=(const uint64_t& other) {
      if (this->value_ == 0 || other == 0) throw std::domain_error("Division assignment by zero");
      if (this->copy_ == nullptr) this->copy_ = std::make_unique<uint_t>(this->value_);
      markAsUsed(); this->value_ /= other; return *this;
    }
    inline SafeUint_t<Size>& operator/=(const int& other) {
      if (other <= 0) throw std::invalid_argument("Cannot divide by a negative value.");
      if (this->value_ == 0) throw std::domain_error("Division assignment by zero");
      auto other_uint = static_cast<uint_t>(other);
      if (this->copy_ == nullptr) this->copy_ = std::make_unique<uint_t>(this->value_);
      markAsUsed(); this->value_ /= other_uint; return *this;
    }
    ///@}

    ///@{
    /**
     * Modulus assignment operator.
     * @param other The integer to take the modulus of.
     * @return A reference to this SafeUint_t.
     * @throw std::domain_error if the other value is zero.
     * @throw std::invalid_argument if the other value is negative.
     */
    inline SafeUint_t<Size>& operator%=(const SafeUint_t<Size>& other) {
      if (this->value_ == 0 || other.get() == 0) throw std::domain_error("Modulus assignment by zero");
      if (this->copy_ == nullptr) this->copy_ = std::make_unique<uint_t>(this->value_);
      markAsUsed(); this->value_ %= other.get(); return *this;
    }
    inline SafeUint_t<Size>& operator%=(const uint_t& other) {
      if (this->value_ == 0 || other == 0) throw std::domain_error("Modulus assignment by zero");
      if (this->copy_ == nullptr) this->copy_ = std::make_unique<uint_t>(this->value_);
      markAsUsed(); this->value_ %= other; return *this;
    }
    template<typename T = uint_t>
    requires (!std::is_same<T, uint64_t>::value)
    SafeUint_t<Size>& operator%=(const uint64_t& other) {
      if (this->value_ == 0 || other == 0) throw std::domain_error("Modulus assignment by zero");
      if (this->copy_ == nullptr) this->copy_ = std::make_unique<uint_t>(this->value_);
      markAsUsed(); this->value_ %= other; return *this;
    }
    inline SafeUint_t<Size>& operator%=(const int& other) {
      if (other <= 0) throw std::invalid_argument("Cannot modulus by a negative value.");
      if (this->value_ == 0) throw std::domain_error("Modulus assignment by zero");
      auto other_uint = static_cast<uint_t>(other);
      if (this->copy_ == nullptr) this->copy_ = std::make_unique<uint_t>(this->value_);
      markAsUsed(); this->value_ %= other_uint; return *this;
    }
    ///@}

    ///@{
    /**
     * Bitwise AND assignment operator.
     * @param other The integer to apply AND.
     * @return A reference to this SafeUint_t.
     * @throw std::invalid_argument if the other value is negative.
     */
    inline SafeUint_t<Size>& operator&=(const SafeUint_t<Size>& other) {
      if (this->copy_ == nullptr) this->copy_ = std::make_unique<uint_t>(this->value_);
      markAsUsed(); this->value_ &= other.get(); return *this;
    }
    template<typename T = uint_t>
    requires (!std::is_same<T, uint64_t>::value)
    SafeUint_t<Size>& operator&=(const uint_t& other) {
      if (this->copy_ == nullptr) this->copy_ = std::make_unique<uint_t>(this->value_);
      markAsUsed(); this->value_ &= other; return *this;
    }
    inline SafeUint_t<Size>& operator&=(const uint64_t& other) {
      if (this->copy_ == nullptr) this->copy_ = std::make_unique<uint_t>(this->value_);
      markAsUsed(); this->value_ &= other; return *this;
    }
    inline SafeUint_t<Size>& operator&=(const int& other) {
      if (other < 0) throw std::invalid_argument("Cannot perform bitwise AND with a negative value.");
      auto other_uint = static_cast<uint_t>(other);
      if (this->copy_ == nullptr) this->copy_ = std::make_unique<uint_t>(this->value_);
      markAsUsed(); this->value_ &= other_uint; return *this;
    }
    ///@}

    ///@{
    /**
     * Bitwise OR assignment operator.
     * @param other The integer to apply OR.
     * @return A reference to this SafeUint_t.
     * @throw std::invalid_argument if the other value is negative.
     */
    inline SafeUint_t<Size>& operator|=(const SafeUint_t<Size>& other) {
      if (this->copy_ == nullptr) this->copy_ = std::make_unique<uint_t>(this->value_);
      markAsUsed(); this->value_ |= other.get(); return *this;
    }
    template<typename T = uint_t>
    requires (!std::is_same<T, uint64_t>::value)
    SafeUint_t<Size>& operator|=(const uint_t& other) {
      if (this->copy_ == nullptr) this->copy_ = std::make_unique<uint_t>(this->value_);
      markAsUsed(); this->value_ |= other; return *this;
    }
    inline SafeUint_t<Size>& operator|=(const uint64_t& other) {
      if (this->copy_ == nullptr) this->copy_ = std::make_unique<uint_t>(this->value_);
      markAsUsed(); this->value_ |= other; return *this;
    }
    inline SafeUint_t<Size>& operator|=(const int& other) {
      if (other < 0) throw std::invalid_argument("Cannot perform bitwise OR with a negative value.");
      auto other_uint = static_cast<uint_t>(other);
      if (this->copy_ == nullptr) this->copy_ = std::make_unique<uint_t>(this->value_);
      markAsUsed(); this->value_ |= other_uint; return *this;
    }
    ///@}

    ///@{
    /**
     * Bitwise XOR assignment operator.
     * @param other The integer to apply XOR.
     * @return A reference to this SafeUint_t.
     * @throw std::invalid_argument if the other value is negative.
     */
    inline SafeUint_t<Size>& operator^=(const SafeUint_t<Size>& other) {
      if (this->copy_ == nullptr) this->copy_ = std::make_unique<uint_t>(this->value_);
      markAsUsed(); this->value_ ^= other.get(); return *this;
    }
    template<typename T = uint_t>
    requires (!std::is_same<T, uint64_t>::value)
    SafeUint_t<Size>& operator^=(const uint_t& other) {
      if (this->copy_ == nullptr) this->copy_ = std::make_unique<uint_t>(this->value_);
      markAsUsed(); this->value_ ^= other; return *this;
    }
    inline SafeUint_t<Size>& operator^=(const uint64_t& other) {
      if (this->copy_ == nullptr) this->copy_ = std::make_unique<uint_t>(this->value_);
      markAsUsed(); this->value_ ^= other; return *this;
    }
    inline SafeUint_t<Size>& operator^=(const int& other) {
      if (other < 0) throw std::invalid_argument("Cannot perform bitwise XOR with a negative value.");
      auto other_uint = static_cast<uint_t>(other);
      if (this->copy_ == nullptr) this->copy_ = std::make_unique<uint_t>(this->value_);
      markAsUsed(); this->value_ ^= other_uint; return *this;
    }
    ///@}

    ///@{
    /**
     * Left shift assignment operator.
     * @param other The integer to shift.
     * @return A reference to this SafeUint_t.
     * @throw std::invalid_argument if the other value is negative.
     */
    inline SafeUint_t<Size>& operator<<=(const SafeUint_t<Size>& other) {
      if (this->copy_ == nullptr) this->copy_ = std::make_unique<uint_t>(this->value_);
      markAsUsed(); this->value_ <<= other.get(); return *this;
    }
    template<typename T = uint_t>
    requires (!std::is_same<T, uint64_t>::value)
    SafeUint_t<Size>& operator<<=(const uint_t& other) {
      if (this->copy_ == nullptr) this->copy_ = std::make_unique<uint_t>(this->value_);
      markAsUsed(); this->value_ <<= other; return *this;
    }
    inline SafeUint_t<Size>& operator<<=(const uint64_t& other) {
      if (this->copy_ == nullptr) this->copy_ = std::make_unique<uint_t>(this->value_);
      markAsUsed(); this->value_ <<= other; return *this;
    }
    inline SafeUint_t<Size>& operator<<=(const int& other) {
      if (other < 0) throw std::invalid_argument("Cannot perform bitwise left shift with a negative value.");
      auto other_uint = static_cast<uint_t>(other);
      if (this->copy_ == nullptr) this->copy_ = std::make_unique<uint_t>(this->value_);
      markAsUsed(); this->value_ <<= other_uint; return *this;
    }
    ///@}

    ///@{
    /**
     * Right shift assignment operator.
     * @param other The integer to shift.
     * @return A reference to this SafeUint_t.
     * @throw std::invalid_argument if the other value is negative.
     */
    inline SafeUint_t<Size>& operator>>=(const SafeUint_t<Size>& other) {
      if (this->copy_ == nullptr) this->copy_ = std::make_unique<uint_t>(this->value_);
      markAsUsed(); this->value_ >>= other.get(); return *this;
    }
    inline SafeUint_t<Size>& operator>>=(const uint_t& other) {
      if (this->copy_ == nullptr) this->copy_ = std::make_unique<uint_t>(this->value_);
      markAsUsed(); this->value_ >>= other; return *this;
    }
    template<typename T = uint_t>
    requires (!std::is_same<T, uint64_t>::value)
    SafeUint_t<Size>& operator>>=(const uint64_t& other) {
      if (this->copy_ == nullptr) this->copy_ = std::make_unique<uint_t>(this->value_);
      markAsUsed(); this->value_ >>= other; return *this;
    }
    inline SafeUint_t<Size>& operator>>=(const int& other) {
      if (other < 0) throw std::invalid_argument("Cannot perform bitwise right shift with a negative value.");
      auto other_uint = static_cast<uint_t>(other);
      if (this->copy_ == nullptr) this->copy_ = std::make_unique<uint_t>(this->value_);
      markAsUsed(); this->value_ >>= other_uint; return *this;
    }
    ///@}

    /**
     * Prefix increment operator.
     * @throw std::overflow_error if an overflow happens.
     * @return A reference to this SafeUint_t.
     */
    inline SafeUint_t<Size>& operator++() {
      if (this->value_ == std::numeric_limits<uint_t>::max()) {
        throw std::overflow_error("Overflow in prefix increment operation.");
      }
      if (this->copy_ == nullptr) this->copy_ = std::make_unique<uint_t>(this->value_);
      markAsUsed(); ++(this->value_); return *this;
    }

    /**
     * Postfix increment operator.
     * @throw std::overflow_error if an overflow happens.
     * @return A new SafeUint_t with the value before the increment.
     */
    inline SafeUint_t<Size> operator++(int) {
      if (this->value_ == std::numeric_limits<uint_t>::max()) {
        throw std::overflow_error("Overflow in postfix increment operation.");
      }
      if (this->copy_ == nullptr) this->copy_ = std::make_unique<uint_t>(this->value_);
      SafeUint_t<Size> tmp(this->value_);
      markAsUsed(); ++(this->value_); return tmp;
    }

    /**
     * Prefix decrement operator.
     * @throw std::underflow_error if an underflow happens.
     * @return A reference to this SafeUint_t.
     */
    inline SafeUint_t<Size>& operator--() {
      if (this->value_ == 0) throw std::underflow_error("Underflow in prefix decrement operation.");
      if (this->copy_ == nullptr) this->copy_ = std::make_unique<uint_t>(this->value_);
      markAsUsed(); --(this->value_); return *this;
    }

    /**
     * Postfix decrement operator.
     * @throw std::underflow_error if an underflow happens.
     * @return A new SafeUint_t with the value before the decrement.
     */
    inline SafeUint_t<Size> operator--(int) {
      if (this->value_ == 0) throw std::underflow_error("Underflow in postfix decrement operation.");
      if (this->copy_ == nullptr) this->copy_ = std::make_unique<uint_t>(this->value_);
      SafeUint_t<Size> tmp(this->value_);
      markAsUsed(); --(this->value_); return tmp;
    }

    /// Commit the value.
    inline void commit() override { this->copy_ = nullptr; this->registered_ = false; };

    /// Revert the value.
    inline void revert() override { this->value_ = *this->copy_; this->copy_ = nullptr; this->registered_ = false; };
};

#endif // SAFEUINT_T_H
