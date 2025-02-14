
#ifndef CONTRACT_HOST_H
#define CONTRACT_HOST_H

// utils/{contractreflectioninterface.h, db.h, safehash.h, (strings.h -> hex.h, evmc/evmc.hpp), utils.h},
#include "contractmanager.h"
#include "../core/storage.h"
#include "calltracer.h"
#include "bytes/join.h"
#include "bytes/cast.h"
#include "gas.h"
#include "concepts.h"
#include "executioncontext.h"
#include "messagedispatcher.h"
#include "packedmessages.h"
#include "calltracer.h"
#include "costs.h"

// TODO: EVMC Static Mode Handling
// TODO: Contract creating other contracts (EVM Factories)
// TODO: Proper gas limit tests.

/**
 * The ContractHost class is the class which holds a single line of execution for a contract. (This also includes nested calls)
 * It MUST be unique for each line of execution, as it holds the used stack, accounts, and storage for the contract.
 * The ContractHost class have the following responsibilities:
 * - Be the EVMC Host implementation for EVM contracts
 * - Hold the stack (ContractStack) of the execution.
 * - Allow both C++ and EVM contracts to be called.
 * - Allow interoperability between C++ and EVM contracts.
 * - Process the commit/revert of the stack during **destructor** call.
 */

/**
 * GasLimit Rules
 * Any call: 21000
 * C++ Call: 1000
 * EVM Call: 5000
 * Any EVM Contract: 100000
 * Any CPP Contract: 50000
 */

class ContractHost {
  private:
    Storage& storage_;
    mutable ContractStack stack_;
    bool mustRevert_ = true; // We always assume that we must revert until proven otherwise.
    ExecutionContext& context_;
    CallTracer<MessageDispatcher> messageHandler_;

  public:
    ContractHost(evmc_vm* vm,
                 Storage& storage,
                 const Hash& randomnessSeed,
                 ExecutionContext& context) :
    storage_(storage),
    stack_(),
    context_(context),
    messageHandler_(MessageDispatcher(context_, CppContractExecutor(context_, *this), EvmContractExecutor(context_, vm, storage.getIndexingMode()), PrecompiledContractExecutor(RandomGen(randomnessSeed))), storage.getIndexingMode()) {
      messageHandler_.handler().evmExecutor().setMessageHandler(AnyEncodedMessageHandler::from(messageHandler_)); // TODO: is this really required?
    }

    // Rule of five, no copy/move allowed.
    ContractHost(const ContractHost&) = delete;
    ContractHost(ContractHost&&) = delete;
    ContractHost& operator=(const ContractHost&) = delete;
    ContractHost& operator=(ContractHost&&) = delete;
    ~ContractHost();

    decltype(auto) simulate(concepts::Message auto&& msg) {
      decltype(auto) result = execute(std::forward<decltype(msg)>(msg));
      mustRevert_ = true;
      return result;
    }

    decltype(auto) execute(concepts::Message auto&& msg) {
      try {
        mustRevert_ = false;
        msg.gas().use(CONTRACT_EXECUTION_COST);
        return dispatchMessage(std::forward<decltype(msg)>(msg));
      } catch (const std::exception& err) {
        mustRevert_ = true;
        throw;
      }
    }

    /**
     * Call a contract view function based on the basic requirements of a contract call.
     * @tparam R The return type of the view function.
     * @tparam C The contract type.
     * @tparam Args The argument types of the view function.
     * @param caller Pointer to the contract that made the call.
     * @param targetAddr The address of the contract to call.
     * @param func The view function to call.
     * @param args The arguments to pass to the view function.
     * @return The result of the view function.
     */
    template <typename R, typename C, typename... Args>
    R callContractViewFunction(const BaseContract* caller, const Address& targetAddr, R(C::*func)(const Args&...) const, const Args&... args) {
      PackedStaticCallMessage<decltype(func), const Args&...> msg(
        caller->getContractAddress(),
        targetAddr,
        this->getCurrentGas(),
        *caller,
        func,
        args...);

      return this->dispatchMessage(std::move(msg));
    }

    /**
     * Call a contract non-view function based on the basic requirements of a contract call.
     * @tparam R The return type of the view function.
     * @tparam C The contract type.
     * @tparam Args The argument types of the view function.
     * @param caller Pointer to the contract that made the call.
     * @param targetAddr The address of the contract to call.
     * @param value The value to use in the call.
     * @param func The view function to call.
     * @param args The arguments to pass to the view function.
     * @return The result of the non-view function.
     */
    template <typename R, typename C, typename... Args>
    R callContractFunction(
      BaseContract* caller, const Address& targetAddr,
      const uint256_t& value,
      R(C::*func)(const Args&...), const Args&... args) {

      PackedCallMessage<decltype(func), const Args&...> msg(
        caller->getContractAddress(),
        targetAddr,
        this->getCurrentGas(),
        value,
        *caller,
        func,
        args...);

      return this->dispatchMessage(std::move(msg));
    }

    /**
     * Call the createNewContract function of a contract.
     * Used by DynamicContract to create new contracts.
     * @tparam TContract The contract type.
     * @param caller Pointer to the contract that made the call.
     * @param fullData The caller data.
     * @return The address of the new contract.
     */
    template<typename ContractType, typename... Args>
    Address callCreateContract(BaseContract& caller, Args&&... args) {
      const uint256_t value = 0;
      PackedCreateMessage<ContractType, Args...> msg(
        caller.getContractAddress(),
        this->getCurrentGas(),
        value,
        caller,
        std::forward<decltype(args)>(args)...
      );

      return this->dispatchMessage(std::move(msg));
    }

    /**
     * Get a contract by its address.
     * Used by DynamicContract to access view/const functions of other contracts.
     * @tparam T The contract type.
     * @param address The address of the contract.
     * @return A pointer to the contract.
     * @throw DynamicException if contract is not found or not of the requested type.
     */
    template <typename T>
    const T* getContract(View<Address> address) const {
      const T* pointer = dynamic_cast<const T*>(&context_.getContract(address));

      if (pointer == nullptr) {
        throw DynamicException("Wrong contract type");
      }

      return pointer;
    }

    /**
     * Get a contract by its address (non-const).
     * Used by DynamicContract to access view/const functions of other contracts.
     * @tparam T The contract type.
     * @param address The address of the contract.
     * @return A pointer to the contract.
     * @throw DynamicException if contract is not found or not of the requested type.
     */
    template <typename T> T* getContract(const Address& address) {
      T* pointer = dynamic_cast<T*>(&context_.getContract(address));
      if (pointer == nullptr) throw DynamicException("Wrong contract type");
      return pointer;
    }

    /**
     * Emit a contract event.
     * @tparam Args The event's arguments.
     * @tparam Flags The event's indexed flags.
     * @param name The event's name.
     * @param contract The address of the contract that emitted the event.
     * @param args A list of event arguments.
     * @param anonymous Whether the event is anonymous or not.
     */
    template <typename... Args, bool... Flags> void emitEvent(
      const std::string& name,
      const Address& contract,
      const std::tuple<EventParam<Args, Flags>...>& args,
      bool anonymous
    ) {
      if (storage_.getIndexingMode() == IndexingMode::RPC_TRACE) {
        context_.addEvent(name, contract, args, anonymous);
      }
    }

    ExecutionContext& context() { return context_; }

    const ExecutionContext& context() const { return context_; }

    void registerVariableUse(SafeBase& var) { stack_.registerVariableUse(var); }

    uint256_t getRandomValue();

private:
  decltype(auto) dispatchMessage(auto&& msg) {
    return messageHandler_.onMessage(std::forward<decltype(msg)>(msg));
  }

  Gas& getCurrentGas() {
    return messageHandler_.handler().cppExecutor().currentGas();
  }
};

#endif // CONTRACT_HOST_H
