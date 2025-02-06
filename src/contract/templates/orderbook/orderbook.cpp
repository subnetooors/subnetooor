/*
  Copyright (c) [2023] [Sparq Network]

  This software is distributed under the MIT License.
  See the LICENSE.txt file in the project root for more information.
*/

#include "orderbook.h"

OrderBook::OrderBook(const Address& addA, const std::string& tickerA, const uint8_t decA,
                     const Address& addB, const std::string& tickerB, const uint8_t decB,
                     const Address& address, const Address& creator,
                     const uint64_t& chainId)
  : DynamicContract("OrderBook", address, creator, chainId),
    nextOrderID_(this),
    addressAssetA_(this),
    tickerAssetA_(this),
    addressAssetB_(this),
    tickerAssetB_(this),
    spread_(this),
    tickSize_(this),
    lotSize_(this),
    lastPrice_(this),
    bids_(this),
    asks_(this),
    stops_(this)
{
  // set
  this->nextOrderID_ = 0;
  this->addressAssetA_ = addA;
  this->addressAssetB_ = addB;
  this->tickerAssetA_ = tickerA;
  this->tickerAssetB_ = tickerB;
  this->spread_ = 0;
  this->lastPrice_ = 0;
  // constant
  this->tickSize_ = Utils::exp10<uint256_t>(decB - 4);
  this->lotSize_ = Utils::exp10<uint256_t>(decA - 4);
  // commit
  this->nextOrderID_.commit();
  this->addressAssetA_.commit();
  this->addressAssetB_.commit();
  this->tickerAssetA_.commit();
  this->tickerAssetB_.commit();
  this->spread_.commit();
  this->lotSize_.commit();
  this->tickSize_.commit();
  this->lastPrice_.commit();
  this->bids_.commit();
  this->asks_.commit();
  this->stops_.commit();
  // register functions
  this->registerContractFunctions();
  // enable register
  this->nextOrderID_.enableRegister();
  this->addressAssetA_.enableRegister();
  this->addressAssetB_.enableRegister();
  this->tickerAssetA_.enableRegister();
  this->tickerAssetB_.enableRegister();
  this->spread_.enableRegister();
  this->lotSize_.enableRegister();
  this->tickSize_.enableRegister();
  this->lastPrice_.enableRegister();
  this->bids_.enableRegister();
  this->asks_.enableRegister();
  this->stops_.enableRegister();
}

OrderBook::OrderBook(const Address& address,
                     const DB &db)
  : DynamicContract(address, db),
    nextOrderID_(this),
    addressAssetA_(this),
    tickerAssetA_(this),
    addressAssetB_(this),
    tickerAssetB_(this),
    spread_(this)
{
  // set
  this->nextOrderID_ = UintConv::bytesToUint256(db.get(std::string("nextOrderID_"), this->getDBPrefix()));
  this->addressAssetA_ = Address(db.get(std::string("addressAssetA_"), this->getDBPrefix()));
  this->addressAssetB_ = Address(db.get(std::string("addressAssetB_"), this->getDBPrefix()));
  this->tickerAssetA_ = StrConv::bytesToString(db.get(std::string("tickerAssetA_"), this->getDBPrefix()));
  this->tickerAssetB_ = StrConv::bytesToString(db.get(std::string("tickerAssetB_"), this->getDBPrefix()));
  this->spread_ = UintConv::bytesToUint256(db.get(std::string("spread_"), this->getDBPrefix()));
  this->tickSize_ = UintConv::bytesToUint256(db.get(std::string("tickSize_"), this->getDBPrefix()));
  this->lotSize_ = UintConv::bytesToUint256(db.get(std::string("lotSize_"), this->getDBPrefix()));
  this->lastPrice_ = UintConv::bytesToUint256(db.get(std::string("lastPrice_"), this->getDBPrefix()));
  // commit
  this->nextOrderID_.commit();
  this->addressAssetA_.commit();
  this->addressAssetB_.commit();
  this->tickerAssetA_.commit();
  this->tickerAssetB_.commit();
  this->spread_.commit();
  this->lotSize_.commit();
  this->tickSize_.commit();
  this->lastPrice_.commit();
  this->bids_.commit();
  this->asks_.commit();
  this->stops_.commit();
  // register functions
  this->registerContractFunctions();
  // enable register
  this->nextOrderID_.enableRegister();
  this->addressAssetA_.enableRegister();
  this->addressAssetB_.enableRegister();
  this->tickerAssetA_.enableRegister();
  this->tickerAssetB_.enableRegister();
  this->spread_.enableRegister();
  this->lotSize_.enableRegister();
  this->tickSize_.enableRegister();
  this->lastPrice_.enableRegister();
  this->bids_.enableRegister();
  this->asks_.enableRegister();
  this->stops_.enableRegister();
}

inline void OrderBook::insertAskOrder(const Order& askOrder)
{
  this->asks_.insert(std::move(askOrder));
}

inline void OrderBook::insertBidOrder(const Order& bidOrder)
{
  this->bids_.insert(std::move(bidOrder));
}

inline void OrderBook::eraseAskOrder(const Order& askOrder)
{
  this->asks_.erase(askOrder);
}

inline void OrderBook::eraseBidOrder(const Order& bidOrder)
{
  this->bids_.erase(bidOrder);
}

void OrderBook::executeOrder(const Address& askOwner,
                             const Address& bidOwner,
                             uint256_t tokensToBePaid,
                             uint256_t tokenAmount)
{
  this->callContractFunction(this->addressAssetB_.get(), &ERC20::transfer, askOwner, tokensToBePaid);
  this->callContractFunction(this->addressAssetA_.get(), &ERC20::transfer, bidOwner, this->tokensLot(tokenAmount));
}

Order* OrderBook::findMatchAskOrder(const Order& bidOrder)
{
  auto& bidTokenAmount = std::get<3>(bidOrder);
  // do we have any asks orders?
  if (this->asks_.empty()) {
    return nullptr;
  }
  // get the first ask order
  const Order* askOrder = &(*(this->asks_.begin()));
  auto& askTokenPrice = std::get<4>(*askOrder);
  auto& bidTokenPrice = std::get<4>(bidOrder);
  auto& bidOrderType = std::get<5>(bidOrder);

  switch (bidOrderType) {
    // doesn't matter the price return the first ask order found
  case OrderType::MARKET: {
    return const_cast<Order*>(askOrder);
  }
    // we want to buy for the lowest price
  case OrderType::LIMIT: {
    return ((bidTokenPrice <= askTokenPrice) ? const_cast<Order*>(askOrder) : nullptr);
  }
    // default do nothing
  default:
    break;
  }
  // not found
  return nullptr;
}

Order* OrderBook::findMatchBidOrder(const Order& askOrder)
{
  auto& askTokenAmount = std::get<3>(askOrder);
  // do we have bid orders?
  if (this->bids_.empty()) {
    return nullptr;
  }
  // get the first bid order (the higher value)
  const Order* bidOrder = &(*(this->bids_.begin()));
  // we want to sell for the higher bid price
  // but never not lower to the ask price limit
  auto& bidTokenPrice = std::get<4>(*bidOrder);
  auto& askTokenPrice = std::get<4>(askOrder);
  auto& askOrderType = std::get<5>(askOrder);
  switch (askOrderType) {
    // doesn't matter the price return the first bid order found
  case OrderType::MARKET: {
    return const_cast<Order*>(bidOrder);
  }
    // we want to sell at the highest price
  case OrderType::LIMIT: {
    return ((bidTokenPrice >= askTokenPrice) ? const_cast<Order*>(bidOrder) : nullptr);
  }
  default:
    break;
  }
  // not found
  return nullptr;
}

void OrderBook::evaluateMarketBidOrder(Order&& bidOrder)
{
  Order *matchAskOrder;
  // get bid order attributes values
  const auto& bidOwner = std::get<2>(bidOrder);
  auto& bidTokenAmount = std::get<3>(bidOrder);
  uint256_t bidTokensToBePaid = this->tokensTick(bidTokenAmount);
  // find the ask order
  while(((matchAskOrder = findMatchAskOrder(bidOrder)) != nullptr) and \
        (bidTokensToBePaid > 0) and \
        (bidTokenAmount > 0)) {
    const auto& askOwner = std::get<2>(*matchAskOrder);
    auto& askTokenAmount = std::get<3>(*matchAskOrder);
    auto& askTokenPrice = std::get<4>(*matchAskOrder);
    // compute the tokens amount and how much must be transfer (temporary variables)
    uint256_t tokenAmount = std::min(askTokenAmount, ((bidTokensToBePaid * 10000) / askTokenPrice));
    uint256_t tokensToBePaid = this->tokensToBePaid(tokenAmount, askTokenPrice);
    // transfer the tokens to the contract
    this->transferToContract(this->addressAssetB_.get(), tokensToBePaid);
    // executes the order, transfer the tokens from ask owner to bid owner
    this->executeOrder(askOwner, bidOwner, tokensToBePaid, tokenAmount);
    // update bid tokens to be paid information
    bidTokensToBePaid -= tokensToBePaid;
    // update amount information
    bidTokenAmount -= tokenAmount;
    askTokenAmount -= tokenAmount;
    // update the current price
    this->updateLastPrice(askTokenPrice);
    // remove order if it was filled
    if (askTokenAmount == 0) {
      this->eraseAskOrder(*matchAskOrder);
    }
  }
  // update spread and mid price
  this->updateSpreadAndMidPrice();
}

void OrderBook::evaluateBidOrder(Order&& bidOrder)
{
  Order *matchAskOrder;
  // get bid order attributes values
  const auto& bidOwner = std::get<2>(bidOrder);
  auto& bidTokenAmount = std::get<3>(bidOrder);
  auto& bidTokenPrice = std::get<4>(bidOrder);
  while(((matchAskOrder = findMatchAskOrder(bidOrder)) != nullptr) and
        (bidTokenAmount > 0)) {
    // get ask order attributes values
    const auto& askOwner = std::get<2>(*matchAskOrder);
    auto& askTokenAmount = std::get<3>(*matchAskOrder);
    auto& askTokenPrice = std::get<4>(*matchAskOrder);
    // compute the aTokens and bTokens to be transfered
    uint256_t tokenAmount = std::min(askTokenAmount, bidTokenAmount);
    uint256_t tokensToBePaid = this->tokensToBePaid(tokenAmount, askTokenPrice);
    // executes the order, transfer the tokens from ask owner to bid owner
    this->executeOrder(askOwner, bidOwner, tokensToBePaid, tokenAmount);
    // update amount information
    bidTokenAmount -= tokenAmount;
    askTokenAmount -= tokenAmount;
    // update the current price
    this->updateLastPrice(askTokenPrice);
    // erase the ask asset if filled
    if (askTokenAmount == 0) {
      this->eraseAskOrder(*matchAskOrder);
    }
  }
  // handle the bid order that was not filled (remainder)
  if (bidTokenAmount > 0) {
    this->insertBidOrder(bidOrder);
  }
  // update spread and mid price
  this->updateSpreadAndMidPrice();
}

void OrderBook::evaluateAskOrder(Order&& askOrder)
{
  Order *matchBidOrder;
  const auto& askOwner = std::get<2>(askOrder);
  auto& askTokenAmount = std::get<3>(askOrder);
  auto& askTokenPrice = std::get<4>(askOrder);
  auto& askOrderType = std::get<5>(askOrder);
  while (((matchBidOrder = findMatchBidOrder(askOrder)) != nullptr) and \
         (askTokenAmount > 0)) {
    // get bid order attributes values
    const auto& bidOwner = std::get<2>(*matchBidOrder);
    auto& bidTokenAmount = std::get<3>(*matchBidOrder);
    auto& bidTokenPrice = std::get<4>(*matchBidOrder);
    // compute the asset and token amount
    uint256_t tokenAmount = std::min(askTokenAmount, bidTokenAmount);
    uint256_t tokensToBePaid = this->tokensToBePaid(tokenAmount, bidTokenPrice);
    // executes the order, transfer the tokens from ask owner to bid owner
    this->executeOrder(askOwner, bidOwner, tokensToBePaid, tokenAmount);
    // update order asset amounts
    askTokenAmount -= tokenAmount;
    bidTokenAmount -= tokenAmount;
    // update the current price
    this->updateLastPrice(bidTokenPrice);
    // erase the bid order if was filled
    if (bidTokenAmount == 0) {
      this->eraseBidOrder(*matchBidOrder);
    }
  }
  // handle the ask order that was not filled (remainder)
  if (askTokenAmount > 0 and  askOrderType != OrderType::MARKET) {
    this->insertAskOrder(askOrder);
  }
  // update spread and mid price
  this->updateSpreadAndMidPrice();
}

void OrderBook::transferToContract(const Address& assetAddress,
                                   const uint256_t& tokenAmount)
{
  this->callContractFunction(assetAddress,
                             &ERC20::transferFrom,
                             this->getCaller(),
                             this->getContractAddress(),
                             tokenAmount);
}

Order OrderBook::makeOrder(const uint256_t& tokenAmount,
                           const uint256_t& tokenPrice,
                           const OrderType orderType)
{
  return Order(this->nextOrderID_.get(),
               this->getCurrentTimestamp(),
               this->getCaller(),
               tokenAmount,
               tokenPrice,
               orderType);
}

void OrderBook::addBidLimitOrder(const uint256_t& tokenAmount,
                                 // we want to buy for the lowest price
                                 const uint256_t& tokenPrice)
{
  // set the amount of B tokens available
  uint256_t tokensBTotalBalance = \
    this->callContractViewFunction(this->addressAssetB_.get(),
                                   &ERC20::balanceOf,
                                   this->getCaller());

  // convert to the number of tokens
  uint256_t tokensBToBePaid = this->tokensToBePaid(tokenAmount, tokenPrice);

  // verify the tokens balance
  if (tokensBToBePaid > tokensBTotalBalance) {
    throw std::runtime_error("OrderBook::addBidLimitOrder: INSUFFICIENT_BALANCE");
  }
  // transfer token to be paid to order book contract
  // evaluate the bid limit order and increment the next order id
  this->transferToContract(this->addressAssetB_.get(), tokensBToBePaid);
  this->evaluateBidOrder(std::move(this->makeOrder(tokenAmount,
                                                   tokenPrice,
                                                   OrderType::LIMIT)));
  this->nextOrderID_++;
}

void OrderBook::delBidLimitOrder(const uint256_t& id)
{
  this->bids_.erase_if([&id, this](const Order& bidOrder) {
    auto const& bidId = std::get<0>(bidOrder);
    if (bidId != id) {
      return false;
    }
    auto const& bidOwner = std::get<2>(bidOrder);
    auto const& bidTokenAmount = std::get<3>(bidOrder);
    auto const& bidTokenPrice = std::get<4>(bidOrder);
    if (bidOwner != this->getCaller()) {
      throw std::runtime_error("OrderBook::delBidLimitOrder: INVALID_OWNER");
    }
    uint256_t tokenAmount = this->tokensToBePaid(bidTokenAmount, bidTokenPrice);
    this->callContractFunction(this->addressAssetB_.get(),
                               &ERC20::transfer,
                               bidOwner,
                               tokenAmount);
    return true;
  });
}

// you can sell for the limit value that you want, but
// the value must be a multiplier of the lot size and
// and the tokens amount needs to be less then the total of A tokens
// available
void OrderBook::addAskLimitOrder(const uint256_t& tokenAmount,
                                 // remember this is the low limit, we
                                 // want to sell for the biggest available in
                                 // the order book
                                 const uint256_t& tokenPrice)
{
  uint256_t tokensTotalBalance = \
    this->callContractViewFunction(this->addressAssetA_.get(),
                                   &ERC20::balanceOf,
                                   this->getCaller());

  // convert tokens amount to tokens lot
  uint256_t tokensLot = this->tokensLot(tokenAmount);
  // verify tokens available
  if (tokensLot > tokensTotalBalance) {
    throw std::runtime_error("OrderBook::addAskLimitOrder:" \
                             "Insufficient number of tokens");
  }
  // verify if asset price is of lot sizable
  if (not(isLotSizable(tokensLot))) {
    throw std::runtime_error("OrderBook::addAskLimitOrder:" \
                             "The asset amount must be a multiple of the lot size");
  }
  // transfer lot amount to order book contract
  // evaluate the the nearly created ask limit order and increment next order id
  this->transferToContract(this->addressAssetA_.get(), this->tokensLot(tokenAmount));
  // this should be transfer to another thread of execution?
  this->evaluateAskOrder(std::move(this->makeOrder(tokenAmount,
                                                   tokenPrice,
                                                   OrderType::LIMIT)));
  this->nextOrderID_++;
}

void OrderBook::delAskLimitOrder(const uint256_t& id)
{
  this->asks_.erase_if([&id, this](const Order& askOrder) {
    auto const& askId = std::get<0>(askOrder);
    if (askId != id) {
      return false;
    }
    auto const& askOwner = std::get<2>(askOrder);
    auto const& askTokenAmount = std::get<3>(askOrder);
    if (askOwner != this->getCaller()) {
      throw std::runtime_error("OrderBook::delAskLimitOrder: INVALID_OWNER");
    }
    this->callContractFunction(this->addressAssetA_.get(),
                               &ERC20::transfer,
                               askOwner,
                               this->tokensLot(askTokenAmount));
    return true;
  });
}

void OrderBook::addAskMarketOrder(const uint256_t& tokenAmount,
                                  const uint256_t& tokenPrice)
{
  // set tokens balance
  uint256_t tokenBalance = \
    this->callContractViewFunction(this->addressAssetA_.get(),
                                   &ERC20::balanceOf,
                                   this->getCaller());
  // convert lot amount
  uint256_t tokenLotAmount = this->tokensLot(tokenAmount);
  // verify if token lot amount is bigger than user token balance
  if (tokenLotAmount > tokenBalance) {
    throw std::runtime_error("OrderBook::addAskMarketOrder: INSUFFICIENT_BALANCE");
  }
  this->transferToContract(this->addressAssetA_.get(), tokenLotAmount);
  this->evaluateAskOrder(std::move(this->makeOrder(tokenAmount, 0, OrderType::MARKET)));
  this->nextOrderID_++;
}

void OrderBook::addBidMarketOrder(const uint256_t& tokenAmount,
                                  const uint256_t& tokenPrice)
{
  // set asset balance
  uint256_t tokenBalance = \
    this->callContractViewFunction(this->addressAssetB_.get(),
                                   &ERC20::balanceOf,
                                   this->getCaller());
  // convert tick amount
  uint256_t tokensTick = this->tokensTick(tokenAmount);
  // verify if tick amount is bigger than user balance
  if (tokensTick > tokenBalance) {
    throw std::runtime_error("OrderBook::addBidMarketOrder: INSUFFICIENT_BALANCE");
  }
  this->evaluateMarketBidOrder(std::move(this->makeOrder(tokenAmount, 0, OrderType::MARKET)));
  this->nextOrderID_++;
}

inline void OrderBook::updateLastPrice(const uint256_t &price)
{
  this->lastPrice_ = price;
}

void OrderBook::updateSpreadAndMidPrice()
{
  if (this->bids_.empty() or
      this->asks_.empty())
    return;
  uint256_t bidPrice = std::get<4>(*this->bids_.cbegin());
  uint256_t askPrice = std::get<4>(*this->asks_.cbegin());
  this->spread_ = (std::max(bidPrice, askPrice) -\
                   std::min(bidPrice, askPrice));
}

uint64_t OrderBook::getCurrentTimestamp() const
{
  return std::chrono::duration_cast<std::chrono::milliseconds>
    (std::chrono::system_clock::now().time_since_epoch()).count();
}

Order OrderBook::getFirstBid() const
{
  return *(this->bids_.cbegin());
}

Order OrderBook::getFirstAsk() const
{
  return *(this->asks_.cbegin());
}

std::vector<Order> OrderBook::getBids() const
{
  std::vector<Order> ret;
  for (const auto& bid : this->bids_) {
    ret.push_back(bid);
  }
  return ret;
}

std::vector<Order> OrderBook::getAsks() const
{
  std::vector<Order> ret;
  for (const auto& ask : this->asks_) {
    ret.push_back(ask);
  }
  return ret;
}

std::tuple<std::vector<Order>,
           std::vector<Order>,
           std::vector<StopOrder>>
OrderBook::getUserOrders(const Address& user) const
{
  std::tuple<std::vector<Order>, std::vector<Order>, std::vector<StopOrder>> ret;
  auto& bids = std::get<0>(ret);
  auto& asks = std::get<1>(ret);
  auto& stops = std::get<2>(ret);
  for (const auto& ask : this->asks_) {
    const auto& askAddress = std::get<2>(ask);
    if (askAddress == user) asks.push_back(ask);
  }
  for (const auto& bid : this->bids_) {
    const auto& bidAddress = std::get<2>(bid);
    if (bidAddress == user) bids.push_back(bid);
  }
  for (const auto& stop : this->stops_) {
    const auto& stopAddress = std::get<2>(stop);
    if (stopAddress == user) stops.push_back(stop);
  }
  return ret;
}

void OrderBook::registerContractFunctions()
{
  registerContract();
  this->registerMemberFunction("getNextOrderID", &OrderBook::getNextOrderID, FunctionTypes::View, this);
  this->registerMemberFunction("getAddressAssetA", &OrderBook::getAddressAssetA, FunctionTypes::View, this);
  this->registerMemberFunction("getAddressAssetB", &OrderBook::getAddressAssetB, FunctionTypes::View, this);
  this->registerMemberFunction("getTickerAssetA", &OrderBook::getTickerAssetA, FunctionTypes::View, this);
  this->registerMemberFunction("getTickerAssetB", &OrderBook::getTickerAssetB, FunctionTypes::View, this);
  this->registerMemberFunction("getSpread", &OrderBook::getSpread, FunctionTypes::View, this);
  this->registerMemberFunction("getTickSize", &OrderBook::getTickSize, FunctionTypes::View, this);
  this->registerMemberFunction("getLotSize", &OrderBook::getLotSize, FunctionTypes::View, this);
  this->registerMemberFunction("getLastPrice", &OrderBook::getLastPrice, FunctionTypes::View, this);
  this->registerMemberFunction("getPrecision", &OrderBook::getPrecision, FunctionTypes::View, this);
  this->registerMemberFunction("getAsks", &OrderBook::getAsks, FunctionTypes::View, this);
  this->registerMemberFunction("getBids", &OrderBook::getBids, FunctionTypes::View, this);
  this->registerMemberFunction("getFirstAsk", &OrderBook::getFirstAsk, FunctionTypes::View, this);
  this->registerMemberFunction("getFirstBid", &OrderBook::getFirstBid, FunctionTypes::View, this);
  this->registerMemberFunction("getUserOrders", &OrderBook::getUserOrders, FunctionTypes::View, this);
  this->registerMemberFunction("addAskLimitOrder", &OrderBook::addAskLimitOrder, FunctionTypes::NonPayable, this);
  this->registerMemberFunction("addBidLimitOrder", &OrderBook::addBidLimitOrder, FunctionTypes::NonPayable, this);
  this->registerMemberFunction("delAskLimitOrder", &OrderBook::delAskLimitOrder, FunctionTypes::NonPayable, this);
  this->registerMemberFunction("delBidLimitOrder", &OrderBook::delBidLimitOrder, FunctionTypes::NonPayable, this);
  this->registerMemberFunction("addAskMarketOrder", &OrderBook::addAskMarketOrder, FunctionTypes::NonPayable, this);
  this->registerMemberFunction("addBidMarketOrder", &OrderBook::addBidMarketOrder, FunctionTypes::NonPayable, this);
}

DBBatch OrderBook::dump() const
{
  DBBatch b = BaseContract::dump();

  b.push_back(StrConv::stringToBytes("nextOrderID_"), UintConv::uint256ToBytes(this->nextOrderID_.get()), this->getDBPrefix());
  b.push_back(StrConv::stringToBytes("addressAssetA_"), this->addressAssetA_.get().view(), this->getDBPrefix());
  b.push_back(StrConv::stringToBytes("addressAssetB_"), this->addressAssetB_.get().view(), this->getDBPrefix());
  b.push_back(StrConv::stringToBytes("tickerAssetA_"), StrConv::stringToBytes(this->tickerAssetA_.get()), this->getDBPrefix());
  b.push_back(StrConv::stringToBytes("tickerAssetB_"), StrConv::stringToBytes(this->tickerAssetB_.get()), this->getDBPrefix());
  b.push_back(StrConv::stringToBytes("spread_"), UintConv::uint256ToBytes(this->spread_.get()), this->getDBPrefix());
  b.push_back(StrConv::stringToBytes("tickSize_"), UintConv::uint256ToBytes(this->tickSize_.get()), this->getDBPrefix());
  b.push_back(StrConv::stringToBytes("lotSize_"), UintConv::uint256ToBytes(this->lotSize_.get()), this->getDBPrefix());
  b.push_back(StrConv::stringToBytes("lastPrice_"), UintConv::uint256ToBytes(this->lastPrice_.get()), this->getDBPrefix());

  return b;
}
