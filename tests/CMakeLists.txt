set(TESTS_HEADERS
  ""
  ${CMAKE_SOURCE_DIR}/tests/sdktestsuite.hpp
  ${CMAKE_SOURCE_DIR}/tests/blockchainwrapper.hpp
  ${CMAKE_SOURCE_DIR}/tests/statetest.hpp
  PARENT_SCOPE
)

set (TESTS_SOURCES
  ""
  ${CMAKE_SOURCE_DIR}/tests/utils/block.cpp
  ${CMAKE_SOURCE_DIR}/tests/utils/block_throw.cpp
  ${CMAKE_SOURCE_DIR}/tests/utils/clargs.cpp
  ${CMAKE_SOURCE_DIR}/tests/utils/db.cpp
  ${CMAKE_SOURCE_DIR}/tests/utils/ecdsa.cpp
  ${CMAKE_SOURCE_DIR}/tests/utils/evmcconv.cpp
  ${CMAKE_SOURCE_DIR}/tests/utils/hex.cpp
  ${CMAKE_SOURCE_DIR}/tests/utils/jsonabi.cpp
  ${CMAKE_SOURCE_DIR}/tests/utils/merkle.cpp
  ${CMAKE_SOURCE_DIR}/tests/utils/randomgen.cpp
  ${CMAKE_SOURCE_DIR}/tests/utils/strings.cpp
  ${CMAKE_SOURCE_DIR}/tests/utils/tx.cpp
  ${CMAKE_SOURCE_DIR}/tests/utils/tx_throw.cpp
  ${CMAKE_SOURCE_DIR}/tests/utils/utils.cpp
  ${CMAKE_SOURCE_DIR}/tests/utils/safehash.cpp
  ${CMAKE_SOURCE_DIR}/tests/utils/options.cpp
  ${CMAKE_SOURCE_DIR}/tests/utils/dynamicexception.cpp
  ${CMAKE_SOURCE_DIR}/tests/utils/strconv.cpp
  ${CMAKE_SOURCE_DIR}/tests/utils/uintconv.cpp
  ${CMAKE_SOURCE_DIR}/tests/utils/intconv.cpp
  ${CMAKE_SOURCE_DIR}/tests/contract/abi.cpp
  ${CMAKE_SOURCE_DIR}/tests/contract/event.cpp
  ${CMAKE_SOURCE_DIR}/tests/contract/erc20.cpp
  ${CMAKE_SOURCE_DIR}/tests/contract/erc20wrapper.cpp
  ${CMAKE_SOURCE_DIR}/tests/contract/erc721test.cpp
  ${CMAKE_SOURCE_DIR}/tests/contract/nativewrapper.cpp
  ${CMAKE_SOURCE_DIR}/tests/contract/contractabigenerator.cpp
  ${CMAKE_SOURCE_DIR}/tests/contract/dexv2.cpp
  ${CMAKE_SOURCE_DIR}/tests/contract/simplecontract.cpp
  ${CMAKE_SOURCE_DIR}/tests/contract/evm.cpp
  ${CMAKE_SOURCE_DIR}/tests/contract/randomness.cpp
  ${CMAKE_SOURCE_DIR}/tests/contract/precompiles.cpp
  ${CMAKE_SOURCE_DIR}/tests/contract/calltracer.cpp
  ${CMAKE_SOURCE_DIR}/tests/contract/createcontract.cpp
  ${CMAKE_SOURCE_DIR}/tests/contract/pebble.cpp
  ${CMAKE_SOURCE_DIR}/tests/contract/executioncontext.cpp
  ${CMAKE_SOURCE_DIR}/tests/contract/evmcontractexecutor.cpp
  ${CMAKE_SOURCE_DIR}/tests/contract/throwtest.cpp
  ${CMAKE_SOURCE_DIR}/tests/contract/orderbook.cpp
  ${CMAKE_SOURCE_DIR}/tests/contract/variables/safeuint_t.cpp
  ${CMAKE_SOURCE_DIR}/tests/contract/variables/safeint_t.cpp
  ${CMAKE_SOURCE_DIR}/tests/contract/variables/safestring.cpp
  ${CMAKE_SOURCE_DIR}/tests/contract/variables/safeaddress.cpp
  ${CMAKE_SOURCE_DIR}/tests/contract/variables/safeunorderedmap.cpp
  ${CMAKE_SOURCE_DIR}/tests/contract/variables/safebool.cpp
  ${CMAKE_SOURCE_DIR}/tests/contract/variables/safebytes.cpp
  ${CMAKE_SOURCE_DIR}/tests/contract/variables/safevector.cpp
  ${CMAKE_SOURCE_DIR}/tests/contract/variables/safearray.cpp
  ${CMAKE_SOURCE_DIR}/tests/contract/variables/safetuple.cpp
  ${CMAKE_SOURCE_DIR}/tests/core/rdpos.cpp
  ${CMAKE_SOURCE_DIR}/tests/core/storage.cpp
  ${CMAKE_SOURCE_DIR}/tests/core/state.cpp
  ${CMAKE_SOURCE_DIR}/tests/core/dumpmanager.cpp
  #${CMAKE_SOURCE_DIR}/tests/core/blockchain.cpp # TODO: Blockchain is failing due to rdPoSWorker.
  ${CMAKE_SOURCE_DIR}/tests/net/p2p/encoding.cpp
  ${CMAKE_SOURCE_DIR}/tests/net/p2p/nodeinfo.cpp
  ${CMAKE_SOURCE_DIR}/tests/net/p2p/p2p.cpp
  ${CMAKE_SOURCE_DIR}/tests/net/http/error.cpp
  ${CMAKE_SOURCE_DIR}/tests/net/http/httpjsonrpc.cpp
  ${CMAKE_SOURCE_DIR}/tests/net/http/parser.cpp
  ${CMAKE_SOURCE_DIR}/tests/sdktestsuite.cpp
)

# Conditionally add variables tests based on the BUILD_VARIABLES_TESTS option
# the reason we made this is because safeuint_t and safeint_t are extremely slow to compile
# TODO: change this when we take out boost multiprecision from the project for C23 BigInt
if (BUILD_VARIABLES_TESTS)
  list(APPEND TESTS_SOURCES
    ${CMAKE_SOURCE_DIR}/tests/contract/variables/reentrancyguard.cpp
    ${CMAKE_SOURCE_DIR}/tests/contract/variables/safeaddress.cpp
    ${CMAKE_SOURCE_DIR}/tests/contract/variables/safearray.cpp
    ${CMAKE_SOURCE_DIR}/tests/contract/variables/safebool.cpp
    ${CMAKE_SOURCE_DIR}/tests/contract/variables/safebytes.cpp
    ${CMAKE_SOURCE_DIR}/tests/contract/variables/safeint_t.cpp
    ${CMAKE_SOURCE_DIR}/tests/contract/variables/safestring.cpp
    ${CMAKE_SOURCE_DIR}/tests/contract/variables/safetuple.cpp
    ${CMAKE_SOURCE_DIR}/tests/contract/variables/safeuint_t.cpp
    ${CMAKE_SOURCE_DIR}/tests/contract/variables/safeunorderedmap.cpp
    ${CMAKE_SOURCE_DIR}/tests/contract/variables/safevector.cpp
  )
endif()

# Add benchmark.cpp if BUILD_BENCHMARK is set
if (BUILD_BENCHMARK)
  message(STATUS "Building benchmark tests")
  list(APPEND TESTS_SOURCES
    ${CMAKE_SOURCE_DIR}/tests/benchmark/erc20.cpp
    ${CMAKE_SOURCE_DIR}/tests/benchmark/snailtracer.cpp
    ${CMAKE_SOURCE_DIR}/tests/benchmark/snailtraceroptimized.cpp
    ${CMAKE_SOURCE_DIR}/tests/benchmark/uniswapv2.cpp
    ${CMAKE_SOURCE_DIR}/tests/benchmark/erc721.cpp
  )
endif()

set(TESTS_SOURCES "${TESTS_SOURCES}" PARENT_SCOPE)
