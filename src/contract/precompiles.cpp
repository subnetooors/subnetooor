#include "precompiles.h"
#include "utils/signature.h"
#include "utils/ecdsa.h"
#include <openssl/sha.h>
#include <openssl/ripemd.h>

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

Hash sha256(View<Bytes> input) {
  Hash output;
  SHA256(input.data(), input.size(), output.data());
  return output;
}

Bytes20 ripemd160(View<Bytes> input) {
  Bytes20 output;
  RIPEMD160(input.data(), input.size(), output.data()); // TODO: this is deprecated
  return output;
}

void blake2f(std::span<uint64_t, 8> h, std::span<const uint64_t, 16> m, 
             uint64_t c0, uint64_t c1, bool flag, uint32_t rounds) {           
 
  static constexpr uint64_t iv[8] = {
    0x6A09E667F3BCC908, 0xBB67AE8584CAA73B, 0x3C6EF372FE94F82B, 0xA54FF53A5F1D36F1,
    0x510E527FADE682D1, 0x9B05688C2B3E6C1F, 0x1F83D9ABFB41BD6B, 0x5BE0CD19137E2179
  };

  static constexpr unsigned char precomputed[10][16] = {
    {0, 2, 4, 6, 1, 3, 5, 7, 8, 10, 12, 14, 9, 11, 13, 15},
    {14, 4, 9, 13, 10, 8, 15, 6, 1, 0, 11, 5, 12, 2, 7, 3},
    {11, 12, 5, 15, 8, 0, 2, 13, 10, 3, 7, 9, 14, 6, 1, 4},
    {7, 3, 13, 11, 9, 1, 12, 14, 2, 5, 4, 15, 6, 10, 0, 8},
    {9, 5, 2, 10, 0, 7, 4, 15, 14, 11, 6, 3, 1, 12, 8, 13},
    {2, 6, 0, 8, 12, 10, 11, 3, 4, 7, 15, 1, 13, 5, 14, 9},
    {12, 1, 14, 4, 5, 15, 13, 10, 0, 6, 9, 8, 7, 3, 2, 11},
    {13, 7, 12, 3, 11, 14, 1, 9, 5, 15, 8, 2, 0, 4, 6, 10},
    {6, 14, 11, 0, 15, 9, 3, 8, 12, 13, 1, 10, 2, 7, 4, 5},
    {10, 8, 7, 1, 2, 4, 6, 5, 15, 9, 3, 13, 11, 14, 12, 0}
  };

    uint64_t v0 = h[0], v1 = h[1], v2 = h[2], v3 = h[3],
           v4 = h[4], v5 = h[5], v6 = h[6], v7 = h[7],
           v8 = iv[0], v9 = iv[1], v10 = iv[2], v11 = iv[3],
           v12 = iv[4], v13 = iv[5], v14 = iv[6], v15 = iv[7];

  v12 ^= c0;
  v13 ^= c1;
  
  if (flag) {
    v14 ^= 0xFFFFFFFFFFFFFFFF;
  }

  for (uint32_t i = 0; i < rounds; i++) {
    const auto* s = precomputed[i % 10];

    v0 += m[s[0]];
    v0 += v4;
    v12 ^= v0;
    v12 = std::rotr(v12, 32);
    v8 += v12;
    v4 ^= v8;
    v4 = std::rotr(v4, 24);
    v1 += m[s[1]];
    v1 += v5;
    v13 ^= v1;
    v13 = std::rotr(v13, 32);
    v9 += v13;
    v5 ^= v9;
    v5 = std::rotr(v5, 24);
    v2 += m[s[2]];
    v2 += v6;
    v14 ^= v2;
    v14 = std::rotr(v14, 32);
    v10 += v14;
    v6 ^= v10;
    v6 = std::rotr(v6, 24);
    v3 += m[s[3]];
    v3 += v7;
    v15 ^= v3;
    v15 = std::rotr(v15, 32);
    v11 += v15;
    v7 ^= v11;
    v7 = std::rotr(v7, 24);

    v0 += m[s[4]];
    v0 += v4;
    v12 ^= v0;
    v12 = std::rotr(v12, 16);
    v8 += v12;
    v4 ^= v8;
    v4 = std::rotr(v4, 63);
    v1 += m[s[5]];
    v1 += v5;
    v13 ^= v1;
    v13 = std::rotr(v13, 16);
    v9 += v13;
    v5 ^= v9;
    v5 = std::rotr(v5, 63);
    v2 += m[s[6]];
    v2 += v6;
    v14 ^= v2;
    v14 = std::rotr(v14, 16);
    v10 += v14;
    v6 ^= v10;
    v6 = std::rotr(v6, 63);
    v3 += m[s[7]];
    v3 += v7;
    v15 ^= v3;
    v15 = std::rotr(v15, 16);
    v11 += v15;
    v7 ^= v11;
    v7 = std::rotr(v7, 63);

    v0 += m[s[8]];
    v0 += v5;
    v15 ^= v0;
    v15 = std::rotr(v15, 32);
    v10 += v15;
    v5 ^= v10;
    v5 = std::rotr(v5, 24);
    v1 += m[s[9]];
    v1 += v6;
    v12 ^= v1;
    v12 = std::rotr(v12, 32);
    v11 += v12;
    v6 ^= v11;
    v6 = std::rotr(v6, 24);
    v2 += m[s[10]];
    v2 += v7;
    v13 ^= v2;
    v13 = std::rotr(v13, 32);
    v8 += v13;
    v7 ^= v8;
    v7 = std::rotr(v7, 24);
    v3 += m[s[11]];
    v3 += v4;
    v14 ^= v3;
    v14 = std::rotr(v14, 32);
    v9 += v14;
    v4 ^= v9;
    v4 = std::rotr(v4, 24);

    v0 += m[s[12]];
    v0 += v5;
    v15 ^= v0;
    v15 = std::rotr(v15, 16);
    v10 += v15;
    v5 ^= v10;
    v5 = std::rotr(v5, 63);
    v1 += m[s[13]];
    v1 += v6;
    v12 ^= v1;
    v12 = std::rotr(v12, 16);
    v11 += v12;
    v6 ^= v11;
    v6 = std::rotr(v6, 63);
    v2 += m[s[14]];
    v2 += v7;
    v13 ^= v2;
    v13 = std::rotr(v13, 16);
    v8 += v13;
    v7 ^= v8;
    v7 = std::rotr(v7, 63);
    v3 += m[s[15]];
    v3 += v4;
    v14 ^= v3;
    v14 = std::rotr(v14, 16);
    v9 += v14;
    v4 ^= v9;
    v4 = std::rotr(v4, 63);
  }

  h[0] ^= v0 ^ v8;
  h[1] ^= v1 ^ v9;
  h[2] ^= v2 ^ v10;
  h[3] ^= v3 ^ v11;
  h[4] ^= v4 ^ v12;
  h[5] ^= v5 ^ v13;
  h[6] ^= v6 ^ v14;
  h[7] ^= v7 ^ v15;
}
