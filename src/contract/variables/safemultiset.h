/*
Copyright (c) [2023] [Sparq Network]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#ifndef SAFEMULTISET_H
#define SAFEMULTISET_H

#include <functional>
#include <set>
#include <utility>
#include "safebase.h"

// TODO: missing functions (not sure if necessary):
// merge, equal_range, operator==, operator<=>

/**
 * Safe wrapper for std::multiset.
 * @tparam Key Defines the type of the set key element.
 * @tparam Compare Defines the function object for performing comparisons.
 *                 Defaults to std::less (elements ordered from lower to higher).
 * @see SafeBase
 */
template <class Key, class Compare = std::less<Key>>  // TODO: Allocator? Do we need it?
class SafeMultiSet : public SafeBase {
  private:
    std::multiset<Key, Compare> set_;                           ///< The original set.
    mutable std::unique_ptr<std::multiset<Key, Compare>> tmp_;  ///< The temporary set.

    /// Check the tmp_ variable and initialize it if necessary.
    inline void check() const override {
      if (tmp_ == nullptr) tmp_ = std::make_unique<std::multiset<Key, Compare>>(set_);
    }

  public:
    /// Default Constructor.
    SafeMultiSet() : SafeBase(nullptr) {};

    /**
     * Constructor with owner.
     * @param owner The owner of the variable.
     */
    SafeMultiSet(DynamicContract* owner) : SafeBase(owner) {};

    /**
     * Constructor with iterators.
     * @param first Iterator to the start of the data.
     * @param end Iterator to the end of the data.
     */
    template<class InputIt> SafeMultiSet(InputIt first, InputIt last) {
      check(); tmp_->insert(first, last);
    }

    /**
     * Constructor with initializer list.
     * @param init The initializer list.
     */
    SafeMultiSet(std::initializer_list<Key> init) {
      check(); for (const auto& val : init) tmp_->emplace(val);
    }

    /// Copy constructor.
    SafeMultiSet(const SafeMultiSet& other) { check(); other.check(); *tmp_ = *(other.tmp_); }

    /// Return the TEMPORARY set const begin().
    inline std::multiset<Key>::iterator begin() const { check(); return tmp_->begin(); }

    /// Return the TEMPORARY set const end().
    inline std::multiset<Key>::iterator end() const { check(); return tmp_->end(); }

    /// Return the TEMPORARY set const crbegin().
    inline std::multiset<Key>::reverse_iterator rbegin() const { check(); return tmp_->rbegin(); }

    /// Return the TEMPORARY set const crend().
    inline std::multiset<Key>::reverse_iterator rend() const { check(); return tmp_->rend(); }

    /// Return the ORIGINAL set const begin().
    inline std::multiset<Key>::const_iterator cbegin() const { return set_.cbegin(); }

    /// Return the ORIGINAL set const end().
    inline std::multiset<Key>::const_iterator cend() const { return set_.cend(); }

    /// Return the ORIGINAL set const crbegin().
    inline std::multiset<Key>::const_reverse_iterator crbegin() const { return set_.crbegin(); }

    /// Return the ORIGINAL set const crend().
    inline std::multiset<Key>::const_reverse_iterator crend() const { return set_.crend(); }

    /// Check if temporary set is empty.
    inline bool empty() const { check(); return tmp_->empty(); }

    /// Get temporary set size.
    inline std::size_t size() const { check(); return tmp_->size(); }

    /// Get temporary set max_size.
    inline std::size_t max_size() const { check(); return tmp_->max_size(); }

    /// Clear sets.
    inline void clear() { check(); markAsUsed(); tmp_->clear(); }

    /**
     * Insert an element into the set.
     * @param value The value to insert
     * @return An iterator to the inserted position.
     */
    std::multiset<Key>::iterator insert(const Key& value) {
      check(); markAsUsed(); return tmp_->insert(value);
    }

    /// Move overload for insert().
    std::multiset<Key>::iterator insert(Key&& value) {
      check(); markAsUsed(); return tmp_->insert(value);
    }

    /// Iterator overload for insert().
    std::multiset<Key>::iterator insert(
      std::multiset<Key>::const_iterator pos, const Key& value
    ) {
      check(); markAsUsed(); return tmp_->insert(pos, value);
    }

    /// Iterator move overload for insert().
    std::multiset<Key>::iterator insert(
      std::multiset<Key>::const_iterator pos, Key&& value
    ) {
      check(); markAsUsed(); return tmp_->insert(pos, value);
    }

    /// Range iterator overload for insert().
    template<class InputIt> void insert(InputIt first, InputIt last) {
      check(); markAsUsed(); tmp_->insert(first, last);
    }

    /// Initializer list overload for insert().
    void insert(std::initializer_list<Key> ilist) {
      check(); markAsUsed(); tmp_->insert(ilist);
    }

    /**
     * Construct elements inplace in the temporary set.
     * @param args The elements to insert.
     * @return An iterator to the last inserted element.
     */
    template<class... Args> std::multiset<Key>::iterator emplace(Args&&... args) {
      check(); markAsUsed(); return tmp_->emplace(args...);
    }

    /**
     * Construct elements inplace using a hint.
     * @param hint The hint as to where to insert the elements.
     * @param args The elements to insert.
     * @return An iterator to the last inserted element.
n     */
    template<class... Args> std::multiset<Key>::iterator emplace_hint(
      std::multiset<Key>::const_iterator hint, Args&&... args
    ) {
      check(); markAsUsed(); return tmp_->emplace_hint(hint, args...);
    }

    /**
     * Erase an element from the set.
     * @param pos An iterator to the element to be erased.
     * @return An iterator to the first element following the erased one.
     */
    std::multiset<Key>::const_iterator erase(std::multiset<Key>::const_iterator pos) {
      check(); markAsUsed(); return tmp_->erase(pos);
    }

    /// Ranged overload of erase(). Erases [ first, last ) .
    std::multiset<Key>::const_iterator erase(
      std::multiset<Key>::const_iterator first, std::multiset<Key>::const_iterator last
    ) {
      check(); markAsUsed(); return tmp_->erase(first, last);
    }

    /// Element-specific overload of erase(). Returns the number of erased elements.
    size_t erase(const Key& key) {
      check(); markAsUsed(); return tmp_->erase(key);
    }

    /**
     * Swap elements with another set. Swaps only elements from the TEMPORARY sets.
     * @param other The set to swap with.
     */
    void swap(SafeMultiSet& other) {
      check(); other.check(); markAsUsed(); other.markAsUsed(); this->tmp_->swap(*other.tmp_);
    }

    /**
     * Extract an element from the temporary set. Get the value itself with `.value()`.
     * @param pos An iterator to the element to be extracted.
     * @return The extracted element.
     */
    std::multiset<Key>::node_type extract(std::multiset<Key>::iterator pos) {
      check(); markAsUsed(); return tmp_->extract(pos);
    }

    /// Element-specific overload of extract(), copy-wise.
    std::multiset<Key>::node_type extract(const Key& x) {
      check(); markAsUsed(); return tmp_->extract(x);
    }

    /// Element-specific overload of extract(), move-wise.
    std::multiset<Key>::node_type extract(Key&& x) {
      check(); markAsUsed(); return tmp_->extract(x);
    }

    /**
     * Count the number of elements that exist in the set.
     * @param key The key value to count.
     * @return The number of found elements.
     */
    size_t count(const Key& key) const { check(); return tmp_->count(key); }

    /**
     * Find an element in the temporary set.
     * @param key The key to search for.
     * @return An iterator to the found element.
     */
    std::multiset<Key>::const_iterator find(const Key& key) const {
      check(); return tmp_->find(key);
    }

    /**
     * Check if the set contains a given element.
     * @param key The key to check.
     * @return `true` if set contains the key, `false` otherwise.
     */
    bool contains(const Key& key) const { check(); return tmp_->contains(key); }

    /**
     * Get the first element that is not less than the given one.
     * @param key The key to use for comparison.
     * @return An iterator to the found element.
     */
    std::multiset<Key>::const_iterator lower_bound(const Key& key) {
      check(); return tmp_->lower_bound(key);
    }

    /**
     * Get the first element that is greater than the given one.
     * @param key The key to use for comparison.
     * @return An iterator to the found element.
     */
    std::multiset<Key>::const_iterator upper_bound(const Key& key) {
      check(); return tmp_->upper_bound(key);
    }

    /// Get the function that compares the keys (same as value_comp).
    std::multiset<Key>::key_compare key_comp() const {
      check();
      return tmp_->key_comp();
    }

    /// Get the function that compares the values (same as key_comp).
    std::multiset<Key>::value_compare value_comp() const {
      check();
      return tmp_->value_comp();
    }

    /**
     * Erase all elements that satisfy the predicate from the container.
     * @param pred The predicate that returns `true` if the element should be erased.
     * @return The number of erased elements.
     */
    template<class Pred> size_t erase_if(Pred pred) {
      check();
      size_t old_size = tmp_->size();
      for (auto first = tmp_->begin(), last = tmp_->end(); first != last;) {
        if (pred(*first)) { markAsUsed(); first = tmp_->erase(first); } else first++;
      }
      return old_size - tmp_->size();
    }

    /// Commit function.
    void commit() override {
      check();
      set_.clear();
      set_.insert(tmp_->begin(), tmp_->end());
      this->registered_ = false;
    }

    /// Rollback function.
    void revert() override {
      tmp_->clear();
      tmp_ = nullptr;
      this->registered_ = false;
    }

    /// Get the inner set (for const functions!)
    inline const std::multiset<Key>& get() const { return set_; }
};

#endif  // SAFEMULTISET_H
