#ifndef BDK_UTILS_FIXEDBYTES_H
#define BDK_UTILS_FIXEDBYTES_H

#include "bytesinterface.h"
#include "hex.h"

/**
 * Abstraction of a fixed-size bytes container.
 * `FixedBytes<10>` would have *exactly* 10 bytes, no more, no less.
 * Used as a base for both aliases (e.g. PrivKey, PubKey, etc.) and classes inheriting it (e.g. Hash, Signature, etc.).
 */
template <size_t N>
class FixedBytes : public BytesInterface<FixedBytes<N>, N> {
public:

  /**
   * Constructs a fixed bytes container with all bits clear
   */
  constexpr FixedBytes() : data_() {}

  /**
   * Constructs a fixed bytes container from the given initializer list
   * 
   * @param initList the initalizer list
   * @throw invalid argument exception if the initializer list size does not match the container size
   */
  constexpr FixedBytes(std::initializer_list<Byte> initList) {
    if (initList.size() != N)
      throw std::invalid_argument("Given initializer list of size " + std::to_string(initList.size()) + 
        " is not suitable for initializing a FixedBytes<" + std::to_string(N) + ">");

    std::ranges::copy(initList, data_.begin());
  }  

  /**
   * Constructs a fixed bytes container from the given bytes initializer
   * 
   * @param initializer the bytes initializer
   */
  constexpr FixedBytes(bytes::Initializer auto&& initializer) { initializer.to(data_); }

  /**
   * Constructs a fixed bytes container by copying the bytes from the given range.
   * 
   * @param input the input bytes
   * @throw invalid argument exception if the input size does not match the container size
   */
  explicit constexpr FixedBytes(const bytes::Range auto& input) {
    if (const size_t size = std::ranges::size(input); size != N) {
      throw std::invalid_argument("Given bytes range of size " + std::to_string(size) +
        " is not suitable for initializing a FixedBytes<" + std::to_string(N) + ">");
    }

    std::ranges::copy(input, data_.begin());
  }

  /**
   * @return the beginning iterator of this container bytes
   */
  constexpr auto begin() { return data_.begin(); }

  /**
   * @return the beginning constant iterator of this container bytes
   */
  constexpr auto begin() const { return data_.begin(); }

private:
  BytesArr<N> data_;

  friend zpp::bits::access;
  using serialize = zpp::bits::members<1>;
};

using Bytes1 = FixedBytes<1>;

using Bytes2 = FixedBytes<2>;

using Bytes3 = FixedBytes<3>;

using Bytes4 = FixedBytes<4>;

using Bytes5 = FixedBytes<5>;

using Bytes6 = FixedBytes<6>;

using Bytes7 = FixedBytes<7>;

using Bytes8 = FixedBytes<8>;

using Bytes9 = FixedBytes<9>;

using Bytes10 = FixedBytes<10>;

using Bytes11 = FixedBytes<11>;

using Bytes12 = FixedBytes<12>;

using Bytes13 = FixedBytes<13>;

using Bytes14 = FixedBytes<14>;

using Bytes15 = FixedBytes<15>;

using Bytes16 = FixedBytes<16>;

using Bytes17 = FixedBytes<17>;

using Bytes18 = FixedBytes<18>;

using Bytes19 = FixedBytes<19>;

using Bytes20 = FixedBytes<20>;

using Bytes21 = FixedBytes<21>;

using Bytes22 = FixedBytes<22>;

using Bytes23 = FixedBytes<23>;

using Bytes24 = FixedBytes<24>;

using Bytes25 = FixedBytes<25>;

using Bytes26 = FixedBytes<26>;

using Bytes27 = FixedBytes<27>;

using Bytes28 = FixedBytes<28>;

using Bytes29 = FixedBytes<29>;

using Bytes30 = FixedBytes<30>;

using Bytes31 = FixedBytes<31>;

using Bytes32 = FixedBytes<32>;

#endif // BDK_UTILS_FIXEDBYTES_H
