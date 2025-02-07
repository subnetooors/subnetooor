#include "precompiles.h"
#include <boost/multiprecision/cpp_int.hpp>
#include "contract/abi.h"

namespace {

using BigInt = boost::multiprecision::cpp_int;
using boost::multiprecision::import_bits;
using boost::multiprecision::export_bits;

constexpr int bitLength(auto value) {
  int count = 0;
  while (value) {
    value >>= 1;
    ++count;
  }
  return count;
}

constexpr uint32_t multiplicationComplexity(uint16_t bSize, uint16_t mSize) {
  const uint16_t maxSize = std::max(bSize, mSize);
  const uint32_t words = (maxSize / 8) + bool(maxSize % 8);
  return words * words;
}

constexpr uint32_t iterationCount(uint16_t eSize, const BigInt& exp) {
  uint32_t count;

  if (eSize <= 32 && exp == 0) {
    count = 0;
  } else if (eSize <= 32) {
    count = bitLength(exp) - 1;
  } else {
    count = (8 * (eSize - 32)) + (bitLength(exp & ((BigInt(1) << 256) - 1)) - 1);
  }

  return std::max(count, uint32_t(1));
}

inline uint64_t gasRequired(uint16_t bSize, uint16_t mSize, uint16_t eSize, const BigInt& exp) {
  const uint64_t complexity = multiplicationComplexity(bSize, mSize);
  const uint64_t count = iterationCount(eSize, exp);
  return std::max(uint64_t(200), complexity * count / 3);
}

std::tuple<uint16_t, uint16_t, uint16_t> decodeSizes(View<Bytes> input) {
  const auto [baseSize, expSize, modSize] = ABI::Decoder::decodeData<uint256_t, uint256_t, uint256_t>(input);

  const uint256_t expectedSize = (32 * 3) + baseSize + expSize + modSize;

  if (input.size() < expectedSize) {
    throw std::invalid_argument("not enough bytes given");
  } else if (input.size() > expectedSize) {
    throw std::invalid_argument("too much bytes given");
  }

  if (baseSize > std::numeric_limits<uint16_t>::max()) {
    throw std::invalid_argument("base size is too big");
  }

  if (expSize > std::numeric_limits<uint16_t>::max()) {
    throw std::invalid_argument("exp size is too big");
  }

  if (modSize > std::numeric_limits<uint16_t>::max()) {
    throw std::invalid_argument("mod size is too big");
  }

  return std::make_tuple(uint16_t(baseSize), uint16_t(expSize), uint16_t(modSize));
}

} // namespace


Bytes precompiles::modexp(View<Bytes> input, Gas& gas) {
  const auto [baseSize, expSize, modSize] = decodeSizes(input);

  auto in = input.begin() + (32 * 3);

  BigInt base = 0, exp = 0, mod = 0, result = 1;

  if (baseSize > 0) {
    import_bits(base, in, in + baseSize);
    in += baseSize;
  }

  if (expSize > 0) {
    import_bits(exp, in, in + expSize);
    in += expSize;
  }

  if (modSize > 0) {
    import_bits(mod, in, in + modSize);
  }

  gas.use(gasRequired(baseSize, modSize, expSize, exp));

  Bytes output;
  output.resize(modSize);

  if (mod == 1) {
    return output; // means: return 0
  }

  base = base % mod;

  while (exp > 0) {
    if (exp & 1 != 0) {
      result = (result * base) % mod;
    }

    exp >>= 1;
    base = (base * base) % mod;
  }

  export_bits(result, output.begin(), 8);
  return output;
}
