
#ifndef TENSOR_TIGER_STRUCTURES
#define TENSOR_TIGER_STRUCTURES

#include "utils.hpp"

constexpr u64 DEFAULT_FNV_PRIME = 0x00000100000001B3;
constexpr u64 DEFAULT_FNV_OFFSET = 0xCBF29CE484222325;

template <typename T>
u64 fnv_1a_hash(const T& t) {
  const u8 *data = reinterpret_cast<const u8 *>(&t);
  u64 size = sizeof(T);

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
    T v {};
  };
  // Just want to see a warning.
  static_assert(sizeof(_Slot) < 64);

  Hash_Set() : data{}, size{} {}

  Hash_Set(void *memory) : data((_Slot *)memory), size(0) {
    for (u64 i = 0; i < Capacity; i++) {
      new (data + i) _Slot;
    }
  }

  _Slot data[Capacity];
  u64 size;

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

  Hash_Map() : data{}, size{} {
    for (u64 i = 0; i < Capacity; i++) {
      new (data + i) _Slot;
    }
  }

  _Slot data[Capacity]; // Not owning.
  u64 size;

  u64 hash(const Key& key) { return fnv_1a_hash(key) % Capacity; }

  u64 next_index(u64 index) { return (index + 1) % Capacity; }

  // Returns the index of the value, or an empty slot.
  u64 find_empty_slot_or_value(const Key& key) {
    const u64 original_hash = hash(key);
    u64 pool_index = original_hash;

    while (data[pool_index].state == _Slot::State::OCCUPIED) {

      if (data[pool_index] == key) return pool_index;

      pool_index = next_index(pool_index);
      assert(pool_index != original_hash && "Looping when finding empty slot in set.");
    }
    return pool_index;
  }

  bool contains(const Key& key) {
    u64 empty_slot_or_value_index = find_empty_slot_or_value(key);
    return data[empty_slot_or_value_index] == _Slot::State::OCCUPIED;
  }

  u64 insert(const Key& key, const Value& value) {
    if (size >= Capacity) std::terminate();

    const u64 pool_index = find_empty_slot_or_value(key);

    if (data[pool_index].state == _Slot::State::OCCUPIED)
      return pool_index;

    new (data + pool_index) _Slot { _Slot::State::OCCUPIED, key, value };
    size++;

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

  u64 get_index(const Key& key) {
    const u64 pool_index = find_empty_slot_or_value(key);

    if (data[pool_index].state != _Slot::State::OCCUPIED)
      std::terminate();

    return pool_index;
  }

  void clear() {
    for (u64 i = 0; i < Capacity; i++) {
      data[i].~_Slot();
      new (data + i) _Slot {};
    }
  }
};

template <typename T, u64 Capacity>
struct Array {
  Array() : size(0) {
    memset(data, 0, Capacity * sizeof(T));
  }

  // are these heap allocated?
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

  bool operator==(const Array<T, Capacity>& other) const {
    if (size != other.size) return false;
    for (u32 i = 0; i < size; i++) {
      if (this->at(i) != other.at(i)) return false;
    }
    return true;
  }

  u64 size;
  T data[Capacity];
};

#endif
