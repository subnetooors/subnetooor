# Compile and link the executable
add_executable(bdkd "main.cpp")

add_dependencies(bdkd bdk_lib)
target_include_directories(bdkd PRIVATE bdk_lib ${OPENSSL_INCLUDE_DIR})
target_link_libraries(bdkd
  bdk_lib Speedb ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} ${ZLIB_LIBRARIES} Secp256k1 Ethash ${ETHASH_BYPRODUCTS}
)