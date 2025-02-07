#include "precompiles.h"
#include "contract/abi.h"
#include "utils/signature.h"
#include "utils/ecdsa.h"

namespace {

constexpr auto ECRECOVER_COST = 3'000;

} // namespace

namespace precompiles {

Address ecrecover(View<Hash> hash, uint8_t v, View<Hash> r, View<Hash> s) {
  if (v == 27) {
    v = 0;
  } else if (v == 28) {
    v = 1;
  } else {
    return Address{};
  }

  Signature sig;
  *std::ranges::copy(s, std::ranges::copy(r, sig.begin()).out).out = v;

  const auto pubkey = Secp256k1::recover(sig, Hash(hash));

  if (!pubkey) {
    return Address{};
  }

  return Secp256k1::toAddress(pubkey);
}

Bytes ecrecover(View<Bytes> input, Gas& gas) {
  gas.use(ECRECOVER_COST);
  try {
    const auto [hash, v, r, s] = ABI::Decoder::decodeData<Hash, uint8_t, Hash, Hash>(input);
    return ABI::Encoder::encodeData(ecrecover(hash, v, r, s));
  } catch (...) {
    return ABI::Encoder::encodeData(Address{});
  }
}

} // namespace precompiles
