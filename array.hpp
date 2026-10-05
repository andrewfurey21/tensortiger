#ifndef TENSOR_TIGER_ARRAY
#define TENSOR_TIGER_ARRAY

#include "utils.hpp"

template <typename T, u64 Capacity>
struct Array {
  Array() : size(0) {
    memset(data, 0, Capacity * sizeof(T));
  }

  Array(std::initializer_list<T> list) : size(list.size()) {
    if (list.size() > Capacity) std::terminate();
    std::copy(list.begin(), list.end(), data);
 }

  void push(const T& t) {
    if (size >= Capacity) std::terminate();
    data[size++] = t;
  }

  const T& operator[](u64 index) const = delete;
  const T& at(u64 index) const {
    if (index >= size) std::terminate();
    return data[index];
  }

  u64 size;
  T data[Capacity];
};

#endif
