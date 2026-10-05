
#ifndef TENSOR_TIGER_UOP
#define TENSOR_TIGER_UOP

#include "utils.hpp"
#include "alloc.hpp"
#include "array.hpp"
#include "set.hpp"


enum class Ops {
  NOOP,
  CONST,
  ADD
};

struct Args {

  enum class Type {
    NONE,
    F32_LITERAL,
  } type;

  union {
    f32 f32_literal;
  };

  bool operator==(const Args& other) {
    if (type != other.type) return false;
    switch (type) {
      case Type::NONE:        return true;
      case Type::F32_LITERAL: return f32_literal == other.f32_literal;
    }
    return false;
  }

};


struct UOp_Cache {

  static constexpr u64 Num_Sources = 2;
  static constexpr u64 UOp_Pool_Size = 100;

  using Sources = Array<u32, Num_Sources>;

  struct _UOp {
    Ops op;
    Args args;
    Sources srcs;

    // TODO: this should be the default?
    bool operator==(const _UOp& other) {
      if (srcs.size != other.srcs.size) return false;
      for (u32 i = 0; i < srcs.size; i++) {
        if (srcs.at(i) != other.srcs.at(i)) return false;
      }
      return op == other.op && args == other.args;
    }
  };

  Hash_Set<_UOp, UOp_Pool_Size> cache;

  UOp_Cache() : cache(Virtual_Memory_Manager::get_instance()) { clear(); }

  void clear() { cache.clear(); }
  u64 insert_uop(const _UOp& uop) { return cache.insert(uop); }
  const _UOp& get_uop(u64 cache_index) { return cache.allocator.at(cache_index).v; }
};

struct UOp {
  u32 cache_index;
  UOp_Cache *cache;

  UOp() {
    static UOp_Cache cache;
    this->cache = &cache;
  }

  static const UOp_Cache::_UOp& find(u64 cache_index) {
    return UOp().cache->get_uop(cache_index);
  }

  UOp(const UOp_Cache::_UOp& _uop) : UOp() {
    this->cache_index = cache->insert_uop(_uop);
  }

  UOp(const u32 cache_index) : UOp() {
    this->cache_index = cache_index;
  }

  UOp(const UOp& other) : UOp() {
    this->cache_index = other.cache_index;
  }

  UOp(f32 f) : // When doing cast, add double first.
    UOp(UOp_Cache::_UOp { Ops::CONST, Args { Args::Type::F32_LITERAL, f } }) {}

  UOp operator+(const UOp& other) {
    const UOp_Cache::Sources srcs = { this->cache_index, other.cache_index };
    return UOp(UOp_Cache::_UOp { Ops::ADD, Args { Args::Type::NONE }, srcs });
  }

  bool operator!=(const UOp& other) const { return cache_index != other.cache_index; }
  bool operator==(const UOp& other) const { return cache_index == other.cache_index; }

  // bottom up greedy rewrite
  // graph_rewrite -> do the constant folding.
  // should return a single UOp for the week 1 stuff
  // 1. toposort from given uop
  // 2. constant fold.

  // 1 uop for now.
  // static UOp graph_rewrite(UOp uop) {
    // this first. needs to match for things that aren't const too.
    // add ( const (a), const (a) ) -> const (2a)
    // add ( const (a), const (b) ) -> const (a + b)
    // add (  )
  // }

  static void assert_same_uop(const UOp& a, const UOp& b) {
    if (a != b) {
      std::cerr << "Assertion error: " << a << " != " << b << "\n";

      // TODO: something nicer than std::terminate.
      std::terminate();
    }
  }

  static void assert_different_uop(const UOp& a, const UOp& b) {
    if (a == b) {
      std::cerr << "Assertion error: " << a << " == " << b << "\n";
      std::terminate();
    }
  }

  friend std::ostream& operator<<(std::ostream& os, const UOp& uop) {
    // TODO: needs to be redone, have a string_builder class
    // takes in a block of memory, doesn't own it (temporary arena)
    // shouldn't print out uop.cache_index, but some counter thing.
    // should go through the graph
    // have nice colors too :)
    // should generate uir
    const UOp_Cache::_UOp& _uop = UOp::find(uop.cache_index);
    switch (_uop.op) {
      case Ops::CONST: {
        os << "Result: " << _uop.args.f32_literal;
      } break;
      case Ops::NOOP:
      case Ops::ADD:
        os << "Need to implement.";
    }
    return os;
  }

};


#endif
