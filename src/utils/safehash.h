/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#ifndef HASH_H
#define HASH_H

#include <memory>
#include <boost/asio/ip/address.hpp>
#include <boost/container_hash/hash.hpp>

#include "../bytes/join.h"
#include "../libs/wyhash.h"

#include "tx.h" // ecdsa.h -> utils.h -> strings.h, bytes/join.h, boost/asio/ip/address.hpp
#include "uintconv.h"

/**
 * Custom hashing implementation for use in `boost::unordered_flat_map`
 * We use the highest and fastest quality hash function available for size_t (64-bit) hashes (Wyhash)
 */
struct SafeHash {
  using is_transparent = void;

  ///@{
  /** Wrapper for `wyhash()`. */
  size_t operator()(const uint64_t& i) const {
    return wyhash(std::bit_cast<const void*>(&i), sizeof(i), 0, _wyp);
  }

  size_t operator()(const uint256_t& i) const {
    return (*this)(Hash(i));
  }

  size_t operator()(const std::string& str) const {
    return wyhash(str.c_str(), str.size(), 0, _wyp);
  }

  size_t operator()(const std::string_view& str) const {
    return wyhash(str.data(), str.size(), 0, _wyp);
  }

  size_t operator()(const Functor& functor) const {
    return functor.value; // Functor is already a hash. Just return it.
  }

  size_t operator()(const Hash& hash) const {
    return wyhash(std::bit_cast<const void*>(hash.data()), hash.size(), 0, _wyp);
  }

  template <typename T> size_t operator()(const std::shared_ptr<T>& ptr) const {
    return SafeHash()(*ptr->get());
  }

  size_t operator()(View<Bytes> data) const {
    return wyhash(data.data(), data.size(), 0, _wyp);
  }

  template<typename T, typename U> size_t operator()(const std::pair<T, U>& pair) const {
    size_t hash = (*this)(pair.first);
    boost::hash_combine(hash, pair.second);
    return hash;
  }

  template <typename Key, typename T> size_t operator()(const boost::unordered_flat_map<Key, T, SafeHash>& a) const {
    // TODO: replace this with wyhash somehow.
    return splitmix(std::hash<boost::unordered_flat_map<Key, T, SafeHash>>()(a));
  }

  size_t operator()(const std::pair<boost::asio::ip::address, uint16_t>& nodeId) const {
    // Make it compatible with SafeHash<Bytes>.
    Bytes bytes;
    if (nodeId.first.is_v4()) {
      bytes = Utils::makeBytes(bytes::join(
        nodeId.first.to_v4().to_bytes(), UintConv::uint16ToBytes(nodeId.second)
      ));
    } else {
      bytes = Utils::makeBytes(bytes::join(
        nodeId.first.to_v6().to_bytes(), UintConv::uint16ToBytes(nodeId.second)
      ));
    }
    return SafeHash()(bytes);
  }
  ///@}
};

struct SafeCompare {
  using is_transparent = void;

  constexpr bool operator()(View<Bytes> lhs, View<Bytes> rhs) const {
    return std::ranges::equal(lhs, rhs);
  }

  constexpr bool operator()(const std::pair<View<Bytes>, View<Bytes>>& lhs, const std::pair<View<Bytes>, View<Bytes>>& rhs) const {
    return (*this)(lhs.first, rhs.first) && (*this)(lhs.second, rhs.second);
  }
};

/**
 * [Fowler-Noll-Vo](https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function)
 * hash struct used within broadcast messages.
 * This skips compiler optimizations and forces the whole string to be hashed,
 * diminishing the chance of collisions (remember we're using 64-bit hashes).
 */
struct FNVHash {
  /**
   * Call operator.
   * @param s The string to hash.
   */
  size_t operator()(View<Bytes> s) const {
    size_t result = 2166136261U;
    for (auto it = s.begin(); it != s.end(); it++) result = (16777619 * result) ^ (*it);
    return result;
  }
};

#endif  // HASH_H
