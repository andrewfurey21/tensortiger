
#ifndef TENSOR_TIGER_HASH_SET
#define TENSOR_TIGER_HASH_SET

#include "utils.hpp"
#include "alloc.hpp"

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

  Hash_Set(Virtual_Memory_Manager& vmm) : allocator(vmm), size(0) {
    (void) allocator.alloc(Capacity);
    for (u64 i = 0; i < Capacity; i++) {
      new (&allocator.at(i)) _Slot;
    }
  }

  Arena_Allocator<_Slot, Capacity> allocator;
  u64 size;

  u64 hash(const T& t) { return fnv_1a_hash(t) % Capacity; }

  u64 next_index(u64 index) { return (index + 1) % Capacity; }

  // Returns the index of the value, or an empty slot.
  u64 find_empty_slot_or_value(const T& value) {
    const u64 original_hash = hash(value);
    u64 pool_index = original_hash;

    while (allocator.at(pool_index).state == _Slot::State::OCCUPIED) {

      if (allocator.at(pool_index).v == value) return pool_index;

      pool_index = next_index(pool_index);
      assert(pool_index != original_hash && "Looping when finding empty slot in set.");
    }
    return pool_index;
  }

  bool contains(const T& value) {
    u64 empty_slot_or_value_index = find_empty_slot_or_value(value);
    return allocator.at(empty_slot_or_value_index).state == _Slot::State::OCCUPIED;
  }

  u64 insert(const T& value) {
    if (size >= Capacity) std::terminate();

    const u64 pool_index = find_empty_slot_or_value(value);

    if (allocator.at(pool_index).state == _Slot::State::OCCUPIED)
      return pool_index;

    new (&allocator.at(pool_index)) _Slot { _Slot::State::OCCUPIED, value };
    size++;

    return pool_index;
  }

  void remove(const T& value) {
    const u64 pool_index = find_empty_slot_or_value(value);

    if (allocator.at(pool_index).state != _Slot::State::OCCUPIED)
      return;

    allocator.at(pool_index).v.~T();
    allocator.at(pool_index).state = _Slot::State::DELETED;
    size--;
  }

  u64 get_index(const T& value) {
    const u64 pool_index = find_empty_slot_or_value(value);

    if (allocator.at(pool_index).state != _Slot::State::OCCUPIED)
      std::terminate();

    return pool_index;
  }

  void clear() {
    for (u64 i = 0; i < Capacity; i++) {
      allocator.at(i).~_Slot();
      new (&allocator.at(i)) _Slot;
    }
  }
};

#endif
