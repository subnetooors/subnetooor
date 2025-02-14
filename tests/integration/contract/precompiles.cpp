/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#include "libs/catch2/catch_amalgamated.hpp"

#include "contract/precompiles.h"

#include "bytes/hex.h"

#include "../../sdktestsuite.hpp"

namespace TPRECOMPILES {
  struct EcRecoverCaller {
    Address callEcRecover(const Hash&, const uint8_t&, const Hash&, const Hash&) const { return Address{}; }

    static void registerContract() {
      ContractReflectionInterface::registerContractMethods<EcRecoverCaller>(
        std::vector<std::string>{},
        std::make_tuple("callEcRecover", &EcRecoverCaller::callEcRecover, FunctionTypes::View, std::vector<std::string>{"hash", "v", "r", "s"})
      );
    }
  };

  TEST_CASE("Precompiles", "[integration][contract][precompiles]") {
    SECTION("valid ecrecover") {
      const Hash hash = bytes::hex("0x456e9aea5e197a1f1af7a3e85a3212fa4049a3ba34c2289b4c860fc0b0c64ef3");
      const uint8_t v = 28;
      const Hash r = bytes::hex("0x9242685bf161793cc25603c231bc2f568eb630ea16aa137d2664ac8038825608");
      const Hash s = bytes::hex("0x4f8ae3bd7535248d0bd448298cc2e2071e56992d0774dc340c368ae950852ada");

      const Address expectedResult = bytes::hex("0x7156526fbd7a3c72969b54f64e42c10fbb768c8a");

      REQUIRE(ecrecover(hash, v, r, s) == expectedResult);
    }

    SECTION("contract call for ecrecover") {
      const Bytes bytecode = Utils::makeBytes(bytes::hex("0x608060405234801561000f575f80fd5b5061035f8061001d5f395ff3fe608060405234801561000f575f80fd5b5060043610610029575f3560e01c8063265dc68c1461002d575b5f80fd5b61004760048036038101906100429190610194565b61005d565b6040516100549190610237565b60405180910390f35b5f806001868686866040515f8152602001604052604051610081949392919061026e565b6020604051602081039080840390855afa1580156100a1573d5f803e3d5ffd5b5050506020604051035190505f73ffffffffffffffffffffffffffffffffffffffff168173ffffffffffffffffffffffffffffffffffffffff160361011b576040517f08c379a00000000000000000000000000000000000000000000000000000000081526004016101129061030b565b60405180910390fd5b80915050949350505050565b5f80fd5b5f819050919050565b61013d8161012b565b8114610147575f80fd5b50565b5f8135905061015881610134565b92915050565b5f60ff82169050919050565b6101738161015e565b811461017d575f80fd5b50565b5f8135905061018e8161016a565b92915050565b5f805f80608085870312156101ac576101ab610127565b5b5f6101b98782880161014a565b94505060206101ca87828801610180565b93505060406101db8782880161014a565b92505060606101ec8782880161014a565b91505092959194509250565b5f73ffffffffffffffffffffffffffffffffffffffff82169050919050565b5f610221826101f8565b9050919050565b61023181610217565b82525050565b5f60208201905061024a5f830184610228565b92915050565b6102598161012b565b82525050565b6102688161015e565b82525050565b5f6080820190506102815f830187610250565b61028e602083018661025f565b61029b6040830185610250565b6102a86060830184610250565b95945050505050565b5f82825260208201905092915050565b7f696e76616c6964207369676e61747572650000000000000000000000000000005f82015250565b5f6102f56011836102b1565b9150610300826102c1565b602082019050919050565b5f6020820190508181035f830152610322816102e9565b905091905056fea26469706673582212200c75aacb55079af649692b63d700a6647842c0bd30bfef639e59862b2acbbed564736f6c63430008140033"));

      SDKTestSuite sdk = SDKTestSuite::createNewEnvironment("TestEcRecoverCall");
      const Address contract = sdk.deployBytecode(bytecode);

      const Hash hash = bytes::hex("0x456e9aea5e197a1f1af7a3e85a3212fa4049a3ba34c2289b4c860fc0b0c64ef3");
      const uint8_t v = 28;
      const Hash r = bytes::hex("0x9242685bf161793cc25603c231bc2f568eb630ea16aa137d2664ac8038825608");
      const Hash s = bytes::hex("0x4f8ae3bd7535248d0bd448298cc2e2071e56992d0774dc340c368ae950852ada");

      const Address expectedResult = bytes::hex("0x7156526fbd7a3c72969b54f64e42c10fbb768c8a");

      Address result = sdk.callViewFunction(contract, &EcRecoverCaller::callEcRecover, hash, v, r, s);

      REQUIRE(result == expectedResult);
    }
  }
}

