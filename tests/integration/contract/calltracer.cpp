/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#include "libs/catch2/catch_amalgamated.hpp"

#include "contract/calltracer.h"

#include "contract/templates/erc20.h"
#include "contract/templates/erc20wrapper.h"
#include "contract/templates/simplecontract.h"

#include "utils/uintconv.h"

#include "bytes/hex.h"

#include "../../sdktestsuite.hpp"

static const Bytes testBytecode = Hex::toBytes("0x608060405234801561001057600080fd5b50610219806100206000396000f3fe608060405234801561001057600080fd5b50600436106100415760003560e01c80631003e2d2146100465780634fa522db14610062578063853255cc14610092575b600080fd5b610060600480360381019061005b9190610129565b6100b0565b005b61007c60048036038101906100779190610129565b6100cb565b6040516100899190610165565b60405180910390f35b61009a6100e5565b6040516100a79190610165565b60405180910390f35b806000808282546100c191906101af565b9250508190555050565b60006100d6826100b0565b6100de6100e5565b9050919050565b60008054905090565b600080fd5b6000819050919050565b610106816100f3565b811461011157600080fd5b50565b600081359050610123816100fd565b92915050565b60006020828403121561013f5761013e6100ee565b5b600061014d84828501610114565b91505092915050565b61015f816100f3565b82525050565b600060208201905061017a6000830184610156565b92915050565b7f4e487b7100000000000000000000000000000000000000000000000000000000600052601160045260246000fd5b60006101ba826100f3565b91506101c5836100f3565b92508282019050808211156101dd576101dc610180565b5b9291505056fea264697066735822122010806f8bd0eb78dd8bf1e05d0621ad54dfe78cd22c6a67e02decd89cd4a2208064736f6c63430008130033");
static const Bytes testProxyBytecode = Hex::toBytes("0x608060405234801561001057600080fd5b50610341806100206000396000f3fe608060405234801561001057600080fd5b50600436106100365760003560e01c80637714eaca1461003b5780638e6113211461006b575b600080fd5b61005560048036038101906100509190610232565b61009b565b6040516100629190610281565b60405180910390f35b6100856004803603810190610080919061029c565b610121565b6040516100929190610281565b60405180910390f35b60008273ffffffffffffffffffffffffffffffffffffffff16634fa522db836040518263ffffffff1660e01b81526004016100d69190610281565b6020604051808303816000875af11580156100f5573d6000803e3d6000fd5b505050506040513d601f19601f8201168201806040525081019061011991906102de565b905092915050565b60008173ffffffffffffffffffffffffffffffffffffffff1663853255cc6040518163ffffffff1660e01b8152600401602060405180830381865afa15801561016e573d6000803e3d6000fd5b505050506040513d601f19601f8201168201806040525081019061019291906102de565b9050919050565b600080fd5b600073ffffffffffffffffffffffffffffffffffffffff82169050919050565b60006101c98261019e565b9050919050565b6101d9816101be565b81146101e457600080fd5b50565b6000813590506101f6816101d0565b92915050565b6000819050919050565b61020f816101fc565b811461021a57600080fd5b50565b60008135905061022c81610206565b92915050565b6000806040838503121561024957610248610199565b5b6000610257858286016101e7565b92505060206102688582860161021d565b9150509250929050565b61027b816101fc565b82525050565b60006020820190506102966000830184610272565b92915050565b6000602082840312156102b2576102b1610199565b5b60006102c0848285016101e7565b91505092915050565b6000815190506102d881610206565b92915050565b6000602082840312156102f4576102f3610199565b5b6000610302848285016102c9565b9150509291505056fea26469706673582212201c63da23a5ecb525a33d51b61ad178576a9dbc8733cc98401d2be1db76021bbf64736f6c63430008130033");
static const Bytes bankBytecode = Hex::toBytes("0x608060405234801561001057600080fd5b5061032d806100206000396000f3fe60806040526004361061003f5760003560e01c80631b9265b8146100445780632e1a7d4d1461004e578063b69ef8a814610077578063b6b55f25146100a2575b600080fd5b61004c6100cb565b005b34801561005a57600080fd5b506100756004803603810190610070919061018c565b6100cd565b005b34801561008357600080fd5b5061008c61012d565b60405161009991906101c8565b60405180910390f35b3480156100ae57600080fd5b506100c960048036038101906100c4919061018c565b610136565b005b565b600054811115610112576040517f08c379a000000000000000000000000000000000000000000000000000000000815260040161010990610240565b60405180910390fd5b80600080828254610123919061028f565b9250508190555050565b60008054905090565b8060008082825461014791906102c3565b9250508190555050565b600080fd5b6000819050919050565b61016981610156565b811461017457600080fd5b50565b60008135905061018681610160565b92915050565b6000602082840312156101a2576101a1610151565b5b60006101b084828501610177565b91505092915050565b6101c281610156565b82525050565b60006020820190506101dd60008301846101b9565b92915050565b600082825260208201905092915050565b7f496e73756666696369656e742066756e64730000000000000000000000000000600082015250565b600061022a6012836101e3565b9150610235826101f4565b602082019050919050565b600060208201905081810360008301526102598161021d565b9050919050565b7f4e487b7100000000000000000000000000000000000000000000000000000000600052601160045260246000fd5b600061029a82610156565b91506102a583610156565b92508282039050818111156102bd576102bc610260565b5b92915050565b60006102ce82610156565b91506102d983610156565b92508282019050808211156102f1576102f0610260565b5b9291505056fea264697066735822122039cd50686dd7d95580c137e4e22559d51652bbb6168e491335e61f023637c90064736f6c63430008130033");
static const Bytes userBytecode = Hex::toBytes("0x608060405234801561001057600080fd5b50610286806100206000396000f3fe608060405234801561001057600080fd5b506004361061002b5760003560e01c80637f3358bc14610030575b600080fd5b61004a600480360381019061004591906101b0565b610060565b604051610057919061020b565b60405180910390f35b6000808390508073ffffffffffffffffffffffffffffffffffffffff16632e1a7d4d846040518263ffffffff1660e01b815260040161009f9190610235565b600060405180830381600087803b1580156100b957600080fd5b505af19250505080156100ca575060015b61010b573d80600081146100fa576040519150601f19603f3d011682016040523d82523d6000602084013e6100ff565b606091505b50600092505050610111565b60019150505b92915050565b600080fd5b600073ffffffffffffffffffffffffffffffffffffffff82169050919050565b60006101478261011c565b9050919050565b6101578161013c565b811461016257600080fd5b50565b6000813590506101748161014e565b92915050565b6000819050919050565b61018d8161017a565b811461019857600080fd5b50565b6000813590506101aa81610184565b92915050565b600080604083850312156101c7576101c6610117565b5b60006101d585828601610165565b92505060206101e68582860161019b565b9150509250929050565b60008115159050919050565b610205816101f0565b82525050565b600060208201905061022060008301846101fc565b92915050565b61022f8161017a565b82525050565b600060208201905061024a6000830184610226565b9291505056fea2646970667358221220e204f7b085aefaf07b3264168721ab936407e278559be3bc40986bb0c9cde0d264736f6c63430008130033");

static inline Hash getLatestTransactionHash(const Blockchain& node) {
  return node.latest()->getTxs().front()->hash();
}

struct TestWrapper {
  uint256_t sum() const { return 0; }
  void add(const uint256_t& val) { }
  uint256_t addAndReturn(const uint256_t& val) { return 0; }

  static void registerContract() {
    ContractReflectionInterface::registerContractMethods<TestWrapper>(
      std::vector<std::string>{},
      std::make_tuple("sum", &TestWrapper::sum, FunctionTypes::View, std::vector<std::string>{}),
      std::make_tuple("add", &TestWrapper::add, FunctionTypes::View, std::vector<std::string>{"val"}),
      std::make_tuple("addAndReturn", &TestWrapper::addAndReturn, FunctionTypes::View, std::vector<std::string>{"val"})
    );
  }
};

struct TestProxyWrapper {
  uint256_t sumOf(const Address& addr) const { return 0; }
  uint256_t addToAndReturn(const Address& addr, const uint256_t& val) { return 0; }

  static void registerContract() {
    ContractReflectionInterface::registerContractMethods<TestProxyWrapper>(
      std::vector<std::string>{},

      std::make_tuple("sumOf", &TestProxyWrapper::sumOf, FunctionTypes::View, std::vector<std::string>{"addr"}),
      std::make_tuple("addToAndReturn", &TestProxyWrapper::addToAndReturn, FunctionTypes::View, std::vector<std::string>{"addr", "val"})
    );
  }
};

struct BankWrapper {
  uint256_t balance() const { return 0; }
  void deposit(const uint256_t& amount) {}
  void withdraw(const uint256_t& amount) {}
  void pay() {}

  static void registerContract() {
    ContractReflectionInterface::registerContractMethods<BankWrapper>(
      std::vector<std::string>{},
      std::make_tuple("balance", &BankWrapper::balance, FunctionTypes::View, std::vector<std::string>{}),
      std::make_tuple("deposit", &BankWrapper::deposit, FunctionTypes::NonPayable, std::vector<std::string>{"amount"}),
      std::make_tuple("withdraw", &BankWrapper::withdraw, FunctionTypes::NonPayable, std::vector<std::string>{"amount"}),
      std::make_tuple("pay", &BankWrapper::pay, FunctionTypes::Payable, std::vector<std::string>{})
    );
  }
};

struct UserWrapper {
  bool tryWithdraw(const Address& bank_addr, const uint256_t& amount) { return false; }

  static void registerContract() {
    ContractReflectionInterface::registerContractMethods<UserWrapper>(
      std::vector<std::string>{},
      std::make_tuple("tryWithdraw", &UserWrapper::tryWithdraw, FunctionTypes::NonPayable, std::vector<std::string>{"bank_addr", "amount"})
    );
  }
};

namespace TCallTracer {
  TEST_CASE("CallTracer Tests", "[integration][contract][calltracer]") {
    SECTION("EVM Single Call") {
      SDKTestSuite sdk = SDKTestSuite::createNewEnvironment("TestTraceContracts");

      const Address contractAddress = sdk.deployBytecode(testBytecode);

      uint256_t res;

      sdk.callViewFunction(contractAddress, &TestWrapper::sum);
      REQUIRE(res == 0);

      sdk.callFunction(contractAddress, &TestWrapper::add, uint256_t(33));

      Hash txHash = getLatestTransactionHash(sdk);
      std::optional<trace::Call> callTrace = sdk.getStorage().getCallTrace(txHash);
      REQUIRE(callTrace);

      REQUIRE(callTrace->type == trace::CallType::CALL);
      REQUIRE(callTrace->from == sdk.getOptions().getChainOwner());
      REQUIRE(callTrace->to == contractAddress);
      REQUIRE(callTrace->value == FixedBytes<32>());
      // TODO: gas and gasUsed?
      REQUIRE(FixedBytes<32>(callTrace->input | std::views::drop(4)) == FixedBytes<32>(UintConv::uint256ToBytes(uint256_t(33))));

      REQUIRE(callTrace->output == Bytes());
      REQUIRE(callTrace->calls.empty());

      res = sdk.callViewFunction(contractAddress, &TestWrapper::sum);
      REQUIRE(res == 33);

      sdk.callFunction(contractAddress, &TestWrapper::addAndReturn, uint256_t(66));

      txHash = getLatestTransactionHash(sdk);
      callTrace = sdk.getStorage().getCallTrace(txHash);
      REQUIRE(callTrace);

      REQUIRE(callTrace->type == trace::CallType::CALL);
      REQUIRE(callTrace->from == sdk.getOptions().getChainOwner());
      REQUIRE(callTrace->to == contractAddress);
      REQUIRE(callTrace->value == FixedBytes<32>());
      // TODO: gas and gasUsed?
      REQUIRE(FixedBytes<32>(callTrace->input | std::views::drop(4)) == FixedBytes<32>(UintConv::uint256ToBytes(uint256_t(66))));

      REQUIRE(callTrace->output == Utils::makeBytes(UintConv::uint256ToBytes(uint256_t(99))));
      REQUIRE(callTrace->calls.empty());

      res = sdk.callViewFunction(contractAddress, &TestWrapper::sum);
      REQUIRE(res == 99);
    }

    SECTION("EVM Nested Calls") {
      SDKTestSuite sdk = SDKTestSuite::createNewEnvironment("TestTraceContracts");
      const Address testContractAddress = sdk.deployBytecode(testBytecode);
      const Address testProxyContractAddress = sdk.deployBytecode(testProxyBytecode);

      uint256_t res;

      res = sdk.callViewFunction(testContractAddress, &TestWrapper::sum);
      REQUIRE(res == 0);

      sdk.callFunction(testContractAddress, &TestWrapper::add, uint256_t(45));

      res = sdk.callViewFunction(testProxyContractAddress, &TestProxyWrapper::sumOf, testContractAddress);
      REQUIRE(res == 45);

      sdk.callFunction(testProxyContractAddress, &TestProxyWrapper::addToAndReturn, testContractAddress, uint256_t(55));

      const Hash txHash = getLatestTransactionHash(sdk);
      std::optional<trace::Call> callTrace = sdk.getStorage().getCallTrace(txHash);
      REQUIRE(callTrace);

      REQUIRE(callTrace->type == trace::CallType::CALL);
      REQUIRE(callTrace->from == sdk.getOptions().getChainOwner());
      REQUIRE(callTrace->to == testProxyContractAddress);
      REQUIRE(callTrace->value == FixedBytes<32>());
      // TODO: gas and gasUsed
      REQUIRE(Address(callTrace->input | std::views::drop(16) | std::views::take(20)) == testContractAddress);
      REQUIRE(FixedBytes<32>(callTrace->input | std::views::drop(36)) == FixedBytes<32>(UintConv::uint256ToBytes(uint256_t(55))));

      REQUIRE(callTrace->output == Utils::makeBytes(UintConv::uint256ToBytes(uint256_t(100))));
      REQUIRE(callTrace->calls.size() == 1);

      const trace::Call& nestedCall = callTrace->calls.front();

      REQUIRE(nestedCall.type == trace::CallType::CALL);
      REQUIRE(nestedCall.from == testProxyContractAddress);
      REQUIRE(nestedCall.to == testContractAddress);
      REQUIRE(nestedCall.value == FixedBytes<32>());
      REQUIRE(FixedBytes<32>(nestedCall.input | std::views::drop(4)) == FixedBytes<32>(UintConv::uint256ToBytes(uint256_t(55))));

      REQUIRE(nestedCall.output == Utils::makeBytes(UintConv::uint256ToBytes(uint256_t(100))));
      REQUIRE(nestedCall.calls.empty());

      res = sdk.callViewFunction(testContractAddress, &TestWrapper::sum);
      REQUIRE(res == 100);
    }

    SECTION("Native Contracts") {
      SDKTestSuite sdk = SDKTestSuite::createNewEnvironment("TestCallTracerOfErc20Wrapper");

      const Address erc20 = sdk.deployContract<ERC20>(
        std::string("TestToken"), std::string("TST"), uint8_t(18), uint256_t("1000000000000000000")
      );

      const Address erc20Wrapper = sdk.deployContract<ERC20Wrapper>();
      const Address owner = sdk.getChainOwnerAccount().address;

      for (const auto& [name, address] : sdk.getState().getCppContracts()) {
        if (name == "ERC20")
          REQUIRE(address == erc20);
        else if (name == "ERC20Wrapper")
          REQUIRE(address == erc20Wrapper);
      }

      const Hash approveTx = sdk.callFunction(erc20, &ERC20::approve, erc20Wrapper, uint256_t("500000000000000000"));
      const Hash depositTx = sdk.callFunction(erc20Wrapper, &ERC20Wrapper::deposit, erc20, uint256_t("500000000000000000"));

      const auto approveCallTrace = sdk.getStorage().getCallTrace(approveTx);
      const auto depositCallTrace = sdk.getStorage().getCallTrace(depositTx);

      REQUIRE(approveCallTrace);
      REQUIRE(approveCallTrace->type == trace::CallType::CALL);
      REQUIRE(approveCallTrace->status == trace::CallStatus::SUCCEEDED);
      REQUIRE(approveCallTrace->from == sdk.getOptions().getChainOwner());
      REQUIRE(approveCallTrace->to == erc20);
      REQUIRE(approveCallTrace->value == FixedBytes<32>());
      REQUIRE(approveCallTrace->input == Hex::toBytes("0x095ea7b30000000000000000000000006d48fdfe009e309dd5c4e69dec87365bfa0c811900000000000000000000000000000000000000000000000006f05b59d3b20000"));
      REQUIRE(approveCallTrace->output == Utils::makeBytes(bytes::hex("0x0000000000000000000000000000000000000000000000000000000000000001")));
      REQUIRE(approveCallTrace->calls.empty());

      REQUIRE(depositCallTrace);
      REQUIRE(depositCallTrace->type == trace::CallType::CALL);
      REQUIRE(depositCallTrace->status == trace::CallStatus::SUCCEEDED);
      REQUIRE(depositCallTrace->from == sdk.getOptions().getChainOwner());
      REQUIRE(depositCallTrace->to == erc20Wrapper);
      REQUIRE(depositCallTrace->value == FixedBytes<32>());
      REQUIRE(depositCallTrace->input == Hex::toBytes("0x47e7ef240000000000000000000000005b41cef7f46a4a147e31150c3c5ffd077e54d0e100000000000000000000000000000000000000000000000006f05b59d3b20000"));
      REQUIRE(depositCallTrace->output == Bytes());
      REQUIRE(!depositCallTrace->calls.empty());
      REQUIRE(depositCallTrace->calls[0].type == trace::CallType::CALL);
      REQUIRE(depositCallTrace->calls[0].status == trace::CallStatus::SUCCEEDED);
      REQUIRE(depositCallTrace->calls[0].from == erc20Wrapper);
      REQUIRE(depositCallTrace->calls[0].to == erc20);
      REQUIRE(depositCallTrace->calls[0].value == FixedBytes<32>());
      REQUIRE(depositCallTrace->calls[0].input == Hex::toBytes("0x23b872dd00000000000000000000000000dead00665771855a34155f5e7405489df2c3c60000000000000000000000006d48fdfe009e309dd5c4e69dec87365bfa0c811900000000000000000000000000000000000000000000000006f05b59d3b20000"));
      REQUIRE(depositCallTrace->calls[0].output == Hex::toBytes("0x0000000000000000000000000000000000000000000000000000000000000001"));
      REQUIRE(depositCallTrace->calls[0].calls.empty());
    }

    SECTION("Errors and payable functions") {
      SDKTestSuite sdk = SDKTestSuite::createNewEnvironment("TestCallTracingErrosAndPays");

      const Address bankAddress = sdk.deployBytecode(bankBytecode);
      const Address userAddress = sdk.deployBytecode(userBytecode);

      uint256_t res = sdk.callViewFunction(bankAddress, &BankWrapper::balance);
      REQUIRE(res == 0);

      const Hash depositTxHash = sdk.callFunction(bankAddress, &BankWrapper::deposit, uint256_t(500));
      res = sdk.callViewFunction(bankAddress, &BankWrapper::balance);
      REQUIRE(res == 500);

      const Hash invalidWithdrawTxHash = sdk.callFunction(userAddress, &UserWrapper::tryWithdraw, bankAddress, uint256_t(501));
      const Hash validWithdrawTxHash = sdk.callFunction(userAddress, &UserWrapper::tryWithdraw, bankAddress, uint256_t(300));
      const Hash payTxHash = sdk.callFunction(bankAddress, &BankWrapper::pay, uint256_t(4568));

      const auto errorCallTrace = sdk.getStorage().getCallTrace(invalidWithdrawTxHash);
      const auto successCallTrace = sdk.getStorage().getCallTrace(validWithdrawTxHash);
      const auto payCallTrace = sdk.getStorage().getCallTrace(payTxHash);

      REQUIRE(errorCallTrace);
      REQUIRE(errorCallTrace->type == trace::CallType::CALL);
      REQUIRE(errorCallTrace->status == trace::CallStatus::SUCCEEDED);
      REQUIRE(errorCallTrace->from == sdk.getOptions().getChainOwner());
      REQUIRE(errorCallTrace->to == userAddress);
      REQUIRE(errorCallTrace->value == FixedBytes<32>());
      REQUIRE(errorCallTrace->input == Hex::toBytes("0x7f3358bc0000000000000000000000005b41cef7f46a4a147e31150c3c5ffd077e54d0e100000000000000000000000000000000000000000000000000000000000001f5"));
      REQUIRE(errorCallTrace->output == Bytes(32));
      REQUIRE(!errorCallTrace->calls.empty());
      REQUIRE(errorCallTrace->calls[0].type == trace::CallType::CALL);
      REQUIRE(errorCallTrace->calls[0].status == trace::CallStatus::EXECUTION_REVERTED);
      REQUIRE(errorCallTrace->calls[0].from == userAddress);
      REQUIRE(errorCallTrace->calls[0].to == bankAddress);
      REQUIRE(errorCallTrace->calls[0].value == FixedBytes<32>());
      REQUIRE(errorCallTrace->calls[0].input == Hex::toBytes("0x2e1a7d4d00000000000000000000000000000000000000000000000000000000000001f5"));
      REQUIRE(errorCallTrace->calls[0].output == ABI::Encoder::encodeError("Insufficient funds"));
      REQUIRE(errorCallTrace->calls[0].calls.empty());

      REQUIRE(successCallTrace);
      REQUIRE(successCallTrace->type == trace::CallType::CALL);
      REQUIRE(successCallTrace->status == trace::CallStatus::SUCCEEDED);
      REQUIRE(successCallTrace->from == sdk.getOptions().getChainOwner());
      REQUIRE(successCallTrace->to == userAddress);
      REQUIRE(successCallTrace->value == FixedBytes<32>());
      REQUIRE(successCallTrace->input == Hex::toBytes("0x7f3358bc0000000000000000000000005b41cef7f46a4a147e31150c3c5ffd077e54d0e1000000000000000000000000000000000000000000000000000000000000012c"));
      REQUIRE(successCallTrace->output == Hex::toBytes("0x0000000000000000000000000000000000000000000000000000000000000001"));
      REQUIRE(!successCallTrace->calls.empty());
      REQUIRE(successCallTrace->calls[0].type == trace::CallType::CALL);
      REQUIRE(successCallTrace->calls[0].status == trace::CallStatus::SUCCEEDED);
      REQUIRE(successCallTrace->calls[0].from == userAddress);
      REQUIRE(successCallTrace->calls[0].to == bankAddress);
      REQUIRE(successCallTrace->calls[0].value == FixedBytes<32>());
      REQUIRE(successCallTrace->calls[0].input == Hex::toBytes("0x2e1a7d4d000000000000000000000000000000000000000000000000000000000000012c"));
      REQUIRE(successCallTrace->calls[0].output == Bytes());
      REQUIRE(successCallTrace->calls[0].calls.empty());

      REQUIRE(payCallTrace);
      REQUIRE(payCallTrace->type == trace::CallType::CALL);
      REQUIRE(payCallTrace->status == trace::CallStatus::SUCCEEDED);
      REQUIRE(payCallTrace->from == sdk.getOptions().getChainOwner());
      REQUIRE(payCallTrace->to == bankAddress);
      REQUIRE(payCallTrace->value == FixedBytes<32>(UintConv::uint256ToBytes(uint256_t(4568))));
      REQUIRE(payCallTrace->input == Hex::toBytes("0x1b9265b8"));
      REQUIRE(payCallTrace->output == Bytes());
      REQUIRE(payCallTrace->calls.empty());
    }
  }
}

