set(TESTS_HEADERS "")
set(TESTS_SOURCES "")

# TODO: missing the following tests: contract/evm.cpp, contract/randomness.cpp, core/state.cpp, core/storage.cpp, net/httpjsonrpc.cpp

if(BUILD_TESTS)
  list(APPEND TESTS_HEADERS
    ${CMAKE_SOURCE_DIR}/tests/sdktestsuite.hpp
  )
  list(APPEND TESTS_SOURCES
    ${CMAKE_SOURCE_DIR}/tests/sdktestsuite.cpp
  )

  list(APPEND TESTS_SOURCES
    ${CMAKE_SOURCE_DIR}/tests/unit/contract/abi.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit/contract/contractabigenerator.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit/contract/event.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit/contract/evmcontractexecutor.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit/contract/executioncontext.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit/net/httperror.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit/net/jsonrpcparser.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit/utils/clargs.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit/utils/db.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit/utils/dynamicexception.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit/utils/ecdsa.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit/utils/evmcconv.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit/utils/hex.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit/utils/intconv.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit/utils/jsonabi.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit/utils/merkle.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit/utils/options.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit/utils/randomgen.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit/utils/safehash.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit/utils/strconv.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit/utils/strings.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit/utils/tx.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit/utils/uintconv.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit/utils/utils.cpp
  )

  list(APPEND TESTS_SOURCES
    ${CMAKE_SOURCE_DIR}/tests/integration/contract/calltracer.cpp
    ${CMAKE_SOURCE_DIR}/tests/integration/contract/createcontract.cpp
    ${CMAKE_SOURCE_DIR}/tests/integration/contract/dexv2.cpp
    ${CMAKE_SOURCE_DIR}/tests/integration/contract/erc20.cpp
    ${CMAKE_SOURCE_DIR}/tests/integration/contract/erc20wrapper.cpp
    ${CMAKE_SOURCE_DIR}/tests/integration/contract/erc721test.cpp
    ${CMAKE_SOURCE_DIR}/tests/integration/contract/nativewrapper.cpp
    ${CMAKE_SOURCE_DIR}/tests/integration/contract/pebble.cpp
    ${CMAKE_SOURCE_DIR}/tests/integration/contract/precompiles.cpp
    ${CMAKE_SOURCE_DIR}/tests/integration/contract/simplecontract.cpp
    ${CMAKE_SOURCE_DIR}/tests/integration/contract/throwtest.cpp
    ${CMAKE_SOURCE_DIR}/tests/integration/core/blockchain.cpp
    ${CMAKE_SOURCE_DIR}/tests/integration/core/comet.cpp
  )

  # Conditionally add variables tests based on the respective CMake flag.
  # This is done because SafeUint/SafeInt tests are extremely slow to compile due to heavy usage of templates.
  if(BUILD_SAFEVAR_TESTS)
    list(APPEND TESTS_SOURCES
      ${CMAKE_SOURCE_DIR}/tests/unit/safevars/reentrancyguard.cpp
      ${CMAKE_SOURCE_DIR}/tests/unit/safevars/safeaddress.cpp
      ${CMAKE_SOURCE_DIR}/tests/unit/safevars/safearray.cpp
      ${CMAKE_SOURCE_DIR}/tests/unit/safevars/safebool.cpp
      ${CMAKE_SOURCE_DIR}/tests/unit/safevars/safebytes.cpp
      ${CMAKE_SOURCE_DIR}/tests/unit/safevars/safeint.cpp
      ${CMAKE_SOURCE_DIR}/tests/unit/safevars/safestring.cpp
      ${CMAKE_SOURCE_DIR}/tests/unit/safevars/safetuple.cpp
      ${CMAKE_SOURCE_DIR}/tests/unit/safevars/safeuint.cpp
      ${CMAKE_SOURCE_DIR}/tests/unit/safevars/safeunorderedmap.cpp
      ${CMAKE_SOURCE_DIR}/tests/unit/safevars/safevector.cpp
    )
  endif()

  # Conditionally add benchmark tests based on the respective CMake flag.
  if(BUILD_BENCHMARK_TESTS)
    list(APPEND TESTS_SOURCES
      ${CMAKE_SOURCE_DIR}/tests/benchmark/erc20.cpp
      ${CMAKE_SOURCE_DIR}/tests/benchmark/erc721.cpp
      ${CMAKE_SOURCE_DIR}/tests/benchmark/snailtracer.cpp
      ${CMAKE_SOURCE_DIR}/tests/benchmark/snailtraceroptimized.cpp
      ${CMAKE_SOURCE_DIR}/tests/benchmark/uniswapv2.cpp
    )
  endif()
endif()

set(TESTS_HEADERS ${TESTS_HEADERS} PARENT_SCOPE)
set(TESTS_SOURCES ${TESTS_SOURCES} PARENT_SCOPE)

