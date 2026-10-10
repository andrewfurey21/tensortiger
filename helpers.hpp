
#ifndef TENSOR_TIGER_HELPERS
#define TENSOR_TIGER_HELPERS

#include "utils.hpp"

constexpr u64 DEFAULT_FNV_PRIME = 0x00000100000001B3;
constexpr u64 DEFAULT_FNV_OFFSET = 0xCBF29CE484222325;

template <typename T>
// TODO: how does stdlib deal with this.
// how to hash floats?? ok this doesn't work :')
u64 fnv_1a_hash(const T& t) {
  const u8 *data = reinterpret_cast<const u8 *>(&t);
  u64 size = sizeof(T);
  static_assert(sizeof(T) == 4);

  u64 hash = DEFAULT_FNV_OFFSET;
  for (u64 i = 0; i < size; i++) {
    hash ^= data[i];
    hash *= DEFAULT_FNV_PRIME;
  }

  return hash;
}

template <typename T, u64 Capacity>
struct Hash_Set {

  struct _Slot {
    enum class State { _, EMPTY, OCCUPIED, DELETED, } state = State::EMPTY;
    T v;
  };
  // Just want to see a warning.
  static_assert(sizeof(_Slot) < 64);

  Hash_Set() : size{}, data() {}

  Hash_Set(void *memory) : data((_Slot *)memory), size(0) {
    for (u64 i = 0; i < Capacity; i++) {
      new (data + i) _Slot;
    }
  }

  u64 size;
  _Slot data[Capacity];

  u64 hash(const T& t) { return fnv_1a_hash(t) % Capacity; }

  u64 next_index(u64 index) { return (index + 1) % Capacity; }

  // Returns the index of the value, or an empty slot.
  u64 find_empty_slot_or_value(const T& value) {
    if (data == nullptr) std::terminate();

    const u64 original_hash = hash(value);
    u64 pool_index = original_hash;

    while (data[pool_index].state == _Slot::State::OCCUPIED) {

      if (data[pool_index].v == value) return pool_index;

      pool_index = next_index(pool_index);
      assert(pool_index != original_hash &&
             "Looping when finding empty slot in set.");
    }
    return pool_index;
  }

  bool contains(const T& value) {
    u64 empty_slot_or_value_index = find_empty_slot_or_value(value);
    return data[empty_slot_or_value_index].state == _Slot::State::OCCUPIED;
  }

  u64 insert(const T& value) {
    if (size >= Capacity) std::terminate();

    const u64 pool_index = find_empty_slot_or_value(value);

    if (data[pool_index].state == _Slot::State::OCCUPIED)
      return pool_index;

    new (data + pool_index) _Slot { _Slot::State::OCCUPIED, value };
    size++;

    return pool_index;
  }

  void remove(const T& value) {
    const u64 pool_index = find_empty_slot_or_value(value);

    if (data[pool_index].state != _Slot::State::OCCUPIED)
      return;

    data[pool_index].v.~T();
    data[pool_index].state = _Slot::State::DELETED;
    size--;
  }

  u64 get_index(const T& value) {
    const u64 pool_index = find_empty_slot_or_value(value);

    if (data[pool_index].state != _Slot::State::OCCUPIED)
      std::terminate();

    return pool_index;
  }

  T& at(u64 index) {
    // TODO: panic_if(expr, f string, var args)
    // maybe look at c++23 std::stacktrace or linux backtrace
    assert(data[index].state == _Slot::State::OCCUPIED &&
           "This is not a valid entry in the set");
    return data[index].v;
  }

  void clear() {
    if (data == nullptr) return;

    for (u64 i = 0; i < Capacity; i++) {
      data[i].~_Slot();
      new (data + i) _Slot;
    }
  }
};

template <typename Key, typename Value, u64 Capacity>
struct Hash_Map {

  struct _Slot {
    enum class State { _, EMPTY, OCCUPIED, DELETED, } state = State::EMPTY;
    Key k {};
    Value v {};
  };

  Hash_Map() : size{}, data{} {
    for (u64 i = 0; i < Capacity; i++) {
      new (data + i) _Slot;
    }
  }

  u64 size;
  _Slot data[Capacity];

  u64 hash(const Key& key) const { return fnv_1a_hash(key) % Capacity; }

  // i wonder is there a nicer way to do this without mod
  // since index will never be greater than Capacity.
  u64 next_index(u64 index) const { return (index + 1) % Capacity; }

  // Returns the index of the value, or an empty slot.
  u64 find_empty_slot_or_value(const Key& key) const {
    const u64 original_hash = hash(key);
    u64 pool_index = original_hash;

    while (data[pool_index].state == _Slot::State::OCCUPIED) {

      if (data[pool_index].k == key) return pool_index;

      pool_index = next_index(pool_index);
      assert(pool_index != original_hash &&
             "Looping when finding empty slot in set.");
    }
    return pool_index;
  }

  bool contains(const Key& key) const {
    u64 empty_slot_or_value_index = find_empty_slot_or_value(key);
    return data[empty_slot_or_value_index].state == _Slot::State::OCCUPIED;
  }

  u64 insert(const Key& key, const Value& value) {
    if (size >= Capacity) std::terminate();

    const u64 pool_index = find_empty_slot_or_value(key);

    if (data[pool_index].state == _Slot::State::OCCUPIED) {
      data[pool_index].~_Slot();
    } else {
      size++;
    }

    new (data + pool_index) _Slot ( _Slot::State::OCCUPIED, key, value );

    return pool_index;
  }

  void remove(const Key& key) {
    const u64 pool_index = find_empty_slot_or_value(key);

    if (data[pool_index].state != _Slot::State::OCCUPIED)
      return;

    data[pool_index].k.~Key();
    data[pool_index].v.~Value();
    data[pool_index].state = _Slot::State::DELETED;
    size--;
  }

  u64 get_index(const Key& key) const {
    const u64 pool_index = find_empty_slot_or_value(key);

    if (data[pool_index].state != _Slot::State::OCCUPIED)
      std::terminate();

    return pool_index;
  }

  Value at(const Key& key) const {
    assert(contains(key) && "Hash_Map does not contain the key.");
    return data[get_index(key)].v;
  }

  void clear() {
    for (u64 i = 0; i < Capacity; i++) {
      data[i].~_Slot();
      new (data + i) _Slot();
    }
  }
};

template <typename T, u64 Capacity>
struct Array {

  u64 size;
  T data[Capacity];

  Array() : data({}), size(0) {
    memset(data, 0, Capacity * sizeof(T));
  }

  // are these heap allocated?
  Array(std::initializer_list<T> list) : data(), size(list.size()) {
    memset(data, 0, Capacity * sizeof(T));
    if (list.size() > Capacity) std::terminate();
    std::copy(list.begin(), list.end(), data);
 }

  void push(const T& t) {
    if (size >= Capacity) std::terminate();
    data[size++] = t;
  }

  const T& top() {
    assert(size != 0 && "Cannot get top from empty array.");
    return data[size - 1];
  }

  T pop() {
    assert(size != 0 && "Cannot pop from empty array.");
    size--;
    T v = std::move(data[size]);
    data[size].~T();
    return v;
  }

  const T& operator[](u64 index) const = delete;

  const T& at(u64 index) const {
    if (index >= size) std::terminate();
    return data[index];
  }

  bool operator==(const Array& other) const {
    if (size != other.size) return false;
    for (u32 i = 0; i < size; i++) {
      if (this->at(i) != other.at(i)) return false;
    }
    return true;
  }
};

template <typename T, u64 Capacity>
struct Queue {

  u64 start;
  u64 end;
  T data[Capacity];

  Queue() : start(0), end(0), data() {}

  void push(const T& value) {
    assert(size() < Capacity && "Cannot push onto full queue.");

    data[end] = value;
    end = inc(end);
  }

  T pop() {
    assert(size() > 0 && "Cannot pop on empty queue.");

    T top = data[start];
    data[start].~T();
    start = inc(start);
    return top;
  }

  u64 inc(u64 index) {
    index++;
    if (index >= Capacity) index = 0;
    return index;
  }

  u64 size() {
    if (start > end) {
      return Capacity - (start - end);
    } else {
      return end - start;
    }
  }

  bool in_range(u64 index) {
    if (index >= Capacity) return false;

    if (start > end) {
      return index >= end && index < start;
    }

    return true;
  }

  const T& operator[](u64 index) const = delete;

  const T& at(u64 index) const {
    assert(in_range(index) && "Index out of bounds when accessing queue.");
    return data[index];
  }

};

#endif
