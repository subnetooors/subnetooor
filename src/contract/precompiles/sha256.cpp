#include "precompiles.h"
#include <openssl/sha.h>

namespace {

constexpr size_t OUTPUT_SIZE = 32;

constexpr uint64_t gasRequired(size_t size) {
  return ((size + 31) / 32) * 12 + 60;
}

} // namespace

Bytes precompiles::sha256(View<Bytes> input, Gas& gas) {
  gas.use(gasRequired(input.size()));
  Bytes output(OUTPUT_SIZE);
  SHA256(input.data(), input.size(), output.data());
  return output;
}
