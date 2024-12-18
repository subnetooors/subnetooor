#include "evmcontractexecutor.h"
#include "bytes/cast.h"
#include "common.h"
#include "outofgas.h"

constexpr decltype(auto) getAndThen(auto&& map, const auto& key, auto&& andThen, auto&& orElse) {
  const auto it = map.find(key);

  if (it == map.end()) {
    return std::invoke(orElse);
  } else {
    return std::invoke(andThen, it->second);
  }
}

constexpr auto getAndThen(auto&& map, const auto& key, auto&& andThen) {
  using Result = std::invoke_result_t<decltype(andThen), decltype(map.find(key)->second)>;
  return getAndThen(map, key, andThen, [] () { return Result{}; });
}


template<concepts::Message M>
constexpr evmc_call_kind getEvmcKind(const M& msg) {
  if constexpr (concepts::SaltMessage<M>) {
    return EVMC_CREATE2;
  } else if (concepts::CreateMessage<M>) {
    return EVMC_CREATE;
  } else if (concepts::DelegateCallMessage<M>) {
    return EVMC_DELEGATECALL;
  } else {
    return EVMC_CALL;
  }

  // TODO: CALL CODE!
}

template<concepts::Message M>
constexpr evmc_flags getEvmcFlags(const M& msg) {
  if constexpr (concepts::StaticCallMessage<M>) {
    return EVMC_STATIC;
  } else {
    return evmc_flags{};
  }
}

template<concepts::CallMessage M>
constexpr evmc_message makeEvmcMessage(const M& msg, uint64_t depth) {
  return evmc_message{
    .kind = getEvmcKind(msg),
    .flags = getEvmcFlags(msg),
    .depth = depth,
    .gas = msg.gas().value(),
    .recipient = bytes::cast<evmc_address>(messageRecipientOrDefault(msg)),
    .sender = bytes::cast<evmc_address>(msg.from()),
    .input_data = msg.input().data(),
    .input_size = msg.input().size(),
    .value = Utils::uint256ToEvmcUint256(messageValueOrZero(msg)),
    .create2_salt = evmc_bytes32{},
    .code_address = evmc_address{} // TODO: CALL CODE?
  };
}

template<concepts::CreateMessage M>
constexpr evmc_message makeEvmcMessage(const M& msg, uint64_t depth, View<Address> contractAddress) {
  return evmc_message{
    .kind = getEvmcKind(msg),
    .flags = 0,
    .depth = depth,
    .gas = msg.gas().value(),
    .recipient = bytes::cast<evmc_address>(contractAddress),
    .sender = bytes::cast<evmc_address>(msg.from()),
    .input_data = nullptr,
    .input_size = 0,
    .value = Utils::uint256ToEvmcUint256(messageValueOrZero(msg)),
    .create2_salt = bytes::cast<evmc_bytes32>(messageSaltOrDefault(msg)),
    .code_address = evmc_address{} // TODO: CALL CODE?
  };
}

static Bytes executeEvmcMessage(evmc_vm* vm, const evmc_host_interface* host, evmc_host_context* context, const evmc_message& msg, messages::Gas& gas, View<Bytes> code) {
  evmc::Result result(::evmc_execute(
    vm,
    host,
    context,
    evmc_revision::EVMC_LATEST_STABLE_REVISION,
    &msg,
    code.data(),
    code.size()));

  gas = messages::Gas(result.gas_left);

  if (result.status_code == EVMC_SUCCESS) {
    return Bytes(result.output_data, result.output_data + result.output_size);
  } else if (result.status_code == EVMC_OUT_OF_GAS) {
    throw OutOfGas();
  } else {
    std::string reason = "";

    if (result.output_size > 0) {
      reason = ABI::Decoder::decodeError(View<Bytes>(result.output_data, result.output_size));
    }

    throw DynamicException(reason);
  }
}

static void createContractImpl(auto& msg, ExecutionContext& context, View<Address> contractAddress, evmc_vm *vm, evmc::Host& host, uint64_t depth) {
  Bytes code = executeEvmcMessage(vm, &evmc::Host::get_interface(), host.to_context(),
    makeEvmcMessage(msg, depth, contractAddress), msg.gas(), msg.code());

  Account newAccount;
  newAccount.nonce = 1; // TODO: starting as 1?
  newAccount.codeHash = Utils::sha3(code);
  newAccount.code = std::move(code);
  newAccount.contractType = ContractType::EVM;

  context.addAccount(contractAddress, std::move(newAccount));
  context.notifyNewContract(contractAddress, nullptr);
  context.incrementNonce(msg.from());
}

Bytes EvmContractExecutor::execute(EncodedCallMessage& msg) {
  auto checkpoint = context_.checkpoint();
  auto depthGuard = transactional::copy(depth_); // TODO: checkpoint (and deprecate copy)
  ++depth_;

  if (msg.value() > 0) {
    context_.transferBalance(msg.from(), msg.to(), msg.value());
  }

  const Bytes output = executeEvmcMessage(this->vm_, &this->get_interface(), this->to_context(),
    makeEvmcMessage(msg, depth_), msg.gas(), context_.getAccount(msg.to()).code);

  checkpoint.commit();

  return output;
}

Bytes EvmContractExecutor::execute(EncodedStaticCallMessage& msg) {
  View<Bytes> code = context_.getAccount(msg.to()).code;

  auto depthGuard = transactional::copy(depth_);
  ++depth_;

  return executeEvmcMessage(this->vm_, &this->get_interface(), this->to_context(),
    makeEvmcMessage(msg, depth_), msg.gas(), code);
}

Bytes EvmContractExecutor::execute(EncodedDelegateCallMessage& msg) {
  auto checkpoint = context_.checkpoint();
  auto depthGuard = transactional::copy(depth_);
  ++depth_;

  // TODO: is the value transfer correct for delegate calls?
  if (msg.value() > 0) {
    context_.transferBalance(msg.from(), msg.to(), msg.value());
  }

  const Bytes output = executeEvmcMessage(this->vm_, &this->get_interface(), this->to_context(),
    makeEvmcMessage(msg, depth_), msg.gas(), context_.getAccount(msg.codeAddress()).code);

  checkpoint.commit();

  return output;
}

Address EvmContractExecutor::execute(EncodedCreateMessage& msg) {
  auto checkpoint = context_.checkpoint();
  auto depthGuard = transactional::copy(depth_);
  const Address contractAddress = generateContractAddress(context_.getAccount(msg.from()).nonce, msg.from());
  createContractImpl(msg, context_, contractAddress, vm_, *this, ++depth_);
  checkpoint.commit();
  return contractAddress;
}

Address EvmContractExecutor::execute(EncodedSaltCreateMessage& msg) {
  auto checkpoint = context_.checkpoint();
  auto depthGuard = transactional::copy(depth_);
  const Address contractAddress = generateContractAddress(msg.from(), msg.salt(), msg.code());
  createContractImpl(msg, context_, contractAddress, vm_, *this, ++depth_);
  checkpoint.commit();
  return contractAddress;
}

bool EvmContractExecutor::account_exists(const evmc::address& addr) const noexcept {
  return context_.accountExists(addr);
}

evmc::bytes32 EvmContractExecutor::get_storage(const evmc::address& addr, const evmc::bytes32& key) const noexcept {
  return bytes::cast<evmc::bytes32>(context_.retrieve(addr, key));
}

evmc_storage_status EvmContractExecutor::set_storage(const evmc::address& addr, const evmc::bytes32& key, const evmc::bytes32& value) noexcept {
  context_.store(addr, key, value);
  return EVMC_STORAGE_MODIFIED;
}

evmc::uint256be EvmContractExecutor::get_balance(const evmc::address& addr) const noexcept {
  try {
    return Utils::uint256ToEvmcUint256(context_.getAccount(addr).balance);
  } catch (const std::exception&) {
    return evmc::uint256be{};
  }
}

size_t EvmContractExecutor::get_code_size(const evmc::address& addr) const noexcept {
  try {
    return context_.getAccount(addr).code.size();
  } catch (const std::exception&) {
    return 0;
  }
}

evmc::bytes32 EvmContractExecutor::get_code_hash(const evmc::address& addr) const noexcept {
  try {
    return bytes::cast<evmc::bytes32>(context_.getAccount(addr).codeHash);
  } catch (const std::exception&) {
    return evmc::bytes32{};
  }
}

size_t EvmContractExecutor::copy_code(const evmc::address& addr, size_t code_offset, uint8_t* buffer_data, size_t buffer_size) const noexcept {

  try {
    View<Bytes> code = context_.getAccount(addr).code;

    if (code_offset < code.size()) {
      const auto n = std::min(buffer_size, code.size() - code_offset);
      if (n > 0)
        std::copy_n(&code[code_offset], n, buffer_data);
      return n;
    }

  } catch (const std::exception&) {}

  return 0;
}

bool EvmContractExecutor::selfdestruct(const evmc::address& addr, const evmc::address& beneficiary) noexcept {
  // SELFDESTRUCT is not allowed in the current implementation
  return false;
}

evmc_tx_context EvmContractExecutor::get_tx_context() const noexcept {
  return evmc_tx_context{
    .tx_gas_price = Utils::uint256ToEvmcUint256(context_.getTxGasPrice()),
    .tx_origin = bytes::cast<evmc_address>(context_.getTxOrigin()),
    .block_coinbase = bytes::cast<evmc_address>(context_.getBlockCoinbase()),
    .block_number = context_.getBlockNumber(),
    .block_timestamp = context_.getBlockTimestamp(),
    .block_gas_limit = context_.getBlockGasLimit(),
    .block_prev_randao = {},
    .chain_id = Utils::uint256ToEvmcUint256(context_.getChainId()),
    .block_base_fee = {},
    .blob_base_fee = {},
    .blob_hashes = nullptr,
    .blob_hashes_count = 0
  };
}

evmc::bytes32 EvmContractExecutor::get_block_hash(int64_t number) const noexcept {
  return Utils::uint256ToEvmcUint256(number); // TODO: ???
}

void EvmContractExecutor::emit_log(const evmc::address& addr, const uint8_t* data, size_t dataSize, const evmc::bytes32 topics[], size_t topicsCount) noexcept {
  try {
    // We need the following arguments to build a event:
    // (std::string) name The event's name.
    // (uint64_t) logIndex The event's position on the block.
    // (Hash) txHash The hash of the transaction that emitted the event.
    // (uint64_t) txIndex The position of the transaction in the block.
    // (Hash) blockHash The hash of the block that emitted the event.
    // (uint64_t) blockIndex The height of the block.
    // (Address) address The address that emitted the event.
    // (Bytes) data The event's arguments.
    // (std::vector<Hash>) topics The event's indexed arguments.
    // (bool) anonymous Whether the event is anonymous or not.
    std::vector<Hash> topicsVec;
    topicsVec.reserve(topicsCount);
    for (uint64_t i = 0; i < topicsCount; i++) {
      topicsVec.emplace_back(topics[i]);
    }

    context_.addEvent(addr, View<Bytes>(data, dataSize), std::move(topicsVec));

  } catch (const std::exception& ignored) {
    // TODO: log errors
  }
}

evmc_access_status EvmContractExecutor::access_account(const evmc::address& addr) noexcept {
  return EVMC_ACCESS_WARM;
}

evmc_access_status EvmContractExecutor::access_storage(const evmc::address& addr, const evmc::bytes32& key) noexcept {
  return EVMC_ACCESS_WARM;
}

evmc::bytes32 EvmContractExecutor::get_transient_storage(const evmc::address &addr, const evmc::bytes32 &key) const noexcept {
  return getAndThen(transientStorage_, StorageKeyView(addr, key), [] (const auto& result) { return bytes::cast<evmc_bytes32>(result); });
}

void EvmContractExecutor::set_transient_storage(const evmc::address &addr, const evmc::bytes32 &key, const evmc::bytes32 &value) noexcept {
  // TODO: This also must be controlled by transactions
  transientStorage_.emplace(StorageKeyView{addr, key}, value);
}

evmc::Result EvmContractExecutor::call(const evmc_message& msg) noexcept {
  messages::Gas gas(msg.gas);
  const uint256_t value = Utils::evmcUint256ToUint256(msg.value);

  const auto process = [&] (auto& msg) {
    try {
      const auto output = messageHandler_.onMessage(msg);

      if constexpr (concepts::CreateMessage<decltype(msg)>) {
        return evmc::Result(EVMC_SUCCESS, gas.value(), 0, bytes::cast<evmc_address>(output));
      } else {
        return evmc::Result(EVMC_SUCCESS, gas.value(), 0, output.data(), output.size());
      }
    } catch (const OutOfGas&) { // TODO: ExecutionReverted exception is important
      return evmc::Result(EVMC_OUT_OF_GAS);
    } catch (const std::exception& err) {
      Bytes output;

      if (err.what() != nullptr) {
        output = ABI::Encoder::encodeError(err.what()); // TODO: this may throw...
      }

      return evmc::Result(EVMC_REVERT, gas.value(), 0, output.data(), output.size());
    }
  };

  if (msg.kind == EVMC_DELEGATECALL) {
    std::cout << "from: " << Hex::fromBytes(msg.sender, true) << "\n";
    std::cout << "to: " << Hex::fromBytes(msg.recipient, true) << "\n";
    std::cout << "code address: " << Hex::fromBytes(msg.code_address, true) << "\n";

    EncodedDelegateCallMessage encodedMessage(msg.sender, msg.recipient, gas, value, View<Bytes>(msg.input_data, msg.input_size), msg.code_address);
    return process(encodedMessage);
  } else if (msg.kind == EVMC_CALL && msg.flags == EVMC_STATIC) {
    EncodedStaticCallMessage encodedMessage(msg.sender, msg.recipient, gas, View<Bytes>(msg.input_data, msg.input_size));
    return process(encodedMessage);
  } else if (msg.kind == EVMC_CALL) {
    EncodedCallMessage encodedMessage(msg.sender, msg.recipient, gas, value, View<Bytes>(msg.input_data, msg.input_size));
    return process(encodedMessage);
  } else if (msg.kind == EVMC_CREATE) {
    EncodedCreateMessage encodedMessage(msg.sender, gas, value, View<Bytes>(msg.input_data, msg.input_size));
    return process(encodedMessage);
  } else if (msg.kind == EVMC_CREATE2) {
    EncodedSaltCreateMessage encodedMessage(msg.sender, gas, value, View<Bytes>(msg.input_data, msg.input_size), msg.create2_salt);
    return process(encodedMessage);
  } else if (msg.kind == EVMC_CALLCODE) {
    return evmc::Result{}; // TODO: CALL CODE!!!
  }

  // TODO: proper error result with proper reason (encoded)
  return evmc::Result{};
}
