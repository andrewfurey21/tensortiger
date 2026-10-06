
#ifndef TENSOR_TIGER_UOP
#define TENSOR_TIGER_UOP

#include "utils.hpp"
#include "structures.hpp"

enum class Ops {
  NOOP,
  CONST,
  ADD
};

struct Args {

  enum class Type {
    NONE,
    F32,
  } type;

  union Data {
    f32 const_f32;
  } data;

  bool operator==(const Args& other) const {
    if (type != other.type) return false;
    switch (type) {
      case Type::NONE: return true;
      case Type::F32:  return data.const_f32 == other.data.const_f32;
    }
    return false;
  }

  template <typename T>
  static Args from_const(const T& value) {
    if constexpr (std::is_same_v<T, f32>) {
      return Args { Args::Type::F32, value };
    } else {
      static_assert(false, "Unsupported Args type.");
    }
  }
};

using Sources = Array<u32, 3>;

struct UOp_Cache {

  static constexpr u64 Size = 100;

  struct UOp {
    Ops op;
    Args args;
    Sources srcs;

    bool operator==(const UOp& other) const = default;
  };

  Hash_Set<UOp, Size> cache;

  UOp_Cache() : cache() { cache.insert(UOp {}); }

  u64 insert(const UOp& uop) { return cache.insert(uop); }
  const UOp& get(u64 cache_index) { return cache.data[cache_index].v; }
};

struct UOp {

  // The UOp is just a fancy wrapper around an index into the uop cache.
  u32 cache_index;

  inline static UOp_Cache cache;

  static u32 new_uop(const Ops& op, const Args& args, const Sources& srcs) {
    const UOp_Cache::UOp uop { op, args, srcs };
    return cache.insert(uop);
  }

  UOp() : cache_index(0) {}

  UOp(u32 cache_index) : cache_index(cache_index) {}

  UOp(f32 f) {
    const Ops op       { Ops::CONST };
    const Args args    { Args::from_const(f) };
    const Sources srcs { };

    this->cache_index = new_uop(op, args, srcs);
  }

  UOp operator+(const UOp& other) {
    const Ops op       { Ops::ADD };
    const Args args    { };
    const Sources srcs { this->cache_index, other.cache_index };

    return UOp(new_uop(op, args, srcs));
  }

};

inline const UOp graph_rewrite(const UOp& sink) {

  Queue<UOp, UOp_Cache::Size> uops;
  uops.push(sink);

  // Original UOp -> rewritten UOp.
  Hash_Map<UOp, UOp, UOp_Cache::Size> rewritten_uops;


  // Add the children, and rewrite them first.
  // Rewrite current node and put that in rewritten uops.
  // When looking at sources, must get the source uop from the rewritten uops hash map.

  // Q1: how to do this iteratively
  // Q2: how to do the pattern matching

  while (uops.size() > 0) {}

  return rewritten_uops.at(sink);
}

#endif
