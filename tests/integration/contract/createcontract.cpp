/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#include "libs/catch2/catch_amalgamated.hpp"

#include "contract/templates/randomnesstest.h"

#include "../../sdktestsuite.hpp"

/**
 * Contract Create Another Contract Solidity Contract:
 *   // SPDX-License-Identifier: MIT
 *   pragma solidity ^0.8;
 *
 *   contract DeployedContract {
 *       uint256 public value;
 *
 *       constructor(uint256 _value) {
 *           value = _value;
 *       }
 *   }
 *
 *   contract DeployerContract {
 *       event ContractDeployed(address contractAddress);
 *       event ContractDeployedWithCreate2(address contractAddress, bytes32 salt);
 *
 *       // Deploy using the `new` keyword
 *       function deployWithNew(uint256 _value) public returns (address) {
 *           DeployedContract newContract = new DeployedContract(_value);
 *           address newContractAddress = address(newContract);
 *           emit ContractDeployed(newContractAddress);
 *           return newContractAddress;
 *       }
 *
 *       // Deploy using CREATE2 opcode
 *       function deployWithCreate2(uint256 _value, bytes32 _salt) public returns (address) {
 *           bytes memory bytecode = abi.encodePacked(type(DeployedContract).creationCode, abi.encode(_value));
 *           address newContractAddress;
 *
 *           assembly {
 *               newContractAddress := create2(0, add(bytecode, 0x20), mload(bytecode), _salt)
 *               if iszero(newContractAddress) {
 *                   revert(0, 0)
 *               }
 *           }
 *
 *           emit ContractDeployedWithCreate2(newContractAddress, _salt);
 *           return newContractAddress;
 *       }
 *
 *       // Compute the address of the contract to be deployed with CREATE2
 *       function computeCreate2Address(bytes32 _salt, uint256 _value) public view returns (address) {
 *           bytes memory bytecode = abi.encodePacked(type(DeployedContract).creationCode, abi.encode(_value));
 *           bytes32 bytecodeHash = keccak256(bytecode);
 *           return address(uint160(uint256(keccak256(abi.encodePacked(
 *               bytes1(0xff),
 *               address(this),
 *               _salt,
 *               bytecodeHash
 *           )))));
 *       }
 *   }
 */

class SolCreateContract {
  public:
    Address deployWithNew(const uint256_t& _value) { return Address(); }
    Address deployWithCreate2(const uint256_t& _value, const Hash& _salt) { return Address(); }
    Address computeCreate2Address(const Hash& _salt, const uint256_t& _value) { return Address(); }
    void static registerContract() {
      ContractReflectionInterface::registerContractMethods<
        SolCreateContract
      >(
        std::vector<std::string>{},
        std::make_tuple("deployWithNew", &SolCreateContract::deployWithNew, FunctionTypes::NonPayable, std::vector<std::string>{"_value"}),
        std::make_tuple("deployWithCreate2", &SolCreateContract::deployWithCreate2, FunctionTypes::NonPayable, std::vector<std::string>{"_value", "_salt"}),
        std::make_tuple("computeCreate2Address", &SolCreateContract::computeCreate2Address, FunctionTypes::NonPayable, std::vector<std::string>{"_salt","_value"})
      );
    }
};

namespace TContractRandomness {
  TEST_CASE("Contract Create Another Contract", "[integration][contract][createcontract]") {
    auto contractCreateAnotherContractBytecode = Hex::toBytes("6080604052348015600e575f80fd5b5061079d8061001c5f395ff3fe608060405234801561000f575f80fd5b506004361061003f575f3560e01c80630246c1bd1461004357806393d5252614610073578063fc858c24146100a3575b5f80fd5b61005d60048036038101906100589190610302565b6100d3565b60405161006a919061036c565b60405180910390f35b61008d600480360381019061008891906103b8565b61014d565b60405161009a919061036c565b60405180910390f35b6100bd60048036038101906100b891906103f6565b61020e565b6040516100ca919061036c565b60405180910390f35b5f80826040516100e2906102be565b6100ec9190610443565b604051809103905ff080158015610105573d5f803e3d5ffd5b5090505f8190507f8ffcdc15a283d706d38281f500270d8b5a656918f555de0913d7455e3e6bc1bf8160405161013b919061036c565b60405180910390a18092505050919050565b5f806040518060200161015f906102be565b6020820181038252601f19601f82011660405250846040516020016101849190610443565b6040516020818303038152906040526040516020016101a49291906104c8565b60405160208183030381529060405290505f838251602084015ff59050806101ca575f80fd5b7f83bd07281c54a9ab5bdb03b29af123d7033d997e633859dc80c70022d763771881856040516101fb9291906104fa565b60405180910390a1809250505092915050565b5f8060405180602001610220906102be565b6020820181038252601f19601f82011660405250836040516020016102459190610443565b6040516020818303038152906040526040516020016102659291906104c8565b60405160208183030381529060405290505f8180519060200120905060ff60f81b30868360405160200161029c94939291906105d1565b604051602081830303815290604052805190602001205f1c9250505092915050565b6101498061061f83390190565b5f80fd5b5f819050919050565b6102e1816102cf565b81146102eb575f80fd5b50565b5f813590506102fc816102d8565b92915050565b5f60208284031215610317576103166102cb565b5b5f610324848285016102ee565b91505092915050565b5f73ffffffffffffffffffffffffffffffffffffffff82169050919050565b5f6103568261032d565b9050919050565b6103668161034c565b82525050565b5f60208201905061037f5f83018461035d565b92915050565b5f819050919050565b61039781610385565b81146103a1575f80fd5b50565b5f813590506103b28161038e565b92915050565b5f80604083850312156103ce576103cd6102cb565b5b5f6103db858286016102ee565b92505060206103ec858286016103a4565b9150509250929050565b5f806040838503121561040c5761040b6102cb565b5b5f610419858286016103a4565b925050602061042a858286016102ee565b9150509250929050565b61043d816102cf565b82525050565b5f6020820190506104565f830184610434565b92915050565b5f81519050919050565b5f81905092915050565b5f5b8381101561048d578082015181840152602081019050610472565b5f8484015250505050565b5f6104a28261045c565b6104ac8185610466565b93506104bc818560208601610470565b80840191505092915050565b5f6104d38285610498565b91506104df8284610498565b91508190509392505050565b6104f481610385565b82525050565b5f60408201905061050d5f83018561035d565b61051a60208301846104eb565b9392505050565b5f7fff0000000000000000000000000000000000000000000000000000000000000082169050919050565b5f819050919050565b61056661056182610521565b61054c565b82525050565b5f8160601b9050919050565b5f6105828261056c565b9050919050565b5f61059382610578565b9050919050565b6105ab6105a68261034c565b610589565b82525050565b5f819050919050565b6105cb6105c682610385565b6105b1565b82525050565b5f6105dc8287610555565b6001820191506105ec828661059a565b6014820191506105fc82856105ba565b60208201915061060c82846105ba565b6020820191508190509594505050505056fe6080604052348015600e575f80fd5b506040516101493803806101498339818101604052810190602e9190606b565b805f81905550506091565b5f80fd5b5f819050919050565b604d81603d565b81146056575f80fd5b50565b5f815190506065816046565b92915050565b5f60208284031215607d57607c6039565b5b5f6088848285016059565b91505092915050565b60ac8061009d5f395ff3fe6080604052348015600e575f80fd5b50600436106026575f3560e01c80633fa4f24514602a575b5f80fd5b60306044565b604051603b9190605f565b60405180910390f35b5f5481565b5f819050919050565b6059816049565b82525050565b5f60208201905060705f8301846052565b9291505056fea2646970667358221220800668e87144b8625a7e59ac82528e013d51d6ed08562ba8b641f0a2e66c0f3764736f6c634300081a0033a264697066735822122057c5861494d2efb5d5f4f9c3be1a01d9bb2473e1b53e00c25866e51d52795c8c64736f6c634300081a0033");
    SECTION("EVM Create Another EVM Contract Test") {
      auto sdk = SDKTestSuite::createNewEnvironment("ContractCreateContract");
      auto createContractAddress = sdk.deployBytecode(contractCreateAnotherContractBytecode);
      Hash salt = bytes::random();
      REQUIRE(sdk.getState().getEvmContracts().size() == 1);
      sdk.callFunction(createContractAddress, &SolCreateContract::deployWithNew, uint256_t(100));
      Address newContractAddress = Address();
      // We need to actually loop the list of contracts to find the new contract address
      for (auto& contract : sdk.getState().getEvmContracts()) {
        if (contract != createContractAddress) {
          newContractAddress = contract;
          break;
        }
      }
      REQUIRE(sdk.getState().getEvmContracts().size() == 2);
      sdk.callFunction(createContractAddress, &SolCreateContract::deployWithCreate2, uint256_t(100), salt);
      auto newContractAddressCreate2 = Address();
      // As above, we have to loop the list of contracts to find the new contract address
      for (auto& contract : sdk.getState().getEvmContracts()) {
        if (contract != createContractAddress && contract != newContractAddress) {
          newContractAddressCreate2 = contract;
          break;
        }
      }
      REQUIRE(sdk.getState().getEvmContracts().size() == 3);
      REQUIRE(sdk.getNativeNonce(createContractAddress) == 2); // 1 when contract was created + 1 when contract called CREATE, CREATE2 does **NOT** increment nonce
      REQUIRE(newContractAddress == generateContractAddress(1, createContractAddress));
      Bytes init_code = Hex::toBytes("6080604052348015600e575f80fd5b506040516101493803806101498339818101604052810190602e9190606b565b805f81905550506091565b5f80fd5b5f819050919050565b604d81603d565b81146056575f80fd5b50565b5f815190506065816046565b92915050565b5f60208284031215607d57607c6039565b5b5f6088848285016059565b91505092915050565b60ac8061009d5f395ff3fe6080604052348015600e575f80fd5b50600436106026575f3560e01c80633fa4f24514602a575b5f80fd5b60306044565b604051603b9190605f565b60405180910390f35b5f5481565b5f819050919050565b6059816049565b82525050565b5f60208201905060705f8301846052565b9291505056fea2646970667358221220800668e87144b8625a7e59ac82528e013d51d6ed08562ba8b641f0a2e66c0f3764736f6c634300081a00330000000000000000000000000000000000000000000000000000000000000064");
      REQUIRE(newContractAddressCreate2 == generateContractAddress(createContractAddress, salt, init_code));

      // For coverage
      std::string contractCode = "6080604052348015600e575f80fd5b50600436106026575f3560e01c80633fa4f24514602a575b5f80fd5b60306044565b604051603b9190605f565b60405180910390f35b5f5481565b5f819050919050565b6059816049565b82525050565b5f60208201905060705f8301846052565b9291505056fea2646970667358221220800668e87144b8625a7e59ac82528e013d51d6ed08562ba8b641f0a2e66c0f3764736f6c634300081a0033";
      REQUIRE(Hex::fromBytes(sdk.getState().getContractCode(newContractAddressCreate2)).get() == contractCode);
      Address randAdd(bytes::hex("0x1234567890123456789012345678901234567890"));
      REQUIRE(Hex::fromBytes(sdk.getState().getContractCode(randAdd)).get() == "");
    }
  }
}

