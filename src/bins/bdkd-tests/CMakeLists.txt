# Compile and link the test executable if set to build it
if (BUILD_TESTS)
  add_executable(bdkd-tests ${TESTS_HEADERS} ${TESTS_SOURCES})
  add_dependencies(bdkd-tests bdk_lib)
  target_include_directories(bdkd-tests PRIVATE bdk_lib  ${OPENSSL_INCLUDE_DIR})
  target_link_libraries(bdkd-tests
    bdk_lib Speedb ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} ${ZLIB_LIBRARIES} Secp256k1 catch2 Ethash ${ETHASH_BYPRODUCTS}
  )
endif()