#include "precompiledcontractexecutor.h"
#include "precompiles/precompiles.h"
#include "bytes/hex.h"
#include <algorithm>
#include <ranges>

constexpr Address RANDOM_GENERATOR_ADDRESS = bytes::hex("0x1000000000000000000000000000100000000001");

Bytes PrecompiledContractExecutor::execute(EncodedStaticCallMessage& msg) {
  if (msg.to() == RANDOM_GENERATOR_ADDRESS) {  
    return Utils::makeBytes(UintConv::uint256ToBytes(std::invoke(randomGen_)));
  }

  // assuming isPrecompiled() was already called
  switch (msg.to()[19]) {
    case 0x01:
      return precompiles::ecrecover(msg.input(), msg.gas());

    case 0x02:
      return precompiles::sha256(msg.input(), msg.gas());

    case 0x03:
      return ABI::Encoder::encodeData(Address(precompiles::ripemd160(msg.input(), msg.gas())));

    case 0x04: {
      const uint64_t dynamicGasCost = ((msg.input().size() + 31) / 32) * 3;
      msg.gas().use(15 + dynamicGasCost);
      return Bytes(msg.input());
    }

    case 0x05: 
      return precompiles::modexp(msg.input(), msg.gas());

    case 0x09:
      return precompiles::blake2f(msg.input(), msg.gas());

    default:
      throw DynamicException("Precompiled contract not found");
  }
}

bool PrecompiledContractExecutor::isPrecompiled(View<Address> address) const {
  if (address == RANDOM_GENERATOR_ADDRESS) {
    return true;
  }

  if (std::ranges::any_of(address | std::views::take(19), [] (Byte b) { return b != 0; })) {
    return false;
  }

  return address[19] <= 0x05 || address[19] == 0x09;
}
