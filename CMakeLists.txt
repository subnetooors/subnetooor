# Pre-setup
cmake_minimum_required(VERSION 3.19.0)

include(cmake/cable/bootstrap.cmake)
include(CableBuildInfo)
include(CableBuildType)
include(GNUInstallDirs)
cable_set_build_type(DEFAULT Debug CONFIGURATION_TYPES Debug;Release;RelWithDebInfo)

# Map current configuration to configurations of imported targets.
set(CMAKE_MAP_IMPORTED_CONFIG_Debug Debug)
set(CMAKE_MAP_IMPORTED_CONFIG_RelWithDebInfo Debug)

# Avoid warning about DOWNLOAD_EXTRACT_TIMESTAMP in CMake 3.24+
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
  cmake_policy(SET CMP0135 NEW)
endif()

# Project data
project(bdk VERSION 0.2.0 DESCRIPTION "AppLayer Blockchain Development Kit")
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS OFF)
SET(DEBUG ON CACHE BOOL "Debug mode")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a") # Always look for static libraries - "ZLIB_USE_STATIC_LIBS" was added in 3.24
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # For clang-tidy

# Set compiler flags
# TODO: -Wno-c++26-extensions is included because zpp_libs uses name-independent declarations (vars named "_").
# This should be resolved at a later date, but was turned off for now because it doesn't affect us
if(DEBUG)
  set(CMAKE_CXX_FLAGS "-O0 -g -Wno-c++26-extensions -fsanitize=address -fno-inline -fno-eliminate-unused-debug-types -fstack-protector") # Provides faster compile time.
elseif(SONARQUBE_ANALYSIS)
  set(CMAKE_CXX_FLAGS "-O0 -g -Wno-c++26-extensions --coverage")
else()
  set(CMAKE_CXX_FLAGS "-O2 -Wno-c++26-extensions -Werror=unused-variable")
endif()
find_program(MOLD "mold") # Use mold by default if it is installed
if(MOLD)
  message(STATUS "Using mold as linker")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=mold")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=mold")
endif()

# Set project version inside the code (forcefully so changes in the .in file are always reflected correctly to the compiler)
# if (EXISTS ${CMAKE_SOURCE_DIR}/src/utils/options.h)
#   file(REMOVE ${CMAKE_SOURCE_DIR}/src/utils/options.h)
# endif()
configure_file(
  ${CMAKE_SOURCE_DIR}/src/utils/options.h.in
  ${CMAKE_SOURCE_DIR}/src/utils/options.h
  @ONLY
)

# External project data
set(BUILD_TESTS ON CACHE BOOL "Build helper unit testing program")
set(BUILD_DISCOVERY ON CACHE BOOL "Build helper discovery node program")
set(BUILD_TOOLS OFF CACHE BOOL "Build tools related to subnet")
set(BUILD_TESTNET OFF CACHE BOOL "Build the project for testnet")
set(BUILD_BENCHMARK OFF CACHE BOOL "Build with the benchmark tests")
set(USE_LINT OFF CACHE BOOL "Run linter on compile (clang-tidy)")
set(BUILD_VARIABLES_TESTS ON CACHE BOOL "Build tests for SafeVar (Contract variables)")
if(USE_LINT)
  set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-header-filter=.;-checks=-*,abseil-*,boost-*,bugprone-*,cert-*,clang-analyzer-*,concurrency-*,cppcoreguidelines-*,hicpp-*,misc-*,modernize-*,performance-*,portability-*,readability-*")
endif()

if(BUILD_TESTNET)
  add_definitions(-DBUILD_TESTNET)
endif()

# Echo CMake vars during config
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "C++ standard is required: ${CMAKE_CXX_STANDARD_REQUIRED}")
message(STATUS "C++ extensions: ${CMAKE_CXX_EXTENSIONS}")
message(STATUS "C++ flags: ${CMAKE_CXX_FLAGS}")
message(STATUS "Using PIC: ${CMAKE_POSITION_INDEPENDENT_CODE}")
message(STATUS "Find libs with suffix: ${CMAKE_FIND_LIBRARY_SUFFIXES}")
message("Building tests: ${BUILD_TESTS}")
message("Building Discovery Node: ${BUILD_DISCOVERY}")
message("Building tools: ${BUILD_TOOLS}")
message("Using lint: ${USE_LINT}")

cable_add_buildinfo_library(PROJECT_NAME bdk)

# System package configs (built-in)
set(Boost_USE_STATIC_LIBS ON)
set(OPENSSL_USE_STATIC_LIBS ON)
set(Protobuf_USE_STATIC_LIBS ON)

# Find system packages (built-in)
find_package(Threads)
find_package(Boost 1.83.0 REQUIRED COMPONENTS chrono filesystem program_options system thread nowide)
find_package(OpenSSL 1.1.1 REQUIRED)
find_package(ZLIB REQUIRED)

# Find system packages (custom)
find_package(CryptoPP 8.2.0 REQUIRED)
find_package(Scrypt REQUIRED)



# ----------------------------------------
# Find GRPC and all dependencies

# Check compiler variable sizes
#include(cmake/CheckSizes.cmake)

# absl is needed for GRPC
# For Ubuntu 24 / apt, the package is "absl" (lowercase)
# Also works on Ubuntu 22
# bkd-cpp/cmake/FindAbsl.cmake (i.e. find_package(Absl with capital A) is broken for Ubuntu 22/24
#find_package(Absl REQUIRED) # Built-in is hardcoded to SHARED, this one to STATIC
find_package(absl REQUIRED)

# c-ares is needed for GRPC
# For Ubuntu 24 / apt, the package is "c-ares" (lowercase)
#find_package(c-ares REQUIRED)
#
# BUT
#
# ubuntu 22 requires this one:
# This is still working, so we will use our bdk-cpp/cmake/FindCares.cmake
# FindCares.cmake was fixed to also work with the c-ares lib (dev) that comes with Ubuntu 24
find_package(Cares REQUIRED)

# This one just works
find_package(Protobuf REQUIRED)

# For Ubuntu 24 / apt, the package is "gRPC"
#find_package(gRPC REQUIRED)
#
# ubuntu 22 requires this one, and works on ubuntu 24 
# This is still working, so we will use our bdk-cpp/cmake/FindGRPC.cmake
find_package(GRPC REQUIRED)
# ----------------------------------------



# COMMENT THESE OUT when trying stuff for the GRPC/comet proto files integration
#
# Add external modules
include(cmake/ProjectEthash.cmake)        # Ethash
include(cmake/ProjectSecp256k1.cmake)     # Bitcoin core fast implementation
include(cmake/ProjectSpeedb.cmake)        # Speedb (Level/RocksDB drop-in replacement)
include(cmake/ProjectEVMOne.cmake)        # EVMOne (EVMOne + EVMC)



# Add catch2 as a library
add_library(catch2
  ${CMAKE_SOURCE_DIR}/src/libs/catch2/catch_amalgamated.hpp
  ${CMAKE_SOURCE_DIR}/src/libs/catch2/catch_amalgamated.cpp
)
target_include_directories(catch2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/catch2)
target_compile_definitions(catch2 PRIVATE CATCH_AMALGAMATED_CUSTOM_MAIN)

# Check compiler variable sizes
include(cmake/CheckSizes.cmake)

# Include directories for headers and libs
include_directories(
  "${CMAKE_SOURCE_DIR}"
  "${CMAKE_SOURCE_DIR}/src"
  "${CMAKE_SOURCE_DIR}/src/libs"
  "${CMAKE_SOURCE_DIR}/build"
  "${CMAKE_SOURCE_DIR}/build/deps/include"
)

link_directories(
  "${CMAKE_SOURCE_DIR}"
  "${CMAKE_SOURCE_DIR}/src"
  "${CMAKE_SOURCE_DIR}/src/libs"
  "${CMAKE_SOURCE_DIR}/build"
  "${CMAKE_SOURCE_DIR}/build/deps/lib"
)

# Organize, compile and link bdk libs
add_subdirectory(src/contract)
add_subdirectory(src/core)
add_subdirectory(src/net)
add_subdirectory(src/utils)
add_subdirectory(tests)


# Compile all .proto files to their C++ equivalent for GRPC

#add_custom_command(
#  OUTPUT "${CMAKE_SOURCE_DIR}/proto/cometbft/abci/v1/service.grpc.pb.cc"
#         "${CMAKE_SOURCE_DIR}/proto/cometbft/abci/v1/service.grpc.pb.h"
#  COMMAND "protoc"
#  ARGS --grpc_out="${CMAKE_SOURCE_DIR}/proto"
#       --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
#       --proto_path="${CMAKE_SOURCE_DIR}/proto"
#       "${CMAKE_SOURCE_DIR}/proto/cometbft/abci/v1/service.proto"
#)

# Compile all .proto files to their C++ equivalent for Protobuf

#add_custom_command(
#  OUTPUT "${CMAKE_SOURCE_DIR}/proto/cometbft/abci/v1/service.pb.cc"
#         "${CMAKE_SOURCE_DIR}/proto/cometbft/abci/v1/service.pb.h"
#  COMMAND "protoc"
#  ARGS --cpp_out="${CMAKE_SOURCE_DIR}/proto"
#       --proto_path="${CMAKE_SOURCE_DIR}/proto"
#       "${CMAKE_SOURCE_DIR}/proto/cometbft/abci/v1/service.proto"
#)


# Generate a static (.a) library called ProtoFiles with all the generated cc/h for Protobuf
#add_library(ProtoFiles
#"${CMAKE_SOURCE_DIR}/proto/cometbft/abci/v1/service.pb.cc"
#"${CMAKE_SOURCE_DIR}/proto/cometbft/abci/v1/service.pb.h"
#)

# Generate a static (.a) library called gen-grpc with all the generated cc/h for GRPC
#add_library (gen-grpc
#"${CMAKE_SOURCE_DIR}/proto/cometbft/abci/v1/service.grpc.pb.cc"
#"${CMAKE_SOURCE_DIR}/proto/cometbft/abci/v1/service.grpc.pb.h"
#${ProtoFiles}
#)

# Define the root proto directory
set(PROTO_ROOT_DIR "${CMAKE_SOURCE_DIR}/proto")

# Find all .proto files in the proto/ directory recursively
file(GLOB_RECURSE PROTO_FILES "${PROTO_ROOT_DIR}/*.proto")

# Custom target to clean all generated .cc and .h files
add_custom_target(CleanProtoFiles ALL
    COMMAND ${CMAKE_COMMAND} -E echo "Cleaning old generated .cc and .h files..."
    COMMAND ${CMAKE_COMMAND} -P "${CMAKE_BINARY_DIR}/CleanProtoFiles.cmake"
)

# Create a CMake script to remove files
file(WRITE "${CMAKE_BINARY_DIR}/CleanProtoFiles.cmake"
"file(GLOB_RECURSE pb_files \"${PROTO_ROOT_DIR}/*.pb.cc\" \"${PROTO_ROOT_DIR}/*.pb.h\")
file(GLOB_RECURSE grpc_files \"${PROTO_ROOT_DIR}/*.grpc.pb.cc\" \"${PROTO_ROOT_DIR}/*.grpc.pb.h\")

#file(REMOVE \${pb_files})
#file(REMOVE \${grpc_files})

# Remove .pb files if they exist
if(pb_files)
    file(REMOVE \${pb_files})
    message(STATUS \"Removed .pb.cc and .pb.h files.\")
else()
    message(STATUS \"No .pb.cc or .pb.h files to remove.\")
endif()

# Remove .grpc.pb files if they exist
if(grpc_files)
    file(REMOVE \${grpc_files})
    message(STATUS \"Removed .grpc.pb.cc and .grpc.pb.h files.\")
else()
    message(STATUS \"No .grpc.pb.cc or .grpc.pb.h files to remove.\")
endif()

message(STATUS \"Removed generated .cc and .h files.\")"
)

# This forces the removal of all cc and h files from proto right here
add_custom_target(ForceClean ALL DEPENDS CleanProtoFiles)
set(clean_exit_message "Cleaning cc and h done.")
message(STATUS ${clean_exit_message})
# Exit CMake after running the cleaning command
#return()


# Echo the proto files list
message(STATUS "Proto files found:")
foreach(PROTO_FILE ${PROTO_FILES})
    message(STATUS "  ${PROTO_FILE}")
endforeach()




# FIXME: for some reason, the proto/cometbft/consensus/v1/*.proto stuff is generating
#        garbage cc/h files that are internally inconsistent and don't compile.
#
# This removes the consensus/ folder from the proto files to compile
#
# types.proto cc/h files fail
# wal.proto cc/h fails because it includes types.proto h file
#
# Create a list to store the files to be removed
set(REMOVE_LIST)
# Loop through each file in PROTO_FILES
#foreach(PROTO_FILE ${PROTO_FILES})
#    # Check if the file matches the pattern "/consensus/"
#    if(PROTO_FILE MATCHES "/consensus/")
#        list(APPEND REMOVE_LIST ${PROTO_FILE})
#    endif()
#endforeach()
# Remove the matching files from the PROTO_FILES list
####list(REMOVE_ITEM PROTO_FILES ${REMOVE_LIST})


# Echo the files that matched the pattern
message(STATUS "Removed proto files:")
foreach(REMOVE_FILE ${REMOVE_LIST})
    message(STATUS "  ${REMOVE_FILE}")
endforeach()


# Initialize lists to hold generated files
set(PROTO_SOURCES)
set(GRPC_SOURCES)
set(PROTO_HEADERS)
set(GRPC_HEADERS)

# Loop through each .proto file and create custom commands
#foreach(PROTO_FILE IN LISTS PROTO_FILES)
    # Get the relative path from the proto/ directory
#    file(RELATIVE_PATH REL_PATH "${PROTO_ROOT_DIR}" "${PROTO_FILE}")

    # Remove the .proto extension
#    string(REPLACE ".proto" "" PROTO_BASE "${REL_PATH}")

    # Define the output files for Protobuf
#    set(PB_CC_FILE "${PROTO_ROOT_DIR}/${PROTO_BASE}.pb.cc")
#    set(PB_H_FILE "${PROTO_ROOT_DIR}/${PROTO_BASE}.pb.h")

    # Define the output files for gRPC
#    set(GRPC_PB_CC_FILE "${PROTO_ROOT_DIR}/${PROTO_BASE}.grpc.pb.cc")
#    set(GRPC_PB_H_FILE "${PROTO_ROOT_DIR}/${PROTO_BASE}.grpc.pb.h")

    # Add the custom command for generating Protobuf files
#    add_custom_command(
#      OUTPUT ${PB_CC_FILE} ${PB_H_FILE}
#      COMMAND protoc
#      ARGS --cpp_out=${PROTO_ROOT_DIR}
#           --proto_path=${PROTO_ROOT_DIR}
#           --experimental_allow_proto3_optional
#           ${PROTO_FILE}
#    )

    # Add the custom command for generating gRPC files
#    add_custom_command(
#      OUTPUT ${GRPC_PB_CC_FILE} ${GRPC_PB_H_FILE}
#      COMMAND protoc
#      ARGS --grpc_out=${PROTO_ROOT_DIR}
#           --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
#           --proto_path=${PROTO_ROOT_DIR}
#           --experimental_allow_proto3_optional
#            ${PROTO_FILE}
#    )

    # Add the generated source files to the lists
#    list(APPEND PROTO_SOURCES ${PB_CC_FILE})
#    list(APPEND GRPC_SOURCES ${GRPC_PB_CC_FILE})

    # Add the generated header files to the header lists (if needed for later includes)
#    list(APPEND PROTO_HEADERS ${PB_H_FILE})
#    list(APPEND GRPC_HEADERS ${GRPC_PB_H_FILE})

#endforeach()


# proto$ find . -name "*.proto" -print
#./cometbft/libs/bits/v1/types.proto
#./cometbft/state/v1/types.proto
#./cometbft/services/block/v1/block_service.proto
#./cometbft/services/block/v1/block.proto
#./cometbft/services/version/v1/version_service.proto
#./cometbft/services/version/v1/version.proto
#./cometbft/services/block_results/v1/block_results.proto
#./cometbft/services/block_results/v1/block_results_service.proto
#./cometbft/services/pruning/v1/service.proto
#./cometbft/services/pruning/v1/pruning.proto
#./cometbft/consensus/v1/types.proto
#./cometbft/consensus/v1/wal.proto
#./cometbft/crypto/v1/keys.proto
#./cometbft/crypto/v1/proof.proto
#./cometbft/privval/v1/types.proto
#./cometbft/abci/v1/types.proto
#./cometbft/abci/v1/service.proto
#./cometbft/types/v1/block.proto
#./cometbft/types/v1/types.proto
#./cometbft/types/v1/events.proto
#./cometbft/types/v1/params.proto
#./cometbft/types/v1/validator.proto
#./cometbft/types/v1/canonical.proto
#./cometbft/types/v1/evidence.proto
#./cometbft/p2p/v1/types.proto
#./cometbft/p2p/v1/pex.proto
#./cometbft/p2p/v1/conn.proto
#./cometbft/statesync/v1/types.proto
#./cometbft/blocksync/v1/types.proto
#./cometbft/mempool/v1/types.proto
#./cometbft/store/v1/types.proto
#./cometbft/version/v1/types.proto
#./gogoproto/gogo.proto

# Expanded template (.proto) to create the other two lists
#
#list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/libs/bits/v1/types.proto")
#list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/state/v1/types.proto")
#list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/services/block/v1/block_service.proto")
#list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/services/block/v1/block.proto")
#list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/services/version/v1/version_service.proto")
#list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/services/version/v1/version.proto")
#list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/services/block_results/v1/block_results.proto")
#list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/services/block_results/v1/block_results_service.proto")
#list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/services/pruning/v1/service.proto")
#list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/services/pruning/v1/pruning.proto")
#list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/consensus/v1/types.proto")
#list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/consensus/v1/wal.proto")
#list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/crypto/v1/keys.proto")
#list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/crypto/v1/proof.proto")
#list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/privval/v1/types.proto")
#list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/abci/v1/types.proto")
#list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/abci/v1/service.proto")
#list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/types/v1/block.proto")
#list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/types/v1/types.proto")
#list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/types/v1/events.proto")
#list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/types/v1/params.proto")
#list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/types/v1/validator.proto")
#list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/types/v1/canonical.proto")
#list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/types/v1/evidence.proto")
#list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/p2p/v1/types.proto")
#list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/p2p/v1/pex.proto")
#list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/p2p/v1/conn.proto")
#list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/statesync/v1/types.proto")
#list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/blocksync/v1/types.proto")
#list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/mempool/v1/types.proto")
#list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/store/v1/types.proto")
#list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/version/v1/types.proto")
#list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/gogoproto/gogo.proto")

list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/libs/bits/v1/types.pb.cc")
list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/state/v1/types.pb.cc")
list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/services/block/v1/block_service.pb.cc")
list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/services/block/v1/block.pb.cc")
list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/services/version/v1/version_service.pb.cc")
list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/services/version/v1/version.pb.cc")
list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/services/block_results/v1/block_results.pb.cc")
list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/services/block_results/v1/block_results_service.pb.cc")
list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/services/pruning/v1/service.pb.cc")
list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/services/pruning/v1/pruning.pb.cc")
list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/crypto/v1/keys.pb.cc")
list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/crypto/v1/proof.pb.cc")
list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/privval/v1/types.pb.cc")
list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/abci/v1/types.pb.cc")
list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/abci/v1/service.pb.cc")
list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/types/v1/block.pb.cc")
list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/types/v1/types.pb.cc")
list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/types/v1/events.pb.cc")
list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/types/v1/params.pb.cc")
list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/types/v1/validator.pb.cc")
list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/types/v1/canonical.pb.cc")
list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/types/v1/evidence.pb.cc")
list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/p2p/v1/types.pb.cc")
list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/p2p/v1/pex.pb.cc")
list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/p2p/v1/conn.pb.cc")
list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/statesync/v1/types.pb.cc")
list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/blocksync/v1/types.pb.cc")
list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/mempool/v1/types.pb.cc")
list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/store/v1/types.pb.cc")
list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/version/v1/types.pb.cc")
list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/gogoproto/gogo.pb.cc")
# *** FIXME : these are not compiling
#list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/consensus/v1/types.pb.cc")
#list(APPEND PROTO_SOURCES "${PROTO_ROOT_DIR}/cometbft/consensus/v1/wal.pb.cc")

# Append .h equivalent
foreach(source ${PROTO_SOURCES})
    string(REPLACE ".cc" ".h" header ${source})
    list(APPEND PROTO_HEADERS ${header})
endforeach()
list(APPEND PROTO_SOURCES ${PROTO_HEADERS})

list(APPEND GRPC_SOURCES "${PROTO_ROOT_DIR}/cometbft/libs/bits/v1/types.grpc.pb.cc")
list(APPEND GRPC_SOURCES "${PROTO_ROOT_DIR}/cometbft/state/v1/types.grpc.pb.cc")
list(APPEND GRPC_SOURCES "${PROTO_ROOT_DIR}/cometbft/services/block/v1/block_service.grpc.pb.cc")
list(APPEND GRPC_SOURCES "${PROTO_ROOT_DIR}/cometbft/services/block/v1/block.grpc.pb.cc")
list(APPEND GRPC_SOURCES "${PROTO_ROOT_DIR}/cometbft/services/version/v1/version_service.grpc.pb.cc")
list(APPEND GRPC_SOURCES "${PROTO_ROOT_DIR}/cometbft/services/version/v1/version.grpc.pb.cc")
list(APPEND GRPC_SOURCES "${PROTO_ROOT_DIR}/cometbft/services/block_results/v1/block_results.grpc.pb.cc")
list(APPEND GRPC_SOURCES "${PROTO_ROOT_DIR}/cometbft/services/block_results/v1/block_results_service.grpc.pb.cc")
list(APPEND GRPC_SOURCES "${PROTO_ROOT_DIR}/cometbft/services/pruning/v1/service.grpc.pb.cc")
list(APPEND GRPC_SOURCES "${PROTO_ROOT_DIR}/cometbft/services/pruning/v1/pruning.grpc.pb.cc")
list(APPEND GRPC_SOURCES "${PROTO_ROOT_DIR}/cometbft/crypto/v1/keys.grpc.pb.cc")
list(APPEND GRPC_SOURCES "${PROTO_ROOT_DIR}/cometbft/crypto/v1/proof.grpc.pb.cc")
list(APPEND GRPC_SOURCES "${PROTO_ROOT_DIR}/cometbft/privval/v1/types.grpc.pb.cc")
list(APPEND GRPC_SOURCES "${PROTO_ROOT_DIR}/cometbft/abci/v1/types.grpc.pb.cc")
list(APPEND GRPC_SOURCES "${PROTO_ROOT_DIR}/cometbft/abci/v1/service.grpc.pb.cc")
list(APPEND GRPC_SOURCES "${PROTO_ROOT_DIR}/cometbft/types/v1/block.grpc.pb.cc")
list(APPEND GRPC_SOURCES "${PROTO_ROOT_DIR}/cometbft/types/v1/types.grpc.pb.cc")
list(APPEND GRPC_SOURCES "${PROTO_ROOT_DIR}/cometbft/types/v1/events.grpc.pb.cc")
list(APPEND GRPC_SOURCES "${PROTO_ROOT_DIR}/cometbft/types/v1/params.grpc.pb.cc")
list(APPEND GRPC_SOURCES "${PROTO_ROOT_DIR}/cometbft/types/v1/validator.grpc.pb.cc")
list(APPEND GRPC_SOURCES "${PROTO_ROOT_DIR}/cometbft/types/v1/canonical.grpc.pb.cc")
list(APPEND GRPC_SOURCES "${PROTO_ROOT_DIR}/cometbft/types/v1/evidence.grpc.pb.cc")
list(APPEND GRPC_SOURCES "${PROTO_ROOT_DIR}/cometbft/p2p/v1/types.grpc.pb.cc")
list(APPEND GRPC_SOURCES "${PROTO_ROOT_DIR}/cometbft/p2p/v1/pex.grpc.pb.cc")
list(APPEND GRPC_SOURCES "${PROTO_ROOT_DIR}/cometbft/p2p/v1/conn.grpc.pb.cc")
list(APPEND GRPC_SOURCES "${PROTO_ROOT_DIR}/cometbft/statesync/v1/types.grpc.pb.cc")
list(APPEND GRPC_SOURCES "${PROTO_ROOT_DIR}/cometbft/blocksync/v1/types.grpc.pb.cc")
list(APPEND GRPC_SOURCES "${PROTO_ROOT_DIR}/cometbft/mempool/v1/types.grpc.pb.cc")
list(APPEND GRPC_SOURCES "${PROTO_ROOT_DIR}/cometbft/store/v1/types.grpc.pb.cc")
list(APPEND GRPC_SOURCES "${PROTO_ROOT_DIR}/cometbft/version/v1/types.grpc.pb.cc")
list(APPEND GRPC_SOURCES "${PROTO_ROOT_DIR}/gogoproto/gogo.grpc.pb.cc")
# *** FIXME : these are not compiling
#list(APPEND GRPC_SOURCES "${PROTO_ROOT_DIR}/cometbft/consensus/v1/types.grpc.pb.cc")
#list(APPEND GRPC_SOURCES "${PROTO_ROOT_DIR}/cometbft/consensus/v1/wal.grpc.pb.cc")

# Append .h equivalent
foreach(source ${GRPC_SOURCES})
    string(REPLACE ".cc" ".h" header ${source})
    list(APPEND GRPC_HEADERS ${header})
endforeach()
list(APPEND GRPC_SOURCES ${GRPC_HEADERS})

# Expanded add_custom_command for grpc-gen lib

# generate GRPC files
#   add_custom_command(
#    OUTPUT "${CMAKE_SOURCE_DIR}/proto/vm.grpc.pb.cc"
#           "${CMAKE_SOURCE_DIR}/proto/vm.grpc.pb.h"
#    COMMAND "protoc"
#    ARGS --grpc_out="${CMAKE_SOURCE_DIR}/proto"
#         --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
#         --proto_path="${CMAKE_SOURCE_DIR}/proto"
#         --experimental_allow_proto3_optional
#         "${CMAKE_SOURCE_DIR}/proto/vm.proto"
#  )

# Template
#add_custom_command(
#  OUTPUT "${PROTO_ROOT_DIR}/cometbft/libs/bits/v1/types.grpc.pb.cc"
#         "${PROTO_ROOT_DIR}/cometbft/libs/bits/v1/types.grpc.pb.h"
#  COMMAND "protoc"
#  ARGS --grpc_out="${CMAKE_SOURCE_DIR}/proto"
#       --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
#       --proto_path="${CMAKE_SOURCE_DIR}/proto"
#       --experimental_allow_proto3_optional
#       "${PROTO_ROOT_DIR}/cometbft/libs/bits/v1/types.proto"
#)

add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/libs/bits/v1/types.grpc.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/libs/bits/v1/types.grpc.pb.h"
  COMMAND "protoc"
  ARGS --grpc_out="${CMAKE_SOURCE_DIR}/proto"
       --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
       --proto_path="${CMAKE_SOURCE_DIR}/proto"
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/libs/bits/v1/types.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/state/v1/types.grpc.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/state/v1/types.grpc.pb.h"
  COMMAND "protoc"
  ARGS --grpc_out="${CMAKE_SOURCE_DIR}/proto"
       --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
       --proto_path="${CMAKE_SOURCE_DIR}/proto"
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/state/v1/types.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/services/block/v1/block_service.grpc.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/services/block/v1/block_service.grpc.pb.h"
  COMMAND "protoc"
  ARGS --grpc_out="${CMAKE_SOURCE_DIR}/proto"
       --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
       --proto_path="${CMAKE_SOURCE_DIR}/proto"
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/services/block/v1/block_service.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/services/block/v1/block.grpc.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/services/block/v1/block.grpc.pb.h"
  COMMAND "protoc"
  ARGS --grpc_out="${CMAKE_SOURCE_DIR}/proto"
       --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
       --proto_path="${CMAKE_SOURCE_DIR}/proto"
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/services/block/v1/block.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/services/version/v1/version_service.grpc.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/services/version/v1/version_service.grpc.pb.h"
  COMMAND "protoc"
  ARGS --grpc_out="${CMAKE_SOURCE_DIR}/proto"
       --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
       --proto_path="${CMAKE_SOURCE_DIR}/proto"
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/services/version/v1/version_service.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/services/version/v1/version.grpc.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/services/version/v1/version.grpc.pb.h"
  COMMAND "protoc"
  ARGS --grpc_out="${CMAKE_SOURCE_DIR}/proto"
       --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
       --proto_path="${CMAKE_SOURCE_DIR}/proto"
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/services/version/v1/version.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/services/block_results/v1/block_results.grpc.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/services/block_results/v1/block_results.grpc.pb.h"
  COMMAND "protoc"
  ARGS --grpc_out="${CMAKE_SOURCE_DIR}/proto"
       --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
       --proto_path="${CMAKE_SOURCE_DIR}/proto"
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/services/block_results/v1/block_results.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/services/block_results/v1/block_results_service.grpc.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/services/block_results/v1/block_results_service.grpc.pb.h"
  COMMAND "protoc"
  ARGS --grpc_out="${CMAKE_SOURCE_DIR}/proto"
       --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
       --proto_path="${CMAKE_SOURCE_DIR}/proto"
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/services/block_results/v1/block_results_service.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/services/pruning/v1/service.grpc.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/services/pruning/v1/service.grpc.pb.h"
  COMMAND "protoc"
  ARGS --grpc_out="${CMAKE_SOURCE_DIR}/proto"
       --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
       --proto_path="${CMAKE_SOURCE_DIR}/proto"
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/services/pruning/v1/service.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/services/pruning/v1/pruning.grpc.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/services/pruning/v1/pruning.grpc.pb.h"
  COMMAND "protoc"
  ARGS --grpc_out="${CMAKE_SOURCE_DIR}/proto"
       --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
       --proto_path="${CMAKE_SOURCE_DIR}/proto"
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/services/pruning/v1/pruning.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/consensus/v1/types.grpc.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/consensus/v1/types.grpc.pb.h"
  COMMAND "protoc"
  ARGS --grpc_out="${CMAKE_SOURCE_DIR}/proto"
       --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
       --proto_path="${CMAKE_SOURCE_DIR}/proto"
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/consensus/v1/types.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/consensus/v1/wal.grpc.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/consensus/v1/wal.grpc.pb.h"
  COMMAND "protoc"
  ARGS --grpc_out="${CMAKE_SOURCE_DIR}/proto"
       --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
       --proto_path="${CMAKE_SOURCE_DIR}/proto"
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/consensus/v1/wal.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/crypto/v1/keys.grpc.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/crypto/v1/keys.grpc.pb.h"
  COMMAND "protoc"
  ARGS --grpc_out="${CMAKE_SOURCE_DIR}/proto"
       --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
       --proto_path="${CMAKE_SOURCE_DIR}/proto"
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/crypto/v1/keys.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/crypto/v1/proof.grpc.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/crypto/v1/proof.grpc.pb.h"
  COMMAND "protoc"
  ARGS --grpc_out="${CMAKE_SOURCE_DIR}/proto"
       --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
       --proto_path="${CMAKE_SOURCE_DIR}/proto"
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/crypto/v1/proof.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/privval/v1/types.grpc.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/privval/v1/types.grpc.pb.h"
  COMMAND "protoc"
  ARGS --grpc_out="${CMAKE_SOURCE_DIR}/proto"
       --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
       --proto_path="${CMAKE_SOURCE_DIR}/proto"
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/privval/v1/types.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/abci/v1/service.grpc.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/abci/v1/service.grpc.pb.h"
  COMMAND "protoc"
  ARGS --grpc_out="${CMAKE_SOURCE_DIR}/proto"
       --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
       --proto_path="${CMAKE_SOURCE_DIR}/proto"
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/abci/v1/service.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/abci/v1/types.grpc.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/abci/v1/types.grpc.pb.h"
  COMMAND "protoc"
  ARGS --grpc_out="${CMAKE_SOURCE_DIR}/proto"
       --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
       --proto_path="${CMAKE_SOURCE_DIR}/proto"
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/abci/v1/types.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/types/v1/block.grpc.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/types/v1/block.grpc.pb.h"
  COMMAND "protoc"
  ARGS --grpc_out="${CMAKE_SOURCE_DIR}/proto"
       --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
       --proto_path="${CMAKE_SOURCE_DIR}/proto"
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/types/v1/block.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/types/v1/types.grpc.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/types/v1/types.grpc.pb.h"
  COMMAND "protoc"
  ARGS --grpc_out="${CMAKE_SOURCE_DIR}/proto"
       --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
       --proto_path="${CMAKE_SOURCE_DIR}/proto"
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/types/v1/types.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/types/v1/events.grpc.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/types/v1/events.grpc.pb.h"
  COMMAND "protoc"
  ARGS --grpc_out="${CMAKE_SOURCE_DIR}/proto"
       --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
       --proto_path="${CMAKE_SOURCE_DIR}/proto"
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/types/v1/events.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/types/v1/params.grpc.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/types/v1/params.grpc.pb.h"
  COMMAND "protoc"
  ARGS --grpc_out="${CMAKE_SOURCE_DIR}/proto"
       --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
       --proto_path="${CMAKE_SOURCE_DIR}/proto"
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/types/v1/params.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/types/v1/validator.grpc.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/types/v1/validator.grpc.pb.h"
  COMMAND "protoc"
  ARGS --grpc_out="${CMAKE_SOURCE_DIR}/proto"
       --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
       --proto_path="${CMAKE_SOURCE_DIR}/proto"
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/types/v1/validator.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/types/v1/canonical.grpc.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/types/v1/canonical.grpc.pb.h"
  COMMAND "protoc"
  ARGS --grpc_out="${CMAKE_SOURCE_DIR}/proto"
       --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
       --proto_path="${CMAKE_SOURCE_DIR}/proto"
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/types/v1/canonical.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/types/v1/evidence.grpc.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/types/v1/evidence.grpc.pb.h"
  COMMAND "protoc"
  ARGS --grpc_out="${CMAKE_SOURCE_DIR}/proto"
       --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
       --proto_path="${CMAKE_SOURCE_DIR}/proto"
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/types/v1/evidence.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/p2p/v1/types.grpc.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/p2p/v1/types.grpc.pb.h"
  COMMAND "protoc"
  ARGS --grpc_out="${CMAKE_SOURCE_DIR}/proto"
       --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
       --proto_path="${CMAKE_SOURCE_DIR}/proto"
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/p2p/v1/types.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/p2p/v1/pex.grpc.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/p2p/v1/pex.grpc.pb.h"
  COMMAND "protoc"
  ARGS --grpc_out="${CMAKE_SOURCE_DIR}/proto"
       --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
       --proto_path="${CMAKE_SOURCE_DIR}/proto"
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/p2p/v1/pex.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/p2p/v1/conn.grpc.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/p2p/v1/conn.grpc.pb.h"
  COMMAND "protoc"
  ARGS --grpc_out="${CMAKE_SOURCE_DIR}/proto"
       --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
       --proto_path="${CMAKE_SOURCE_DIR}/proto"
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/p2p/v1/conn.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/statesync/v1/types.grpc.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/statesync/v1/types.grpc.pb.h"
  COMMAND "protoc"
  ARGS --grpc_out="${CMAKE_SOURCE_DIR}/proto"
       --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
       --proto_path="${CMAKE_SOURCE_DIR}/proto"
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/statesync/v1/types.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/blocksync/v1/types.grpc.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/blocksync/v1/types.grpc.pb.h"
  COMMAND "protoc"
  ARGS --grpc_out="${CMAKE_SOURCE_DIR}/proto"
       --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
       --proto_path="${CMAKE_SOURCE_DIR}/proto"
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/blocksync/v1/types.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/mempool/v1/types.grpc.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/mempool/v1/types.grpc.pb.h"
  COMMAND "protoc"
  ARGS --grpc_out="${CMAKE_SOURCE_DIR}/proto"
       --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
       --proto_path="${CMAKE_SOURCE_DIR}/proto"
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/mempool/v1/types.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/store/v1/types.grpc.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/store/v1/types.grpc.pb.h"
  COMMAND "protoc"
  ARGS --grpc_out="${CMAKE_SOURCE_DIR}/proto"
       --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
       --proto_path="${CMAKE_SOURCE_DIR}/proto"
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/store/v1/types.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/version/v1/types.grpc.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/version/v1/types.grpc.pb.h"
  COMMAND "protoc"
  ARGS --grpc_out="${CMAKE_SOURCE_DIR}/proto"
       --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
       --proto_path="${CMAKE_SOURCE_DIR}/proto"
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/version/v1/types.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/gogoproto/gogo.grpc.pb.cc"
         "${PROTO_ROOT_DIR}/gogoproto/gogo.grpc.pb.h"
  COMMAND "protoc"
  ARGS --grpc_out="${CMAKE_SOURCE_DIR}/proto"
       --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
       --proto_path="${CMAKE_SOURCE_DIR}/proto"
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/gogoproto/gogo.proto"
)

# Expanded add_custom_command for ProtoFiles lib 

# generate PROTOBUF files
#  add_custom_command(
#    OUTPUT "${CMAKE_SOURCE_DIR}/proto/vm.pb.cc"
#           "${CMAKE_SOURCE_DIR}/proto/vm.pb.h"
#    COMMAND "protoc"
#    ARGS --cpp_out="${CMAKE_SOURCE_DIR}/proto"
#         --proto_path="${CMAKE_SOURCE_DIR}/proto"
#         --experimental_allow_proto3_optional
#         "${CMAKE_SOURCE_DIR}/proto/vm.proto"
#  )

# Template
#add_custom_command(
#  OUTPUT "${PROTO_ROOT_DIR}/cometbft/libs/bits/v1/types.pb.cc"
#         "${PROTO_ROOT_DIR}/cometbft/libs/bits/v1/types.pb.h"
#  COMMAND protoc
#  ARGS --cpp_out=${PROTO_ROOT_DIR}
#       --proto_path=${PROTO_ROOT_DIR}
#       --experimental_allow_proto3_optional
#       "${PROTO_ROOT_DIR}/cometbft/libs/bits/v1/types.proto"
#)

add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/libs/bits/v1/types.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/libs/bits/v1/types.pb.h"
  COMMAND protoc
  ARGS --cpp_out=${PROTO_ROOT_DIR}
       --proto_path=${PROTO_ROOT_DIR}
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/libs/bits/v1/types.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/state/v1/types.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/state/v1/types.pb.h"
  COMMAND protoc
  ARGS --cpp_out=${PROTO_ROOT_DIR}
       --proto_path=${PROTO_ROOT_DIR}
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/state/v1/types.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/services/block/v1/block_service.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/services/block/v1/block_service.pb.h"
  COMMAND protoc
  ARGS --cpp_out=${PROTO_ROOT_DIR}
       --proto_path=${PROTO_ROOT_DIR}
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/services/block/v1/block_service.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/services/block/v1/block.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/services/block/v1/block.pb.h"
  COMMAND protoc
  ARGS --cpp_out=${PROTO_ROOT_DIR}
       --proto_path=${PROTO_ROOT_DIR}
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/services/block/v1/block.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/services/version/v1/version_service.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/services/version/v1/version_service.pb.h"
  COMMAND protoc
  ARGS --cpp_out=${PROTO_ROOT_DIR}
       --proto_path=${PROTO_ROOT_DIR}
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/services/version/v1/version_service.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/services/version/v1/version.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/services/version/v1/version.pb.h"
  COMMAND protoc
  ARGS --cpp_out=${PROTO_ROOT_DIR}
       --proto_path=${PROTO_ROOT_DIR}
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/services/version/v1/version.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/services/block_results/v1/block_results.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/services/block_results/v1/block_results.pb.h"
  COMMAND protoc
  ARGS --cpp_out=${PROTO_ROOT_DIR}
       --proto_path=${PROTO_ROOT_DIR}
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/services/block_results/v1/block_results.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/services/block_results/v1/block_results_service.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/services/block_results/v1/block_results_service.pb.h"
  COMMAND protoc
  ARGS --cpp_out=${PROTO_ROOT_DIR}
       --proto_path=${PROTO_ROOT_DIR}
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/services/block_results/v1/block_results_service.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/services/pruning/v1/service.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/services/pruning/v1/service.pb.h"
  COMMAND protoc
  ARGS --cpp_out=${PROTO_ROOT_DIR}
       --proto_path=${PROTO_ROOT_DIR}
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/services/pruning/v1/service.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/services/pruning/v1/pruning.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/services/pruning/v1/pruning.pb.h"
  COMMAND protoc
  ARGS --cpp_out=${PROTO_ROOT_DIR}
       --proto_path=${PROTO_ROOT_DIR}
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/services/pruning/v1/pruning.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/consensus/v1/types.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/consensus/v1/types.pb.h"
  COMMAND protoc
  ARGS --cpp_out=${PROTO_ROOT_DIR}
       --proto_path=${PROTO_ROOT_DIR}
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/consensus/v1/types.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/consensus/v1/wal.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/consensus/v1/wal.pb.h"
  COMMAND protoc
  ARGS --cpp_out=${PROTO_ROOT_DIR}
       --proto_path=${PROTO_ROOT_DIR}
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/consensus/v1/wal.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/crypto/v1/keys.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/crypto/v1/keys.pb.h"
  COMMAND protoc
  ARGS --cpp_out=${PROTO_ROOT_DIR}
       --proto_path=${PROTO_ROOT_DIR}
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/crypto/v1/keys.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/crypto/v1/proof.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/crypto/v1/proof.pb.h"
  COMMAND protoc
  ARGS --cpp_out=${PROTO_ROOT_DIR}
       --proto_path=${PROTO_ROOT_DIR}
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/crypto/v1/proof.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/privval/v1/types.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/privval/v1/types.pb.h"
  COMMAND protoc
  ARGS --cpp_out=${PROTO_ROOT_DIR}
       --proto_path=${PROTO_ROOT_DIR}
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/privval/v1/types.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/abci/v1/service.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/abci/v1/service.pb.h"
  COMMAND protoc
  ARGS --cpp_out=${PROTO_ROOT_DIR}
       --proto_path=${PROTO_ROOT_DIR}
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/abci/v1/service.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/abci/v1/types.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/abci/v1/types.pb.h"
  COMMAND protoc
  ARGS --cpp_out=${PROTO_ROOT_DIR}
       --proto_path=${PROTO_ROOT_DIR}
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/abci/v1/types.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/types/v1/block.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/types/v1/block.pb.h"
  COMMAND protoc
  ARGS --cpp_out=${PROTO_ROOT_DIR}
       --proto_path=${PROTO_ROOT_DIR}
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/types/v1/block.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/types/v1/types.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/types/v1/types.pb.h"
  COMMAND protoc
  ARGS --cpp_out=${PROTO_ROOT_DIR}
       --proto_path=${PROTO_ROOT_DIR}
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/types/v1/types.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/types/v1/events.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/types/v1/events.pb.h"
  COMMAND protoc
  ARGS --cpp_out=${PROTO_ROOT_DIR}
       --proto_path=${PROTO_ROOT_DIR}
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/types/v1/events.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/types/v1/params.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/types/v1/params.pb.h"
  COMMAND protoc
  ARGS --cpp_out=${PROTO_ROOT_DIR}
       --proto_path=${PROTO_ROOT_DIR}
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/types/v1/params.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/types/v1/validator.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/types/v1/validator.pb.h"
  COMMAND protoc
  ARGS --cpp_out=${PROTO_ROOT_DIR}
       --proto_path=${PROTO_ROOT_DIR}
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/types/v1/validator.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/types/v1/canonical.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/types/v1/canonical.pb.h"
  COMMAND protoc
  ARGS --cpp_out=${PROTO_ROOT_DIR}
       --proto_path=${PROTO_ROOT_DIR}
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/types/v1/canonical.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/types/v1/evidence.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/types/v1/evidence.pb.h"
  COMMAND protoc
  ARGS --cpp_out=${PROTO_ROOT_DIR}
       --proto_path=${PROTO_ROOT_DIR}
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/types/v1/evidence.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/p2p/v1/types.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/p2p/v1/types.pb.h"
  COMMAND protoc
  ARGS --cpp_out=${PROTO_ROOT_DIR}
       --proto_path=${PROTO_ROOT_DIR}
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/p2p/v1/types.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/p2p/v1/pex.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/p2p/v1/pex.pb.h"
  COMMAND protoc
  ARGS --cpp_out=${PROTO_ROOT_DIR}
       --proto_path=${PROTO_ROOT_DIR}
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/p2p/v1/pex.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/p2p/v1/conn.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/p2p/v1/conn.pb.h"
  COMMAND protoc
  ARGS --cpp_out=${PROTO_ROOT_DIR}
       --proto_path=${PROTO_ROOT_DIR}
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/p2p/v1/conn.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/statesync/v1/types.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/statesync/v1/types.pb.h"
  COMMAND protoc
  ARGS --cpp_out=${PROTO_ROOT_DIR}
       --proto_path=${PROTO_ROOT_DIR}
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/statesync/v1/types.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/blocksync/v1/types.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/blocksync/v1/types.pb.h"
  COMMAND protoc
  ARGS --cpp_out=${PROTO_ROOT_DIR}
       --proto_path=${PROTO_ROOT_DIR}
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/blocksync/v1/types.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/mempool/v1/types.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/mempool/v1/types.pb.h"
  COMMAND protoc
  ARGS --cpp_out=${PROTO_ROOT_DIR}
       --proto_path=${PROTO_ROOT_DIR}
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/mempool/v1/types.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/store/v1/types.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/store/v1/types.pb.h"
  COMMAND protoc
  ARGS --cpp_out=${PROTO_ROOT_DIR}
       --proto_path=${PROTO_ROOT_DIR}
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/store/v1/types.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/cometbft/version/v1/types.pb.cc"
         "${PROTO_ROOT_DIR}/cometbft/version/v1/types.pb.h"
  COMMAND protoc
  ARGS --cpp_out=${PROTO_ROOT_DIR}
       --proto_path=${PROTO_ROOT_DIR}
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/cometbft/version/v1/types.proto"
)
add_custom_command(
  OUTPUT "${PROTO_ROOT_DIR}/gogoproto/gogo.pb.cc"
         "${PROTO_ROOT_DIR}/gogoproto/gogo.pb.h"
  COMMAND protoc
  ARGS --cpp_out=${PROTO_ROOT_DIR}
       --proto_path=${PROTO_ROOT_DIR}
       --experimental_allow_proto3_optional
       "${PROTO_ROOT_DIR}/gogoproto/gogo.proto"
)










# Create the ProtoFiles static library from the generated Protobuf files
add_library(ProtoFiles STATIC ${PROTO_SOURCES})

# Create the gen-grpc static library from the generated gRPC files
#
#   You probably don't actually need ${PROTO_SOURCES} here and ${ProtoFiles} that was here before was probably a harmless bug (empty var)
#
#
#add_library(gen-grpc STATIC ${GRPC_SOURCES} ${ProtoFiles})
#add_library(gen-grpc STATIC ${GRPC_SOURCES} ${PROTO_SOURCES})
add_library(gen-grpc STATIC ${GRPC_SOURCES})

# So that the #include "cometbft/..." etc. statements in the generate cc/h files work
target_include_directories(ProtoFiles PUBLIC "${PROTO_ROOT_DIR}")
target_include_directories(gen-grpc PUBLIC "${PROTO_ROOT_DIR}")


# The gen-grpc library (GRPC C++ code generated from the .proto files) needs to link against these
target_link_libraries(gen-grpc PUBLIC ${Protobuf_LIBRARIES} ${GRPC_LIBRARIES} ${CARES_LIBRARY} ${OPENSSL_LIBRARIES} ${ZLIB_LIBRARIES} absl::flags)



# Hack to get protoc to run immediately (here)
# Create a custom target to generate the proto files
#add_custom_target(GenerateProtoFiles ALL
#  DEPENDS "${CMAKE_SOURCE_DIR}/proto/cometbft/abci/v1/service.grpc.pb.cc"
#          "${CMAKE_SOURCE_DIR}/proto/cometbft/abci/v1/service.grpc.pb.h"
#          "${CMAKE_SOURCE_DIR}/proto/cometbft/abci/v1/service.pb.cc"
#          "${CMAKE_SOURCE_DIR}/proto/cometbft/abci/v1/service.pb.h"
#)




add_library(bdk_lib STATIC
  ${UTILS_HEADERS}
  ${UTILS_SOURCES}
  ${CONTRACT_HEADERS}
  ${CONTRACT_SOURCES}
  ${CORE_HEADERS}
  ${CORE_SOURCES}
  ${NET_HEADERS}
  ${NET_SOURCES}
)

# Link the cc/h files generated by protoc into bdk_lib
#
#   This here would resolve all the dependencies, meaning gen-grpc probably doesn't need to also bundle the ProtoFiles objects
#
add_dependencies(bdk_lib gen-grpc ProtoFiles)

target_include_directories(bdk_lib PRIVATE ${CMAKE_SOURCE_DIR}/include ${OPENSSL_INCLUDE_DIR})

target_link_libraries(bdk_lib PRIVATE EvmcInstructions EvmcLoader EvmcTooling Evmone
  ${CRYPTOPP_LIBRARIES} ${SCRYPT_LIBRARY} Secp256k1 Ethash ${ETHASH_BYPRODUCTS}
  Speedb ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES}

  ${Protobuf_LIBRARIES} ${GRPC_LIBRARIES} ${CARES_LIBRARY} absl::flags

  #grpc
  #grpc++
  #grpc++_reflection
  #grpc_unsecure
  #protobuf::libprotobuf
  #absl::base
  #absl::synchronization
  #cares
  )

add_subdirectory(src/bins)

