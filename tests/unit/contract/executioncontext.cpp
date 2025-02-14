/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#include "libs/catch2/catch_amalgamated.hpp"

#include "contract/executioncontext.h"

#include "bytes/hex.h"

static inline void addAccount(ExecutionContext& context, View<Address> address, const Account& account) {
  auto pointer = context.getAccount(address);
  pointer.setBalance(account.balance);
  pointer.setNonce(account.nonce);
  pointer.setCode(account.code);
  pointer.setContractType(account.contractType);
}

TEST_CASE("Execution Context Test Cases", "[unit][contract][executioncontext]") {
  SECTION("Building correctly") {
    ExecutionContext::Accounts accounts;
    ExecutionContext::Storage storage;

    const Address accountAddress1 = bytes::hex("0xa29F7649159DBF66daaa6D03F9ed5733c85BDc27");
    const Address accountAddress2 = bytes::hex("0x87e42c3307c79334e4A22EF406BDe0A004D9c8C7");

    accounts.emplace(accountAddress1, Account(1000, 5));
    accounts.emplace(accountAddress2, Account(666, 3));

    const Hash slot1 = bytes::hex("0x0000000000000000000000000000000000000000000000000000000000000001");
    const Hash slot2 = bytes::hex("0x0000000000000000000000000000000000000000000000000000000000000002");
    const Hash data = bytes::hex("0xc89a747ae61fb49aeefadaa8d6fce73ab2f61b444a196c026d46cbe550b90b5b");

    storage.emplace(StorageKeyView(accountAddress1, slot1), data);

    const int64_t blockGasLimit = 5000;
    const int64_t blockNumber = 123;
    const int64_t blockTimestamp = 31987233180528;
    const int64_t txIndex = 2;
    const Address blockCoinbase = bytes::hex("0xe4d327487dd563e93dd09743d1aad131f89e1866");
    const Address txOrigin = bytes::hex("0x35b5758593b3da41a2cc5574d1c4a9aa7cc994f4");
    const Hash blockHash = bytes::hex("0x044475f2cb0876a477b9f7fb401162317ea6ae98c5a7fc84b84cda820c864541");
    const Hash txHash = bytes::hex("0xac7dbb9fd2bf03c58b61664bf453bf7760de39edd91cf812f4b8d49763d29a03");
    const uint256_t chainId = 45;
    const uint256_t txGasPrice = 1234753453245;

    ExecutionContext context = ExecutionContext::Builder()
      .storage(storage)
      .accounts(accounts)
      .blockHash(blockHash)
      .txHash(txHash)
      .txOrigin(txOrigin)
      .blockCoinbase(blockCoinbase)
      .txIndex(txIndex)
      .blockNumber(blockNumber)
      .blockTimestamp(blockTimestamp)
      .blockGasLimit(blockGasLimit)
      .txGasPrice(txGasPrice)
      .chainId(chainId)
      .build();

    REQUIRE(context.getBlockHash() == blockHash);
    REQUIRE(context.getTxHash() == txHash);
    REQUIRE(context.getTxOrigin() == txOrigin);
    REQUIRE(context.getBlockCoinbase() == blockCoinbase);
    REQUIRE(context.getTxIndex() == txIndex);
    REQUIRE(context.getBlockNumber() == blockNumber);
    REQUIRE(context.getBlockTimestamp() == blockTimestamp);
    REQUIRE(context.getBlockGasLimit() == blockGasLimit);
    REQUIRE(context.getTxGasPrice() == txGasPrice);
    REQUIRE(context.getChainId() == chainId);
    REQUIRE(context.retrieve(accountAddress1, slot1) == data);
    REQUIRE(context.retrieve(accountAddress1, slot2) == Hash());
    REQUIRE(context.retrieve(accountAddress2, slot1) == Hash());
    REQUIRE(context.getAccount(accountAddress1).getBalance() == 1000);
    REQUIRE(context.getAccount(accountAddress2).getBalance() == 666);
  }

  SECTION("Checkpoint revert to accounts") {
    ExecutionContext::Accounts accounts;
    ExecutionContext::Storage storage;

    const std::array<const Account, 10> accountsArr = {
      Account(200, 3), Account(100, 2), Account(150, 1),
      Account(800, 2), Account(100, 4), Account(666, 0),
      Account(111, 0), Account(222, 3), Account(0, 0)
    };

    const std::array<const Address, 10> addresses = {
      bytes::hex("0xbb1fe74127ed514da3715ea15f567095215dbf9c"),
      bytes::hex("0x0eb51dd6be2169a9a0696a34034a31886647dc3f"),
      bytes::hex("0x2a2baa3d4772ed5f2fa34758e4f23633c70fcc9b"),
      bytes::hex("0x349dbcc860b78cdeac089bace1eb6a212d8460c4"),
      bytes::hex("0xaba088c6004908ab1eee30598ffe528472c15f0f"),
      bytes::hex("0x6efa98097f6b6b8f4e6347af82220eb969e1f509"),
      bytes::hex("0xaa8b62e65fb53029d46de33e0073a4e813249ab2"),
      bytes::hex("0x2f6ea6302015283fe13a38e8d8ba813426acacf2"),
      bytes::hex("0x9a6cabc2cd5f053961bbc2dae59f0bde2d5daa76"),
      bytes::hex("0xa0ca021de6ee33fd980129ce06af50764a461771")
    };

    accounts.emplace(addresses[0], accountsArr[0]);

    ExecutionContext context = ExecutionContext::Builder()
      .storage(storage)
      .accounts(accounts)
      .build();

    addAccount(context, addresses[1], accountsArr[1]);
    addAccount(context, addresses[2], accountsArr[2]);

    REQUIRE(context.getAccount(addresses[0]).getBalance() == accountsArr[0].balance);
    REQUIRE(context.getAccount(addresses[1]).getBalance() == accountsArr[1].balance);
    REQUIRE(context.getAccount(addresses[2]).getBalance() == accountsArr[2].balance);

    auto checkpoint = context.checkpoint();

    addAccount(context, addresses[3], accountsArr[3]);
    addAccount(context, addresses[4], accountsArr[4]);

    REQUIRE(context.getAccount(addresses[0]).getBalance() == accountsArr[0].balance);
    REQUIRE(context.getAccount(addresses[1]).getBalance() == accountsArr[1].balance);
    REQUIRE(context.getAccount(addresses[2]).getBalance() == accountsArr[2].balance);
    REQUIRE(context.getAccount(addresses[3]).getBalance() == accountsArr[3].balance);
    REQUIRE(context.getAccount(addresses[4]).getBalance() == accountsArr[4].balance);

    context.transferBalance(addresses[0], addresses[1], 100);
    REQUIRE(context.getAccount(addresses[0]).getBalance() == accountsArr[0].balance - 100);
    REQUIRE(context.getAccount(addresses[1]).getBalance() == accountsArr[1].balance + 100);
    REQUIRE_THROWS(context.transferBalance(addresses[0], addresses[1], 101));

    checkpoint.revert();

    REQUIRE(context.getAccount(addresses[0]).getBalance() == accountsArr[0].balance);
    REQUIRE(context.getAccount(addresses[1]).getBalance() == accountsArr[1].balance);
    REQUIRE(context.getAccount(addresses[2]).getBalance() == accountsArr[2].balance);
    REQUIRE(context.getAccount(addresses[3]).getBalance() == uint256_t(0));
    REQUIRE(context.getAccount(addresses[4]).getBalance() == uint256_t(0));

    {
      auto checkpoint2 = context.checkpoint();

      addAccount(context, addresses[5], accountsArr[5]);
      addAccount(context, addresses[6], accountsArr[6]);
      context.transferBalance(addresses[1], addresses[2], 50);

      checkpoint2.commit();
    }

    REQUIRE(context.getAccount(addresses[1]).getBalance() == accountsArr[1].balance - 50);
    REQUIRE(context.getAccount(addresses[2]).getBalance() == accountsArr[2].balance + 50);
    REQUIRE(context.getAccount(addresses[5]).getBalance() == accountsArr[5].balance);
    REQUIRE(context.getAccount(addresses[6]).getBalance() == accountsArr[6].balance);
  }

  SECTION("Nested: revert, revert, revert") {

    std::array<const Address, 10> addr = {
      bytes::hex("5f6e2d4d9d847b820362bf62e1e8b4d4897ce760"),
      bytes::hex("c84c5ecbcc4d5932dfdb4034ab2b8e2b246aef41"),
      bytes::hex("69ca633ac018da9f3356148717b6818b8b37c379"),
      bytes::hex("cc75db077372b3dfcfbf0f3bb37b995aaf8ef155"),
      bytes::hex("0625070d23c0228e75c08c2aac6beba86d349f76"),
      bytes::hex("9c1d041e67b9fb6367a857a98e7a22097f02b28d"),
      bytes::hex("671a7c172bd07caf482c6b6a0e8f74740bf0b2b4"),
      bytes::hex("404e19eca6b9ce9012f6959650a7d19cea4cfb39"),
      bytes::hex("47212cde146e973b8291038e4b7da890e128d2d9"),
      bytes::hex("061e32675637d8accd4a23e32cb6d31dd082f344")
    };

    std::array<const Hash, 10> slots = {
      Hash(uint256_t(0)), Hash(uint256_t(1)), Hash(uint256_t(2)),
      Hash(uint256_t(3)), Hash(uint256_t(4)), Hash(uint256_t(5)),
      Hash(uint256_t(6)), Hash(uint256_t(7)), Hash(uint256_t(8)),
      Hash(uint256_t(9))
    };

    std::array<const Hash, 10> data = {
      bytes::hex("a2122ba388a9bf54565d31a711a1a863cb0c7472433253122f8307f8edca73a8"),
      bytes::hex("4a84e91e23c4872751210b7088ba266cbdf515e0a053e95f567aac5e1a45b537"),
      bytes::hex("432b7b1c5c00d2bd3b37d72672328468271884b311cd094fa5759cbd66310048"),
      bytes::hex("b80d1fcfb14cc5b8a01f7264f326786cefe5db0fc6b1a75d3b7dd0730120678f"),
      bytes::hex("a559ea3e765536ab3610fcf455e44320a5ce221275eb3ed7f84470a370ac2aee"),
      bytes::hex("78886f68e66985f4e7813dc929e6ad0e427ae577ca0a6336442e18426f5a80a1"),
      bytes::hex("66db09b48ea9a16269f07cbd6442abdf19a11d442fe5697b833c99e6cf9e3794"),
      bytes::hex("41047dcdfb2e6dd03a61511beaabc0be7c7f72a74bb999622566bb2e1d0628ca"),
      bytes::hex("6f9f1638daa7e7a8bd2bfaab03db43e29f78f1c920414638cfcebaff41c70ffa"),
      bytes::hex("7bc77af21a9f554fb5d8e5f12255191b96b830c953420072f62b1e507aeed638")
    };

    ExecutionContext::Accounts accounts;
    ExecutionContext::Storage storage;

    storage.emplace(StorageKeyView(addr[0], slots[0]), data[0]);

    ExecutionContext context = ExecutionContext::Builder()
      .storage(storage)
      .accounts(accounts)
      .build();

    context.store(addr[1], slots[1], data[1]);
    context.store(addr[2], slots[2], data[2]);

    {
      auto cp1 = context.checkpoint();

      context.store(addr[3], slots[3], data[3]);
      context.store(addr[4], slots[4], data[4]);

      {
        auto cp2 = context.checkpoint();

        context.store(addr[5], slots[5], data[5]);
        context.store(addr[6], slots[6], data[6]);

        REQUIRE(context.retrieve(addr[0], slots[0]) == data[0]);
        REQUIRE(context.retrieve(addr[1], slots[1]) == data[1]);
        REQUIRE(context.retrieve(addr[2], slots[2]) == data[2]);
        REQUIRE(context.retrieve(addr[3], slots[3]) == data[3]);
        REQUIRE(context.retrieve(addr[4], slots[4]) == data[4]);
        REQUIRE(context.retrieve(addr[5], slots[5]) == data[5]);
        REQUIRE(context.retrieve(addr[6], slots[6]) == data[6]);
      }

      REQUIRE(context.retrieve(addr[0], slots[0]) == data[0]);
      REQUIRE(context.retrieve(addr[1], slots[1]) == data[1]);
      REQUIRE(context.retrieve(addr[2], slots[2]) == data[2]);
      REQUIRE(context.retrieve(addr[3], slots[3]) == data[3]);
      REQUIRE(context.retrieve(addr[4], slots[4]) == data[4]);
      REQUIRE(context.retrieve(addr[5], slots[5]) == Hash());
      REQUIRE(context.retrieve(addr[6], slots[6]) == Hash());
    }

    REQUIRE(context.retrieve(addr[0], slots[0]) == data[0]);
    REQUIRE(context.retrieve(addr[1], slots[1]) == data[1]);
    REQUIRE(context.retrieve(addr[2], slots[2]) == data[2]);
    REQUIRE(context.retrieve(addr[3], slots[3]) == Hash());
    REQUIRE(context.retrieve(addr[4], slots[4]) == Hash());
    REQUIRE(context.retrieve(addr[5], slots[5]) == Hash());
    REQUIRE(context.retrieve(addr[6], slots[6]) == Hash());

    context.revert();

    REQUIRE(context.retrieve(addr[0], slots[0]) == data[0]);
    REQUIRE(context.retrieve(addr[1], slots[1]) == Hash());
    REQUIRE(context.retrieve(addr[2], slots[2]) == Hash());
    REQUIRE(context.retrieve(addr[3], slots[3]) == Hash());
    REQUIRE(context.retrieve(addr[4], slots[4]) == Hash());
    REQUIRE(context.retrieve(addr[5], slots[5]) == Hash());
    REQUIRE(context.retrieve(addr[6], slots[6]) == Hash());
  }

    SECTION("Nested: commit, revert, commit") {

    std::array<const Address, 10> addr = {
      bytes::hex("5f6e2d4d9d847b820362bf62e1e8b4d4897ce760"),
      bytes::hex("c84c5ecbcc4d5932dfdb4034ab2b8e2b246aef41"),
      bytes::hex("69ca633ac018da9f3356148717b6818b8b37c379"),
      bytes::hex("cc75db077372b3dfcfbf0f3bb37b995aaf8ef155"),
      bytes::hex("0625070d23c0228e75c08c2aac6beba86d349f76"),
      bytes::hex("9c1d041e67b9fb6367a857a98e7a22097f02b28d"),
      bytes::hex("671a7c172bd07caf482c6b6a0e8f74740bf0b2b4"),
      bytes::hex("404e19eca6b9ce9012f6959650a7d19cea4cfb39"),
      bytes::hex("47212cde146e973b8291038e4b7da890e128d2d9"),
      bytes::hex("061e32675637d8accd4a23e32cb6d31dd082f344")
    };

    std::array<const Hash, 10> slots = {
      Hash(uint256_t(0)), Hash(uint256_t(1)), Hash(uint256_t(2)),
      Hash(uint256_t(3)), Hash(uint256_t(4)), Hash(uint256_t(5)),
      Hash(uint256_t(6)), Hash(uint256_t(7)), Hash(uint256_t(8)),
      Hash(uint256_t(9))
    };

    std::array<const Hash, 10> data = {
      bytes::hex("a2122ba388a9bf54565d31a711a1a863cb0c7472433253122f8307f8edca73a8"),
      bytes::hex("4a84e91e23c4872751210b7088ba266cbdf515e0a053e95f567aac5e1a45b537"),
      bytes::hex("432b7b1c5c00d2bd3b37d72672328468271884b311cd094fa5759cbd66310048"),
      bytes::hex("b80d1fcfb14cc5b8a01f7264f326786cefe5db0fc6b1a75d3b7dd0730120678f"),
      bytes::hex("a559ea3e765536ab3610fcf455e44320a5ce221275eb3ed7f84470a370ac2aee"),
      bytes::hex("78886f68e66985f4e7813dc929e6ad0e427ae577ca0a6336442e18426f5a80a1"),
      bytes::hex("66db09b48ea9a16269f07cbd6442abdf19a11d442fe5697b833c99e6cf9e3794"),
      bytes::hex("41047dcdfb2e6dd03a61511beaabc0be7c7f72a74bb999622566bb2e1d0628ca"),
      bytes::hex("6f9f1638daa7e7a8bd2bfaab03db43e29f78f1c920414638cfcebaff41c70ffa"),
      bytes::hex("7bc77af21a9f554fb5d8e5f12255191b96b830c953420072f62b1e507aeed638")
    };

    ExecutionContext::Accounts accounts;
    ExecutionContext::Storage storage;

    storage.emplace(StorageKeyView(addr[0], slots[0]), data[0]);

    ExecutionContext context = ExecutionContext::Builder()
      .storage(storage)
      .accounts(accounts)
      .build();

    context.store(addr[1], slots[1], data[1]);
    context.store(addr[2], slots[2], data[2]);

    {
      auto cp1 = context.checkpoint();

      context.store(addr[3], slots[3], data[3]);
      context.store(addr[4], slots[4], data[4]);

      {
        auto cp2 = context.checkpoint();

        context.store(addr[5], slots[5], data[5]);
        context.store(addr[6], slots[6], data[6]);

        REQUIRE(context.retrieve(addr[0], slots[0]) == data[0]);
        REQUIRE(context.retrieve(addr[1], slots[1]) == data[1]);
        REQUIRE(context.retrieve(addr[2], slots[2]) == data[2]);
        REQUIRE(context.retrieve(addr[3], slots[3]) == data[3]);
        REQUIRE(context.retrieve(addr[4], slots[4]) == data[4]);
        REQUIRE(context.retrieve(addr[5], slots[5]) == data[5]);
        REQUIRE(context.retrieve(addr[6], slots[6]) == data[6]);

        cp2.commit();
      }

      REQUIRE(context.retrieve(addr[0], slots[0]) == data[0]);
      REQUIRE(context.retrieve(addr[1], slots[1]) == data[1]);
      REQUIRE(context.retrieve(addr[2], slots[2]) == data[2]);
      REQUIRE(context.retrieve(addr[3], slots[3]) == data[3]);
      REQUIRE(context.retrieve(addr[4], slots[4]) == data[4]);
      REQUIRE(context.retrieve(addr[5], slots[5]) == data[5]);
      REQUIRE(context.retrieve(addr[6], slots[6]) == data[6]);
    }

    REQUIRE(context.retrieve(addr[0], slots[0]) == data[0]);
    REQUIRE(context.retrieve(addr[1], slots[1]) == data[1]);
    REQUIRE(context.retrieve(addr[2], slots[2]) == data[2]);
    REQUIRE(context.retrieve(addr[3], slots[3]) == Hash());
    REQUIRE(context.retrieve(addr[4], slots[4]) == Hash());
    REQUIRE(context.retrieve(addr[5], slots[5]) == Hash());
    REQUIRE(context.retrieve(addr[6], slots[6]) == Hash());

    context.commit();

    REQUIRE(context.retrieve(addr[0], slots[0]) == data[0]);
    REQUIRE(context.retrieve(addr[1], slots[1]) == data[1]);
    REQUIRE(context.retrieve(addr[2], slots[2]) == data[2]);
    REQUIRE(context.retrieve(addr[3], slots[3]) == Hash());
    REQUIRE(context.retrieve(addr[4], slots[4]) == Hash());
    REQUIRE(context.retrieve(addr[5], slots[5]) == Hash());
    REQUIRE(context.retrieve(addr[6], slots[6]) == Hash());
  }
}

