
#ifndef TENSOR_TIGER_ARENA
#define TENSOR_TIGER_ARENA

#include "utils.hpp"

struct Arena_Allocator {
  Arena_Allocator(u64 num_bytes) {
    backing_memory = (u8 *)os_alloc(num_bytes);

    num_bytes_allocated = 0;
    num_bytes_available = num_bytes;
  }

  Arena_Allocator(const Arena_Allocator& other) = delete;
  Arena_Allocator& operator=(const Arena_Allocator& other) = delete;

  ~Arena_Allocator() {
    os_dealloc(backing_memory, num_bytes_available);
  }

  void clear() { num_bytes_allocated = 0; }

  u8 *alloc(u64 num_bytes_to_allocate) {
    if (num_bytes_allocated + num_bytes_to_allocate >= num_bytes_available)
      throw std::bad_alloc();

    u8 *allocation = backing_memory + num_bytes_allocated;
    num_bytes_allocated += num_bytes_to_allocate;
    return allocation;
  }

  void *os_alloc(u64 num_bytes) {
    void *buffer = mmap(nullptr,
                        num_bytes,
                        PROT_READ | PROT_WRITE,
                        MAP_ANON | MAP_PRIVATE,
                        0,
                        0);

    if (buffer == MAP_FAILED) return nullptr;
    return buffer;
  }

  i32 os_dealloc(void *addr, u64 len) {
    if (addr == nullptr) return 0;
    return munmap(addr, len);
  }

  u8 *backing_memory;
  u64 num_bytes_allocated;
  u64 num_bytes_available;
};


#endif
