
#ifndef TENSOR_TIGER_UOP
#define TENSOR_TIGER_UOP

#include "utils.hpp"
#include "helpers.hpp"

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
      static_assert(false, "Unsupported Args const type.");
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
  const UOp get(u64 cache_index) { return cache.at(cache_index); }
};

struct UOp {

  // The UOp is just a fancy wrapper around an index into the uop cache.
  u32 cache_index;

  inline static UOp_Cache cache;

  static u32 new_uop(const Ops& op = Ops::NOOP, const Args& args = {}, const Sources& srcs = {}) {
    const UOp_Cache::UOp uop { op, args, srcs };
    return cache.insert(uop);
  }

  UOp() : cache_index(0) {}

  UOp(const UOp& other) : cache_index(other.cache_index) {}

  UOp(u32 cache_index) : cache_index(cache_index) {}

  UOp(f32 f) {
    const Ops op    { Ops::CONST };
    const Args args { Args::from_const(f) };

    this->cache_index = new_uop(op, args);
  }

  UOp operator+(const UOp& other) {
    const Ops op       { Ops::ADD };
    const Sources srcs { this->cache_index, other.cache_index };

    return UOp(new_uop(op, {}, srcs));
  }

  bool operator==(const UOp& other) const = default;
};

struct Pattern_Matcher {

  // Loop through each pattern, check for a match, apply pattern.
  // Create a new uop from applying that pattern. Srcs should be maintained if necessary
  // or not used if e.g. const folded.
  // If no patterns can be applied, just return the same uop.
  const UOp rewrite(const UOp& uop) const {
    return UOp { uop };
  }
};

// needs to take some pattern matching thing with a uop rewrite function.
inline const UOp walk_rewrite(const UOp& sink, const Pattern_Matcher& pm) {

  #define UOP(uop) (UOp::cache.cache.at(uop.cache_index))

  using Stack = Array<UOp, UOp_Cache::Size>;
  Stack explored   = { };
  Stack unexplored = { sink };

  Hash_Map<UOp, UOp, UOp_Cache::Size> rewritten_uops;

  while (unexplored.size > 0) {
    const UOp uop = unexplored.pop();
    explored.push(uop);

    // Don't rewrite the same uop twice.
    if (rewritten_uops.contains(uop)) { continue; }
    rewritten_uops.insert(uop, uop);

    const Sources& srcs = UOP(uop).srcs;
    for (u32 i = 0; i < srcs.size; i++) {
      unexplored.push(srcs.at(i));
    }
  }

  while (explored.size > 1) {
    const UOp uop = explored.pop();
    rewritten_uops.insert(uop, pm.rewrite(uop));
  }

  return pm.rewrite(sink);
}

#endif
