#ifndef _COLLECTION_H
#define _COLLECTION_H

#include "element/element.h"

#include "hash.hpp"
#include "list.hpp"
#include "util.hpp"
#include <cstring>
#include <list>
#include <ostream>
#include <queue>
#include <stack>
#include <stdint.h>
#include <string>
#include <type_traits>
#include <vector>

#define INIT_CAP (4)
typedef enum Order {
  LT, // less than
  EQ, // equal
  GT  // greater than
} Order;

template <typename T> struct unbound_vector {
  static_assert(std::is_pointer<T>::value, "");
  std::vector<T> vec;

  using reference = typename std::vector<T>::reference;
  using const_reference = typename std::vector<T>::const_reference;
  using size_type = typename std::vector<T>::size_type;
  using difference_type = typename std::vector<T>::difference_type;
  using iterator = typename std::vector<T>::iterator;
  using const_iterator = typename std::vector<T>::const_iterator;

  unbound_vector() {}
  unbound_vector(size_type s, const T &v) : vec(s, v) {}

  size_type size() const { return vec.size(); }

  void clear() { vec.clear(); }

  reference at(size_type index) { return vec.at(index); }

  reference operator[](size_type index) { return vec[index]; }

  void resize(size_type sz) { vec.resize(sz); }

  void resize(size_type sz, const T &c) { vec.resize(sz, c); }

  iterator begin() noexcept { return vec.begin(); }
  const_iterator begin() const noexcept { return vec.begin(); }
  iterator end() noexcept { return vec.end(); }
  const_iterator end() const noexcept { return vec.end(); }

  T read(int index) const {
    if (0 <= index && index < vec.size())
      return vec.at(index);
    return nullptr;
  }

  void write(size_type index, const T &value) {
    if (index >= vec.size()) {
      vec.resize(index + 1, nullptr);
    }
    vec.at(index) = value;
  }
};

typedef intptr_t CollectionInt;

template <typename K, typename V>
inline std::ostream &operator<<(std::ostream &os,
                                const std::map<K, V> &rbt) {
  for (auto &v : rbt)
    os << v.second;
  return os;
}

struct DisjointSetForest {
  DisjointSetForest *parent;
  int rank;
  DisjointSetForest() {
    parent = this;
    rank = 0;
  }
};

DisjointSetForest *makeDisjointSetForest();
void freeDisjointSetForest(DisjointSetForest *x);
void initializeDisjointSetForest(DisjointSetForest *x);
DisjointSetForest *findDisjointSetForest(DisjointSetForest *x);
void unionDisjointSetForest(DisjointSetForest *x, DisjointSetForest *y);
Bool isInSameDisjointSetForest(DisjointSetForest *x, DisjointSetForest *y);

#endif