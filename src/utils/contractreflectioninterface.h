/*
Copyright (c) [2023] [Sparq Network]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#ifndef CONTRACTREFLECTIONINTERFACE_H
#define CONTRACTREFLECTIONINTERFACE_H

#include "contract/abi.h"

/**
 * This namespace contains the reflection interface for the contract
 * classes.
 * Observations about ContractReflectionsInterface
 * Only the following functions are used in normal operation
 * registerContract() -> By the derived DynamicContract class, to register the contract class methods, arguments, etc
 * getConstructorArgumentTypesString<TContract>() -> By ContractFactory and ContractManager, to get the list of constructor argument types .e.g "uint256,uint256"
 * getMethodMutability<TContract>(methodName) -> By the DynamicContract* class, after derived class calling registerMemberFunction(), to get the mutability of the method
 * isContractRegistered<TContract>() -> By ContractFactory and ContractManager, to check if the contract is registered
 * These functions only access the following mappings
 * registeredContractsMap -> To check if the contract is registered (by isContractRegistered<TContract>())
 * methodMutabilityMap -> To get the mutability of a method (by getMethodMutability<TContract>(methodName))
 * getConstructorArgumentTypesString<TContract>() derives from TContract::ConstructorArguments to get the constructor argument types, no access to mappings
 * The remaining functions and mapping are accessed for JSON ABI purposes only
 * TODO: Add support for overloaded methods! This will require a change in the mappings and templates...
 */

namespace ContractReflectionInterface {

/// Key (ClassName) -> Value (boolean) (true if registered, false otherwise)
extern std::unordered_map<std::string, bool> registeredContractsMap; ///< Map of registered contracts.
/// Key (ClassName) -> Value (std::vector<std::string>) (ConstructorArgumentNames)
extern std::unordered_map<std::string, std::vector<std::string>> constructorArgumentNamesMap; /// Map to store constructor argument names
/// Key (ClassName) -> Key (MethodName) -> Value (Mutability) ("view", "nonpayable", "payable")
extern std::unordered_map<std::string, std::unordered_map<std::string, std::string>> methodMutabilityMap; //// Map to store method mutability
/// Key (ClassName) -> Key (MethodName) -> Value (std::vector<std::string>) (ArgumentNames)
extern std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> argumentNamesMap; /// Map to store method argument names
/// Key (ClassName) -> Key (MethodName) -> Value (std::string) (ArgumentTypes, Single string, separated by comma ",", no spaces, () for tuples, [] for arrays).
extern std::unordered_map<std::string, std::unordered_map<std::string, std::string>> methodArgumentsTypesMap; ///< Map to store method argument types
/// Key (ClassName) -> Key (MethodName) -> Value (std::string) (ReturnTypes, same as ArgumentTypes)
extern std::unordered_map<std::string, std::unordered_map<std::string, std::string>> methodReturnTypesMap; ///< Map of method return types.

/** Helper struct to extract the Args... from a function pointer */

template <typename T>
struct populateMethodTypesMapHelper;

// For member functions with no arguments
template <typename TContract, typename R>
struct populateMethodTypesMapHelper<R(TContract::*)()> {
  using ReturnType = R;
  using ClassType = TContract;
  static std::string getFunctionArgs(const std::initializer_list<std::string>& args) {
    static_assert(args.size() == 0, "CRI: Wrong number of arguments");
    return "";
  }
  static std::string getFunctionRets() {
    return ABI::FunctorEncoder::listArgumentTypes<R>();
  }
};

// For const member functions with no arguments
template <typename TContract, typename R>
struct populateMethodTypesMapHelper<R(TContract::*)() const> {
  using ReturnType = R;
  using ClassType = TContract;
  static std::string getFunctionArgs(const std::initializer_list<std::string>& args) {
    static_assert(args.size() == 0, "CRI: Wrong number of arguments");
    return "";
  }
  static std::string getFunctionRets() {
    return ABI::FunctorEncoder::listArgumentTypes<R>();
  }
};

// For member functions with non-const, non-reference arguments
template <typename TContract, typename R, typename... Args>
struct populateMethodTypesMapHelper<R(TContract::*)(Args...)> {
  using ReturnType = R;
  using ClassType = TContract;
  static std::string getFunctionArgs(const std::initializer_list<std::string>& args) {
    static_assert(sizeof...(Args) == args.size(), "CRI: Wrong number of arguments");
    return ABI::FunctorEncoder::listArgumentTypes<Args...>();
  }
  static std::string getFunctionRets() {
    return ABI::FunctorEncoder::listArgumentTypes<R>();
  }
};

// For const member functions with non-const, non-reference arguments
template <typename TContract, typename R, typename... Args>
struct populateMethodTypesMapHelper<R(TContract::*)(Args...) const> {
  using ReturnType = R;
  using ClassType = TContract;
  static std::string getFunctionArgs(const std::initializer_list<std::string>& args) {
    static_assert(sizeof...(Args) == args.size(), "CRI: Wrong number of arguments");
    return ABI::FunctorEncoder::listArgumentTypes<Args...>();
  }
  static std::string getFunctionRets() {
    return ABI::FunctorEncoder::listArgumentTypes<R>();
  }
};

// For member functions with const reference arguments
template <typename TContract, typename R, typename... Args>
struct populateMethodTypesMapHelper<R(TContract::*)(const Args&...)> {
  using ReturnType = R;
  using ClassType = TContract;
  static std::string getFunctionArgs(const std::initializer_list<std::string>& args) {
    static_assert(sizeof...(Args) == args.size(), "CRI: Wrong number of arguments");
    return ABI::FunctorEncoder::listArgumentTypes<Args...>();
  }
  static std::string getFunctionRets() {
    return ABI::FunctorEncoder::listArgumentTypes<R>();
  }
};

// For const member functions with const reference arguments
template <typename TContract, typename R, typename... Args>
struct populateMethodTypesMapHelper<R(TContract::*)(const Args&...) const> {
  using ReturnType = R;
  using ClassType = TContract;
  static std::string getFunctionArgs(const std::initializer_list<std::string>& args) {
    static_assert(sizeof...(Args) == args.size(), "CRI: Wrong number of arguments");
    return ABI::FunctorEncoder::listArgumentTypes<Args...>();
  }
  static std::string getFunctionRets() {
    return ABI::FunctorEncoder::listArgumentTypes<R>();
  }
};

/**
 * This function populates the constructor argument names map.
 * @tparam TContract The contract type.
 */
template <typename TContract>
void inline populateMethodTypesMap(const std::string& functionName, const std::string& functionArgs, const std::string& funcRets) {
  std::string contractName = Utils::getRealTypeName<TContract>();
  methodArgumentsTypesMap[contractName][functionName] = functionArgs;
  methodReturnTypesMap[contractName][functionName] = funcRets;
}

/**
 * Template function to get the constructor data structure of a contract.
 * @tparam TContract The contract to get the constructor data structure of.
 * @return The constructor data structure in ABI format.
 */
template <typename TContract> bool isContractRegistered() {
  return registeredContractsMap.contains(Utils::getRealTypeName<TContract>());
}

/**
 * Template function to register a contract class.
 * @tparam TContract The contract class to register.
 * @tparam Args The constructor argument types.
 * @tparam Methods The methods to register.
 * @param ctorArgs The constructor argument names.
 * @param methods The methods to register.
 * methods = std::tuple<std::string, FunctionPointer, std::string, std::vector<std::string>>
 * std::get<0>(methods) = Method name
 * std::get<1>(methods) = Method pointer
 * std::get<2>(methods) = Method mutability
 * std::get<3>(methods) = Method argument names
 */
template <typename TContract, typename... Args, typename... Methods>
void inline registerContract(const std::vector<std::string> &ctorArgs,
                             Methods &&...methods) {
  if (isContractRegistered<TContract>()) {
    /// Already registered, do nothing
    return;
  }
  std::string contractName = Utils::getRealTypeName<TContract>();
  // Store constructor argument names in the constructorArgumenqtNamesMap
  constructorArgumentNamesMap[contractName] = ctorArgs;
  // Register methods and store the stateMutability string and argument names
  ((methodMutabilityMap[contractName][std::get<0>(std::forward<Methods>(methods))] =
        std::get<2>(std::forward<Methods>(methods)),
    argumentNamesMap[contractName][std::get<0>(std::forward<Methods>(methods))] =
        std::get<3>(std::forward<Methods>(methods))),
   ...);

  ((populateMethodTypesMap<TContract>(std::get<0>(methods),
    populateMethodTypesMapHelper<std::decay_t<decltype(std::get<1>(methods))>>::getFunctionArgs(std::get<3>(std::forward<Methods>(methods))),
    populateMethodTypesMapHelper<std::decay_t<decltype(std::get<1>(methods))>>::getFunctionRets())), ...);

  registeredContractsMap[Utils::getRealTypeName<TContract>()] = true;
}

/**
* Template function to get the list of constructor argument types of a
* contract.
* @tparam Contract The contract to get the constructor argument types of.
* @return The list of constructor argument in string format. (same as ABI::Functor::listArgumentTypes)
*/
template <typename TContract>
std::string inline getConstructorArgumentTypesString() {
  using ConstructorArgs = typename TContract::ConstructorArguments;
  std::string ret = ABI::FunctorEncoder::listArgumentTypes<ConstructorArgs>();
  // TContract::ConstructorArguments is a tuple, so we need to remove the "(...)" from the string
  ret.erase(0,1);
  ret.pop_back();
  return ret;
}

/**
 * Template function to get the constructor ABI data structure of a contract.
 * @tparam Contract The contract to get the constructor argument names of.
 * @return The constructor ABI data structure.
 */
template <typename Contract>
ABI::MethodDescription inline getConstructorDataStructure() {
  if (!isContractRegistered<Contract>()) {
    throw std::runtime_error("Contract " + Utils::getRealTypeName<Contract>() + " not registered");
  }
  /// Derive from Contract::ConstructorArguments to get the constructor
  ABI::MethodDescription constructorDescription;
  return constructorDescription;
}

/**
 * Template function to get the function ABI data structure of a contract.
 * @tparam Contract The contract to get the function data structure of.
 * @return The function ABI data structure.
 */
template <typename Contract>
std::vector<ABI::MethodDescription> inline getFunctionDataStructure() {
  std::vector<ABI::MethodDescription> descriptions;
  return descriptions;
}

/**
 * Getter for the mutability of a method.
  * @param methodName The name of the method to get the mutability of.
  * @return The mutability of the method.
  */
template <typename Contract>
std::string inline getMethodMutability(const std::string& methodName) {
  if (!isContractRegistered<Contract>()) {
    throw std::runtime_error("Contract " + Utils::getRealTypeName<Contract>() + " not registered");
  }
  std::string contractName = Utils::getRealTypeName<Contract>();
  auto cIt = methodMutabilityMap.find(contractName);
  if (cIt != methodMutabilityMap.end()) {
    const auto& methodMaps = cIt->second;
    auto mIt = methodMaps.find(methodName);
    if (mIt != methodMaps.end()) {
        return mIt->second;
    }
  }
  throw std::runtime_error("Method " + contractName + "::" + methodName + " not found");
}

} // namespace ContractReflectionInterface

#endif // CONTRACTREFLECTIONINTERFACE_H