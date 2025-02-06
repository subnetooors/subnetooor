/*
  Copyright (c) [2023-2024] [AppLayer Developers]

  This software is distributed under the MIT License.
  See the LICENSE.txt file in the project root for more information.
*/

#include "../../src/utils/db.h"
#include "../../src/core/rdpos.h"
#include "../../src/contract/abi.h"
#include "../../src/utils/options.h"
#include "../../src/contract/contractmanager.h"
#include "../../src/libs/catch2/catch_amalgamated.hpp"

#include "../../src/contract/templates/orderbook/orderbook.h"
#include "../sdktestsuite.hpp"

#include <filesystem>

namespace TORDERBOOK {
  std::string testDumpPath = Utils::getTestDumpPath();
  TEST_CASE("OrderBook Class", "[contract][orderbook]") {
    SECTION("Orderbook creation") {
      SDKTestSuite sdk = SDKTestSuite::createNewEnvironment(testDumpPath + "/testOrderBookCreation");
      REQUIRE(sdk.getState().getDumpManagerSize() == 3);
      Address askAddr = sdk.deployContract<ERC20>(std::string("A_Token"),
                                                  std::string("TST"),
                                                  uint8_t(18),
                                                  uint256_t("1000000000000000000"));
      REQUIRE(sdk.getState().getDumpManagerSize() == 4);
      REQUIRE(sdk.callViewFunction(askAddr, &ERC20::name) == "A_Token");
      REQUIRE(sdk.callViewFunction(askAddr, &ERC20::decimals) > 8);

      Address bidAddr = sdk.deployContract<ERC20>(std::string("B_Token"),
                                                  std::string("TST"),
                                                  uint8_t(18),
                                                  uint256_t("1000000000000000000"));
      REQUIRE(sdk.getState().getDumpManagerSize() == 5);
      REQUIRE(sdk.callViewFunction(bidAddr, &ERC20::name) == "B_Token");

      uint8_t decA = sdk.callViewFunction(askAddr, &ERC20::decimals);
      uint8_t decB = sdk.callViewFunction(bidAddr, &ERC20::decimals);
      Address orderBook = sdk.deployContract<OrderBook>(askAddr, std::string("A_Token"), decA,
                                                        bidAddr, std::string("B_Token"), decB);
      REQUIRE(sdk.getState().getDumpManagerSize() == 6);
    }

    SECTION("Orderbook add bid limit order") {
      // start the sdk environment
      SDKTestSuite sdk = SDKTestSuite::createNewEnvironment(testDumpPath + "/testOrderBookCreation");
      Address owner = sdk.getChainOwnerAccount().address;
      // create the ERC20 contract
      Address askAddr = sdk.deployContract<ERC20>(std::string("A_Token"), std::string("TKN_A"), uint8_t(18), uint256_t("2000000000000000000"));
      Address bidAddr = sdk.deployContract<ERC20>(std::string("B_Token"), std::string("TKN_B"), uint8_t(18), uint256_t("2000000000000000000"));
      // get decimals from the ERC20 contract
      uint8_t decA = sdk.callViewFunction(askAddr, &ERC20::decimals);
      uint8_t decB = sdk.callViewFunction(bidAddr, &ERC20::decimals);
      // create the contract
      Address orderBook = sdk.deployContract<OrderBook>(askAddr, std::string("A_Token"), decA,
                                                        bidAddr, std::string("B_Token"), decB);
      // approve orderbook to transfer the tokens
      sdk.callFunction(askAddr, &ERC20::approve, orderBook, uint256_t("2000000000000000000"));
      sdk.callFunction(bidAddr, &ERC20::approve, orderBook, uint256_t("2000000000000000000"));
      // add bid order
      sdk.callFunction(orderBook, &OrderBook::addBidLimitOrder, uint256_t("100"), uint256_t("10"));
      sdk.callFunction(orderBook, &OrderBook::addBidLimitOrder, uint256_t("100"), uint256_t("10"));
      sdk.callFunction(orderBook, &OrderBook::addBidLimitOrder, uint256_t("100"), uint256_t("10"));
      // get bids
      auto bids = sdk.callViewFunction(orderBook, &OrderBook::getBids);
      // verify the number of bid orders
      REQUIRE(bids.size() == 3);
    }

    SECTION("Orderbook add ask limit order") {
      SDKTestSuite sdk = SDKTestSuite::createNewEnvironment(testDumpPath + "/testOrderBookCreation");
      Address owner = sdk.getChainOwnerAccount().address;
      Address askAddr = sdk.deployContract<ERC20>(std::string("A_Token"), std::string("TKN_A"), uint8_t(18), uint256_t("2000000000000000000"));
      Address bidAddr = sdk.deployContract<ERC20>(std::string("B_Token"), std::string("TKN_B"), uint8_t(18), uint256_t("2000000000000000000"));
      uint8_t decA = sdk.callViewFunction(askAddr, &ERC20::decimals);
      uint8_t decB = sdk.callViewFunction(bidAddr, &ERC20::decimals);
      // create the contract
      Address orderBook = sdk.deployContract<OrderBook>(askAddr, std::string("A_Token"), decA,
                                                        bidAddr, std::string("B_Token"), decB);
      // approve orderbook to transfer the tokens
      sdk.callFunction(askAddr, &ERC20::approve, orderBook, uint256_t("2000000000000000000"));
      sdk.callFunction(bidAddr, &ERC20::approve, orderBook, uint256_t("2000000000000000000"));
      // add bid order
      sdk.callFunction(orderBook, &OrderBook::addAskLimitOrder, uint256_t("100"), uint256_t("10"));
      sdk.callFunction(orderBook, &OrderBook::addAskLimitOrder, uint256_t("100"), uint256_t("10"));
      sdk.callFunction(orderBook, &OrderBook::addAskLimitOrder, uint256_t("100"), uint256_t("10"));
      // get asks
      auto asks = sdk.callViewFunction(orderBook, &OrderBook::getAsks);
      // verify the number of bid orders
      REQUIRE(asks.size() == 3);
    }

    SECTION("Orderbook add bid and ask order limit to match a transaction") {
      SDKTestSuite sdk = SDKTestSuite::createNewEnvironment(testDumpPath + "/testOrderBookCreation");
      Address owner = sdk.getChainOwnerAccount().address;
      Address askAddr = sdk.deployContract<ERC20>(std::string("A_Token"), std::string("TKN_A"), uint8_t(18), uint256_t("2000000000000000000"));
      Address bidAddr = sdk.deployContract<ERC20>(std::string("B_Token"), std::string("TKN_B"), uint8_t(18), uint256_t("2000000000000000000"));
      uint8_t decA = sdk.callViewFunction(askAddr, &ERC20::decimals);
      uint8_t decB = sdk.callViewFunction(bidAddr, &ERC20::decimals);
      // create the contract
      Address orderBook = sdk.deployContract<OrderBook>(askAddr, std::string("A_Token"), decA,
                                                        bidAddr, std::string("B_Token"), decB);
      // approve orderbook to transfer the tokens
      sdk.callFunction(askAddr, &ERC20::approve, orderBook, uint256_t("2000000000000000000"));
      sdk.callFunction(bidAddr, &ERC20::approve, orderBook, uint256_t("2000000000000000000"));
      // add bid order
      sdk.callFunction(orderBook, &OrderBook::addBidLimitOrder, uint256_t("100"), uint256_t("10"));
      sdk.callFunction(orderBook, &OrderBook::addAskLimitOrder, uint256_t("100"), uint256_t("10"));
      // get asks and bids
      auto asks = sdk.callViewFunction(orderBook, &OrderBook::getAsks);
      auto bids = sdk.callViewFunction(orderBook, &OrderBook::getBids);
      // verify the number of bid orders
      REQUIRE(asks.size() == 0);
      REQUIRE(bids.size() == 0);
    }

    SECTION("Orderbook add ask and bid limit order to match a transaction") {
      SDKTestSuite sdk = SDKTestSuite::createNewEnvironment(testDumpPath + "/testOrderBookCreation");
      Address owner = sdk.getChainOwnerAccount().address;
      Address askAddr = sdk.deployContract<ERC20>(std::string("A_Token"), std::string("TKN_A"), uint8_t(18), uint256_t("2000000000000000000"));
      Address bidAddr = sdk.deployContract<ERC20>(std::string("B_Token"), std::string("TKN_B"), uint8_t(18), uint256_t("2000000000000000000"));
      uint8_t decA = sdk.callViewFunction(askAddr, &ERC20::decimals);
      uint8_t decB = sdk.callViewFunction(bidAddr, &ERC20::decimals);
      // create the contract
      Address orderBook = sdk.deployContract<OrderBook>(askAddr, std::string("A_Token"), decA,
                                                        bidAddr, std::string("B_Token"), decB);
      // approve orderbook to transfer the tokens
      sdk.callFunction(askAddr, &ERC20::approve, orderBook, uint256_t("2000000000000000000"));
      sdk.callFunction(bidAddr, &ERC20::approve, orderBook, uint256_t("2000000000000000000"));
      // add bid order
      sdk.callFunction(orderBook, &OrderBook::addAskLimitOrder, uint256_t("100"), uint256_t("10"));
      auto askAddrBalance = sdk.callViewFunction(askAddr, &ERC20::balanceOf, owner);
      // get bids
      auto asks = sdk.callViewFunction(orderBook, &OrderBook::getAsks);
      // verify the number of bid orders
      REQUIRE(asks.size() == 1);
      // get ask id
      uint256_t id = std::get<0>(*(asks.cbegin()));
      // delete bid order
      sdk.callFunction(orderBook, &OrderBook::delAskLimitOrder, id);
      sdk.callFunction(orderBook, &OrderBook::addBidLimitOrder, uint256_t("2000"), uint256_t("10000"));
      // verify balance
      auto bidAddrBalance = sdk.callViewFunction(bidAddr, &ERC20::balanceOf, owner);
      // add ask limit order again
      sdk.callFunction(orderBook, &OrderBook::addAskLimitOrder, uint256_t("100"), uint256_t("10"));
      // get asks and bids
      asks = sdk.callViewFunction(orderBook, &OrderBook::getAsks);
      auto bids = sdk.callViewFunction(orderBook, &OrderBook::getBids);
      // verify the number of bid orders
      REQUIRE(asks.size() == 0);
      REQUIRE(bids.size() == 1);
    }

    SECTION("Orderbook delete bid limit order") {
      SDKTestSuite sdk = SDKTestSuite::createNewEnvironment(testDumpPath + "/testOrderBookCreation");
      Address owner = sdk.getChainOwnerAccount().address;
      Address askAddr = sdk.deployContract<ERC20>(std::string("A_Token"), std::string("TKN_A"), uint8_t(18), uint256_t("2000000000000000000"));
      Address bidAddr = sdk.deployContract<ERC20>(std::string("B_Token"), std::string("TKN_B"), uint8_t(18), uint256_t("2000000000000000000"));
      uint8_t decA = sdk.callViewFunction(askAddr, &ERC20::decimals);
      uint8_t decB = sdk.callViewFunction(bidAddr, &ERC20::decimals);
      // create the contract
      Address orderBook = sdk.deployContract<OrderBook>(askAddr, std::string("A_Token"), decA,
                                                        bidAddr, std::string("B_Token"), decB);
      // approve orderbook to transfer the tokens
      sdk.callFunction(bidAddr, &ERC20::approve, orderBook, uint256_t("2000000000000000000"));
      // add bid order
      sdk.callFunction(orderBook, &OrderBook::addBidLimitOrder, uint256_t("100"), uint256_t("10"));
      // get bids
      auto bids = sdk.callViewFunction(orderBook, &OrderBook::getBids);
      // verify the number of bid orders
      REQUIRE(bids.size() == 1);
      // get bid id
      uint256_t id = std::get<0>(*(bids.cbegin()));
      // delete bid order
      sdk.callFunction(orderBook, &OrderBook::delBidLimitOrder, id);
      // verify the number of bid orders
      REQUIRE(sdk.callViewFunction(orderBook, &OrderBook::getBids).size() == 0);
    }

    SECTION("Orderbook delete ask limit order") {
      SDKTestSuite sdk = SDKTestSuite::createNewEnvironment(testDumpPath + "/testOrderBookCreation");
      Address owner = sdk.getChainOwnerAccount().address;
      Address askAddr = sdk.deployContract<ERC20>(std::string("A_Token"), std::string("TKN_A"), uint8_t(18), uint256_t("2000000000000000000"));
      Address bidAddr = sdk.deployContract<ERC20>(std::string("B_Token"), std::string("TKN_B"), uint8_t(18), uint256_t("2000000000000000000"));
      uint8_t decA = sdk.callViewFunction(askAddr, &ERC20::decimals);
      uint8_t decB = sdk.callViewFunction(bidAddr, &ERC20::decimals);
      // create the contract
      Address orderBook = sdk.deployContract<OrderBook>(askAddr, std::string("A_Token"), decA,
                                                        bidAddr, std::string("B_Token"), decB);
      // approve orderbook to transfer the tokens
      sdk.callFunction(askAddr, &ERC20::approve, orderBook, uint256_t("2000000000000000000"));
      // add bid order
      sdk.callFunction(orderBook, &OrderBook::addAskLimitOrder, uint256_t("100"), uint256_t("10"));
      // get bids
      auto asks = sdk.callViewFunction(orderBook, &OrderBook::getAsks);
      // verify the number of bid orders
      REQUIRE(asks.size() == 1);
      // get ask id
      uint256_t id = std::get<0>(*(asks.cbegin()));
      // delete bid order
      sdk.callFunction(orderBook, &OrderBook::delAskLimitOrder, id);
      // verify the number of bid orders
      REQUIRE(sdk.callViewFunction(orderBook, &OrderBook::getAsks).size() == 0);
    }

    SECTION("Orderbook add market order") {
      SDKTestSuite sdk = SDKTestSuite::createNewEnvironment(testDumpPath + "/testOrderBookCreation");
      Address owner = sdk.getChainOwnerAccount().address;
      Address askAddr = sdk.deployContract<ERC20>(std::string("A_Token"), std::string("TKN_A"), uint8_t(18), uint256_t("2000000000000000000"));
      Address bidAddr = sdk.deployContract<ERC20>(std::string("B_Token"), std::string("TKN_B"), uint8_t(18), uint256_t("2000000000000000000"));
      uint8_t decA = sdk.callViewFunction(askAddr, &ERC20::decimals);
      uint8_t decB = sdk.callViewFunction(bidAddr, &ERC20::decimals);
      // create the contract
      Address orderBook = sdk.deployContract<OrderBook>(askAddr, std::string("A_Token"), decA,
                                                        bidAddr, std::string("B_Token"), decB);
      // approve orderbook to transfer the tokens
      sdk.callFunction(askAddr, &ERC20::approve, orderBook, uint256_t("2000000000000000000"));
      sdk.callFunction(bidAddr, &ERC20::approve, orderBook, uint256_t("2000000000000000000"));
      // add ask market order
      sdk.callFunction(orderBook, &OrderBook::addAskMarketOrder, uint256_t("100"), uint256_t("10"));
      // get asks orders
      auto asks = sdk.callViewFunction(orderBook, &OrderBook::getAsks);
      // verify the number of ask orders
      REQUIRE(asks.size() == 0);
    }

    SECTION("Orderbook add bid market order") {
      SDKTestSuite sdk = SDKTestSuite::createNewEnvironment(testDumpPath + "/testOrderBookCreation");
      Address owner = sdk.getChainOwnerAccount().address;
      Address askAddr = sdk.deployContract<ERC20>(std::string("A_Token"), std::string("TKN_A"), uint8_t(18), uint256_t("2000000000000000000"));
      Address bidAddr = sdk.deployContract<ERC20>(std::string("B_Token"), std::string("TKN_B"), uint8_t(18), uint256_t("2000000000000000000"));
      uint8_t decA = sdk.callViewFunction(askAddr, &ERC20::decimals);
      uint8_t decB = sdk.callViewFunction(bidAddr, &ERC20::decimals);
      // create the contract
      Address orderBook = sdk.deployContract<OrderBook>(askAddr, std::string("A_Token"), decA,
                                                        bidAddr, std::string("B_Token"), decB);
      // approve orderbook to transfer the tokens
      sdk.callFunction(askAddr, &ERC20::approve, orderBook, uint256_t("2000000000000000000"));
      sdk.callFunction(bidAddr, &ERC20::approve, orderBook, uint256_t("2000000000000000000"));
      // add bid market order
      sdk.callFunction(orderBook, &OrderBook::addBidMarketOrder, uint256_t("100"), uint256_t("10"));
      // get bids orders
      auto bids = sdk.callViewFunction(orderBook, &OrderBook::getBids);
      // verify the number of bid market orders
      REQUIRE(bids.size() == 0);
    }

    SECTION("Orderbook add ask limit order and bid market order to match a transaction") {
      SDKTestSuite sdk = SDKTestSuite::createNewEnvironment(testDumpPath + "/testOrderBookCreation");
      Address owner = sdk.getChainOwnerAccount().address;
      Address askAddr = sdk.deployContract<ERC20>(std::string("A_Token"), std::string("TKN_A"), uint8_t(18), uint256_t("2000000000000000000"));
      Address bidAddr = sdk.deployContract<ERC20>(std::string("B_Token"), std::string("TKN_B"), uint8_t(18), uint256_t("2000000000000000000"));
      uint8_t decA = sdk.callViewFunction(askAddr, &ERC20::decimals);
      uint8_t decB = sdk.callViewFunction(bidAddr, &ERC20::decimals);
      // create the contract
      Address orderBook = sdk.deployContract<OrderBook>(askAddr, std::string("A_Token"), decA,
                                                        bidAddr, std::string("B_Token"), decB);
      // approve orderbook to transfer the tokens
      sdk.callFunction(askAddr, &ERC20::approve, orderBook, uint256_t("2000000000000000000"));
      sdk.callFunction(bidAddr, &ERC20::approve, orderBook, uint256_t("2000000000000000000"));
      // add bid order
      sdk.callFunction(orderBook, &OrderBook::addAskLimitOrder, uint256_t("100"), uint256_t("20"));
      sdk.callFunction(orderBook, &OrderBook::addBidMarketOrder, uint256_t("100"), uint256_t("10"));
      // get asks and bids
      auto asks = sdk.callViewFunction(orderBook, &OrderBook::getAsks);
      auto bids = sdk.callViewFunction(orderBook, &OrderBook::getBids);
      // verify the number of bid orders
      REQUIRE(asks.size() == 0);
      REQUIRE(bids.size() == 0);
    }

    SECTION("Orderbook add bid limit order and ask market order to match a transaction") {
      SDKTestSuite sdk = SDKTestSuite::createNewEnvironment(testDumpPath + "/testOrderBookCreation");
      Address owner = sdk.getChainOwnerAccount().address;
      Address askAddr = sdk.deployContract<ERC20>(std::string("A_Token"), std::string("TKN_A"), uint8_t(18), uint256_t("2000000000000000000"));
      Address bidAddr = sdk.deployContract<ERC20>(std::string("B_Token"), std::string("TKN_B"), uint8_t(18), uint256_t("2000000000000000000"));
      uint8_t decA = sdk.callViewFunction(askAddr, &ERC20::decimals);
      uint8_t decB = sdk.callViewFunction(bidAddr, &ERC20::decimals);
      // create the contract
      Address orderBook = sdk.deployContract<OrderBook>(askAddr, std::string("A_Token"), decA,
                                                        bidAddr, std::string("B_Token"), decB);
      // approve orderbook to transfer the tokens
      sdk.callFunction(askAddr, &ERC20::approve, orderBook, uint256_t("2000000000000000000"));
      sdk.callFunction(bidAddr, &ERC20::approve, orderBook, uint256_t("2000000000000000000"));
      // add bid order
      sdk.callFunction(orderBook, &OrderBook::addBidLimitOrder, uint256_t("50"), uint256_t("20"));
      sdk.callFunction(orderBook, &OrderBook::addBidLimitOrder, uint256_t("50"), uint256_t("20"));
      sdk.callFunction(orderBook, &OrderBook::addBidLimitOrder, uint256_t("100"), uint256_t("30"));
      sdk.callFunction(orderBook, &OrderBook::addAskMarketOrder, uint256_t("100"), uint256_t("10"));
      sdk.callFunction(orderBook, &OrderBook::addAskMarketOrder, uint256_t("100"), uint256_t("10"));
      // get asks and bids
      auto asks = sdk.callViewFunction(orderBook, &OrderBook::getAsks);
      auto bids = sdk.callViewFunction(orderBook, &OrderBook::getBids);
      // verify the number of bid orders
      REQUIRE(asks.size() == 0);
      REQUIRE(bids.size() == 0);
    }
  }
}
