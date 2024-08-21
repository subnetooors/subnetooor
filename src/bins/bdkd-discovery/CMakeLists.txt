# Compile and link the Discovery Node test executable if set to build it
if (BUILD_DISCOVERY)
  add_executable(bdkd-discovery "main.cpp")
  add_dependencies(bdkd-discovery bdk_lib)
  target_include_directories(bdkd-discovery PRIVATE bdk_lib ${OPENSSL_INCLUDE_DIR})
  target_link_libraries(bdkd-discovery
    bdk_lib Speedb ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} ${ZLIB_LIBRARIES} Secp256k1 Ethash ${ETHASH_BYPRODUCTS}
  )
endif()
