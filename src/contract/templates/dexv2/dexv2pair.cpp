/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#include "dexv2pair.h"
#include "dexv2factory.h"

#include "../../../utils/uintconv.h"
#include "../../../utils/strconv.h"

DEXV2Pair::DEXV2Pair(const Address& address, const DB& db
) : ERC20(address, db), factory_(this), token0_(this), token1_(this),
  reserve0_(this), reserve1_(this), blockTimestampLast_(this),
  price0CumulativeLast_(this), price1CumulativeLast_(this), kLast_(this)
{
  Hex prefix = Hex::fromBytes(this->getDBPrefix());
  Utils::safePrintTest("Loading contract DEXV2Pair from DB with prefix: " + prefix.get());
  auto factoryBytes = db.get(std::string("factory_"), this->getDBPrefix());
  auto token0Bytes = db.get(std::string("token0_"), this->getDBPrefix());
  auto token1Bytes = db.get(std::string("token1_"), this->getDBPrefix());
  auto reserve0Bytes = db.get(std::string("reserve0_"), this->getDBPrefix());
  auto reserve1Bytes = db.get(std::string("reserve1_"), this->getDBPrefix());
  auto blockTimestampLastBytes = db.get(std::string("blockTimestampLast_"), this->getDBPrefix());
  auto price0CumulativeLastBytes = db.get(std::string("price0CumulativeLast_"), this->getDBPrefix());
  auto price1CumulativeLastBytes = db.get(std::string("price1CumulativeLast_"), this->getDBPrefix());
  auto kLastBytes = db.get(std::string("kLast_"), this->getDBPrefix());

  Utils::safePrintTest("Factory Bytes: " + Hex::fromBytes(factoryBytes).get());
  Utils::safePrintTest("Token0 Bytes: " + Hex::fromBytes(token0Bytes).get());
  Utils::safePrintTest("Token1 Bytes: " + Hex::fromBytes(token1Bytes).get());
  Utils::safePrintTest("Reserve0 Bytes: " + Hex::fromBytes(reserve0Bytes).get());
  Utils::safePrintTest("Reserve1 Bytes: " + Hex::fromBytes(reserve1Bytes).get());
  Utils::safePrintTest("BlockTimestampLast Bytes: " + Hex::fromBytes(blockTimestampLastBytes).get());
  Utils::safePrintTest("Price0CumulativeLast Bytes: " + Hex::fromBytes(price0CumulativeLastBytes).get());
  Utils::safePrintTest("Price1CumulativeLast Bytes: " + Hex::fromBytes(price1CumulativeLastBytes).get());
  Utils::safePrintTest("KLast Bytes: " + Hex::fromBytes(kLastBytes).get());


  this->factory_ = Address(factoryBytes);
  this->token0_ = Address(token0Bytes);
  this->token1_ = Address(token1Bytes);
  this->reserve0_ = UintConv::bytesToUint112(reserve0Bytes);
  this->reserve1_ = UintConv::bytesToUint112(reserve1Bytes);
  this->blockTimestampLast_ = UintConv::bytesToUint32(blockTimestampLastBytes);
  this->price0CumulativeLast_ = UintConv::bytesToUint256(price0CumulativeLastBytes);
  this->price1CumulativeLast_ = UintConv::bytesToUint256(price1CumulativeLastBytes);
  this->kLast_ = UintConv::bytesToUint256(kLastBytes);

  this->factory_.commit();
  this->token0_.commit();
  this->token1_.commit();
  this->reserve0_.commit();
  this->reserve1_.commit();
  this->blockTimestampLast_ .commit();
  this->price0CumulativeLast_.commit();
  this->price1CumulativeLast_.commit();
  this->kLast_.commit();

  this->registerContractFunctions();

  this->factory_.enableRegister();
  this->token0_.enableRegister();
  this->token1_.enableRegister();
  this->reserve0_.enableRegister();
  this->reserve1_.enableRegister();
  this->blockTimestampLast_ .enableRegister();
  this->price0CumulativeLast_.enableRegister();
  this->price1CumulativeLast_.enableRegister();
  this->kLast_.enableRegister();
  Utils::safePrintTest("Loaded from DB DEXV2Pair contract");
  Utils::safePrintTest("Factory: " + this->factory_.get().hex().get());
  Utils::safePrintTest("Token0: " + this->token0_.get().hex().get());
  Utils::safePrintTest("Token1: " + this->token1_.get().hex().get());
  Utils::safePrintTest("Reserve0: " + this->reserve0_.get().str());
  Utils::safePrintTest("Reserve1: " + this->reserve1_.get().str());
  Utils::safePrintTest("BlockTimestampLast: " + std::to_string(this->blockTimestampLast_.get()));
  Utils::safePrintTest("Price0CumulativeLast: " + this->price0CumulativeLast_.get().str());
  Utils::safePrintTest("Price1CumulativeLast: " + this->price1CumulativeLast_.get().str());
  Utils::safePrintTest("KLast: " + this->kLast_.get().str());

}

DEXV2Pair::DEXV2Pair(
  const Address& address, const Address& creator, const uint64_t& chainId
) : ERC20("DEXV2Pair", "DEX V2", "DEX-V2", 18, 0, address, creator, chainId),
  factory_(this), token0_(this), token1_(this), reserve0_(this), reserve1_(this),
  blockTimestampLast_(this), price0CumulativeLast_(this), price1CumulativeLast_(this), kLast_(this)
{
  // Explicitly initialize numbers to 0 to avoid junk values on DB load
  this->factory_ = creator;
  this->reserve0_ = 0;
  this->reserve1_ = 0;
  this->blockTimestampLast_ = 0;
  this->price0CumulativeLast_ = 0;
  this->price1CumulativeLast_ = 0;
  this->kLast_ = 0;

  this->factory_.commit();
  this->token0_.commit();
  this->token1_.commit();
  this->reserve0_.commit();
  this->reserve1_.commit();
  this->blockTimestampLast_ .commit();
  this->price0CumulativeLast_.commit();
  this->price1CumulativeLast_.commit();
  this->kLast_.commit();

  this->registerContractFunctions();

  this->factory_.enableRegister();
  this->token0_.enableRegister();
  this->token1_.enableRegister();
  this->reserve0_.enableRegister();
  this->reserve1_.enableRegister();
  this->blockTimestampLast_ .enableRegister();
  this->price0CumulativeLast_.enableRegister();
  this->price1CumulativeLast_.enableRegister();
  this->kLast_.enableRegister();
  Utils::safePrintTest("Creating new DEXV2Pair contract");
  Utils::safePrintTest("Factory: " + this->factory_.get().hex().get());
  Utils::safePrintTest("Token0: " + this->token0_.get().hex().get());
  Utils::safePrintTest("Token1: " + this->token1_.get().hex().get());
  Utils::safePrintTest("Reserve0: " + this->reserve0_.get().str());
  Utils::safePrintTest("Reserve1: " + this->reserve1_.get().str());
  Utils::safePrintTest("BlockTimestampLast: " + std::to_string(this->blockTimestampLast_.get()));
  Utils::safePrintTest("Price0CumulativeLast: " + this->price0CumulativeLast_.get().str());
  Utils::safePrintTest("Price1CumulativeLast: " + this->price1CumulativeLast_.get().str());
  Utils::safePrintTest("KLast: " + this->kLast_.get().str());
}

DEXV2Pair::~DEXV2Pair() {};

void DEXV2Pair::registerContractFunctions() {
  registerContract();
  this->registerMemberFunction("initialize", &DEXV2Pair::initialize, FunctionTypes::NonPayable, this);
  this->registerMemberFunction("getReserves", &DEXV2Pair::getReserves, FunctionTypes::View, this);
  this->registerMemberFunction("factory", &DEXV2Pair::factory, FunctionTypes::View, this);
  this->registerMemberFunction("token0", &DEXV2Pair::token0, FunctionTypes::View, this);
  this->registerMemberFunction("token1", &DEXV2Pair::token1, FunctionTypes::View, this);
  this->registerMemberFunction("price0CumulativeLast", &DEXV2Pair::price0CumulativeLast, FunctionTypes::View, this);
  this->registerMemberFunction("price1CumulativeLast", &DEXV2Pair::price1CumulativeLast, FunctionTypes::View, this);
  this->registerMemberFunction("kLast", &DEXV2Pair::kLast, FunctionTypes::View, this);
  this->registerMemberFunction("mint", &DEXV2Pair::mint, FunctionTypes::NonPayable, this);
  this->registerMemberFunction("burn", &DEXV2Pair::burn, FunctionTypes::NonPayable, this);
  this->registerMemberFunction("swap", &DEXV2Pair::swap, FunctionTypes::NonPayable, this);
  this->registerMemberFunction("skim", &DEXV2Pair::skim, FunctionTypes::NonPayable, this);
  this->registerMemberFunction("sync", &DEXV2Pair::sync, FunctionTypes::NonPayable, this);
}

void DEXV2Pair::_safeTransfer(const Address& token, const Address& to, const uint256_t& value) {
  this->callContractFunction(token, &ERC20::transfer, to, value);
}

void DEXV2Pair::_update(const uint256_t& balance0, const uint256_t& balance1, const uint256_t& reserve0, const uint256_t& reserve1) {
  // Timestamp is in microseconds, we want in seconds
  auto blockTimestamp = uint32_t(ContractGlobals::getBlockTimestamp() / 1000000);
  if (
    uint32_t timeElapsed = blockTimestamp - this->blockTimestampLast_.get();
    timeElapsed > 0 && reserve0 != 0 && reserve1 != 0
  ) {
    this->price0CumulativeLast_ += uint256_t(UQ112x112::uqdiv(UQ112x112::encode(uint112_t(reserve1)), uint112_t(reserve0))) * timeElapsed;
    this->price1CumulativeLast_ += uint256_t(UQ112x112::uqdiv(UQ112x112::encode(uint112_t(reserve0)), uint112_t(reserve1))) * timeElapsed;
  }
  this->reserve0_ = uint112_t(balance0);
  this->reserve1_ = uint112_t(balance1);
  this->blockTimestampLast_ = blockTimestamp;
}

bool DEXV2Pair::_mintFee(uint112_t reserve0, uint112_t reserve1) {
  Address feeTo = this->callContractViewFunction(this->factory_.get(), &DEXV2Factory::feeTo);
  bool feeOn = feeTo ? true : false;
  uint256_t _kLast = this->kLast_.get();
  if (feeOn && _kLast != 0) {
    uint256_t rootK = boost::multiprecision::sqrt(uint256_t(reserve0) * uint256_t(reserve1));
    uint256_t rootKLast = boost::multiprecision::sqrt(_kLast);
    if (rootK > rootKLast) {
      uint256_t numerator = this->totalSupply_.get() * (rootK - rootKLast);
      uint256_t denominator = rootK * 5 + rootKLast;
      uint256_t liquidity = numerator / denominator;
      if (liquidity > 0) this->mintValue_(feeTo, liquidity);
    }
  } else if (_kLast != 0) {
    this->kLast_ = 0;
  }
  return feeOn;
}

void DEXV2Pair::initialize(const Address& token0, const Address& token1) {
  if (this->factory_ != this->getCaller()) throw DynamicException("DEXV2Pair: FORBIDDEN");
  this->token0_ = token0;
  this->token1_ = token1;
  Utils::safePrintTest("Initialized DEXV2Pair contract with token0: " + token0.hex().get() + " and token1: " + token1.hex().get());
  Utils::safePrintTest("Initialized Reserves 0: " + this->reserve0_.get().str() + " and Reserves 1: " + this->reserve1_.get().str());
}

std::pair<uint256_t, uint256_t> DEXV2Pair::getReservess() const {
  return std::make_pair(this->reserve0_.get(), this->reserve1_.get());
}

std::tuple<uint256_t, uint256_t, uint256_t> DEXV2Pair::getReserves() const {
  return std::make_tuple(this->reserve0_.get(), this->reserve1_.get(), this->blockTimestampLast_.get());
}

Address DEXV2Pair::factory() const { return this->factory_.get(); }

Address DEXV2Pair::token0() const { return this->token0_.get(); }

Address DEXV2Pair::token1() const { return this->token1_.get(); }

uint256_t DEXV2Pair::price0CumulativeLast() const { return this->price0CumulativeLast_.get(); }

uint256_t DEXV2Pair::price1CumulativeLast() const { return this->price1CumulativeLast_.get(); }

uint256_t DEXV2Pair::kLast() const { return this->kLast_.get(); }

uint256_t DEXV2Pair::mint(const Address& to) {
  ReentrancyGuard reentrancyGuard(this->reentrancyLock_);
  uint256_t liquidity = 0;
  uint256_t balance0 = this->callContractViewFunction(this->token0_.get(), &ERC20::balanceOf, this->getContractAddress());
  uint256_t balance1 = this->callContractViewFunction(this->token1_.get(), &ERC20::balanceOf, this->getContractAddress());
  uint256_t amount0 = balance0 - this->reserve0_.get();
  uint256_t amount1 = balance1 - this->reserve1_.get();

  bool feeOn = this->_mintFee(this->reserve0_.get(), this->reserve1_.get());
  if (uint256_t totalSupply = this->totalSupply_.get(); totalSupply == 0) {
    // Permanently lock the first MINIMUM_LIQUIDITY tokens
    liquidity = boost::multiprecision::sqrt(amount0 * amount1) - MINIMUM_LIQUIDITY;
    this->mintValue_(Address(Hex::toBytes("0x0000000000000000000000000000000000000000")), MINIMUM_LIQUIDITY);
  } else {
    liquidity = std::min(amount0 * totalSupply / this->reserve0_.get(), amount1 * totalSupply / this->reserve1_.get());
  }

  if (liquidity == 0) throw DynamicException("DEXV2Pair: INSUFFICIENT_LIQUIDITY_MINTED");
  this->mintValue_(to, liquidity);
  this->_update(balance0, balance1, this->reserve0_.get(), this->reserve1_.get());
  if (feeOn) this->kLast_ = uint256_t(this->reserve0_.get()) * uint256_t(this->reserve1_.get());
  return liquidity;
}

std::tuple<uint256_t, uint256_t> DEXV2Pair::burn(const Address& to) {
  ReentrancyGuard reentrancyGuard(this->reentrancyLock_);
  uint256_t balance0 = this->callContractViewFunction(
    this->token0_.get(), &ERC20::balanceOf, this->getContractAddress()
  );
  uint256_t balance1 = this->callContractViewFunction(
    this->token1_.get(), &ERC20::balanceOf, this->getContractAddress()
  );
  uint256_t liquidity = this->balanceOf(this->getContractAddress());

  bool feeOn = this->_mintFee(this->reserve0_.get(), this->reserve1_.get());
  uint256_t totalSupply = this->totalSupply_.get();
  uint256_t amount0 = liquidity * balance0 / totalSupply;
  uint256_t amount1 = liquidity * balance1 / totalSupply;
  if (amount0 == 0 || amount1 == 0) throw DynamicException("DEXV2Pair: INSUFFICIENT_LIQUIDITY_BURNED");
  this->burnValue_(this->getContractAddress(), liquidity);
  this->_safeTransfer(this->token0_.get(), to, amount0);
  this->_safeTransfer(this->token1_.get(), to, amount1);
  balance0 = this->callContractViewFunction(this->token0_.get(), &ERC20::balanceOf, this->getContractAddress());
  balance1 = this->callContractViewFunction(this->token1_.get(), &ERC20::balanceOf, this->getContractAddress());
  this->_update(balance0, balance1, this->reserve0_.get(), this->reserve1_.get());
  if (feeOn) kLast_ = uint256_t(this->reserve0_.get()) * uint256_t(this->reserve1_.get());
  return std::make_tuple(amount0, amount1);
}

void DEXV2Pair::swap(const uint256_t& amount0Out, const uint256_t& amount1Out, const Address& to) {
  ReentrancyGuard reentrancyGuard(this->reentrancyLock_);
  if (amount0Out == 0 && amount1Out == 0) throw DynamicException("DEXV2Pair: INSUFFICIENT_OUTPUT_AMOUNT");
  if (reserve0_ <= uint112_t(amount0Out) && reserve1_ <= uint112_t(amount1Out)) throw DynamicException("DEXV2Pair: INSUFFICIENT_LIQUIDITY");
  if (token0_ == to || token1_ == to) throw DynamicException("DEXV2Pair: INVALID_TO");
  if (amount0Out > 0) this->_safeTransfer(this->token0_.get(), to, amount0Out);
  if (amount1Out > 0) this->_safeTransfer(this->token1_.get(), to, amount1Out);
  uint256_t balance0 = this->callContractViewFunction(this->token0_.get(), &ERC20::balanceOf, this->getContractAddress());
  uint256_t balance1 = this->callContractViewFunction(this->token1_.get(), &ERC20::balanceOf, this->getContractAddress());
  uint256_t amount0In = balance0 > this->reserve0_.get() - uint112_t(amount0Out) ? balance0 - this->reserve0_.get() + uint112_t(amount0Out) : 0;
  uint256_t amount1In = balance1 > this->reserve1_.get() - uint112_t(amount1Out) ? balance1 - this->reserve1_.get() + uint112_t(amount1Out) : 0;
  if (amount0In == 0 && amount1In == 0) throw DynamicException("DEXV2Pair: INSUFFICIENT_INPUT_AMOUNT");
  uint256_t balance0Adjusted = balance0 * 1000 - amount0In * 3;
  uint256_t balance1Adjusted = balance1 * 1000 - amount1In * 3;
  if (
    uint256_t balTotal = balance0Adjusted * balance1Adjusted;
    balTotal < uint256_t(this->reserve0_.get()) * this->reserve1_.get() * 1000 * 1000
  ) {
    throw DynamicException("DEXV2Pair: K");
  }
  this->_update(balance0, balance1, this->reserve0_.get(), this->reserve1_.get());
}

void DEXV2Pair::skim(const Address &to) {
  ReentrancyGuard reentrancyGuard(this->reentrancyLock_);
  this->_safeTransfer(this->token0_.get(), to, this->callContractViewFunction(
    this->token0_.get(), &ERC20::balanceOf, this->getContractAddress()
  ) - this->reserve0_.get());
  this->_safeTransfer(this->token1_.get(), to, this->callContractViewFunction(
    this->token1_.get(), &ERC20::balanceOf, this->getContractAddress()
  ) - this->reserve1_.get());
}

void DEXV2Pair::sync() {
  ReentrancyGuard reentrancyGuard(this->reentrancyLock_);
  this->_update(
    this->callContractViewFunction(this->token0_.get(), &ERC20::balanceOf, this->getContractAddress()),
    this->callContractViewFunction(this->token1_.get(), &ERC20::balanceOf, this->getContractAddress()),
    this->reserve0_.get(), this->reserve1_.get()
  );
}


DBBatch DEXV2Pair::dump() const {
  // We have to dump the tokens as well
  DBBatch dbBatch = BaseContract::dump();
  DBBatch erc20Batch = ERC20::dump();
  for (const auto& dbItem : erc20Batch.getPuts()) dbBatch.push_back(dbItem);
  for (const auto& dbItem : erc20Batch.getDels()) dbBatch.delete_key(dbItem);

  Utils::safePrintTest("Dumping DEXV2Pair to DB");
  uint112_t zeroUint = 0;
  uint112_t oneUint = 1;
  auto zeroUintBytes = UintConv::uint112ToBytes(zeroUint);
  auto oneUintBytes = UintConv::uint112ToBytes(oneUint);
  Utils::safePrintTest("Zero Uint: " + zeroUint.str());
  Utils::safePrintTest("One Uint: " + oneUint.str());
  Utils::safePrintTest("Zero Uint Bytes: " + Hex::fromBytes(zeroUintBytes).get());
  Utils::safePrintTest("One Uint Bytes: " + Hex::fromBytes(oneUintBytes).get());

  Bytes zeroUintRawBytes;
  zeroUintRawBytes.reserve(14);
  Bytes oneUintRawBytes;
  oneUintRawBytes.reserve(14);
  boost::multiprecision::export_bits(zeroUint, std::back_inserter(zeroUintRawBytes), 8);
  boost::multiprecision::export_bits(oneUint, std::back_inserter(oneUintRawBytes), 8);
  Utils::safePrintTest("Zero Uint Raw Bytes: " + Hex::fromBytes(zeroUintRawBytes).get());
  Utils::safePrintTest("One Uint Raw Bytes: " + Hex::fromBytes(oneUintRawBytes).get());

  Utils::safePrintTest("Factory: " + this->factory_.get().hex().get());
  Utils::safePrintTest("Token0: " + this->token0_.get().hex().get());
  Utils::safePrintTest("Token1: " + this->token1_.get().hex().get());
  Utils::safePrintTest("Reserve0 Before: " + this->reserve0_.get().str());
  Utils::safePrintTest("Reserve1 Before: " + this->reserve1_.get().str());
  auto reserve0Bytes = UintConv::uint112ToBytes(this->reserve0_.get());
  auto reserve1Bytes = UintConv::uint112ToBytes(this->reserve1_.get());
  Utils::safePrintTest("Reserve0 After: " + this->reserve0_.get().str());
  Utils::safePrintTest("Reserve1 After: " + this->reserve1_.get().str());
  Utils::safePrintTest("reserve0 Bytes: " + Hex::fromBytes(reserve0Bytes).get());
  Utils::safePrintTest("reserve1 Bytes: " + Hex::fromBytes(reserve1Bytes).get());

  Utils::safePrintTest("BlockTimestampLast: " + std::to_string(this->blockTimestampLast_.get()));
  Utils::safePrintTest("Price0CumulativeLast: " + this->price0CumulativeLast_.get().str());
  Utils::safePrintTest("Price1CumulativeLast: " + this->price1CumulativeLast_.get().str());
  Utils::safePrintTest("KLast: " + this->kLast_.get().str());

  dbBatch.push_back(StrConv::stringToBytes("factory_"), this->factory_.get().view(), this->getDBPrefix());
  dbBatch.push_back(StrConv::stringToBytes("token0_"), this->token0_.get().view(), this->getDBPrefix());
  dbBatch.push_back(StrConv::stringToBytes("token1_"), this->token1_.get().view(), this->getDBPrefix());
  dbBatch.push_back(StrConv::stringToBytes("reserve0_"), reserve0Bytes, this->getDBPrefix());
  dbBatch.push_back(StrConv::stringToBytes("reserve1_"), reserve1Bytes, this->getDBPrefix());
  dbBatch.push_back(StrConv::stringToBytes("blockTimestampLast_"), UintConv::uint32ToBytes(this->blockTimestampLast_.get()), this->getDBPrefix());
  dbBatch.push_back(StrConv::stringToBytes("price0CumulativeLast_"), UintConv::uint256ToBytes(this->price0CumulativeLast_.get()), this->getDBPrefix());
  dbBatch.push_back(StrConv::stringToBytes("price1CumulativeLast_"), UintConv::uint256ToBytes(this->price1CumulativeLast_.get()), this->getDBPrefix());
  dbBatch.push_back(StrConv::stringToBytes("kLast_"), UintConv::uint256ToBytes(this->kLast_.get()), this->getDBPrefix());
  const auto& puts = dbBatch.getPuts();
  for (const auto& dbItem : puts) {
    // Take of the prefix based on this->getDBPrefix().size() from the dbItem.key
    Bytes prefixBytes(dbItem.key.begin(), dbItem.key.begin() + this->getDBPrefix().size());
    Hex prefix = Hex::fromBytes(prefixBytes);
    // Then take dbItem.key + this->getDBPrefix().size() to the end
    std::string key(dbItem.key.begin() + this->getDBPrefix().size(), dbItem.key.end());
    Hex value = Hex::fromBytes(dbItem.value);
    Utils::safePrintTest("Dumping to DB with prefix : " + prefix.get() + " key: " + key + " value: " + value.get());
  }
  return dbBatch;
}

