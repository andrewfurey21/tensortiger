
#ifndef TENSOR_TIGER_ALLOC
#define TENSOR_TIGER_ALLOC

#include "utils.hpp"

enum class Page_Size : u32 {
  DEFAULT  = 0,
  _16_KB   = MAP_HUGE_16KB  | MAP_HUGETLB,
  _64_KB   = MAP_HUGE_64KB  | MAP_HUGETLB,
  _512_KB  = MAP_HUGE_512KB | MAP_HUGETLB,
  _1_MB    = MAP_HUGE_1MB   | MAP_HUGETLB,
  _2_MB    = MAP_HUGE_2MB   | MAP_HUGETLB,
  _8_MB    = MAP_HUGE_8MB   | MAP_HUGETLB,
  _16_MB   = MAP_HUGE_16MB  | MAP_HUGETLB,
  _32_MB   = MAP_HUGE_32MB  | MAP_HUGETLB,
  _256_MB  = MAP_HUGE_256MB | MAP_HUGETLB,
  _512_MB  = MAP_HUGE_512MB | MAP_HUGETLB,
  _1_GB    = MAP_HUGE_1GB   | MAP_HUGETLB,
  _2_GB    = MAP_HUGE_2GB   | MAP_HUGETLB,
  _16_GB   = MAP_HUGE_16GB  | MAP_HUGETLB,
};

constexpr u64 page_size_in_bytes(const Page_Size page_size) {

  #define KB * 1024ull
  #define MB KB KB
  #define GB MB KB

  switch (page_size) {
    case Page_Size::_16_KB:  return 16  KB;
    case Page_Size::_64_KB:  return 64  KB;
    case Page_Size::_512_KB: return 512 KB;

    case Page_Size::_1_MB:   return 1   MB;
    case Page_Size::_2_MB:   return 2   MB;
    case Page_Size::_8_MB:   return 8   MB;
    case Page_Size::_16_MB:  return 16  MB;
    case Page_Size::_32_MB:  return 32  MB;
    case Page_Size::_256_MB: return 256 MB;
    case Page_Size::_512_MB: return 512 MB;

    case Page_Size::_1_GB:   return 1   GB;
    case Page_Size::_2_GB:   return 2   GB;
    case Page_Size::_16_GB:  return 16  GB;

    case Page_Size::DEFAULT: return 4   KB;
    default: std::terminate();
  };
}

inline u8 *os_alloc(u64 num_bytes) {
  u8 *buffer =
    reinterpret_cast<u8 *>(mmap(nullptr,
                                num_bytes,
                                PROT_READ | PROT_WRITE,
                                MAP_ANON | MAP_PRIVATE,
                                0,
                                0));

  if (buffer == MAP_FAILED) return nullptr;
  return buffer;
}

inline i32 os_dealloc(void *addr, u64 len) {
  if (addr == nullptr) return 0;
  // TODO: Should add a warning here maybe, when would munmap fail?
  return munmap(addr, len);
}

template <typename T, u64 Capacity>
struct Pool_Allocator {

    union Chunk {
      i32 next_free;
      T element;
    };

    Pool_Allocator() {
      next_free = 0;
      clear();
    }

    T *alloc() {
      if (next_free < 0) throw std::bad_alloc();
      i32 allocated_index = next_free;
      next_free = memory[allocated_index].next_free;
      return &memory[allocated_index];
    }

    void destruct_and_dealloc(T *element) {
      element->~T();
      dealloc(element);
    }

    void dealloc(T *element) {
      u32 newly_freed =
        static_cast<u32>(reinterpret_cast<Chunk *>(element) - memory);

      memory[newly_freed].next_free = next_free;
      next_free = newly_freed;
    }

    void clear() {
      u32 i;
      for (i = 0; i <= Capacity; i++) {
        memory[i].next_free = i + 1;
      }
      memory[i].next_free = -1;
    }

    i32 next_free;
    Chunk memory[Capacity];
};

#endif
