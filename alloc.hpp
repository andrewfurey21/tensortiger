
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

struct Virtual_Memory_Manager {

  u8 *backing_memory;
  u32 num_pages_reserved;
  u32 page_size;
  u32 num_pages_allocated;

  static void Allocate_Pages(u32 num_pages, Page_Size page_size = Page_Size::DEFAULT) {
    (void) get_instance(num_pages, page_size_in_bytes(page_size));
  }

  static Virtual_Memory_Manager& get_instance(u32 num_pages = 0, u32 page_size = 0) {
    static Virtual_Memory_Manager vmm(num_pages, page_size);
    return vmm;
  }

  Virtual_Memory_Manager(const Virtual_Memory_Manager& other) = delete;
  Virtual_Memory_Manager& operator=(const Virtual_Memory_Manager& other) = delete;

  void *alloc_contiguous_pages(u32 num_bytes_to_allocate) {
    const u32 num_pages_to_allocate = (num_bytes_to_allocate + page_size - 1) / page_size;
    if (num_pages_allocated + num_pages_to_allocate >= num_pages_reserved) {
      throw std::bad_alloc();
    }

    void *allocation = &backing_memory[num_pages_allocated * page_size];
    num_pages_allocated += num_pages_to_allocate;
    return allocation;
  }

  // No dealloc yet, bare bones.
  void return_contiguous_pages(void *pages, u32 num_bytes_to_return) {}

private:

  Virtual_Memory_Manager() :
    backing_memory(nullptr),
    num_pages_reserved(0),
    page_size(0),
    num_pages_allocated(0) {}

  Virtual_Memory_Manager(u32 num_pages, u32 page_size) {
    this->backing_memory = os_alloc(num_pages * page_size);
    if (backing_memory == nullptr) throw std::bad_alloc();

    this->num_pages_reserved = num_pages;
    this->page_size = page_size;
    this->num_pages_allocated = 0;
  }

  ~Virtual_Memory_Manager() {
    os_dealloc(backing_memory, page_size * num_pages_reserved);
  }

  u8 *os_alloc(u64 num_bytes) {
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

  i32 os_dealloc(void *addr, u64 len) {
    if (addr == nullptr) return 0;
    // TODO: Should add a warning here maybe, when would munmap fail?
    return munmap(addr, len);
  }
};

template <typename T, u64 Capacity>
struct Arena_Allocator {
  Arena_Allocator(Virtual_Memory_Manager& vmm) :
    vmm(vmm),
    memory((T *)vmm.alloc_contiguous_pages(sizeof(T) * Capacity)),
    num_elements_allocated(0) {}

  T *alloc(u64 num_elements_to_allocate = 1) {
    if (num_elements_allocated + num_elements_to_allocate > Capacity)
      throw std::bad_alloc();

    T *allocation = memory + num_elements_allocated;
    num_elements_allocated += num_elements_to_allocate;
    return allocation;
  }

  void dealloc(T *element, u64 num_elements_to_deallocate) {}

  T& at(u64 index) {
    if (index > num_elements_allocated) std::terminate();
    return memory[index];
  }

  Virtual_Memory_Manager& vmm;
  T *memory;
  u64 num_elements_allocated;
};

template <typename T, u64 Capacity>
struct Pool_Allocator {

    union Chunk {
      i32 next_free;
      T element;
    };

    Pool_Allocator(Virtual_Memory_Manager& vmm) : vmm(vmm) {
      memory = vmm.alloc_contiguous_pages(Capacity * sizeof(Chunk));
      next_free = 0;
      clear();
    }

    ~Pool_Allocator() {
      vmm.return_contiguous_pages((void *)memory, Capacity * sizeof(Chunk));
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

    // TODO: should have an at(i) function.

    Virtual_Memory_Manager& vmm;
    Chunk *memory;
    i32 next_free;
};

#endif
