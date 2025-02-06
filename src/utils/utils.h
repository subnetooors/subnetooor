/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#ifndef UTILS_H
#define UTILS_H

#include <filesystem>
#include <fstream>
#include <regex>
#include <span>
#include <string_view>
#include <thread>
#include <tuple>
#include <variant>
#include <cmath>

#include <boost/beast/core/error.hpp>
#include <boost/asio/ip/address.hpp>

#include <ethash/keccak.h>

#include "../libs/json.hpp" // algorithm, array, tuple

#include "../bytes/join.h" // range.h, view.h, initializer.h

#include "../contract/variables/safeint.h"
#include "../contract/variables/safeuint.h"

#include "dynamicexception.h" // included by strings.h, leave it for now to avoid AddressSanitizer runtime errors - TODO: see create_view_span()
#include "logger.h"
#include "strings.h" // hex.h, openssl/rand.h, libs/zpp_bits.h -> algorithm, array, span, variant

/// Localhost IPv4 address constant
inline const boost::asio::ip::address LOCALHOST = boost::asio::ip::address::from_string("127.0.0.1");

/// @file utils.h
// Forward declaration.
class Hash;

/// Typedef for json.
using json = nlohmann::ordered_json;

// TODO: duplicated in hex.h, strconv.h, evmcconv.h and (u)intconv.h, find a better way to handle this
using Byte = uint8_t; ///< Typedef for Byte.
using Bytes = std::vector<Byte>; ///< Typedef for Bytes.
template <std::size_t N> using BytesArr = std::array<Byte, N>; ///< Typedef for BytesArr.

/// Base case for the recursive helper - now using requires for an empty body function.
template <size_t I = 0, typename... Tp> requires (I == sizeof...(Tp))
void printDurationsHelper(std::string_view, std::tuple<Tp...>&, const std::array<std::string, sizeof...(Tp)>&) {
  // Empty body, stopping condition for the recursion
}

/// Recursive helper function to print each duration - with requires.
template <size_t I = 0, typename... Tp> requires (I < sizeof...(Tp))
void printDurationsHelper(std::string_view id, std::tuple<Tp...>& t, const std::array<std::string, sizeof...(Tp)>& names) {
  auto now = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - std::get<I>(t));
  std::string funcName = names[I]; // Creating a copy to use with std::move
  Logger::logToDebug(LogType::DEBUG, Log::P2PManager, std::move(funcName),
    "Timepoint at: " + std::string(id) + " for " + names[I] + ": "
      + std::to_string(std::get<I>(t).time_since_epoch().count()) + "ms "
      + " Duration for " + names[I] + ": " + std::to_string(duration.count())
      + "ms, exitted at: " + std::to_string(now.time_since_epoch().count()) + "ms"
  );
  // Recursive call for the next element in the tuple
  printDurationsHelper<I + 1, Tp...>(id, t, names);
}

/// Helper templated struct for profiling names and time points.
template <typename... Tp> struct printAtExit {
  std::tuple<Tp...> timePoints; ///< List of time points.
  std::array<std::string, sizeof...(Tp)> names; ///< List of names.
  std::string_view id;  ///< ID of the struct.

  /**
   * Constructor.
   * @param id_ ID of the struct.
   * @param names_ List of names to profile.
   * @param timePoints_ List of time points to profile.
   */
  printAtExit(const std::string& id_, const std::array<std::string, sizeof...(Tp)>& names_, Tp&... timePoints_)
    : timePoints(std::tie(timePoints_...)), names(names_), id(id_) {}

  /// Destructor.
  ~printAtExit() { printDurationsHelper(id, timePoints, names); }
};

/**
 * Map with addresses for contracts deployed at protocol level (name -> address).
 * These contracts are deployed at the beginning of the chain and cannot be
 * destroyed or dynamically deployed like other contracts.
 * Instead, they are deployed in the constructor of State.
 */
const boost::unordered_flat_map<std::string, Address> ProtocolContractAddresses = {
  {"rdPoS", Address(Hex::toBytes("0xb23aa52dbeda59277ab8a962c69f5971f22904cf"))},           // Sha3("randomDeterministicProofOfStake").substr(0,20)
  {"ContractManager", Address(Hex::toBytes("0x0001cb47ea6d8b55fe44fdd6b1bdb579efb43e61"))}  // Sha3("ContractManager").substr(0,20)
};

/**
* Fail a function with a given message.
* @param cl The class name.
* @param func The function name.
* @param ec The error code.
* @param what The message to print.
*/
void fail(const std::string& cl, std::string&& func, boost::beast::error_code ec, const char* what);

/// Enum for network type.
enum class Networks { Mainnet, Testnet, LocalTestnet };

/// Enum for FunctionType
enum class FunctionTypes { View, NonPayable, Payable };

/// Enum for the type of the contract.
enum class ContractType { NOT_A_CONTRACT, EVM, CPP };

/**
 * Abstraction of balance and nonce for a single account.
 * Used with Address on State in an unordered_map to track native accounts.
 * We store both the code and code hash here, but the EVM Storage (map<address,map<bytes32,bytes32>) is
 * directly implemented in the State as map<StorageKey,Hash> to avoid nested maps.
 * @see State
 */
struct Account {
  uint256_t balance = 0;                       ///< Account balance.
  uint64_t nonce = 0;                          ///< Account nonce.
  Hash codeHash = Hash();                      ///< Account code hash (if any)
  Bytes code = Bytes();                        ///< Account code (if any)
  ContractType contractType = ContractType::NOT_A_CONTRACT; ///< Account contract type.

  /// Default constructor.
  Account() = default;

  /// Copy constructor.
  Account(const uint256_t& balance, const uint64_t& nonce) : balance(balance), nonce(nonce) {}

  /// Move constructor.
  Account(uint256_t&& balance, uint64_t&& nonce) : balance(std::move(balance)), nonce(std::move(nonce)) {}

  /// Deserialize constructor.
  Account(const View<Bytes>& bytes);

  /**
   * Serialize the account.
   * We serialize as balance + nonce + codeHash + contractType + code (if any)
   * 32 bytes + 8 bytes + 32 bytes + 1 byte + N (0 or more bytes) = 73 + N bytes
   */
  Bytes serialize() const;

  /// Check if the account address is a contract.
  bool isContract() const { return contractType != ContractType::NOT_A_CONTRACT; }
};

/// Wrapper around std::unique_ptr that ensures the pointer is never null.
template<typename T> class NonNullUniquePtr {
  private:
    std::unique_ptr<T> ptr; ///< Pointer value.

  public:
    /// Constructor that calls T<Ts...> with the provided arguments.
    template<typename... Ts> explicit NonNullUniquePtr(Ts&&... args) : ptr(std::make_unique<T>(std::forward<Ts>(args)...)) {}

    NonNullUniquePtr(const NonNullUniquePtr&) = delete; ///< Copy constructor (deleted to prevent copying, Rule of Zero)
    NonNullUniquePtr(NonNullUniquePtr&& other) = default; ///< Move constructor (allowed as default, Rule of Zero)
    NonNullUniquePtr& operator=(const NonNullUniquePtr&) = delete; ///< Copy assignment operator (deleted to prevent copying, Rule of Zero)
    NonNullUniquePtr& operator=(NonNullUniquePtr&&) = default; ///< Move assignment operator (allowed as default, Rule of Zero)

    /// Dereference operator.
    T& operator*() const { return *ptr; }

    /// Member access operator.
    T* operator->() const { return ptr.get(); }

    /// Getter for raw pointer (optional, use with care).
    T* get() const { return ptr.get(); }
};

/// Wrapper around a raw pointer that ensures the pointer will be null at destruction.
template<typename T> class PointerNullifier {
  private:
    T*& ptr; ///< Pointer value.

  public:
    /// Constructor.
    PointerNullifier(T*& item) : ptr(item) {}

    /// Destructor.
    ~PointerNullifier() { ptr = nullptr; }
};

/**
 * Struct for abstracting a Solidity event parameter.
 * @tparam T The parameter's type.
 * @tparam Index Whether the parameter is indexed or not.
 */
template<typename T, bool Index> struct EventParam {
  using type = T; ///< Event param type.
  const T& value; ///< Event param value.
  static constexpr bool isIndexed = Index;  ///< Indexed status.
  EventParam(const T& value) : value(value) {}  ///< Constructor.
};

/// Namespace for utility functions.
namespace Utils {

  template<typename... Ts>
  struct Overloaded : Ts... { using Ts::operator()...; };

  std::string getTestDumpPath(); ///< Get the path to the test dump folder.

  /**
   * Create a Bytes string out of a data range.
   * @param data The range to use.
   * @return A bytes string.
   */
  constexpr Bytes makeBytes(const bytes::DataRange auto& data) {
    Bytes res(std::ranges::size(data));
    std::ranges::copy(data, res.begin());
    return res;
  }

  /**
   * Create a Bytes string out of a sized initializer.
   * @param initializer The initializer to use.
   * @return A bytes string.
   */
  constexpr Bytes makeBytes(bytes::SizedInitializer auto&& initializer) {
    Bytes res(initializer.size());
    initializer.to(res);
    return res;
  }

  /**
   * Helper function for removeQualifiers
   * @tparam TTuple The tuple type to remove qualifiers from.
   * @tparam I The index sequence.
   */
  template <typename TTuple, std::size_t... I> auto removeQualifiersImpl(std::index_sequence<I...>) {
    return std::tuple<std::decay_t<std::tuple_element_t<I, TTuple>>...>{};
  }

  /**
   * Remove the qualifiers from a tuple type.
   * @tparam TTuple The tuple type to remove qualifiers from.
   * @return A tuple with the same types but qualifiers removed.
   */
  template <typename TTuple> auto removeQualifiers() {
    return removeQualifiersImpl<TTuple>(std::make_index_sequence<std::tuple_size_v<TTuple>>{});
  }

  /**
   * Template for identifying if a type is a uint between 8 and 256 bits.
   * @tparam T The type to check.
   * @tparam N The number of bits.
   */
  template<typename T, std::size_t N> struct isRangedUint {
    /// Indicates whether the type is a uint between 8 and 256 bits.
    static const bool value = std::is_integral_v<T> && std::is_unsigned_v<T> && (sizeof(T) * 8 <= N);
  };

  /**
   * Template for identifying if a type is an int between 8 and 256 bits.
   * @tparam T The type to check.
   * @tparam N The number of bits.
   */
  template<typename T, std::size_t N> struct isRangedInt {
    /// Indicates whether the type is an int between 8 and 256 bits.
    static const bool value = std::is_integral_v<T> && std::is_signed_v<T> && (sizeof(T) * 8 <= N);
  };

  /**
   * Template for identifying if a type is a tuple.
   * @tparam T The type to check.
   */
  template <typename T> struct is_tuple : std::false_type {};

  /**
   * Template explicit specialization for identifying if a type is a tuple.
   * @tparam Ts The types to check.
   */
  template <typename... Ts> struct is_tuple<std::tuple<Ts...>> : std::true_type {};

  /// Helper struct to conditionally append a type to a tuple.
  template <bool Flag, typename T, typename Tuple> struct conditional_tuple_append {
    using type = Tuple; ///< Typedef.
  };

  /// Helper struct to conditionally append a type to a tuple.
  template <typename T, typename... Ts> struct conditional_tuple_append<false, T, std::tuple<Ts...>> {
    using type = std::tuple<Ts..., T>;  ///< Typedef.
  };

  /// Helper struct for making a tuple type.
  template<typename Accumulated, typename... Rest> struct makeTupleTypeHelper {
    using type = Accumulated; ///< Typedef.
  };

  /// Helper struct for making a tuple type.
  template<typename Accumulated, typename T, bool Flag, typename... Rest>
  struct makeTupleTypeHelper<Accumulated, EventParam<T, Flag>, Rest...> {
    using NextAccumulated = typename conditional_tuple_append<Flag, T, Accumulated>::type;  ///< Typedef.
    using type = typename makeTupleTypeHelper<NextAccumulated, Rest...>::type;  ///< Typedef.
  };

  /// Helper struct for making a tuple type.
  template<typename... Args> struct makeTupleType;

  /// Helper struct for making a tuple type.
  template<typename... Args, bool... Flags> struct makeTupleType<EventParam<Args, Flags>...> {
    using type = typename makeTupleTypeHelper<std::tuple<>, EventParam<Args, Flags>...>::type;  ///< Typedef.
  };

  /**
   * %Log a string to a file called `log.txt`.
   * @param str The string to log.
   */
  void logToFile(std::string_view str);

  /**
   * Create a Functor based on a std::string with the function signature (e.g. "functionName(uint256,uint256)").
   * @param functionSignature The function signature.
   * @return The created Functor.
   */
  Functor makeFunctor(std::string_view functionSignature);

  /**
   * Print a string to stdout. Does not print if in a test.
   * @param str The string to print.
   */
  void safePrint(std::string_view str);

  /**
   * Print a string to stdout, including if it is in a test.
   * @param str The string to print.
   */
  void safePrintTest(std::string_view str);

  /**
   * %Hash a given input using SHA3.
   * @param input The string to hash.
   * @return The SHA3-hashed string.
   */
  Hash sha3(const View<Bytes> input);

  /**
   * Generate a random bytes string of a given size.
   * @param size The size of the string.
   * @return The generated bytes string.
   */
  Bytes randBytes(const int& size);

  /**
   * Convert a big-endian byte-stream represented on a templated collection to a templated integer value.
   * `In` will typically be either std::string or bytes.
   * `T` will typically be unsigned, u160, u256 or boost::multiprecision::number.
   * @param bytes The byte stream to convert.
   * @return The converted integer type.
   */
  template <class T, class In> T fromBigEndian(const In& bytes) {
    auto ret = (T)0;
    for (auto i : bytes) {
      ret = (T)((ret << 8) | (uint8_t)(typename std::make_unsigned_t<decltype(i)>) i);
    }
    return ret;
  }

  /**
   * Load HTTP port settings from a config file. Creates the file if it doesn't exist.
   * @return A JSON object with the settings.
   */
  // TODO: Organize every "ruleset read-only" as a "Settings" defined class to avoid variable redefinition
  json readConfigFile();

  /**
   * Tells how many bytes are required to store a given integer.
   * @param i The integer to check.
   * @return The number of bytes required to store the integer.
   */
  template <class T> inline unsigned bytesRequired(T i) {
    // bigint does not carry sign bit on shift
    using bigint = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<>>;
    static_assert(std::is_same_v<bigint, T> || !std::numeric_limits<T>::is_signed, "only unsigned types or bigint supported");
    unsigned ic = 0;
    for (; i != 0; ++ic, i >>= 8);
    return ic;
  }

  /**
   * Convert an unsigned integer to bytes.
   * Takes uint as little endian and has no padding, as opposed to uintXToBytes().
   * @param i The integer to convert.
   * @return The converted bytes string.
   */
  template <class T> Bytes uintToBytes(T i) {
    Bytes ret(bytesRequired(i));
    auto b = ret.end(); // Bytes::iterator
    for (; i; i >>= 8) {
      if (b == ret.begin()) break;
      *(--b) = (uint8_t)(i & 0xff);
    }
    return ret;
  }

  /**
   * Get the real type name of a type.
   * For example, `getRealTypeName<std::string>()` will return
   * "std::basic_string<char, std::char_traits<char>, std::allocator<char>>".
   * @tparam T The type to get the name of.
   * @return The real type name.
   */
  template <typename T> std::string getRealTypeName() {
    int status;
    char* demangledName = nullptr;
    std::string mangledName = typeid(T).name();
    demangledName = abi::__cxa_demangle(mangledName.c_str(), nullptr, nullptr, &status);
    if (status == 0 && demangledName != nullptr) {
      std::string realName(demangledName);
      free(demangledName);
      return realName;
    } else {
      return mangledName;
    }
  }

  /**
   * Convert a vector to const span.
   * @param vec The vector to convert.
   * @return The converted span.
   */
  inline View<Bytes> create_view_span(const Bytes& vec) {
    return View<Bytes>(vec.data(), vec.size());
  }

  /**
   * Convert a "subvector" to const span.
   * @param vec The vector to convert.
   * @param start The start index of the subvector.
   * @param size The size of the subvector.
   * @return The converted span.
   * @throw DynamicException if range is invalid.
   */
   View<Bytes> create_view_span(const Bytes& vec, size_t start, size_t size);

  /**
   * Convert an array to const span.
   * @param arr The array to convert.
   * @return The converted span.
   */
  template<std::size_t N> inline View<Bytes> create_view_span(const BytesArr<N>& arr) {
    return View<Bytes>(arr.data(), arr.size());
  }

  /**
   * Convert a "subarray" to const span.
   * @param arr The array to convert.
   * @param start The start index of the subarray.
   * @param size The size of the subarray.
   * @return The converted span.
   * @throw DynamicException if range is invalid.
   */
  template<std::size_t N> inline View<Bytes> create_view_span(
    const BytesArr<N>& arr, size_t start, size_t size
  ) {
    // TODO: somehow migrate this to the cpp file so we can include dynamicexception.h only there, OR get rid of the exception altogether
    if (start + size > arr.size()) throw DynamicException("Invalid range for span");
    return View<Bytes>(arr.data() + start, size);
  }

  /**
   * Convert a string to const span.
   * @param str The string to convert.
   * @return The converted span.
   */
  inline View<Bytes> create_view_span(const std::string_view str) {
    return View<Bytes>(reinterpret_cast<const uint8_t*>(str.data()), str.size());
  }

  /**
   * Convert a substring to span.
   * @param str The string to convert.
   * @param start The start index of the substring.
   * @param size The size of the substring.
   * @return The converted span.
   * @throw DynamicException if range is invalid.
   */
  View<Bytes> create_view_span(const std::string_view str, size_t start, size_t size);

  /**
   * Append a vector to another.
   * This function is a workaround for calling insert() on a vector with a function
   * as a parameter, since that is not possible as we need to call begin() and end()
   * from the returned value, and we are argumenting two different function calls.
   * Inline is used for best performance.
   * @tparam T Can be either std::vector, std::string, std::span, or std::array.
   * @param vec The vector to append to.
   * @param bytes The vector to be appended.
   */
  template<typename T> inline void appendBytes(Bytes& vec, const T& bytes) {
    vec.insert(vec.end(), bytes.cbegin(), bytes.cend());
  }

  /**
   * Shorthand for obtaining a milliseconds-since-epoch uint64_t timestamp from std::chrono
   * @return Milliseconds elapsed since epoch.
   */
  inline uint64_t getCurrentTimeMillisSinceEpoch() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
  }

  /**
   * Given an UNIX signal number, return the name followed by the number in parenthesis.
   * @param signum The signal number.
   * @return A string containing the signal name (or "Unknown signal") and number.
   */
  std::string getSignalName(int signum);

  /**
   * Templated function for calculating 10^exponent
   */
  template <typename T>
  T exp10(const uint64_t& exponent) {
    T base = 10;  // Base 10 for decimal exponentiation
    if (exponent == 0) {
     return T(1);
    }
    return boost::multiprecision::pow(base, exponent);
  }
};

#endif  // UTILS_H
