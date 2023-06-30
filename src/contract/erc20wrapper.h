#ifndef ERC20WRAPPER_H
#define ERC20WRAPPER_H

#include <memory>
#include <tuple>

#include "../utils/db.h"
#include "abi.h"
#include "contractmanager.h"
#include "dynamiccontract.h"
#include "erc20.h"
#include "variables/safeuint256_t.h"
#include "variables/safeunorderedmap.h"

/// Template for an ERC20Wrapper contract.
class ERC20Wrapper : public DynamicContract {
  private:
    /**
     * Map for tokens and balances. Solidity counterpart:
     * mapping(address => mapping(address => uint256)) internal _tokensAndBalances;
     */
    SafeUnorderedMap<Address, std::unordered_map<Address, uint256_t, SafeHash>> _tokensAndBalances;

    /// Function for calling the register functions for contracts.
    void registerContractFunctions() override;

  public:

    /**
    * ConstructorArguments is a tuple of the contract constructor arguments in the order they appear in the constructor.
    */
    using ConstructorArguments = std::tuple<>;
    
    /**
     * Constructor for loading contract from DB.
     * @param interface Reference to the contract manager interface.
     * @param contractAddress The address where the contract will be deployed.
     * @param db Reference pointer to the database object.
     */
    ERC20Wrapper(
      ContractManagerInterface& interface,
      const Address& contractAddress, const std::unique_ptr<DB>& db
    );

    /**
     * Constructor for building a new contract from scratch.
     * @param interface Reference to the contract manager interface.
     * @param address The address where the contract will be deployed.
     * @param creator The address of the creator of the contract.
     * @param chainId The chain id of the contract.
     * @param db Reference pointer to the database object.
     */
    ERC20Wrapper(
      ContractManagerInterface& interface,
      const Address& address, const Address& creator,
      const uint64_t& chainId, const std::unique_ptr<DB>& db
    );

    /// Register contract class via ContractReflectionInterface.
    static void registerContract() {
      ContractReflectionInterface::registerContract<
        ERC20Wrapper, ContractManagerInterface &,
        const Address &, const Address &, const uint64_t &,
        const std::unique_ptr<DB> &
      >(
        std::vector<std::string>{},
        std::make_tuple("getContractBalance", &ERC20Wrapper::getContractBalance, "view", std::vector<std::string>{"tokenAddress"}),
        std::make_tuple("getUserBalance", &ERC20Wrapper::getUserBalance, "view", std::vector<std::string>{"tokenAddress", "userAddress"}),
        std::make_tuple("withdraw", &ERC20Wrapper::withdraw, "nonpayable", std::vector<std::string>{"tokenAddress", "value"}),
        std::make_tuple("transferTo", &ERC20Wrapper::transferTo, "nonpayable", std::vector<std::string>{"tokenAddress", "toAddress", "value"}),
        std::make_tuple("deposit", &ERC20Wrapper::deposit, "nonpayable", std::vector<std::string>{"tokenAddress", "value"})
      );
    }

    /// Destructor.
    ~ERC20Wrapper() override;

    /**
     * Get the balance of the contract for a specific token. Solidity counterpart:
     * function getContractBalance(address _token) public view returns (uint256) { return _tokensAndBalances[_token][address(this)]; }
     * @param token The address of the token.
     * @return The contract's given token balance.
     */
    uint256_t getContractBalance(const Address& token) const;

    /**
     * Get the balance of a specific user for a specific token. Solidity counterpart:
     * function getUserBalance(address _token, address _user) public view returns (uint256) { return _tokensAndBalances[_token][_user]; }
     * @param token The address of the token.
     * @param user The address of the user.
     * @return The user's given token balance.
     */
    uint256_t getUserBalance(const Address& token, const Address& user) const;

    /**
     * Withdraw a specific amount of tokens from the contract. Solidity counterpart:
     * function withdraw (address _token, uint256 _value) public returns (bool)
     * @param token The address of the token.
     * @param value The amount of tokens to withdraw.
     * @throw std::runtime_error if the contract does not have enough tokens,
     * or if the token was not found.
     */
    void withdraw(const Address& token, const uint256_t& value);

    /**
     * Transfer a specific amount of tokens from the contract to a user. Solidity counterpart:
     * function transferTo(address _token, address _to, uint256 _value) public returns (bool)
     * @param token The address of the token.
     * @param to The address of the user to send tokens to.
     * @param value The amount of tokens to transfer.
     * @throw std::runtime_error if the contract does not have enough tokens,
     * or if either the token or the user were not found.
     */
    void transferTo(const Address& token, const Address& to, const uint256_t& value);

    /**
     * Deposit a specific amount of tokens to the contract. Solidity counterpart:
     * function deposit(address _token, uint256 _value) public returns (bool)
     * @param token The address of the token.
     * @param value The amount of tokens to deposit.
     */
    void deposit(const Address& token, const uint256_t& value);
};

#endif // ERC20WRAPPER_H
