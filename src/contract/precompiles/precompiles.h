#ifndef BDK_PRECOMPILES_H
#define BDK_PRECOMPILES_H

#include "utils/bytes.h"
#include "utils/view.h"
#include "contract/gas.h"

namespace precompiles {

Address ecrecover(View<Hash> hash, uint8_t v, View<Hash> r, View<Hash> s);

Bytes ecrecover(View<Bytes> input, Gas& gas);

Bytes sha256(View<Bytes> input, Gas& gas);

Bytes ripemd160(View<Bytes> input, Gas& gas);

Bytes modexp(View<Bytes> input, Gas& gas);

void blake2f(std::span<uint64_t, 8> h, std::span<const uint64_t, 16> m, 
             uint64_t c0, uint64_t c1, bool flag, uint32_t rounds);

Bytes blake2f(View<Bytes> input, Gas& gas);

Bytes modexp(View<Bytes> input, Gas& gas);

} // namespace precompiles

#endif // BDK_PRECOMPILES_H
