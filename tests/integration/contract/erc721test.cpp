/*
  Copyright (c) [2023-2024] [AppLayer Developers]
  This software is distributed under the MIT License.
  See the LICENSE.txt file in the project root for more information.
*/

#include "libs/catch2/catch_amalgamated.hpp"

#include "contract/templates/erc721test.h"

#include "bytes/hex.h"

#include "../../sdktestsuite.hpp"

namespace TERC721Test {
  TEST_CASE("ERC721Test Class", "[integration][contract][erc721test]") {
    SECTION("ERC721Test Creation + Dump") {
      Address ERC721Address;
      std::unique_ptr<Options> options;
      {
        SDKTestSuite sdk = SDKTestSuite::createNewEnvironment("testERC721TestCreation");
        ERC721Address = sdk.deployContract<ERC721Test>(std::string("My Test NFT!"), std::string("NFT"), uint64_t(100));
        REQUIRE(sdk.callViewFunction(ERC721Address, &ERC721Test::name) == "My Test NFT!");
        REQUIRE(sdk.callViewFunction(ERC721Address, &ERC721Test::symbol) == "NFT");
        REQUIRE(sdk.callViewFunction(ERC721Address, &ERC721Test::maxTokens) == 100);
        REQUIRE(sdk.callViewFunction(ERC721Address, &ERC721Test::tokenIdCounter) == 0);
        // Dump to database
        options = std::make_unique<Options>(sdk.getOptions());
        sdk.saveSnapshot();
      }

      // SDKTestSuite should automatically load the state from the DB if we construct it with an Options object
      // (The createNewEnvironment DELETES the DB if any is found)
      SDKTestSuite sdk(*options);
      REQUIRE(sdk.callViewFunction(ERC721Address, &ERC721Test::name) == "My Test NFT!");
      REQUIRE(sdk.callViewFunction(ERC721Address, &ERC721Test::symbol) == "NFT");
      REQUIRE(sdk.callViewFunction(ERC721Address, &ERC721Test::maxTokens) == 100);
      REQUIRE(sdk.callViewFunction(ERC721Address, &ERC721Test::tokenIdCounter) == 0);
    }

    SECTION("ERC721Test 1 Token (Mint + Dump + Burn + Transfer)") {
      Address ERC721Address;
      std::unique_ptr<Options> options;
      {
        SDKTestSuite sdk = SDKTestSuite::createNewEnvironment("testERC721TestOneToken");
        ERC721Address = sdk.deployContract<ERC721Test>(std::string("My Test NFT!"), std::string("NFT"), uint64_t(100));
        // Mint exactly one token for the chain owner
        auto mintTx = sdk.callFunction(ERC721Address, &ERC721Test::mint, sdk.getChainOwnerAccount().address);
        auto mintEvents = sdk.getEventsEmittedByTx(mintTx, &ERC721Test::Transfer);
        REQUIRE(mintEvents.size() == 1);
        REQUIRE(std::get<0>(ABI::Decoder::decodeData<Address>(mintEvents[0].getTopics()[1].asBytes())) == Address());
        REQUIRE(std::get<0>(ABI::Decoder::decodeData<Address>(mintEvents[0].getTopics()[2].asBytes())) == sdk.getChainOwnerAccount().address);
        REQUIRE(std::get<0>(ABI::Decoder::decodeData<uint256_t>(mintEvents[0].getTopics()[3].asBytes())) == uint256_t(0));
        // Confirm that token is minted and owned by the chain owner
        auto owner = sdk.callViewFunction(ERC721Address, &ERC721Test::ownerOf, uint256_t(0));
        REQUIRE(owner == sdk.getChainOwnerAccount().address);
        // Dump to database
        options = std::make_unique<Options>(sdk.getOptions());
        sdk.saveSnapshot();
      }

      // SDKTestSuite should automatically load the state from the DB if we construct it with an Options object
      // (The createNewEnvironment DELETES the DB if any is found)
      SDKTestSuite sdk(*options);
      auto owner = sdk.callViewFunction(ERC721Address, &ERC721Test::ownerOf, uint256_t(0));
      REQUIRE(owner == sdk.getChainOwnerAccount().address);
      REQUIRE(sdk.callViewFunction(ERC721Address, &ERC721Test::balanceOf, sdk.getChainOwnerAccount().address) == 1);
      REQUIRE(sdk.callViewFunction(ERC721Address, &ERC721Test::totalSupply) == 1);

      // For coverage
      // Try minting to zero address
      REQUIRE_THROWS(sdk.callFunction(ERC721Address, &ERC721Test::mint, Address()));

      // Try transferring to zero address and from wrong owner
      Address add1(bytes::hex("0x1234567890123456789012345678901234567890"));
      Address add2(bytes::hex("0x0987654321098765432109876543210987654321"));
      REQUIRE_THROWS(sdk.callFunction(ERC721Address, &ERC721Test::transferFrom, sdk.getChainOwnerAccount().address, Address(), uint256_t(0)));
      REQUIRE_THROWS(sdk.callFunction(ERC721Address, &ERC721Test::transferFrom, add1, add2, uint256_t(0)));

      // Burn the token and try to burn it again then transfer it
      REQUIRE_NOTHROW(sdk.callFunction(ERC721Address, &ERC721Test::burn, uint256_t(0)));
      REQUIRE_THROWS(sdk.callFunction(ERC721Address, &ERC721Test::burn, uint256_t(0))); // Already burnt
      REQUIRE_THROWS(sdk.callFunction(ERC721Address, &ERC721Test::transferFrom, sdk.getChainOwnerAccount().address, add1, uint256_t(0)));
    }

    SECTION("ERC721Test Mint 100 Token Same Address") {
      SDKTestSuite sdk = SDKTestSuite::createNewEnvironment("testERC721TestMint100TokenSameAddress");
      auto ERC721Address = sdk.deployContract<ERC721Test>(std::string("My Test NFT!"), std::string("NFT"), uint64_t(100));
      for (uint64_t i = 0; i < 100; ++i) {
        auto mintTx = sdk.callFunction(ERC721Address, &ERC721Test::mint, sdk.getChainOwnerAccount().address);
        auto mintEvents = sdk.getEventsEmittedByTx(mintTx, &ERC721Test::Transfer);
        REQUIRE(mintEvents.size() == 1);
        REQUIRE(std::get<0>(ABI::Decoder::decodeData<Address>(mintEvents[0].getTopics()[1].asBytes())) == Address());
        REQUIRE(std::get<0>(ABI::Decoder::decodeData<Address>(mintEvents[0].getTopics()[2].asBytes())) == sdk.getChainOwnerAccount().address);
        REQUIRE(std::get<0>(ABI::Decoder::decodeData<uint256_t>(mintEvents[0].getTopics()[3].asBytes())) == uint256_t(i));
      }
      // Check if token Id 0...99 is minted and owned by the chain owner
      for (uint64_t i = 0; i < 100; ++i) {
        auto owner = sdk.callViewFunction(ERC721Address, &ERC721Test::ownerOf, uint256_t(i));
        REQUIRE(owner == sdk.getChainOwnerAccount().address);
      }
      REQUIRE(sdk.callViewFunction(ERC721Address, &ERC721Test::balanceOf, sdk.getChainOwnerAccount().address) == 100);
      REQUIRE(sdk.callViewFunction(ERC721Address, &ERC721Test::totalSupply) == 100);

      // Try minting a 101th time (for coverage)
      REQUIRE_THROWS(sdk.callFunction(ERC721Address, &ERC721Test::mint, sdk.getChainOwnerAccount().address));
    }

    SECTION("ERC721Test Mint 100 Different Addresses") {
      // Generate 100 different TestAccounts
      std::vector<TestAccount> accounts;
      for (int i = 0; i < 100; ++i) {
        accounts.emplace_back(TestAccount::newRandomAccount());
      }
      SDKTestSuite sdk = SDKTestSuite::createNewEnvironment("testERC721TestMint100DifferentAddresses", accounts);
      auto ERC721Address = sdk.deployContract<ERC721Test>(std::string("My Test NFT!"), std::string("NFT"), uint64_t(100));
      for (int i = 0; i < 100; ++i) {
        auto mintTx = sdk.callFunction(ERC721Address, &ERC721Test::mint, accounts[i].address);
        auto mintEvents = sdk.getEventsEmittedByTx(mintTx, &ERC721Test::Transfer);
        REQUIRE(mintEvents.size() == 1);
        REQUIRE(std::get<0>(ABI::Decoder::decodeData<Address>(mintEvents[0].getTopics()[1].asBytes())) == Address());
        REQUIRE(std::get<0>(ABI::Decoder::decodeData<Address>(mintEvents[0].getTopics()[2].asBytes())) == accounts[i].address);
        REQUIRE(std::get<0>(ABI::Decoder::decodeData<uint256_t>(mintEvents[0].getTopics()[3].asBytes())) == uint256_t(i));
      }
      // Check if token Id 0...99 is minted and owned by each respective account
      for (int i = 0; i < 100; ++i) {
        REQUIRE(sdk.callViewFunction(ERC721Address, &ERC721Test::ownerOf, uint256_t(i)) == accounts[i].address);
        REQUIRE(sdk.callViewFunction(ERC721Address, &ERC721Test::balanceOf, accounts[i].address) == 1);
      }
      REQUIRE(sdk.callViewFunction(ERC721Address, &ERC721Test::totalSupply) == 100);
    }

    SECTION("ERC721Test Mint 100 Different Addresses reverse") {
      // Same as before, but we mint from the test accounts list in reverse order
      std::vector<TestAccount> accounts;
      for (int i = 0; i < 100; ++i) {
        accounts.emplace_back(TestAccount::newRandomAccount());
      }
      SDKTestSuite sdk = SDKTestSuite::createNewEnvironment("testERC721TestMint100DifferentAddressesReverse", accounts);
      auto ERC721Address = sdk.deployContract<ERC721Test>(std::string("My Test NFT!"), std::string("NFT"), uint64_t(100));
      for (int i = 99; i >= 0; i--) {
        auto mintTx = sdk.callFunction(ERC721Address, &ERC721Test::mint, accounts[i].address);
        auto mintEvents = sdk.getEventsEmittedByTx(mintTx, &ERC721Test::Transfer);
        REQUIRE(mintEvents.size() == 1);
        REQUIRE(std::get<0>(ABI::Decoder::decodeData<Address>(mintEvents[0].getTopics()[1].asBytes())) == Address());
        REQUIRE(std::get<0>(ABI::Decoder::decodeData<Address>(mintEvents[0].getTopics()[2].asBytes())) == accounts[i].address);
        REQUIRE(std::get<0>(ABI::Decoder::decodeData<uint256_t>(mintEvents[0].getTopics()[3].asBytes())) == uint256_t(99 - i));
      }
      // Check if token Id 0...99 is minted and owned by each respective account
      uint64_t tokenId = 0;
      for (int i = 99; i >= 0; i--) {
        REQUIRE(sdk.callViewFunction(ERC721Address, &ERC721Test::ownerOf, uint256_t(tokenId++)) == accounts[i].address);
        REQUIRE(sdk.callViewFunction(ERC721Address, &ERC721Test::balanceOf, accounts[i].address) == 1);
      }
      REQUIRE(sdk.callViewFunction(ERC721Address, &ERC721Test::totalSupply) == 100);
    }

    SECTION("ERC721Test Mint 100 and Burn 100 Same Address") {
      SDKTestSuite sdk = SDKTestSuite::createNewEnvironment("testERC721TestMint100AndBurn100SameAddress");
      auto ERC721Address = sdk.deployContract<ERC721Test>(std::string("My Test NFT!"), std::string("NFT"), uint64_t(100));
      for (uint64_t i = 0; i < 100; ++i) {
        auto mintTx = sdk.callFunction(ERC721Address, &ERC721Test::mint, sdk.getChainOwnerAccount().address);
        auto mintEvents = sdk.getEventsEmittedByTx(mintTx, &ERC721Test::Transfer);
        REQUIRE(mintEvents.size() == 1);
        REQUIRE(std::get<0>(ABI::Decoder::decodeData<Address>(mintEvents[0].getTopics()[1].asBytes())) == Address());
        REQUIRE(std::get<0>(ABI::Decoder::decodeData<Address>(mintEvents[0].getTopics()[2].asBytes())) == sdk.getChainOwnerAccount().address);
        REQUIRE(std::get<0>(ABI::Decoder::decodeData<uint256_t>(mintEvents[0].getTopics()[3].asBytes())) == uint256_t(i));
      }
      // Check if token Id 0...99 is minted and owned by the chain owner
      for (uint64_t i = 0; i < 100; ++i) {
        REQUIRE(sdk.callViewFunction(ERC721Address, &ERC721Test::ownerOf, uint256_t(i)) == sdk.getChainOwnerAccount().address);
        REQUIRE(sdk.callViewFunction(ERC721Address, &ERC721Test::balanceOf, sdk.getChainOwnerAccount().address) == 100);
      }
      REQUIRE(sdk.callViewFunction(ERC721Address, &ERC721Test::totalSupply) == 100);
      for (uint64_t i = 0; i < 100; ++i) {
        auto burnTx = sdk.callFunction(ERC721Address, &ERC721Test::burn, uint256_t(i));
        auto burnEvents = sdk.getEventsEmittedByTx(burnTx, &ERC721Test::Transfer);
        REQUIRE(burnEvents.size() == 1);
        REQUIRE(std::get<0>(ABI::Decoder::decodeData<Address>(burnEvents[0].getTopics()[1].asBytes())) == sdk.getChainOwnerAccount().address);
        REQUIRE(std::get<0>(ABI::Decoder::decodeData<Address>(burnEvents[0].getTopics()[2].asBytes())) == Address());
        REQUIRE(std::get<0>(ABI::Decoder::decodeData<uint256_t>(burnEvents[0].getTopics()[3].asBytes())) == uint256_t(i));
      }
      REQUIRE(sdk.callViewFunction(ERC721Address, &ERC721Test::totalSupply) == 0);
      // Make sure that ownerOf throws (token does not exist)
      for (uint64_t i = 0; i < 100; ++i) {
        REQUIRE_THROWS(sdk.callViewFunction(ERC721Address, &ERC721Test::ownerOf, uint256_t(i)));
      }
      REQUIRE(sdk.callViewFunction(ERC721Address, &ERC721Test::balanceOf, sdk.getChainOwnerAccount().address) == 0);
    }

    SECTION("ERC721Test Mint 100 Different Address Burn 100 Different Address") {
      // Generate 100 different TestAccounts
      std::vector<TestAccount> accounts;
      for (int i = 0; i < 100; ++i) {
        accounts.emplace_back(TestAccount::newRandomAccount());
      }
      SDKTestSuite sdk = SDKTestSuite::createNewEnvironment("testERC721TestMint100DifferentAddressBurn100DifferentAddress", accounts);
      auto ERC721Address = sdk.deployContract<ERC721Test>(std::string("My Test NFT!"), std::string("NFT"), uint64_t(100));
      for (int i = 0; i < 100; ++i) {
        auto mintTx = sdk.callFunction(ERC721Address, &ERC721Test::mint, accounts[i].address);
        auto mintEvents = sdk.getEventsEmittedByTx(mintTx, &ERC721Test::Transfer);
        REQUIRE(mintEvents.size() == 1);
        REQUIRE(std::get<0>(ABI::Decoder::decodeData<Address>(mintEvents[0].getTopics()[1].asBytes())) == Address());
        REQUIRE(std::get<0>(ABI::Decoder::decodeData<Address>(mintEvents[0].getTopics()[2].asBytes())) == accounts[i].address);
        REQUIRE(std::get<0>(ABI::Decoder::decodeData<uint256_t>(mintEvents[0].getTopics()[3].asBytes())) == uint256_t(i));
      }
      // Check if token Id 0...99 is minted and owned by each respective account
      for (int i = 0; i < 100; ++i) {
        REQUIRE(sdk.callViewFunction(ERC721Address, &ERC721Test::ownerOf, uint256_t(i)) == accounts[i].address);
        REQUIRE(sdk.callViewFunction(ERC721Address, &ERC721Test::balanceOf, accounts[i].address) == 1);
      }
      REQUIRE(sdk.callViewFunction(ERC721Address, &ERC721Test::totalSupply) == 100);
      for (int i = 0; i < 100; ++i) {
        auto burnTx = sdk.callFunction(ERC721Address, accounts[i], &ERC721Test::burn, uint256_t(i));
        auto burnEvents = sdk.getEventsEmittedByTx(burnTx, &ERC721Test::Transfer);
        REQUIRE(burnEvents.size() == 1);
        REQUIRE(std::get<0>(ABI::Decoder::decodeData<Address>(burnEvents[0].getTopics()[1].asBytes())) == accounts[i].address);
        REQUIRE(std::get<0>(ABI::Decoder::decodeData<Address>(burnEvents[0].getTopics()[2].asBytes())) == Address());
        REQUIRE(std::get<0>(ABI::Decoder::decodeData<uint256_t>(burnEvents[0].getTopics()[3].asBytes())) == uint256_t(i));
      }
      REQUIRE(sdk.callViewFunction(ERC721Address, &ERC721Test::totalSupply) == 0);
      // Make sure that ownerOf throws (token does not exist)
      for (uint64_t i = 0; i < 100; ++i) {
        REQUIRE_THROWS(sdk.callViewFunction(ERC721Address, &ERC721Test::ownerOf, uint256_t(i)));
        REQUIRE(sdk.callViewFunction(ERC721Address, &ERC721Test::balanceOf, accounts[i].address) == 0);
      }
    }

    SECTION("ERC721Test Mint 100 Different Address Burn With Allowance") {
      // Generate 100 different TestAccounts
      std::vector<TestAccount> accounts;
      for (int i = 0; i < 100; ++i) {
        accounts.emplace_back(TestAccount::newRandomAccount());
      }
      SDKTestSuite sdk = SDKTestSuite::createNewEnvironment("testERC721TestMint100DifferentAddressBurnWithAllowance", accounts);
      auto ERC721Address = sdk.deployContract<ERC721Test>(std::string("My Test NFT!"), std::string("NFT"), uint64_t(100));
      for (int i = 0; i < 100; ++i) {
        auto mintTx = sdk.callFunction(ERC721Address, &ERC721Test::mint, accounts[i].address);
        auto mintEvents = sdk.getEventsEmittedByTx(mintTx, &ERC721Test::Transfer);
        REQUIRE(mintEvents.size() == 1);
        REQUIRE(std::get<0>(ABI::Decoder::decodeData<Address>(mintEvents[0].getTopics()[1].asBytes())) == Address());
        REQUIRE(std::get<0>(ABI::Decoder::decodeData<Address>(mintEvents[0].getTopics()[2].asBytes())) == accounts[i].address);
        REQUIRE(std::get<0>(ABI::Decoder::decodeData<uint256_t>(mintEvents[0].getTopics()[3].asBytes())) == uint256_t(i));
      }
      // Check if token Id 0...99 is minted and owned by each respective account
      for (int i = 0; i < 100; ++i) {
        auto owner = sdk.callViewFunction(ERC721Address, &ERC721Test::ownerOf, uint256_t(i));
        REQUIRE(owner == accounts[i].address);
        REQUIRE(sdk.callViewFunction(ERC721Address, &ERC721Test::balanceOf, accounts[i].address) == 1);
      }
      REQUIRE(sdk.callViewFunction(ERC721Address, &ERC721Test::totalSupply) == 100);
      // REQUIRE THROW trying to burn without allowance
      for (int i = 0; i < 100; ++i) {
        REQUIRE_THROWS(sdk.callFunction(ERC721Address, &ERC721Test::burn, uint256_t(i)));
      }
      // REQUIRE THROW trying to authorize using an account that is not the owner
      for (int i = 0; i < 100; ++i) {
        REQUIRE_THROWS(sdk.callFunction(ERC721Address, &ERC721Test::approve, sdk.getChainOwnerAccount().address, uint256_t(i)));
      }
      for (int i = 0; i < 100; ++i) {
        auto approveTx = sdk.callFunction(ERC721Address, accounts[i], &ERC721Test::approve, sdk.getChainOwnerAccount().address, uint256_t(i));
        auto approveEvents = sdk.getEventsEmittedByTx(approveTx, &ERC721Test::Approval);
        REQUIRE(approveEvents.size() == 1);
        REQUIRE(std::get<0>(ABI::Decoder::decodeData<Address>(approveEvents[0].getTopics()[1].asBytes())) == accounts[i].address);
        REQUIRE(std::get<0>(ABI::Decoder::decodeData<Address>(approveEvents[0].getTopics()[2].asBytes())) == sdk.getChainOwnerAccount().address);
        REQUIRE(std::get<0>(ABI::Decoder::decodeData<uint256_t>(approveEvents[0].getTopics()[3].asBytes())) == uint256_t(i));
      }
      // Check allowance
      for (int i = 0; i < 100; ++i) {
        REQUIRE(sdk.callViewFunction(ERC721Address, &ERC721Test::getApproved, uint256_t(i)) == sdk.getChainOwnerAccount().address);
      }
      for (int i = 0; i < 100; ++i) {
        auto burnTx = sdk.callFunction(ERC721Address, &ERC721Test::burn, uint256_t(i));
        auto burnEvents = sdk.getEventsEmittedByTx(burnTx, &ERC721Test::Transfer);
        REQUIRE(burnEvents.size() == 1);
        REQUIRE(std::get<0>(ABI::Decoder::decodeData<Address>(burnEvents[0].getTopics()[1].asBytes())) == accounts[i].address);
        REQUIRE(std::get<0>(ABI::Decoder::decodeData<Address>(burnEvents[0].getTopics()[2].asBytes())) == Address());
        REQUIRE(std::get<0>(ABI::Decoder::decodeData<uint256_t>(burnEvents[0].getTopics()[3].asBytes())) == uint256_t(i));
      }
      REQUIRE(sdk.callViewFunction(ERC721Address, &ERC721Test::totalSupply) == 0);
      // Make sure that ownerOf throws (token does not exist)
      for (uint64_t i = 0; i < 100; ++i) {
        REQUIRE_THROWS(sdk.callViewFunction(ERC721Address, &ERC721Test::ownerOf, uint256_t(i)));
        REQUIRE(sdk.callViewFunction(ERC721Address, &ERC721Test::balanceOf, accounts[i].address) == 0);
      }
      // Make sure allowance doesn't exists anymore
      for (int i = 0; i < 100; ++i) {
        REQUIRE_THROWS(sdk.callViewFunction(ERC721Address, &ERC721Test::getApproved, uint256_t(i)) == Address());
      }
    }

    SECTION("ERC721Test transferFrom with allowance from 100 different accounts") {
      // Generate 100 different TestAccounts
      std::vector<TestAccount> accounts;
      for (int i = 0; i < 100; ++i) {
        accounts.emplace_back(TestAccount::newRandomAccount());
      }
      TestAccount accountToSent = TestAccount::newRandomAccount();
      SDKTestSuite sdk = SDKTestSuite::createNewEnvironment("testERC721TestMint100DifferentAddressBurnWithAllowance", accounts);
      auto ERC721Address = sdk.deployContract<ERC721Test>(std::string("My Test NFT!"), std::string("NFT"), uint64_t(100));
      for (int i = 0; i < 100; ++i) {
        auto mintTx = sdk.callFunction(ERC721Address, &ERC721Test::mint, accounts[i].address);
        auto mintEvents = sdk.getEventsEmittedByTx(mintTx, &ERC721Test::Transfer);
        REQUIRE(mintEvents.size() == 1);
        REQUIRE(std::get<0>(ABI::Decoder::decodeData<Address>(mintEvents[0].getTopics()[1].asBytes())) == Address());
        REQUIRE(std::get<0>(ABI::Decoder::decodeData<Address>(mintEvents[0].getTopics()[2].asBytes())) == accounts[i].address);
        REQUIRE(std::get<0>(ABI::Decoder::decodeData<uint256_t>(mintEvents[0].getTopics()[3].asBytes())) == uint256_t(i));
      }
      // Check if token Id 0...99 is minted and owned by each respective account
      for (int i = 0; i < 100; ++i) {
        auto owner = sdk.callViewFunction(ERC721Address, &ERC721Test::ownerOf, uint256_t(i));
        REQUIRE(owner == accounts[i].address);
        REQUIRE(sdk.callViewFunction(ERC721Address, &ERC721Test::balanceOf, accounts[i].address) == 1);
      }
      REQUIRE(sdk.callViewFunction(ERC721Address, &ERC721Test::totalSupply) == 100);
      // REQUIRE THROW for transferFrom without allowance
      for (int i = 0; i < 100; ++i) {
        REQUIRE_THROWS(sdk.callFunction(ERC721Address, &ERC721Test::transferFrom, accounts[i].address, accountToSent.address, uint256_t(i)));
      }
      // Give allowance
      for (int i = 0; i < 100; ++i) {
        auto approveTx = sdk.callFunction(ERC721Address, accounts[i], &ERC721Test::approve, sdk.getChainOwnerAccount().address, uint256_t(i));
        auto approveEvents = sdk.getEventsEmittedByTx(approveTx, &ERC721Test::Approval);
        REQUIRE(approveEvents.size() == 1);
        REQUIRE(std::get<0>(ABI::Decoder::decodeData<Address>(approveEvents[0].getTopics()[1].asBytes())) == accounts[i].address);
        REQUIRE(std::get<0>(ABI::Decoder::decodeData<Address>(approveEvents[0].getTopics()[2].asBytes())) == sdk.getChainOwnerAccount().address);
        REQUIRE(std::get<0>(ABI::Decoder::decodeData<uint256_t>(approveEvents[0].getTopics()[3].asBytes())) == uint256_t(i));
      }
      // Check allowance
      for (int i = 0; i < 100; ++i) {
        REQUIRE(sdk.callViewFunction(ERC721Address, &ERC721Test::getApproved, uint256_t(i)) == sdk.getChainOwnerAccount().address);
      }
      // Transfer from
      for (int i = 0; i < 100; ++i) {
        auto transferTx = sdk.callFunction(ERC721Address, &ERC721Test::transferFrom, accounts[i].address, accountToSent.address, uint256_t(i));
        auto transferEvents = sdk.getEventsEmittedByTx(transferTx, &ERC721Test::Transfer);
        REQUIRE(transferEvents.size() == 1);
        REQUIRE(std::get<0>(ABI::Decoder::decodeData<Address>(transferEvents[0].getTopics()[1].asBytes())) == accounts[i].address);
        REQUIRE(std::get<0>(ABI::Decoder::decodeData<Address>(transferEvents[0].getTopics()[2].asBytes())) == accountToSent.address);
        REQUIRE(std::get<0>(ABI::Decoder::decodeData<uint256_t>(transferEvents[0].getTopics()[3].asBytes())) == uint256_t(i));
      }
      // Check if token id 0...99 is owned by accountToSent
      for (int i = 0; i < 100; ++i) {
        auto owner = sdk.callViewFunction(ERC721Address, &ERC721Test::ownerOf, uint256_t(i));
        REQUIRE(owner == accountToSent.address);
        REQUIRE(sdk.callViewFunction(ERC721Address, &ERC721Test::balanceOf, accountToSent.address) == 100);
      }
      REQUIRE(sdk.callViewFunction(ERC721Address, &ERC721Test::totalSupply) == 100);
      // Make sure that the allowance was removed af  ter the transfer
      for (int i = 0; i < 100; ++i) {
        REQUIRE(sdk.callViewFunction(ERC721Address, &ERC721Test::getApproved, uint256_t(i)) == Address());
      }
    }
  }
}
