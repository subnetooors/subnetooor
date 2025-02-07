/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#ifndef ABI_H
#define ABI_H

#include "../utils/intconv.h"
#include "../utils/uintconv.h"
#include "../utils/strconv.h"
#include "../utils/utils.h" // FunctionTypes, libs/json.hpp -> string

/// Namespace for Solidity ABI-related operations.
namespace ABI {
  /**
   * Struct that contains the data for a contract method.
   * Inputs and outputs are encoded with ABI::FunctorEncoder::listArgumentTypesV.
   * For inputs, if the arg name is missing it will be replaced with an empty string.
   * Tuples are encoded as (type1,type2,...,typeN), runtime splitting is required.
   */
  struct MethodDescription {
    std::string name; ///< Name of the method.
    std::vector<std::pair<std::string,std::string>> inputs; ///< List of pairs of method input types and names.
    std::vector<std::string> outputs; ///< Vector of method output types (no names).
    FunctionTypes stateMutability; ///< State mutability of the method.
    std::string type; ///< Type of the method.
  };

  /**
   * Struct that contains the data for a contract event.
   * Args are encoded with ABI::FunctorEncoder::listArgumentTypesV.
   * Follows the same rules as MethodDescription, but has an extra bool for indexed args.
   */
  struct EventDescription {
    std::string name; ///< Name of the event.
    std::vector<std::tuple<std::string, std::string, bool>> args; ///< List of tuples of event arg types, names and indexed flag.
    bool anonymous; ///< Whether the event is anonymous or not.
  };

  /// Common struct for functions used by both encoder and decoder.
  template<typename T> struct isTupleOfDynamicTypes;

  /// Forward declaration for std::tuple<Ts...>.
  template<typename... Ts> struct isTupleOfDynamicTypes<std::tuple<Ts...>>;

  /// Forward declaration for std::vector<T>.
  template<typename T> struct isTupleOfDynamicTypes<std::vector<T>>;

  /// Type trait to check if T is a std::vector (defaults to false for types without args).
  template <typename T> struct isVector : std::false_type {};

  /// Type trait to check if T is a std::vector (defaults to true for types with args).
  template <typename... Args> struct isVector<std::vector<Args...>> : std::true_type {};

  /// Helper variable template for is_vector.
  template <typename T> inline constexpr bool isVectorV = isVector<T>::value;

  /// vectorElementType trait to get the element type of a vector.
  template <typename T> struct vectorElementType {};

  /// Getter for the element type of a vector.
  template <typename... Args> struct vectorElementType<std::vector<Args...>> {
    using type = typename std::vector<Args...>::value_type; ///< The element type of the vector.
  };

  /// Helper alias template for vector_element_type.
  template <typename T> using vectorElementTypeT = typename vectorElementType<T>::type;

  /// Helper to check if a type is a std::tuple (defaults to false for types without args).
  template <typename T> struct isTuple : std::false_type {};

  /// Helper to check if a type is a std::tuple (defaults to true for types with args).
  template<typename... Ts> struct isTuple<std::tuple<Ts...>> : std::true_type {};

  /// Helper to check if a type is a EventParam (defaults to false for types without args).
  template <typename T> struct isEventParam : std::false_type {};

  /// Helper to check if a type is a EventParam (defaults to true for types with args).
  template<typename... Ts> struct isEventParam<EventParam<Ts...>> : std::true_type {};

  /**
   * Check if a type is dynamic.
   * @tparam T Any supported ABI type.
   * @return `true` if type is dymanic, `false` otherwise.
   */
  template<typename T> constexpr bool isDynamic() {
    if constexpr (
      std::is_same_v<T, Bytes> || std::is_same_v<T, View<Bytes>> || std::is_same_v<T, std::string> || false
    ) return true;
    if constexpr (isVectorV<T>) return true;
    if constexpr (isTupleOfDynamicTypes<T>::value) return true;
    return false;
  }

  /**
   * Check if a type is a tuple of dynamic types.
   * Default declaration for non tuple types.
   * @tparam T Any type.
   */
  template<typename T> struct isTupleOfDynamicTypes {
    static constexpr bool value = false; ///< Default value is false.
  };

  /**
   * Check if a type is a tuple of dynamic types.
   * Default declaration for std::tuple<Ts...>.
   * @tparam Ts Any list of types.
   */
  template<typename... Ts> struct isTupleOfDynamicTypes<std::tuple<Ts...>> {
    static constexpr bool value = (... || isDynamic<Ts>()); ///< For every type in Ts, check if it is dynamic. If it is, return true.
  };

  /**
   * Check if a std::vector contain a tuple of dynamic types.
   * @tparam T Any type.
   */
  template<typename T> struct isTupleOfDynamicTypes<std::vector<T>> {
    static constexpr bool value = isTupleOfDynamicTypes<T>::value; ///< For every type in T, check if it is dynamic. If it is, return true.
  };

  /**
   * Calculate the total next offset of a given tuple type.
   * @tparam T The tuple type to calculate from.
   * @return The total next offset.
   */
  template<typename T> constexpr uint64_t calculateOffsetForType() {
    if constexpr (isEventParam<T>::value) {
      if constexpr (T::isIndexed) return 0; /// Indexed types are skipped from encoding.
      return calculateOffsetForType<typename T::type>();
    }
    if constexpr (isDynamic<T>()) return 32;
    if constexpr (isTuple<T>::value) return 32 * std::tuple_size<T>::value;
    return 32;
  }

  /**
   * Calculate the total ABI offset start for dynamic types for a list of types.
   * @tparam Ts The types to calculate the offset for.
   * @return The total offset.
   */
  template <typename... Ts> constexpr uint64_t calculateTotalOffset() {
    return (calculateOffsetForType<Ts>() + ...);
  }

  /**
   * Append a Bytes piece to another Bytes piece.
   * @param dest The Bytes piece to append to.
   * @param src The Bytes piece to be appended.
   */
  template <typename T> void append(Bytes &dest, const T &src) {
    dest.insert(dest.end(), std::cbegin(src), std::cend(src));
  }

  /**
   * Always false constexpr function.
   * @tparam T Any type.
   * @return false
   */
  template<typename T>
  constexpr bool always_false() { return false; }

  /// Namespace for Functor encoding functions.
  namespace FunctorEncoder {
    /// @cond
    // General template for type to string conversion
    template<typename T, typename Enable = void>
    struct TypeName {
      static std::string get() {
        static_assert(std::is_same_v<T, void>, "TypeName specialization for this type is not defined");
        return "";
      }
    };

    // Specialization for all types
    template<> struct TypeName<uint8_t> { static std::string get() { return "uint8"; }};
    template<> struct TypeName<uint16_t> { static std::string get() { return "uint16"; }};
    template<> struct TypeName<uint24_t> { static std::string get() { return "uint24"; }};
    template<> struct TypeName<uint32_t> { static std::string get() { return "uint32"; }};
    template<> struct TypeName<uint40_t> { static std::string get() { return "uint40"; }};
    template<> struct TypeName<uint48_t> { static std::string get() { return "uint48"; }};
    template<> struct TypeName<uint56_t> { static std::string get() { return "uint56"; }};
    template<> struct TypeName<uint64_t> { static std::string get() { return "uint64"; }};
    template<> struct TypeName<uint72_t> { static std::string get() { return "uint72"; }};
    template<> struct TypeName<uint80_t> { static std::string get() { return "uint80"; }};
    template<> struct TypeName<uint88_t> { static std::string get() { return "uint88"; }};
    template<> struct TypeName<uint96_t> { static std::string get() { return "uint96"; }};
    template<> struct TypeName<uint104_t> { static std::string get() { return "uint104"; }};
    template<> struct TypeName<uint112_t> { static std::string get() { return "uint112"; }};
    template<> struct TypeName<uint120_t> { static std::string get() { return "uint120"; }};
    template<> struct TypeName<uint128_t> { static std::string get() { return "uint128"; }};
    template<> struct TypeName<uint136_t> { static std::string get() { return "uint136"; }};
    template<> struct TypeName<uint144_t> { static std::string get() { return "uint144"; }};
    template<> struct TypeName<uint152_t> { static std::string get() { return "uint152"; }};
    template<> struct TypeName<uint160_t> { static std::string get() { return "uint160"; }};
    template<> struct TypeName<uint168_t> { static std::string get() { return "uint168"; }};
    template<> struct TypeName<uint176_t> { static std::string get() { return "uint176"; }};
    template<> struct TypeName<uint184_t> { static std::string get() { return "uint184"; }};
    template<> struct TypeName<uint192_t> { static std::string get() { return "uint192"; }};
    template<> struct TypeName<uint200_t> { static std::string get() { return "uint200"; }};
    template<> struct TypeName<uint208_t> { static std::string get() { return "uint208"; }};
    template<> struct TypeName<uint216_t> { static std::string get() { return "uint216"; }};
    template<> struct TypeName<uint224_t> { static std::string get() { return "uint224"; }};
    template<> struct TypeName<uint232_t> { static std::string get() { return "uint232"; }};
    template<> struct TypeName<uint240_t> { static std::string get() { return "uint240"; }};
    template<> struct TypeName<uint248_t> { static std::string get() { return "uint248"; }};
    template<> struct TypeName<uint256_t> { static std::string get() { return "uint256"; }};

    template<> struct TypeName<int8_t> { static std::string get() { return "int8"; }};
    template<> struct TypeName<int16_t> { static std::string get() { return "int16"; }};
    template<> struct TypeName<int24_t> { static std::string get() { return "int24"; }};
    template<> struct TypeName<int32_t> { static std::string get() { return "int32"; }};
    template<> struct TypeName<int40_t> { static std::string get() { return "int40"; }};
    template<> struct TypeName<int48_t> { static std::string get() { return "int48"; }};
    template<> struct TypeName<int56_t> { static std::string get() { return "int56"; }};
    template<> struct TypeName<int64_t> { static std::string get() { return "int64"; }};
    template<> struct TypeName<int72_t> { static std::string get() { return "int72"; }};
    template<> struct TypeName<int80_t> { static std::string get() { return "int80"; }};
    template<> struct TypeName<int88_t> { static std::string get() { return "int88"; }};
    template<> struct TypeName<int96_t> { static std::string get() { return "int96"; }};
    template<> struct TypeName<int104_t> { static std::string get() { return "int104"; }};
    template<> struct TypeName<int112_t> { static std::string get() { return "int112"; }};
    template<> struct TypeName<int120_t> { static std::string get() { return "int120"; }};
    template<> struct TypeName<int128_t> { static std::string get() { return "int128"; }};
    template<> struct TypeName<int136_t> { static std::string get() { return "int136"; }};
    template<> struct TypeName<int144_t> { static std::string get() { return "int144"; }};
    template<> struct TypeName<int152_t> { static std::string get() { return "int152"; }};
    template<> struct TypeName<int160_t> { static std::string get() { return "int160"; }};
    template<> struct TypeName<int168_t> { static std::string get() { return "int168"; }};
    template<> struct TypeName<int176_t> { static std::string get() { return "int176"; }};
    template<> struct TypeName<int184_t> { static std::string get() { return "int184"; }};
    template<> struct TypeName<int192_t> { static std::string get() { return "int192"; }};
    template<> struct TypeName<int200_t> { static std::string get() { return "int200"; }};
    template<> struct TypeName<int208_t> { static std::string get() { return "int208"; }};
    template<> struct TypeName<int216_t> { static std::string get() { return "int216"; }};
    template<> struct TypeName<int224_t> { static std::string get() { return "int224"; }};
    template<> struct TypeName<int232_t> { static std::string get() { return "int232"; }};
    template<> struct TypeName<int240_t> { static std::string get() { return "int240"; }};
    template<> struct TypeName<int248_t> { static std::string get() { return "int248"; }};
    template<> struct TypeName<int256_t> { static std::string get() { return "int256"; }};

    template<> struct TypeName<Address> { static std::string get() { return "address"; }};
    template<> struct TypeName<bool> { static std::string get() { return "bool"; }};
    template<> struct TypeName<Bytes> { static std::string get() { return "bytes"; }};
    template<> struct TypeName<std::string> { static std::string get() { return "string"; }};
    template<> struct TypeName<Hash> { static std::string get() { return "bytes32"; }};

    template<size_t N> struct TypeName<FixedBytes<N>> { static std::string get() { return "bytes" + std::to_string(N); } };

    /// Enum types are encoded as uint8_t
    template<typename T>
    requires std::is_enum_v<T> struct TypeName<T> {
      static std::string get() {
        return TypeName<uint8_t>::get();
      }
    };
    /// @endcond

    /// Helper for tuple types
    template <typename Tuple, typename IndexSequence> struct TupleTypeNameHelper;

    /**
     * Helper that expands the tuple and calls TypeName for each element.
     * @tparam Tuple The tuple type
     * @tparam Is The index sequence
     */
    template <typename Tuple, std::size_t... Is>
    struct TupleTypeNameHelper<Tuple, std::index_sequence<Is...>> {
      /// Get the type name.
      static std::string get() {
        std::string result;
        ((result += TypeName<std::decay_t<std::tuple_element_t<Is, Tuple>>>::get() + ","), ...);
        if (!result.empty()) result.pop_back(); // Remove the last comma ","
        return result;
      }
    };

    /**
     * TypeName specialization for std::tuple.
     * @tparam Args The tuple types.
     */
    template<typename... Args> struct TypeName<std::tuple<Args...>> {
      /// Get the type name.
      static std::string get() {
        return "(" + TupleTypeNameHelper<std::tuple<Args...>, std::index_sequence_for<Args...>>::get() + ")";
      }
    };

    /**
     * TypeName specialization for std::vector.
     * @tparam T The vector type.
     */
    template<typename T> struct TypeName<std::vector<T>> {
      /// Get the type name.
      static std::string get() { return TypeName<T>::get() + "[]"; }
    };

    /**
     * List the argument types in a string, comma separated. Uses () for tuples and [] for arrays.
     * Example: listArgumentTypes<int, std::string, std::tuple<int, int>, std::vector<std::string>>() will result in "int,string,(int,int),string[]".
     * @tparam Args The argument types.
     * @return The string with the argument types.
     */
    template <typename... Args> static std::string listArgumentTypes() {
      std::string result;
      ((result += TypeName<std::decay_t<Args>>::get() + ","), ...);
      if (!result.empty()) result.pop_back(); // Remove the last comma ","
      return result;
    }

    /**
     * List the argument types in a vector of strings. Uses () for tuples and [] for arrays.
     * Example: listArgumentTypesV<int, std::string, std::tuple<int, int>, std::vector<std::string>>() will result in "{"int","string","(int,int)","string[]"}".
     * @tparam Args The argument types.
     * @return The vector with the argument types.
     */
    template<typename... Args>
    static std::vector<std::string> listArgumentTypesV() {
      std::vector<std::string> result;
      ((result.emplace_back(TypeName<std::decay_t<Args>>::get())), ...);
      return result;
    }

    /// Forward declaration.
    template <typename... Args> struct listEventTypesV;

    /**
     * Same as listArgumentTypesV(), but specializes for event functions,
     * since we need to add the indexed flag to the type.
     * Needs to take EventParam<Args, isIndexed>...
     */
    template <typename... Args, bool... Flags>
    struct listEventTypesV<EventParam<Args, Flags>...> {
      /// Get the underlying event param data.
      static std::vector<std::pair<std::string, bool>> get() {
        std::vector<std::pair<std::string, bool>> result;
        ((result.emplace_back(TypeName<std::decay_t<Args>>::get(), Flags)), ...);
        return result;
      }
    };

    /**
     * Helper function for listArgumentTypesVFromTuple.
     * @tparam Tuple The tuple type
     * @tparam I The index sequence
     * @return The vector with the argument types
     */
    template<typename Tuple, std::size_t... I>
    static std::vector<std::string> unpackTupleAndListTypesV(std::index_sequence<I...>) {
      return listArgumentTypesV<std::tuple_element_t<I, Tuple>...>();
    }

    /**
     * Same as listArgumentTypesV, but takes a tuple as template parameter.
     * @tparam Tuple The tuple type
     * @return The vector with the argument types
     */
    template<typename Tuple> static std::vector<std::string> listArgumentTypesVFromTuple() {
      return unpackTupleAndListTypesV<Tuple>(std::make_index_sequence<std::tuple_size_v<Tuple>>{});
    }

    /**
     * Encode a function signature following Solidity rules.
     * @tparam Args The argument types.
     * @param funcSig The function signature (name).
     */
    template <typename... Args> static Functor encode(const std::string& funcSig) {
      Functor ret;
      std::string fullSig = funcSig + "(" + listArgumentTypes<Args...>() + ")";
      auto hash = Utils::sha3(Utils::create_view_span(fullSig));
      ret.value = UintConv::bytesToUint32(hash.view(0,4));
      return ret;
    }
  }; // namespace FunctorEncoder

  /// Namespace for ABI-encoding functions.
  namespace Encoder {
    /**
     * Encode a uint256.
     * @param num The input to encode.
     * @return The encoded input.
     */
    Bytes encodeUint(const uint256_t& num);

    /**
     * Encode an int256.
     * @param num The input to encode.
     * @return The encoded input.
     */
    Bytes encodeInt(const int256_t& num);

    ///@cond
    // General template for encoding type to bytes
    template<typename T, typename Enable = void>
    struct TypeEncoder {
      static Bytes encode(const T&) {
        static_assert(std::is_same_v<T, void>, "TypeName specialization for this type is not defined");
        return Bytes();
      }
    };

    // Specialization for default solidity types
    template <> struct TypeEncoder<Address> { static Bytes encode(const Address& add) { return StrConv::padLeftBytes(add, 32); }};
    template <> struct TypeEncoder<bool> { static Bytes encode(const bool& b) { return StrConv::padLeftBytes((b ? Bytes{0x01} : Bytes{0x00}), 32); }};
    template <> struct TypeEncoder<Hash> { static Bytes encode(const Hash& hash) { return hash.asBytes(); }; };
    template <> struct TypeEncoder<Bytes> {
      static Bytes encode(const Bytes& bytes) {
        int pad = 0;
        do { pad += 32; } while (pad < bytes.size());
        Bytes len = StrConv::padLeftBytes(Utils::uintToBytes(bytes.size()), 32);
        Bytes data = StrConv::padRightBytes(bytes, pad);
        len.reserve(len.size() + data.size());
        len.insert(len.end(), std::make_move_iterator(data.begin()), std::make_move_iterator(data.end()));
        return len;
      }
    };

    template <size_t N> struct TypeEncoder<FixedBytes<N>> {
      static_assert(N <= 32);

      static Bytes encode(const FixedBytes<N>& bytes) {
        Bytes result(32);
        std::ranges::copy(bytes, result.begin());
        return result;
      }
    };

    template <> struct TypeEncoder<std::string> {
      static Bytes encode(const std::string& str) {
        View<Bytes> bytes = Utils::create_view_span(str);
        int pad = 0;
        do { pad += 32; } while (pad < bytes.size());
        Bytes len = StrConv::padLeftBytes(Utils::uintToBytes(bytes.size()), 32);
        Bytes data = StrConv::padRightBytes(bytes, pad);
        len.reserve(len.size() + data.size());
        len.insert(len.end(), std::make_move_iterator(data.begin()), std::make_move_iterator(data.end()));
        return len;
      }
    };

    // Specializations for int types (int8_t, int16_t, int24_t, ..., int256_t)
    // Takes advantage of requires to check if the type is a or int.
    template<typename T>
    requires std::is_same_v<T, int8_t> || std::is_same_v<T, int16_t> || std::is_same_v<T, int24_t> ||
      std::is_same_v<T, int32_t> || std::is_same_v<T, int40_t> || std::is_same_v<T, int48_t> ||
      std::is_same_v<T, int56_t> || std::is_same_v<T, int64_t> || std::is_same_v<T, int72_t> ||
      std::is_same_v<T, int80_t> || std::is_same_v<T, int88_t> || std::is_same_v<T, int96_t> ||
      std::is_same_v<T, int104_t> || std::is_same_v<T, int112_t> || std::is_same_v<T, int120_t> ||
      std::is_same_v<T, int128_t> || std::is_same_v<T, int136_t> || std::is_same_v<T, int144_t> ||
      std::is_same_v<T, int152_t> || std::is_same_v<T, int160_t> || std::is_same_v<T, int168_t> ||
      std::is_same_v<T, int176_t> || std::is_same_v<T, int184_t> || std::is_same_v<T, int192_t> ||
      std::is_same_v<T, int200_t> || std::is_same_v<T, int208_t> || std::is_same_v<T, int216_t> ||
      std::is_same_v<T, int224_t> || std::is_same_v<T, int232_t> || std::is_same_v<T, int240_t> ||
      std::is_same_v<T, int248_t> || std::is_same_v<T, int256_t>
    struct TypeEncoder<T> {
      static Bytes encode(const T& i) {
        return encodeInt(i);
      }
    };

    // Specialization for uint types (uint8_t, uint16_t, uint24_t, ..., uint256_t)
    // Takes advantage of requires to check if the type is a or uint.
    template<typename T>
    requires std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t> || std::is_same_v<T, uint24_t> ||
      std::is_same_v<T, uint32_t> || std::is_same_v<T, uint40_t> || std::is_same_v<T, uint48_t> ||
      std::is_same_v<T, uint56_t> || std::is_same_v<T, uint64_t> || std::is_same_v<T, uint72_t> ||
      std::is_same_v<T, uint80_t> || std::is_same_v<T, uint88_t> || std::is_same_v<T, uint96_t> ||
      std::is_same_v<T, uint104_t> || std::is_same_v<T, uint112_t> || std::is_same_v<T, uint120_t> ||
      std::is_same_v<T, uint128_t> || std::is_same_v<T, uint136_t> || std::is_same_v<T, uint144_t> ||
      std::is_same_v<T, uint152_t> || std::is_same_v<T, uint160_t> || std::is_same_v<T, uint168_t> ||
      std::is_same_v<T, uint176_t> || std::is_same_v<T, uint184_t> || std::is_same_v<T, uint192_t> ||
      std::is_same_v<T, uint200_t> || std::is_same_v<T, uint208_t> || std::is_same_v<T, uint216_t> ||
      std::is_same_v<T, uint224_t> || std::is_same_v<T, uint232_t> || std::is_same_v<T, uint240_t> ||
      std::is_same_v<T, uint248_t> || std::is_same_v<T, uint256_t>
    struct TypeEncoder<T> {
      static Bytes encode(const T& i) {
        return encodeUint(i);
      }
    };

    // Specialization for enum types
    template <typename T>
    requires std::is_enum_v<T>
    struct TypeEncoder<T> {
      static Bytes encode(const T& i) {
        return encodeUint(static_cast<uint8_t>(i));
      }
    };

    // Forward declaration of TypeEncode<std::vector<T>> so TypeEncoder<std::tuple<Ts...>> can see it.
    template <typename T> struct TypeEncoder<std::vector<T>> {
      static Bytes encode(const std::vector<T>& v);
    };

    // Specialization for std::tuple<T>
    template <typename... Ts> struct TypeEncoder<std::tuple<Ts...>> {
      static Bytes encode(const std::tuple<Ts...>& t) {
        Bytes result;
        Bytes dynamicBytes;
        uint64_t nextOffset = calculateTotalOffset<Ts...>();
        std::apply([&result, &dynamicBytes, &nextOffset](const auto&... args) {
          auto encodeItem = [&]<typename ItemType>(const ItemType& item) {
            if (isDynamic<ItemType>()) {
              Bytes packed = TypeEncoder<ItemType>::encode(item);
              append(result, StrConv::padLeftBytes(Utils::uintToBytes(nextOffset), 32));
              nextOffset += packed.size();
              dynamicBytes.insert(dynamicBytes.end(), packed.begin(), packed.end());
            } else {
              append(result, TypeEncoder<ItemType>::encode(item));
            }
          };
          (encodeItem(args), ...);
        }, t);
        result.insert(result.end(), dynamicBytes.begin(), dynamicBytes.end());
        return result;
      }
    };

    // Specialization for std::pair<T, U>
    template<typename T, typename U>
    struct TypeEncoder<std::pair<T, U>> {
      static Bytes encode(const std::pair<T, U>& p) {
        using Tuple = std::tuple<const T&, const U&>;
        return TypeEncoder<Tuple>::encode(Tuple(p.first, p.second));
      }
    };

    // Specialization for std::vector<T>
    template <typename T>
    Bytes TypeEncoder<std::vector<T>>::encode(const std::vector<T>& v) {
      Bytes result;
      uint64_t nextOffset = 32 * v.size();  // First 32 bytes are the length of the dynamic array
      if constexpr (isDynamic<T>()) {
        // If the vector is dynamic, we need to account the offsets of each tuple
        Bytes dynamicData;
        Bytes dynamicOffSets;

        // Encode each item within the vector
        for (const auto& t : v) {
          append(dynamicOffSets, UintConv::uint256ToBytes(nextOffset));
          Bytes dynamicBytes = TypeEncoder<T>::encode(t);  // We're calling the encode function specialized for the T type.
          nextOffset += dynamicBytes.size();
          dynamicData.insert(dynamicData.end(), dynamicBytes.begin(), dynamicBytes.end());
        }

        // Add the array length, dynamic offsets and dynamic data
        append(result, StrConv::padLeftBytes(Utils::uintToBytes(v.size()), 32));
        append(result, dynamicOffSets);
        result.insert(result.end(), dynamicData.begin(), dynamicData.end());
        return result;
      } else {
        // Add array length and append
        append(result, StrConv::padLeftBytes(Utils::uintToBytes(v.size()), 32));
        for (const auto& t : v) append(result, TypeEncoder<T>::encode(t));
        return result;
      }
    };
    ///@endcond

    /**
     * The main encode function. Use this one.
     * @tparam T Any supported ABI type (first one).
     * @tparam Ts Any supported ABI type (any other).
     * @param first First type to encode.
     * @param rest The rest of the types to encode, if any.
     * @return The encoded data.
     */
    template<typename T, typename... Ts> Bytes encodeData(const T& first, const Ts&... rest) {
      Bytes result;
      uint64_t nextOffset = calculateTotalOffset<T, Ts...>();
      Bytes dynamicBytes;
      auto encodeItem = [&]<typename ItemType>(const ItemType& item) {
        if constexpr (isDynamic<ItemType>()) {
          Bytes packed = TypeEncoder<ItemType>::encode(item);
          append(result, StrConv::padLeftBytes(Utils::uintToBytes(nextOffset), 32));
          nextOffset += packed.size();
          dynamicBytes.insert(dynamicBytes.end(), packed.begin(), packed.end());
        } else append(result, TypeEncoder<ItemType>::encode(item));
      };
      encodeItem(first);
      (encodeItem(rest), ...);
      result.insert(result.end(), dynamicBytes.begin(), dynamicBytes.end());
      return result;
    }

    Bytes encodeError(std::string_view reason);
  }; // namespace Encoder

  /**
   * EventEncoder works similarly to FunctionEncoder and the ABI Encoder itself,
   * but with different data structures and rules.
   */
  namespace EventEncoder {
    /**
     * Encode a event signature following Solidity rules.
     * @tparam Args The argument types.
     * @param funcSig The event signature (name).
     */
    template <typename... Args> static Hash encodeSignature(const std::string& funcSig) {
      std::string fullSig = funcSig + "(" + FunctorEncoder::listArgumentTypes<Args...>() + ")";
      return Utils::sha3(Utils::create_view_span(fullSig));
    }

    ///@cond
    template<typename T, typename Enable = void>
    struct TypeEncoder {
      static Bytes encode(const T&) {
        static_assert(std::is_same_v<T, void>, "TypeName specialization for this type is not defined");
        return Bytes();
      }
    };

    // Specialization for default solidity types
    template <> struct TypeEncoder<Address> { static Bytes encode(const Address& add) { return Encoder::TypeEncoder<Address>::encode(add); }};
    template <> struct TypeEncoder<bool> { static Bytes encode(const bool& b) { return Encoder::TypeEncoder<bool>::encode(b); }};
    template <> struct TypeEncoder<Bytes> {
      static Bytes encode(const Bytes& bytes) {
        // Almost the same as ABI::Encoder::encode, but without the padding.
        int pad = 0;
        do { pad += 32; } while (pad < bytes.size());
        return StrConv::padRightBytes(bytes, pad);
      }
    };

    template <> struct TypeEncoder<std::string> {
      static Bytes encode(const std::string& str) {
        View<Bytes> bytes = Utils::create_view_span(str);
        int pad = 0;
        do { pad += 32; } while (pad < bytes.size());
        return StrConv::padRightBytes(bytes, pad);
      }
    };

    // Specializations for int types (int8_t, int16_t, int24_t, ..., int256_t)
    // Takes advantage of requires to check if the type is a or int.
    template<typename T>
    requires std::is_same_v<T, int8_t> || std::is_same_v<T, int16_t> || std::is_same_v<T, int24_t> ||
      std::is_same_v<T, int32_t> || std::is_same_v<T, int40_t> || std::is_same_v<T, int48_t> ||
      std::is_same_v<T, int56_t> || std::is_same_v<T, int64_t> || std::is_same_v<T, int72_t> ||
      std::is_same_v<T, int80_t> || std::is_same_v<T, int88_t> || std::is_same_v<T, int96_t> ||
      std::is_same_v<T, int104_t> || std::is_same_v<T, int112_t> || std::is_same_v<T, int120_t> ||
      std::is_same_v<T, int128_t> || std::is_same_v<T, int136_t> || std::is_same_v<T, int144_t> ||
      std::is_same_v<T, int152_t> || std::is_same_v<T, int160_t> || std::is_same_v<T, int168_t> ||
      std::is_same_v<T, int176_t> || std::is_same_v<T, int184_t> || std::is_same_v<T, int192_t> ||
      std::is_same_v<T, int200_t> || std::is_same_v<T, int208_t> || std::is_same_v<T, int216_t> ||
      std::is_same_v<T, int224_t> || std::is_same_v<T, int232_t> || std::is_same_v<T, int240_t> ||
      std::is_same_v<T, int248_t> || std::is_same_v<T, int256_t>
    struct TypeEncoder<T> {
      static Bytes encode(const T& i) {
        return ABI::Encoder::encodeInt(i);
      }
    };

    // Specialization for uint types (uint8_t, uint16_t, uint24_t, ..., uint256_t)
    // Takes advantage of requires to check if the type is a or uint.
    template<typename T>
    requires std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t> || std::is_same_v<T, uint24_t> ||
      std::is_same_v<T, uint32_t> || std::is_same_v<T, uint40_t> || std::is_same_v<T, uint48_t> ||
      std::is_same_v<T, uint56_t> || std::is_same_v<T, uint64_t> || std::is_same_v<T, uint72_t> ||
      std::is_same_v<T, uint80_t> || std::is_same_v<T, uint88_t> || std::is_same_v<T, uint96_t> ||
      std::is_same_v<T, uint104_t> || std::is_same_v<T, uint112_t> || std::is_same_v<T, uint120_t> ||
      std::is_same_v<T, uint128_t> || std::is_same_v<T, uint136_t> || std::is_same_v<T, uint144_t> ||
      std::is_same_v<T, uint152_t> || std::is_same_v<T, uint160_t> || std::is_same_v<T, uint168_t> ||
      std::is_same_v<T, uint176_t> || std::is_same_v<T, uint184_t> || std::is_same_v<T, uint192_t> ||
      std::is_same_v<T, uint200_t> || std::is_same_v<T, uint208_t> || std::is_same_v<T, uint216_t> ||
      std::is_same_v<T, uint224_t> || std::is_same_v<T, uint232_t> || std::is_same_v<T, uint240_t> ||
      std::is_same_v<T, uint248_t> || std::is_same_v<T, uint256_t>
    struct TypeEncoder<T> {
      static Bytes encode(const T& i) {
        return ABI::Encoder::encodeUint(i);
      }
    };

    // Specialization for enum types
    template <typename T> requires std::is_enum_v<T> struct TypeEncoder<T> {
      static Bytes encode(const T& i) { return ABI::Encoder::encodeUint(static_cast<uint8_t>(i)); }
    };

    // Forward declaration of TypeEncode<std::vector<T>> so TypeEncoder<std::tuple<Ts...>> can see it.
    template <typename T> struct TypeEncoder<std::vector<T>> {
      static Bytes encode(const std::vector<T>& v);
    };

    /// Specialization for std::tuple<T>
    template <typename... Ts> struct TypeEncoder<std::tuple<Ts...>> {
      static Bytes encode(const std::tuple<Ts...>& t) {
        Bytes result;
        std::apply([&result](const auto&... args) {
          auto encodeItem = [&]<typename ItemType>(const ItemType& item) {
            append(result, TypeEncoder<ItemType>::encode(item));
          };
          (encodeItem(args), ...);
        }, t);
        return result;
      }
    };

    // Specialization for std::vector<T>
    template <typename T> Bytes TypeEncoder<std::vector<T>>::encode(const std::vector<T>& v) {
      Bytes result;
      for (const T& item : v) append(result, TypeEncoder<T>::encode(item));
      return result;
    };
    ///@endcond

    /**
     * Encode an indexed parameter for topic storage, as specified here:
     * https://docs.soliditylang.org/en/develop/abi-spec.html#events
     * https://docs.soliditylang.org/en/develop/abi-spec.html#indexed-event-encoding
     * @tparam T Any supported ABI type (first one, it is either a single type a struct)
     * @param item The parameter to encode.
     * @return The encoded data.
     */
    template<typename T> Hash encodeTopicSignature(const T& item) {
      // If it is a string or Bytes, hash the string itself without encoding.
      if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, Bytes>) {
        return Utils::sha3(Utils::create_view_span(item));
      }
      Bytes result = TypeEncoder<T>::encode(item);
      if constexpr (isTuple<T>::value || isVector<T>::value) {
        return Utils::sha3(result); // If it is a dynamic type, hash the encoded result.
      }
      return Hash(result);
    }

    /// Similar to ABI::Encoder::Encode, but instead takes std::tuple<EventParam...> as input.
    template <typename... Args, bool... Flags>
    Bytes encodeEventData(const std::tuple<EventParam<Args, Flags>...>& params) {
      Bytes result;
      uint64_t nextOffset = calculateTotalOffset<Args...>();
      Bytes dynamicBytes;
      auto encodeItem = [&]<typename EventParamType>(const EventParamType& item) {
        if constexpr (EventParamType::isIndexed) return;
        using ItemType = std::decay_t<decltype(item.value)>;
        if constexpr (isDynamic<ItemType>()) {
          Bytes packed = ABI::Encoder::TypeEncoder<ItemType>::encode(item.value);
          append(result, StrConv::padLeftBytes(Utils::uintToBytes(nextOffset), 32));
          nextOffset += packed.size();
          dynamicBytes.insert(dynamicBytes.end(), packed.begin(), packed.end());
        } else append(result, ABI::Encoder::TypeEncoder<ItemType>::encode(item.value));
      };
      std::apply([&encodeItem](const auto&... args) {
        (encodeItem(args), ...);
      }, params);

      result.insert(result.end(), dynamicBytes.begin(), dynamicBytes.end());
      return result;
    }

  }; // namespace EventEncoder

  /// Namespace for ABI-decoding functions.
  namespace Decoder {
    /**
     * Decode a uint256.
     * @param bytes The data string to decode.
     * @param index The point on the encoded string to start decoding.
     * @return The decoded data.
     * @throw std::length_error if data is too short for the type.
     */
    uint256_t decodeUint(const View<Bytes>& bytes, uint64_t& index);

    /**
     * Decode an int256.
     * @param bytes The data string to decode.
     * @param index The point on the encoded string to start decoding.
     * @return The decoded data.
     * @throw std::length_error if data is too short for the type.
     */
    int256_t decodeInt(const View<Bytes>& bytes, uint64_t& index);

    /// @cond
    // General template for bytes to type decoding
    template<typename T, typename Enable = void> struct TypeDecoder {
      static T decode(const View<Bytes>&, const uint64_t&) {
        static_assert(always_false<T>, "TypeName specialization for this type is not defined");
        return T();
      }
    };

    // Specialization for default solidity types
    template <> struct TypeDecoder<Address> {
      static Address decode(const View<Bytes>& bytes, uint64_t& index) {
        if (index + 32 > bytes.size()) throw std::length_error("Data too short for address");
        auto result = Address(bytes.subspan(index + 12, 20));
        index += 32;
        return result;
      }
    };

    template <> struct TypeDecoder<Hash> {
      static Hash decode(const View<Bytes>& bytes, uint64_t& index) {
        if (index + 32 > bytes.size()) { throw std::length_error("Data too short for hash"); }
        auto result = Hash(bytes.subspan(index, 32));
        index += 32;
        return result;
      }
    };

    template <> struct TypeDecoder<bool> {
      static bool decode(const View<Bytes>& bytes, uint64_t& index) {
        if (index + 32 > bytes.size()) throw std::length_error("Data too short for bool");
        bool result = (bytes[index + 31] == 0x01);
        index += 32;
        return result;
      }
    };

    template <> struct TypeDecoder<Bytes> {
      static Bytes decode(const View<Bytes>& bytes, uint64_t& index) {
        if (index + 32 > bytes.size()) throw std::length_error("Data too short for bytes");
        Bytes tmp(bytes.begin() + index, bytes.begin() + index + 32);
        uint64_t bytesStart = Utils::fromBigEndian<uint64_t>(tmp);
        index += 32;

        // Get bytes length
        tmp.clear();
        if (bytesStart + 32 > bytes.size()) throw std::length_error("Data too short for bytes");
        tmp.insert(tmp.end(), bytes.begin() + bytesStart, bytes.begin() + bytesStart + 32);
        uint64_t bytesLength = Utils::fromBigEndian<uint64_t>(tmp);

        // Size sanity check
        if (bytesStart + 32 + bytesLength > bytes.size()) throw std::length_error("Data too short for bytes");

        // Get bytes data
        tmp.clear();
        tmp.insert(tmp.end(), bytes.begin() + bytesStart + 32, bytes.begin() + bytesStart + 32 + bytesLength);
        return tmp;
      }
    };

    template<size_t N> struct TypeDecoder<FixedBytes<N>> {
      static_assert(N <= 32);

      static FixedBytes<N> decode(const View<Bytes>& bytes, uint64_t& index) {
        if (index + 32 > bytes.size())
          throw std::length_error("Data too short for bytes");

        FixedBytes<N> result;
        std::ranges::copy(bytes.subspan(index, N), result.begin());
        return result;
      }
    };

    template <> struct TypeDecoder<std::string> {
      static std::string decode(const View<Bytes>& bytes, uint64_t& index) {
        if (index + 32 > bytes.size()) throw std::length_error("Data too short for string 1");
        std::string tmp(bytes.begin() + index, bytes.begin() + index + 32);
        uint64_t bytesStart = Utils::fromBigEndian<uint64_t>(tmp);
        index += 32;  // Move index to next 32 bytes

        // Get bytes length
        tmp.clear();
        if (bytesStart + 32 > bytes.size()) throw std::length_error("Data too short for string 2");
        tmp.insert(tmp.end(), bytes.begin() + bytesStart, bytes.begin() + bytesStart + 32);
        uint64_t bytesLength = Utils::fromBigEndian<uint64_t>(tmp);

        // Size sanity check
        if (bytesStart + 32 + bytesLength > bytes.size()) throw std::length_error("Data too short for string 3");

        // Get bytes data
        tmp.clear();
        tmp.insert(tmp.end(), bytes.begin() + bytesStart + 32, bytes.begin() + bytesStart + 32 + bytesLength);
        return tmp;
      }
    };

    // Specializations for int types (int8_t, int16_t, int24_t, ..., int256_t)
    // Takes advantage of requires to check if the type is a or int.
    template<typename T>
    requires std::is_same_v<T, int8_t> || std::is_same_v<T, int16_t> || std::is_same_v<T, int24_t> ||
      std::is_same_v<T, int32_t> || std::is_same_v<T, int40_t> || std::is_same_v<T, int48_t> ||
      std::is_same_v<T, int56_t> || std::is_same_v<T, int64_t> || std::is_same_v<T, int72_t> ||
      std::is_same_v<T, int80_t> || std::is_same_v<T, int88_t> || std::is_same_v<T, int96_t> ||
      std::is_same_v<T, int104_t> || std::is_same_v<T, int112_t> || std::is_same_v<T, int120_t> ||
      std::is_same_v<T, int128_t> || std::is_same_v<T, int136_t> || std::is_same_v<T, int144_t> ||
      std::is_same_v<T, int152_t> || std::is_same_v<T, int160_t> || std::is_same_v<T, int168_t> ||
      std::is_same_v<T, int176_t> || std::is_same_v<T, int184_t> || std::is_same_v<T, int192_t> ||
      std::is_same_v<T, int200_t> || std::is_same_v<T, int208_t> || std::is_same_v<T, int216_t> ||
      std::is_same_v<T, int224_t> || std::is_same_v<T, int232_t> || std::is_same_v<T, int240_t> ||
      std::is_same_v<T, int248_t> || std::is_same_v<T, int256_t>
    struct TypeDecoder<T> {
      static T decode(const View<Bytes>& bytes, uint64_t& index) {
        return static_cast<T>(decodeInt(bytes, index));
      }
    };

    // Specialization for uint types (uint8_t, uint16_t, uint24_t, ..., uint256_t)
    // Takes advantage of requires to check if the type is a or uint.
    template <typename T>
    requires std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t> || std::is_same_v<T, uint24_t> ||
      std::is_same_v<T, uint32_t> || std::is_same_v<T, uint40_t> || std::is_same_v<T, uint48_t> ||
      std::is_same_v<T, uint56_t> || std::is_same_v<T, uint64_t> || std::is_same_v<T, uint72_t> ||
      std::is_same_v<T, uint80_t> || std::is_same_v<T, uint88_t> || std::is_same_v<T, uint96_t> ||
      std::is_same_v<T, uint104_t> || std::is_same_v<T, uint112_t> || std::is_same_v<T, uint120_t> ||
      std::is_same_v<T, uint128_t> || std::is_same_v<T, uint136_t> || std::is_same_v<T, uint144_t> ||
      std::is_same_v<T, uint152_t> || std::is_same_v<T, uint160_t> || std::is_same_v<T, uint168_t> ||
      std::is_same_v<T, uint176_t> || std::is_same_v<T, uint184_t> || std::is_same_v<T, uint192_t> ||
      std::is_same_v<T, uint200_t> || std::is_same_v<T, uint208_t> || std::is_same_v<T, uint216_t> ||
      std::is_same_v<T, uint224_t> || std::is_same_v<T, uint232_t> || std::is_same_v<T, uint240_t> ||
      std::is_same_v<T, uint248_t> || std::is_same_v<T, uint256_t>
    struct TypeDecoder<T> {
        static T decode(const View<Bytes>& bytes, uint64_t& index) {
          return static_cast<T>(decodeUint(bytes, index));
        }
    };

    // Specialization for enum types
    template <typename T> requires std::is_enum_v<T> struct TypeDecoder<T> {
      static T decode(const View<Bytes>& bytes, uint64_t& index) {
        return static_cast<T>(decodeUint(bytes, index));
      }
    };

    // Forward declaration of TypeDecode<std::vector<T>> so TypeDecoder<std::tuple<Ts...>> can see it.
    template <typename T> requires isVectorV<T> struct TypeDecoder<T> {
      static T decode(const View<Bytes>& bytes, uint64_t& index);
    };

    /**
     * Decode a packed std::tuple<Args...> individually.
     * Takes advantage of std::tuple_element and template recursion to parse all items within the given tuple.
     * @tparam TupleLike The std::tuple<Args...> structure.
     * @tparam I The current tuple index.
     * @param bytes The data string to decode.
     * @param index The point on the encoded string to start decoding.
     * @param ret The tuple object to "return". Needs to be a reference and created outside the function due to recursion.
     */
    template<typename TupleLike, size_t I = 0> void decodeTuple(const View<Bytes>& bytes, uint64_t& index, TupleLike& ret) {
      if constexpr (I < std::tuple_size_v<TupleLike>) {
        using SelectedType = typename std::tuple_element<I, TupleLike>::type;
        std::get<I>(ret) = TypeDecoder<SelectedType>::decode(bytes, index);
        decodeTuple<TupleLike, I + 1>(bytes, index, ret);
      }
    }

    // Specialization for std::tuple<Ts...>
    template <typename T> requires isTuple<T>::value struct TypeDecoder<T> {
      static T decode(const View<Bytes>& bytes, uint64_t& index) {
        T ret;
        if constexpr (isTupleOfDynamicTypes<T>::value) {
          if (index + 32 > bytes.size()) throw std::length_error("Data too short for tuple of dynamic types");
          Bytes tmp(bytes.begin() + index, bytes.begin() + index + 32);
          uint64_t offset = Utils::fromBigEndian<uint64_t>(tmp);
          index += 32;
          uint64_t newIndex = 0;
          auto view = bytes.subspan(offset);
          decodeTuple<T>(view, newIndex, ret);
          return ret;
        }
        if (index + 32 * std::tuple_size_v<T> > bytes.size()) throw std::length_error("Data too short for tuple");
        decodeTuple<T>(bytes, index, ret);
        return ret;
      }
    };

    /// Specialization for std::vector<T>
    template <typename T> requires isVectorV<T> T TypeDecoder<T>::decode(const View<Bytes>& bytes, uint64_t& index) {
      using ElementType = vectorElementTypeT<T>;
      std::vector<ElementType> retVector;

      // Get array offset
      if (index + 32 > bytes.size()) throw std::length_error("Data too short for vector");
      Bytes tmp(bytes.begin() + index, bytes.begin() + index + 32);
      uint64_t arrayStart = Utils::fromBigEndian<uint64_t>(tmp);
      index += 32;

      // Get array length
      tmp.clear();
      if (arrayStart + 32 > bytes.size()) throw std::length_error("Data too short for vector");
      tmp.insert(tmp.end(), bytes.begin() + arrayStart, bytes.begin() + arrayStart + 32);
      uint64_t arrayLength = Utils::fromBigEndian<uint64_t>(tmp);

      if (arrayStart + 32 > bytes.size()) throw std::length_error("Data too short for vector");
      uint64_t newIndex = 0;
      auto view = bytes.subspan(arrayStart + 32);
      for (uint64_t i = 0; i < arrayLength; i++) {
        retVector.emplace_back(TypeDecoder<ElementType>::decode(view, newIndex)); // Hic sunt recursis
      }
      return retVector;
    };
    ///@endcond

    /// Specialization of decodeTupleHelper() for when tuple index is the last one
    template<std::size_t Index = 0, typename... Args>
    requires (Index == sizeof...(Args))
    void decodeTupleHelper(const View<Bytes>&, const uint64_t&, std::tuple<Args...>&) {
      // End of recursion, do nothing
    }

    /**
     * Recursive helper function to decode each element of the tuple.
     * @tparam Index The current index in the tuple.
     * @tparam Args Tuple types.
     * @param encodedData The encoded data.
     * @param index The current position in the encoded data.
     * @param tuple The tuple to hold the decoded values.
     */
    template<std::size_t Index = 0, typename... Args>
    requires (Index < sizeof...(Args))
    void decodeTupleHelper(const View<Bytes>& encodedData, uint64_t& index, std::tuple<Args...>& tuple) {
      // TODO: Technically, we could pass std::get<Index>(tuple) as a reference to decode<>().
      // But, it is worth to reduce code readability for a few nanoseconds? Need to benchmark.
      std::get<Index>(tuple) = TypeDecoder<std::tuple_element_t<Index, std::tuple<Args...>>>::decode(encodedData, index);
      decodeTupleHelper<Index + 1, Args...>(encodedData, index, tuple);
    }

    /**
     * The main decode function. Use this one.
     * @tparam Args Any supported ABI type.
     * @param encodedData The full encoded data string to decode.
     * @param index The point on the encoded string to start decoding. Defaults to start of string (0).
     * @return A tuple with the decoded data, or an empty tuple if there's no arguments to decode.
     */
    template<typename... Args>
    inline std::tuple<Args...> decodeData(const View<Bytes>& encodedData, uint64_t index = 0) {
      if constexpr (sizeof...(Args) == 0) {
        return std::tuple<>();
      } else {
        std::tuple<Args...> ret;
        decodeTupleHelper<0, Args...>(encodedData, index, ret);
        return ret;
      }
    }

    std::string decodeError(View<Bytes> data);

    /// Specialization for tuples without args.
    template<typename T> struct decodeDataAsTuple {
      /// Decode the tuple.
      static T decode(const View<Bytes>&) {
        static_assert(always_false<T>, "Can't use decodeDataAsTuple with a non-tuple type");
        return T();
      }
    };

    /// Specialization for tuples with args.
    template<typename... Args> struct decodeDataAsTuple<std::tuple<Args...>> {
      /// Decode the tuple.
      static std::tuple<Args...> decode(const View<Bytes>& encodedData) {
        if constexpr (sizeof...(Args) == 0) {
          throw std::invalid_argument("Can't decode empty tuple");
        } else {
          return decodeData<Args...>(encodedData);
        }
      }
    };
  };  // namespace Decoder
}; // namespace ABI

#endif // ABI_H
