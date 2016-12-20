/*
 * Copyright (C) 2016 Olzhas Rakhimov
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/// @file ext.h
/// Helpful facilities as an extension to the standard library or Boost.

#ifndef SCRAM_SRC_EXT_H_
#define SCRAM_SRC_EXT_H_

#include <algorithm>
#include <memory>
#include <vector>

#include <boost/range/algorithm.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/range/iterator_range.hpp>

namespace ext {

/// Iterator adaptor for indication of container ``find`` call results.
/// Conveniently wraps common calls after ``find`` into implicit Boolean value.
///
/// @tparam Iterator  Iterator type belonging to the container.
template <class Iterator>
class find_iterator : public Iterator {
 public:
  /// Initializes the iterator as the result of ``find()``.
  ///
  /// @param[in] it  The result of ``find`` call.
  /// @param[in] it_end  The sentinel iterator indicator ``not-found``.
  find_iterator(Iterator&& it, const Iterator& it_end)
      : Iterator(std::forward<Iterator>(it)),
        found_(it != it_end) {}

  /// @returns true if the iterator indicates that the item is found.
  explicit operator bool() { return found_; }

 private:
  bool found_;  ///< Indicator of the lookup result.
};

/// Wraps ``container::find()`` calls for convenient and efficient lookup
/// with ``find_iterator`` adaptor.
///
/// @tparam T  Container type supporting ``find()`` and ``end()`` calls.
/// @tparam Ts  The argument types to the ``find()`` call.
///
/// @param[in] container  The container to operate upon.
/// @param[in] args  Arguments to the ``find()`` function.
///
/// @returns find_iterator wrapping the resultant iterator.
template <class T, typename... Ts>
auto find(T&& container, Ts&&... args) {
  auto it = container.find(std::forward<Ts>(args)...);
  return find_iterator<decltype(it)>(std::move(it), container.end());
}

/// Determines if two sorted ranges intersect.
/// This function is complementary to std::set_intersection
/// when the actual intersection container is not needed.
///
/// @tparam Iterator1  Forward iterator type of the first range.
/// @tparam Iterator2  Forward iterator type of the second range.
///
/// @param[in] first1  Start of the first range.
/// @param[in] last1  End of the first range.
/// @param[in] first2  Start of the second range.
/// @param[in] last2  End of the second range.
///
/// @returns true if the [first1, last1) and [first2, last2) ranges intersect.
template <typename Iterator1, typename Iterator2>
bool intersects(Iterator1 first1, Iterator1 last1,
                Iterator2 first2, Iterator2 last2) noexcept {
  while (first1 != last1 && first2 != last2) {
    if (*first1 < *first2) {
      ++first1;
    } else if (*first2 < *first1) {
      ++first2;
    } else {
      return true;
    }
  }
  return false;
}

/// False positive Clang warning in 3.8.
#define CLANG_UNUSED_TYPEDEF_BUG \
  defined(__clang__) && __clang_major__ == 3 && __clang_minor__ == 8

/// Range-based version of ``intersects``.
template <class SinglePassRange1, class SinglePassRange2>
bool intersects(const SinglePassRange1& rng1, const SinglePassRange2& rng2) {
#if CLANG_UNUSED_TYPEDEF_BUG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#endif
  BOOST_RANGE_CONCEPT_ASSERT(
      (boost::SinglePassRangeConcept<const SinglePassRange1>));
  BOOST_RANGE_CONCEPT_ASSERT(
      (boost::SinglePassRangeConcept<const SinglePassRange2>));
#if CLANG_UNUSED_TYPEDEF_BUG
#pragma clang diagnostic pop
#endif
  return intersects(boost::begin(rng1), boost::end(rng1),
                    boost::begin(rng2), boost::end(rng2));
}

/// Range-based versions of std algorithms missing in Boost.
/// @{
template <class SinglePassRange, class UnaryPredicate>
bool none_of(const SinglePassRange& rng, UnaryPredicate pred) {
  return boost::end(rng) == boost::find_if(rng, pred);
}
template <class SinglePassRange, class UnaryPredicate>
bool any_of(const SinglePassRange& rng, UnaryPredicate pred) {
  return !none_of(rng, pred);
}
template <class SinglePassRange, class UnaryPredicate>
bool all_of(const SinglePassRange& rng, UnaryPredicate pred) {
#if CLANG_UNUSED_TYPEDEF_BUG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#endif
  BOOST_RANGE_CONCEPT_ASSERT(
      (boost::SinglePassRangeConcept<const SinglePassRange>));
#if CLANG_UNUSED_TYPEDEF_BUG
#pragma clang diagnostic pop
#endif
  return boost::end(rng) ==
         std::find_if_not(boost::begin(rng), boost::end(rng), pred);
}
/// @}

/// Passes an unmanaged resource to a smart pointer
/// with automatic type deduction.
/// This is a helper function to avoid boilerplate code.
/// This helper would be unnecessary
/// if template arguments could be deduced from constructors.
///
/// @param[in] dumb_handle  A raw pointer to the resource.
///
/// @returns A smart pointer exclusively owning the resource.
template <typename T>
auto make_unique(T* dumb_handle) { return std::unique_ptr<T>(dumb_handle); }

/// Read forward-iterator for combination generation.
/// The combination generator guarantees the element order
/// to be the same as in the original source collection.
///
/// @tparam Iterator  A random access iterator type.
/// @tparam value_type  The container type to generate combinations into.
template <class Iterator,
          class value_type = std::vector<typename Iterator::value_type>>
class combination_iterator
    : public boost::iterator_facade<combination_iterator<Iterator>, value_type,
                                    boost::forward_traversal_tag, value_type> {
  friend class boost::iterator_core_access;

 public:
  /// Constructor for a range with N elements to choose from.
  ///
  /// @param[in] k  The number of elements to choose.
  /// @param[in] first1  The start of the range.
  /// @param[in] last1  The sentinel end of the range.
  combination_iterator(int k, Iterator first1, Iterator last1)
      : first1_(first1), bitmask_(std::distance(first1, last1)) {
    assert(k > 0 && "The choice must be positive.");
    assert(k <= std::distance(first1, last1) && "The choice can't exceed N.");
    std::fill_n(bitmask_.begin(), k, 1);
  }

  /// Constructs a special iterator to signal the end of generation.
  explicit combination_iterator(Iterator first1) : first1_(first1) {}

 private:
  /// Standard iterator functionality required by the facade facilities.
  /// @{
  void increment() {
    if (boost::prev_permutation(bitmask_) == false)
      bitmask_.clear();
  }
  bool equal(const combination_iterator& other) const {
    return first1_ == other.first1_ && bitmask_ == other.bitmask_;
  }
  value_type dereference() const {
    assert(!bitmask_.empty() && "Calling on the sentinel iterator.");
    value_type combination;
    for (int i = 0; i < bitmask_.size(); ++i) {
      if (bitmask_[i])
        combination.push_back(*std::next(first1_, i));
    }
    return combination;
  }
  /// @}
  Iterator first1_;           ///< The first element in the collection.
  std::vector<int> bitmask_;  ///< bit-mask for N elements.
};

/// Helper for N-choose-K combination generator range construction.
template <typename Iterator>
auto make_combination_generator(int k, Iterator first1, Iterator last1) {
  return boost::make_iterator_range(
      combination_iterator<Iterator>(k, first1, last1),
      combination_iterator<Iterator>(first1));
}

}  // namespace ext

#endif  // SCRAM_SRC_EXT_H_
