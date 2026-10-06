
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
};

using Sources = Array<u32, 3>;

struct UOp_Cache {

  static constexpr u64 UOp_Cache_Size = 100;

  struct UOp {
    Ops op;
    Args args;
    Sources srcs;

    bool operator==(const UOp& other) const = default;
  };

  Hash_Set<UOp, UOp_Cache_Size> cache;

  u64 insert(const UOp& uop) { return cache.insert(uop); }
  const UOp& get(u64 cache_index) { return cache.data[cache_index].v; }
};

struct UOp {

  u32 cache_index;

  inline static UOp_Cache cache;

  static u32 new_uop(const Ops& op, const Args& args, const Sources& srcs) {
    const UOp_Cache::UOp uop { op, args, srcs };
    return cache.insert(uop);
  }

  UOp(u32 cache_index) : cache_index(cache_index) {}

  UOp(f32 f) {
    const Ops op       { Ops::CONST };
    const Args args    { Args::Type::F32, f };
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


#endif
