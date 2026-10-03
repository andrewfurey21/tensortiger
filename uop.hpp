
#ifndef TENSOR_TIGER_UOP
#define TENSOR_TIGER_UOP

#include "utils.hpp"
#include "arena.hpp"

constexpr u64 UOP_POOL_SIZE = 100;
constexpr u64 UOP_MAX_NUM_SRCS = 3;

enum class Ops {
  CONST,
  ADD
};

struct Arg {

  enum {
    F32,
  } type;

  union {
    f32 f;
  };

  bool operator==(const Arg& other) {
    if (type != other.type) return false;

    switch (type) {
      case F32: return f = other.f;
    }
  }
};

struct internal_UOp {
  Ops op;
  Arg arg;
  internal_UOp *srcs[UOP_MAX_NUM_SRCS]; // in constructor, intialize to nullptr

  bool operator==(const internal_UOp& other) {
    for (u64 i = 0; i < UOP_MAX_NUM_SRCS; i++) {
      if (srcs[i] != other.srcs[i]) return false;
    }
    return op == other.op && arg == other.arg;
  }
};

struct UOp_Pool {
  UOp_Pool() :
    allocator(UOP_POOL_SIZE * sizeof(internal_UOp)),
    uops(reinterpret_cast<internal_UOp *>(allocator.backing_memory)),
    capacity(UOP_POOL_SIZE),
    num_uops(0) {}

  void clear() {
    allocator.clear();
    num_uops = 0;
  }

  i64 find_uop(const internal_UOp& uop) {
    if (num_uops == 0) return -1;
    for (i64 i = 0; i < num_uops; i++) {
      if (uops[i] == uop) return i;
    }
    return -1;
  }

  // UOp new_uop(const Arg& args, const )

  Arena_Allocator allocator;
  internal_UOp *uops;
  u64 capacity;
  u64 num_uops;
};

struct UOp {
  static UOp_Pool cache;
  u64 cache_index;
};

#endif
