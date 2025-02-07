#include "precompiles.h"
#include "utils/address.h"
#include <openssl/ripemd.h>
#include "contract/abi.h"

namespace {

constexpr size_t OUTPUT_SIZE = 20;

constexpr uint64_t gasRequired(size_t size) {
  return ((size + 31) / 32) * 120 + 600;
}

} // namespace

Bytes precompiles::ripemd160(View<Bytes> input, Gas& gas) {
  gas.use(gasRequired(input.size()));
  Bytes output(OUTPUT_SIZE);
  RIPEMD160(input.data(), input.size(), output.data());
  return output;
}
