/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#include "libs/catch2/catch_amalgamated.hpp"
#include "libs/json.hpp"

#include "utils/jsonabi.h" // customcontracts.h -> dynamiccontract.h -> contracthost.h -> contractreflectioninterface.h -> contract/abi.h -> libs/json.hpp -> filesystem

using json = nlohmann::ordered_json; ///< Typedef for json.

/// Namespace for testing expected contract ABI generation outputs
namespace EXPECTED {
  namespace ERC20 {
    json transferFrom = {
      {"inputs", {
        { {"internalType", "address"}, {"name", "from"}, {"type", "address"} },
        { {"internalType", "address"}, {"name", "to"}, {"type", "address"} },
        { {"internalType", "uint256"}, {"name", "value"}, {"type", "uint256"} }
      }},
      {"name", "transferFrom"},
      {"outputs", {
        { {"internalType", "bool"}, {"name", ""}, {"type", "bool"} }
      }},
      {"stateMutability", "nonpayable"},
      {"type", "function"}
    };

    json approve = {
      {"inputs", {
        { {"internalType", "address"}, {"name", "spender"}, {"type", "address"} },
        { {"internalType", "uint256"}, {"name", "value"}, {"type", "uint256"} }
      }},
      {"name", "approve"},
      {"outputs", {
        { {"internalType", "bool"}, {"name", ""}, {"type", "bool"} }
      }},
      {"stateMutability", "nonpayable"},
      {"type", "function"}
    };

    json balanceOf = {
      {"inputs", {
        { {"internalType", "address"}, {"name", "owner"}, {"type", "address"} }
      }},
      {"name", "balanceOf"},
      {"outputs", {
        { {"internalType", "uint256"}, {"name", ""}, {"type", "uint256"} }
      }},
      {"stateMutability", "view"},
      {"type", "function"}
    };

    json totalSupply = {
      {"inputs", json::array()},
      {"name", "totalSupply"},
      {"outputs", {
        { {"internalType", "uint256"}, {"name", ""}, {"type", "uint256"} }
      }},
      {"stateMutability", "view"},
      {"type", "function"}
    };

    json transfer = {
      {"inputs", {
        { {"internalType", "address"}, {"name", "to"}, {"type", "address"} },
        { {"internalType", "uint256"}, {"name", "value"}, {"type", "uint256"} }
      }},
      {"name", "transfer"},
      {"outputs", {
        { {"internalType", "bool"}, {"name", ""}, {"type", "bool"} }
      }},
      {"stateMutability", "nonpayable"},
      {"type", "function"}
    };

    json decimals = {
      {"inputs", json::array()},
      {"name", "decimals"},
      {"outputs", {
        { {"internalType", "uint8"}, {"name", ""}, {"type", "uint8"} }
      }},
      {"stateMutability", "view"},
      {"type", "function"}
    };

    json symbol = {
      {"inputs", json::array()},
      {"name", "symbol"},
      {"outputs", {
        { {"internalType", "string"}, {"name", ""}, {"type", "string"} }
      }},
      {"stateMutability", "view"},
      {"type", "function"}
    };

    json allowance = {
      {"inputs", {
        { {"internalType", "address"}, {"name", "owner"}, {"type", "address"} },
        { {"internalType", "address"}, {"name", "spender"}, {"type", "address"} }
      }},
      {"name", "allowance"},
      {"outputs", {
        { {"internalType", "uint256"}, {"name", ""}, {"type", "uint256"} }
      }},
      {"stateMutability", "view"},
      {"type", "function"}
    };

    json name = {
      {"inputs", json::array()},
      {"name", "name"},
      {"outputs", {
        { {"internalType", "string"}, {"name", ""}, {"type", "string"} }
      }},
      {"stateMutability", "view"},
      {"type", "function"}
    };
  }

  namespace ERC20Wrapper {
    json deposit = {
      {"inputs", {
        { {"internalType", "address"}, {"name", "token"}, {"type", "address"} },
        { {"internalType", "uint256"}, {"name", "value"}, {"type", "uint256"} }
      }},
      {"name", "deposit"},
      {"outputs", json::array()},
      {"stateMutability", "nonpayable"},
      {"type", "function"}
    };

    json transferTo = {
      {"inputs", {
        { {"internalType", "address"}, {"name", "token"}, {"type", "address"} },
        { {"internalType", "address"}, {"name", "to"}, {"type", "address"} },
        { {"internalType", "uint256"}, {"name", "value"}, {"type", "uint256"} }
      }},
      {"name", "transferTo"},
      {"outputs", json::array()},
      {"stateMutability", "nonpayable"},
      {"type", "function"}
    };

    json withdraw = {
      {"inputs", {
        { {"internalType", "address"}, {"name", "token"}, {"type", "address"} },
        { {"internalType", "uint256"}, {"name", "value"}, {"type", "uint256"} }
      }},
      {"name", "withdraw"},
      {"outputs", json::array()},
      {"stateMutability", "nonpayable"},
      {"type", "function"}
    };

    json getUserBalance = {
      {"inputs", {
        { {"internalType", "address"}, {"name", "token"}, {"type", "address"} },
        { {"internalType", "address"}, {"name", "user"}, {"type", "address"} }
      }},
      {"name", "getUserBalance"},
      {"outputs", {
        { {"internalType", "uint256"}, {"name", ""}, {"type", "uint256"} }
      }},
      {"stateMutability", "view"},
      {"type", "function"}
    };

    json getContractBalance = {
      {"inputs", {
        { {"internalType", "address"}, {"name", "token"}, {"type", "address"} }
      }},
      {"name", "getContractBalance"},
      {"outputs", {
        { {"internalType", "uint256"}, {"name", ""}, {"type", "uint256"} }
      }},
      {"stateMutability", "view"},
      {"type", "function"}
    };
  }

  namespace NativeWrapper {
    json withdraw = {
      {"inputs", {
        { {"internalType", "uint256"}, {"name", "value"}, {"type", "uint256"} }
      }},
      {"name", "withdraw"},
      {"outputs", json::array()},
      {"stateMutability", "payable"},
      {"type", "function"}
    };

    json deposit = {
      {"inputs", json::array()},
      {"name", "deposit"},
      {"outputs", json::array()},
      {"stateMutability", "payable"},
      {"type", "function"}
    };
  }

  namespace ContractManager {
    json createNewERC20Contract = {
      {"inputs", {
        { {"internalType", "string"}, {"name", "erc20name"}, {"type", "string"} },
        { {"internalType", "string"}, {"name", "erc20symbol"}, {"type", "string"} },
        { {"internalType", "uint8"}, {"name", "erc20decimals"}, {"type", "uint8"} },
        { {"internalType", "uint256"}, {"name", "mintValue"}, {"type", "uint256"} }
      }},
      {"name", "createNewERC20Contract"},
      {"outputs", json::array()},
      {"stateMutability", "nonpayable"},
      {"type", "function"}
    };

    json createNewERC20WrapperContract = {
      {"inputs", json::array()},
      {"name", "createNewERC20WrapperContract"},
      {"outputs", json::array()},
      {"stateMutability", "nonpayable"},
      {"type", "function"}
    };

    json createNewNativeWrapperContract = {
      {"inputs", {
        { {"internalType", "string"}, {"name", "erc20_name"}, {"type", "string"} },
        { {"internalType", "string"}, {"name", "erc20_symbol"}, {"type", "string"} },
        { {"internalType", "uint8"}, {"name", "erc20_decimals"}, {"type", "uint8"} }
      }},
      {"name", "createNewNativeWrapperContract"},
      {"outputs", json::array()},
      {"stateMutability", "nonpayable"},
      {"type", "function"}
    };

    json createNewSimpleContractContract = {
      {"inputs", {
        { {"internalType", "string"}, {"name", "name_"}, {"type", "string"} },
        { {"internalType", "uint256"}, {"name", "number_"}, {"type", "uint256"} },
        {
          {"components", {
            { {"internalType", "string"}, {"type", "string"} },
            { {"internalType", "uint256"}, {"type", "uint256"} }
          } }, {"name", "tuple_"}, {"type", "tuple"}
        }
      }},
      {"name", "createNewSimpleContractContract"},
      {"outputs", json::array()},
      {"stateMutability", "nonpayable"},
      {"type", "function"}
    };

    json createNewDEXV2PairContract = {
      {"inputs", json::array()},
      {"name", "createNewDEXV2PairContract"},
      {"outputs", json::array()},
      {"stateMutability", "nonpayable"},
      {"type", "function"}
    };

    json createNewDEXV2FactoryContract = {
      {"inputs", {
        { {"internalType", "address"}, {"name", "_feeToSetter"}, {"type", "address"} }
      }},
      {"name", "createNewDEXV2FactoryContract"},
      {"outputs", json::array()},
      {"stateMutability", "nonpayable"},
      {"type", "function"}
    };

    json createNewDEXV2Router02Contract = {
      {"inputs", {
        { {"internalType", "address"}, {"name", "factory"}, {"type", "address"} },
        { {"internalType", "address"}, {"name", "wrappedNative"}, {"type", "address"} }
      }},
      {"name", "createNewDEXV2Router02Contract"},
      {"outputs", json::array()},
      {"stateMutability", "nonpayable"},
      {"type", "function"}
    };

    json createNewERC721Contract = {
      {"inputs", {
        { {"internalType", "string"}, {"name", "erc721name"}, {"type", "string"} },
        { {"internalType", "string"}, {"name", "erc721symbol"}, {"type", "string"} }
      }},
      {"name", "createNewERC721Contract"},
      {"outputs", json::array()},
      {"stateMutability", "nonpayable"},
      {"type", "function"}
    };

    json createNewThrowTestAContract = {
      {"inputs", json::array()},
      {"name", "createNewThrowTestAContract"},
      {"outputs", json::array()},
      {"stateMutability", "nonpayable"},
      {"type", "function"}
    };

    json createNewThrowTestBContract = {
      {"inputs", json::array()},
      {"name", "createNewThrowTestBContract"},
      {"outputs", json::array()},
      {"stateMutability", "nonpayable"},
      {"type", "function"}
    };

    json createNewThrowTestCContract = {
      {"inputs", json::array()},
      {"name", "createNewThrowTestCContract"},
      {"outputs", json::array()},
      {"stateMutability", "nonpayable"},
      {"type", "function"}
    };

    json getDeployedContracts = {
      {"inputs", json::array()},
      {"name", "getDeployedContracts"},
      {"outputs", {
        {
          {"components", {
            { {"internalType", "string"}, {"type", "string"} },
            { {"internalType", "address"}, {"type", "address"} }
          } }, {"type", "tuple[]"}
        }
      }},
      {"stateMutability", "view"},
      {"type", "function"}
    };

    json getDeployedContractsForCreator = {
      {"inputs", {
            { {"internalType", "address"}, {"name", "creator"}, {"type", "address"} }
      }},
      {"name", "getDeployedContractsForCreator"},
      {"outputs", {
          {
            {"components", {
              { {"internalType", "string"}, {"type", "string"} },
              { {"internalType", "address"}, {"type", "address"} }
            } }, {"type", "tuple[]"}
          }
      }},
      {"stateMutability", "view"},
      {"type", "function"}
    };
  }

  namespace SimpleContract {
    json getNamesAndNumbersInArrayOfArrays = {
      {"inputs", {
        { {"internalType", "uint256"}, {"name", "i"}, {"type", "uint256"} }
      }},
      {"name", "getNamesAndNumbersInArrayOfArrays"},
      {"outputs", {
        {
          {"components", {
            { {"internalType", "string"}, {"type", "string"} },
            { {"internalType", "uint256"}, {"type", "uint256"} }
          } }, {"type", "tuple[][]"}
        }
      }},
      {"stateMutability", "view"},
      {"type", "function"}
    };

    json getNamesAndNumbersInTuple = {
      {"inputs", {
        { {"internalType", "uint256"}, {"name", "i"}, {"type", "uint256"} }
      }},
      {"name", "getNamesAndNumbersInTuple"},
      {"outputs", {
        {
          {"components", {
            { {"internalType", "string"}, {"type", "string"} },
            { {"internalType", "uint256"}, {"type", "uint256"} }
          } }, {"type", "tuple[]"}
        }
      }},
      {"stateMutability", "view"},
      {"type", "function"}
    };

    json getNameAndNumber = {
      {"inputs", json::array()},
      {"name", "getNameAndNumber"},
      {"outputs", {
        {
          {"components", {
            { {"internalType", "string"}, {"type", "string"} },
            { {"internalType", "uint256"}, {"type", "uint256"} }
          } }, {"type", "tuple"}
        }
      }},
      {"stateMutability", "view"},
      {"type", "function"}
    };

    json getNumbers = {
      {"inputs", {
        { {"internalType", "uint256"}, {"name", "i"}, {"type", "uint256"} }
      }},
      {"name", "getNumbers"},
      {"outputs", {
        { {"internalType", "uint256[]"}, {"name", ""}, {"type", "uint256[]"} }
      }},
      {"stateMutability", "view"},
      {"type", "function"}
    };

    json getNumber = {
      {"inputs", json::array()},
      {"name", "getNumber"},
      {"outputs", {
        { {"internalType", "uint256"}, {"name", ""}, {"type", "uint256"} }
      }},
      {"stateMutability", "view"},
      {"type", "function"}
    };

    json getName = {
      {"inputs", json::array()},
      {"name", "getName"},
      {"outputs", {
        { {"internalType", "string"}, {"name", ""}, {"type", "string"} }
      }},
      {"stateMutability", "view"},
      {"type", "function"}
    };

    json getTuple = {
      {"inputs", json::array()},
      {"name", "getTuple"},
      {"outputs", {
        {
          {"components", {
            { {"internalType", "string"}, {"type", "string"} },
            { {"internalType", "uint256"}, {"type", "uint256"} }
          } }, {"type", "tuple"}
        }
      }},
      {"stateMutability", "view"},
      {"type", "function"}
    };

    json setNamesAndNumbersInArrayOfArrays = {
      {"inputs", {
        {
          {"components", {
            { {"internalType", "string"}, {"type", "string"} },
            { {"internalType", "uint256"}, {"type", "uint256"} }
          } }, {"name", "argNameAndNumber"}, {"type", "tuple[][]"}
        }
      }},
      {"name", "setNamesAndNumbersInArrayOfArrays"},
      {"outputs", json::array()},
      {"stateMutability", "nonpayable"},
      {"type", "function"}
    };

    json setNamesAndNumbersInTuple = {
      {"inputs", {
        {
          {"components", {
            { {"internalType", "string"}, {"type", "string"} },
            { {"internalType", "uint256"}, {"type", "uint256"} }
          } }, {"name", "argNameAndNumber"}, {"type", "tuple[]"}
        }
      }},
      {"name", "setNamesAndNumbersInTuple"},
      {"outputs", json::array()},
      {"stateMutability", "nonpayable"},
      {"type", "function"}
    };

    json getNames = {
      {"inputs", {
        { {"internalType", "uint256"}, {"name", "i"}, {"type", "uint256"} },
      }},
      {"name", "getNames"},
      {"outputs", {
        { {"internalType", "string[]"}, {"name", ""}, {"type", "string[]"} },
      }},
      {"stateMutability", "view"},
      {"type", "function"}
    };

    json setNumbers = {
      {"inputs", {
        { {"internalType", "uint256[]"}, {"name", "argNumber"}, {"type", "uint256[]"} },
      }},
      {"name", "setNumbers"},
      {"outputs", json::array()},
      {"stateMutability", "nonpayable"},
      {"type", "function"}
    };

    json setNumber = {
      {"inputs", {
        { {"internalType", "uint256"}, {"name", "argNumber"}, {"type", "uint256"} },
      }},
      {"name", "setNumber"},
      {"outputs", json::array()},
      {"stateMutability", "nonpayable"},
      {"type", "function"}
    };

    json setNamesAndNumbers = {
      {"inputs", {
        { {"internalType", "string[]"}, {"name", "argName"}, {"type", "string[]"} },
        { {"internalType", "uint256[]"}, {"name", "argNumber"}, {"type", "uint256[]"} }
      }},
      {"name", "setNamesAndNumbers"},
      {"outputs", json::array()},
      {"stateMutability", "nonpayable"},
      {"type", "function"}
    };

    json setNames = {
      {"inputs", {
        { {"internalType", "string[]"}, {"name", "argName"}, {"type", "string[]"} }
      }},
      {"name", "setNames"},
      {"outputs", json::array()},
      {"stateMutability", "nonpayable"},
      {"type", "function"}
    };

    json getNamesAndNumbers = {
      {"inputs", {
        { {"internalType", "uint256"}, {"name", "i"}, {"type", "uint256"} }
      }},
      {"name", "getNamesAndNumbers"},
      {"outputs", {
        {
          {"components", {
            { {"internalType", "string[]"}, {"type", "string[]"} },
            { {"internalType", "uint256[]"}, {"type", "uint256[]"} }
          } }, {"type", "tuple"}
        }
      }},
      {"stateMutability", "view"},
      {"type", "function"}
    };

    json setName = {
      {"inputs", {
        { {"internalType", "string"}, {"name", "argName"}, {"type", "string"} }
      }},
      {"name", "setName"},
      {"outputs", json::array()},
      {"stateMutability", "nonpayable"},
      {"type", "function"}
    };

    json getNumberOverload = {
      {"inputs", {
        { {"internalType", "uint256"}, {"name", ""}, {"type", "uint256"} }
      }},
      {"name", "getNumber"},
      {"outputs", {
        { {"internalType", "uint256"}, {"name", ""}, {"type", "uint256"} }
      }},
      {"stateMutability", "view"},
      {"type", "function"}
    };

    json setTuple = {
      {"inputs", {
        {
          {"components", {
            { {"internalType", "string"}, {"type", "string"} },
            { {"internalType", "uint256"}, {"type", "uint256"} }
          } }, {"name", "argTuple"}, {"type", "tuple"}
        }
      }},
      {"name", "setTuple"},
      {"outputs", json::array()},
      {"stateMutability", "nonpayable"},
      {"type", "function"}
    };

    json nameAndNumberTupleChanged = {
      {"anonymous", false},
      {"inputs", {
        {
          {"components", {
            { {"internalType", "string"}, {"type", "string"} },
            { {"internalType", "uint256"}, {"type", "uint256"} }
          } }, {"indexed", true}, {"name", "nameAndNumber"}, {"type", "tuple"}
        }
      }},
      {"name", "nameAndNumberTupleChanged"},
      {"type", "event"}
    };

    json nameAndNumberChanged = {
      {"anonymous", false},
      {"inputs", {
        { {"indexed", true}, {"internalType", "string"}, {"name", "name"}, {"type", "string"} },
        { {"indexed", true}, {"internalType", "uint256"}, {"name", "number"}, {"type", "uint256"} }
      }},
      {"name", "nameAndNumberChanged"},
      {"type", "event"}
    };

    json numberChanged = {
      {"anonymous", false},
      {"inputs", {
        { {"indexed", false}, {"internalType", "uint256"}, {"name", "number"}, {"type", "uint256"} }
      }},
      {"name", "numberChanged"},
      {"type", "event"}
    };

    json nameChanged = {
      {"anonymous", false},
      {"inputs", {
        { {"indexed", true}, {"internalType", "string"}, {"name", "name"}, {"type", "string"} }
      }},
      {"name", "nameChanged"},
      {"type", "event"}
    };

    json tupleChanged = {
      {"anonymous", false},
      {"inputs", {
        {
          {"components", {
            { {"internalType", "string"}, {"type", "string"} },
            { {"internalType", "uint256"}, {"type", "uint256"} }
          } }, {"indexed", true}, {"name", "tuple"}, {"type", "tuple"}
        }
      }},
      {"name", "tupleChanged"},
      {"type", "event"}
    };
  }
}

TEST_CASE("ContractABIGenerator Helper", "[unit][contract][contractabigenerator]") {
  SECTION("writeContractsToJson") {
    REQUIRE(JsonAbi::writeContractsToJson<ERC20, ERC20Wrapper, NativeWrapper, SimpleContract>() == 0);
    REQUIRE(std::filesystem::exists("ABI/ERC20.json"));
    REQUIRE(std::filesystem::exists("ABI/ERC20Wrapper.json"));
    REQUIRE(std::filesystem::exists("ABI/NativeWrapper.json"));
    REQUIRE(std::filesystem::exists("ABI/ContractManager.json"));
    REQUIRE(std::filesystem::exists("ABI/SimpleContract.json"));
  }

  SECTION("Check file content ERC20") {
    json j;
    std::ifstream i("ABI/ERC20.json");
    i >> j;
    REQUIRE(j.size() == 11);

    auto findTransferFrom = std::find(j.begin(), j.end(), EXPECTED::ERC20::transferFrom);
    REQUIRE(findTransferFrom != j.end());
    auto findApprove = std::find(j.begin(), j.end(), EXPECTED::ERC20::approve);
    REQUIRE(findApprove != j.end());
    auto findBalanceOf = std::find(j.begin(), j.end(), EXPECTED::ERC20::balanceOf);
    REQUIRE(findBalanceOf != j.end());
    auto findTotalSupply = std::find(j.begin(), j.end(), EXPECTED::ERC20::totalSupply);
    REQUIRE(findTotalSupply != j.end());
    auto findTransfer = std::find(j.begin(), j.end(), EXPECTED::ERC20::transfer);
    REQUIRE(findTransfer != j.end());
    auto findAllowance = std::find(j.begin(), j.end(), EXPECTED::ERC20::allowance);
    REQUIRE(findAllowance != j.end());
    auto findDecimals = std::find(j.begin(), j.end(), EXPECTED::ERC20::decimals);
    REQUIRE(findDecimals != j.end());
    auto findSymbol = std::find(j.begin(), j.end(), EXPECTED::ERC20::symbol);
    REQUIRE(findSymbol != j.end());
    auto findName = std::find(j.begin(), j.end(), EXPECTED::ERC20::name);
    REQUIRE(findName != j.end());
  }

  SECTION("Check file content ERC20Wrapper") {
    json j;
    std::ifstream i("ABI/ERC20Wrapper.json");
    i >> j;
    REQUIRE(j.size() == 5);

    auto findDeposit = std::find(j.begin(), j.end(), EXPECTED::ERC20Wrapper::deposit);
    REQUIRE(findDeposit != j.end());
    auto findTransferTo = std::find(j.begin(), j.end(), EXPECTED::ERC20Wrapper::transferTo);
    REQUIRE(findTransferTo != j.end());
    auto findWithdraw = std::find(j.begin(), j.end(), EXPECTED::ERC20Wrapper::withdraw);
    REQUIRE(findWithdraw != j.end());
    auto findGetUserBalance = std::find(j.begin(), j.end(), EXPECTED::ERC20Wrapper::getUserBalance);
    REQUIRE(findGetUserBalance != j.end());
    auto findGetContractBalance = std::find(j.begin(), j.end(), EXPECTED::ERC20Wrapper::getContractBalance);
    REQUIRE(findGetContractBalance != j.end());
}

  SECTION("Check file content NativeWrapper") {
    json j;
    std::ifstream i("ABI/NativeWrapper.json");
    i >> j;
    REQUIRE(j.size() == 2);

    auto findDeposit = std::find(j.begin(), j.end(), EXPECTED::NativeWrapper::deposit);
    REQUIRE(findDeposit != j.end());
    auto findWithdraw = std::find(j.begin(), j.end(), EXPECTED::NativeWrapper::withdraw);
    REQUIRE(findWithdraw != j.end());
  }

  SECTION("Check file content ContractManager") {
    json j;
    std::ifstream i("ABI/ContractManager.json");
    i >> j;
    REQUIRE(j.size() == std::tuple_size<ContractTypes>() + 2); // (2 extra functions, getDeployedContracts and getDeployedContractsForCreator)

    auto findCreateNewERC20Contract = std::find(j.begin(), j.end(), EXPECTED::ContractManager::createNewERC20Contract);
    REQUIRE(findCreateNewERC20Contract != j.end());
    auto findCreateNewERC20WrapperContract = std::find(j.begin(), j.end(), EXPECTED::ContractManager::createNewERC20WrapperContract);
    REQUIRE(findCreateNewERC20WrapperContract != j.end());
    auto findCreateNewNativeWrapperContract = std::find(j.begin(), j.end(), EXPECTED::ContractManager::createNewNativeWrapperContract);
    REQUIRE(findCreateNewNativeWrapperContract != j.end());
    auto findCreateNewSimpleContractContract = std::find(j.begin(), j.end(), EXPECTED::ContractManager::createNewSimpleContractContract);
    REQUIRE(findCreateNewSimpleContractContract != j.end());
    auto findCreateNewDEXV2PairContract = std::find(j.begin(), j.end(), EXPECTED::ContractManager::createNewDEXV2PairContract);
    REQUIRE(findCreateNewDEXV2PairContract != j.end());
    auto findCreateNewDEXV2Router02Contract = std::find(j.begin(), j.end(), EXPECTED::ContractManager::createNewDEXV2Router02Contract);
    REQUIRE(findCreateNewDEXV2Router02Contract != j.end());
    auto findCreateNewDEXV2FactoryContract = std::find(j.begin(), j.end(), EXPECTED::ContractManager::createNewDEXV2FactoryContract);
    REQUIRE(findCreateNewDEXV2FactoryContract != j.end());
    auto findCreateNewERC721Contract = std::find(j.begin(), j.end(), EXPECTED::ContractManager::createNewERC721Contract);
    REQUIRE(findCreateNewERC721Contract != j.end());
    auto findCreateNewThrowTestAContract = std::find(j.begin(), j.end(), EXPECTED::ContractManager::createNewThrowTestAContract);
    REQUIRE(findCreateNewThrowTestAContract != j.end());
    auto findCreateNewThrowTestBContract = std::find(j.begin(), j.end(), EXPECTED::ContractManager::createNewThrowTestBContract);
    REQUIRE(findCreateNewThrowTestBContract != j.end());
    auto findCreateNewThrowTestCContract = std::find(j.begin(), j.end(), EXPECTED::ContractManager::createNewThrowTestCContract);
    REQUIRE(findCreateNewThrowTestCContract != j.end());
    auto findGetDeployedContracts = std::find(j.begin(), j.end(), EXPECTED::ContractManager::getDeployedContracts);
    REQUIRE(findGetDeployedContracts != j.end());
    auto findGetDeployedContractsForCreator = std::find(j.begin(), j.end(), EXPECTED::ContractManager::getDeployedContractsForCreator);
    REQUIRE(findGetDeployedContractsForCreator != j.end());
  }

  SECTION("Check file content SimpleContract") {
    json j;
    std::ifstream i("ABI/SimpleContract.json");
    i >> j;

    REQUIRE(j.size() == 23);
    auto findSetName = std::find(j.begin(), j.end(), EXPECTED::SimpleContract::setName);
    REQUIRE(findSetName != j.end());
    auto findSetNames = std::find(j.begin(), j.end(), EXPECTED::SimpleContract::setNames);
    REQUIRE(findSetNames != j.end());
    auto findSetNumber = std::find(j.begin(), j.end(), EXPECTED::SimpleContract::setNumber);
    REQUIRE(findSetNumber != j.end());
    auto findSetNumbers = std::find(j.begin(), j.end(), EXPECTED::SimpleContract::setNumbers);
    REQUIRE(findSetNumbers != j.end());
    auto findSetNamesAndNumbers = std::find(j.begin(), j.end(), EXPECTED::SimpleContract::setNamesAndNumbers);
    REQUIRE(findSetNamesAndNumbers != j.end());
    auto findSetNamesAndNumbersInTuple = std::find(j.begin(), j.end(), EXPECTED::SimpleContract::setNamesAndNumbersInTuple);
    REQUIRE(findSetNamesAndNumbersInTuple != j.end());
    auto findSetNamesAndNumbersInArrayOfArrays = std::find(j.begin(), j.end(), EXPECTED::SimpleContract::setNamesAndNumbersInArrayOfArrays);
    REQUIRE(findSetNamesAndNumbersInArrayOfArrays != j.end());
    auto findSetTuple = std::find(j.begin(), j.end(), EXPECTED::SimpleContract::setTuple);
    REQUIRE(findSetTuple != j.end());
    auto findGetName = std::find(j.begin(), j.end(), EXPECTED::SimpleContract::getName);
    REQUIRE(findGetName != j.end());
    auto findGetNames = std::find(j.begin(), j.end(), EXPECTED::SimpleContract::getNames);
    REQUIRE(findGetNames != j.end());
    auto findGetNumber = std::find(j.begin(), j.end(), EXPECTED::SimpleContract::getNumber);
    REQUIRE(findGetNumber != j.end());
    auto findGetNumbers = std::find(j.begin(), j.end(), EXPECTED::SimpleContract::getNumbers);
    REQUIRE(findGetNumbers != j.end());
    auto findGetNameAndNumber = std::find(j.begin(), j.end(), EXPECTED::SimpleContract::getNameAndNumber);
    REQUIRE(findGetNameAndNumber != j.end());
    auto findGetNamesAndNumbers = std::find(j.begin(), j.end(), EXPECTED::SimpleContract::getNamesAndNumbers);
    REQUIRE(findGetNamesAndNumbers != j.end());
    auto findGetNamesAndNumbersInTuple = std::find(j.begin(), j.end(), EXPECTED::SimpleContract::getNamesAndNumbersInTuple);
    REQUIRE(findGetNamesAndNumbersInTuple != j.end());
    auto findGetNamesAndNumbersInArrayOfArrays = std::find(j.begin(), j.end(), EXPECTED::SimpleContract::getNamesAndNumbersInArrayOfArrays);
    REQUIRE(findGetNamesAndNumbersInArrayOfArrays != j.end());
    auto findGetTuple = std::find(j.begin(), j.end(), EXPECTED::SimpleContract::getTuple);
    REQUIRE(findGetTuple != j.end());
    auto findGetNumberOverload = std::find(j.begin(), j.end(), EXPECTED::SimpleContract::getNumberOverload);
    REQUIRE(findGetNumberOverload != j.end());
    auto findNameAndNumberTupleChanged = std::find(j.begin(), j.end(), EXPECTED::SimpleContract::nameAndNumberTupleChanged);
    REQUIRE(findNameAndNumberTupleChanged != j.end());
    auto findNameAndNumberChanged = std::find(j.begin(), j.end(), EXPECTED::SimpleContract::nameAndNumberChanged);
    REQUIRE(findNameAndNumberChanged != j.end());
    auto findNumberChanged = std::find(j.begin(), j.end(), EXPECTED::SimpleContract::numberChanged);
    REQUIRE(findNumberChanged != j.end());
    auto findNameChanged = std::find(j.begin(), j.end(), EXPECTED::SimpleContract::nameChanged);
    REQUIRE(findNameChanged != j.end());
    auto findTupleChanged = std::find(j.begin(), j.end(), EXPECTED::SimpleContract::tupleChanged);
    REQUIRE(findTupleChanged != j.end());
  }

  SECTION("getFunctionName") {
    REQUIRE(ContractReflectionInterface::getFunctionName(&NativeWrapper::deposit) == "deposit");
    REQUIRE(ContractReflectionInterface::getFunctionName(&NativeWrapper::withdraw) == "withdraw");
    REQUIRE(ContractReflectionInterface::getFunctionName(&NativeWrapper::transfer) == "transfer");
    REQUIRE(ContractReflectionInterface::getFunctionName(&ERC20::transfer) == "transfer");
  }
}

