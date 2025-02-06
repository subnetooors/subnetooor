/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#include "templates/erc20.h"
#include "templates/erc20wrapper.h"
#include "templates/nativewrapper.h"
#include "templates/simplecontract.h"
#include "templates/erc721.h"
#include "templates/dexv2/dexv2pair.h"
#include "templates/dexv2/dexv2factory.h"
#include "templates/dexv2/dexv2router02.h"
#include "templates/orderbook/orderbook.h"
#include "templates/throwtestA.h"
#include "templates/throwtestB.h"
#include "templates/throwtestC.h"
#include "templates/erc721test.h"
#include "templates/testThrowVars.h"
#include "templates/randomnesstest.h"
#include "templates/snailtracer.h"
#include "templates/snailtraceroptimized.h"
#include "templates/ownable.h"
#include "templates/pebble.h"

/// Typedef for the blockchain's registered contracts.
#ifdef BUILD_TESTNET
/// Typedef for the blockchain's registered contracts in TESTNET mode.
using ContractTypes = std::tuple<
  ERC20, NativeWrapper, DEXV2Pair, DEXV2Factory, DEXV2Router02, ERC721, ERC721URIStorage, Ownable, Pebble
>;
#else
/// Typedef for the blockchain's registered contracts in normal mode.
using ContractTypes = std::tuple<
  ERC20, ERC20Wrapper, NativeWrapper, SimpleContract, DEXV2Pair, DEXV2Factory,
  DEXV2Router02, ERC721, ThrowTestA, ThrowTestB, ThrowTestC, ERC721Test, TestThrowVars,
  RandomnessTest, SnailTracer, SnailTracerOptimized, Pebble, OrderBook
>;
#endif

