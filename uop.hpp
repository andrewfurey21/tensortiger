
#ifndef TENSOR_TIGER_UOP
#define TENSOR_TIGER_UOP

#include "utils.hpp"
#include "helpers.hpp"

// TODO: once chapter 1 is done, look at microegg and mlir/pdl
// TODO: need to be careful with hash, i'm definitly doing UB somewhere here.
// specifically when constructing an internal UOP. probably bad padding.
// actually understand what constructors get called, and {} brace init.
enum class Ops {
  NOOP,
  CONST,
  ADD
};

inline std::ostream& operator<<(std::ostream& os, const Ops& op) {
  switch (op) {
    case Ops::NOOP: os << "NOOP"; break;
    case Ops::CONST: os << "CONST"; break;
    case Ops::ADD: os << "ADD"; break;
  }
  return os;
}

struct Args {

  enum class Type : u32 {
    NONE,
    F32,
  } type;

  // In order to keep hash identical, need to zero out everything.
  // one member for now so should be fine.
  union Data {
    f32 const_f32;
  } data;

  // TODO: better way for initializating a union in member list?
  // Maybe to similar mixin thing.
  explicit Args() : type(Type::NONE) {
    data.const_f32 = 0.0f;
  }
  explicit Args(f32 f) {
    type = Type::F32;
    data.const_f32 = f;
  }

  bool operator==(const Args& other) const {
    if (type == Type::NONE || other.type == Type::NONE) return true;

    if (type != other.type) return false;
    switch (type) {
      case Type::NONE: return true;
      case Type::F32:  return std::abs(data.const_f32 - other.data.const_f32) < 0.0001f;
    }
    return false;
  }

  bool operator!=(const Args& other) const {
    return !(*this == other);
  }
};

inline std::ostream& operator<<(std::ostream& os, const Args& args) {
  switch (args.type) {
    case Args::Type::NONE: os << "<empty>"; break;
    case Args::Type::F32: os << args.data.const_f32; break;
  }
  return os;
}

using Sources = Array<u32, 3>;

inline std::ostream& operator<<(std::ostream& os, const Sources& srcs) {
  if (srcs.size == 0) {
    os << "<empty>";
    return os;
  }

  for (u32 i = 0; i < srcs.size - 1; i++) {
    os << srcs.at(i) << ", ";
  }
  os << srcs.at(srcs.size - 1);
  return os;
}

struct UOp_Cache {

  static constexpr u64 Size = 100;

  struct UOp {
    Ops op;
    Args args;
    Sources srcs;

    bool operator==(const UOp& other) const = default;
  };

  Hash_Set<UOp, Size> cache;

  UOp_Cache() : cache() { cache.insert(UOp()); }

  u64 insert(const UOp& uop) { return cache.insert(uop); }
  const UOp get(u64 cache_index) { return cache.at(cache_index); }
};

// TODO: better, more generic hash. try each byte again but copy
// into a zeroed out buffer.
template <>
inline u64 fnv_1a_hash<UOp_Cache::UOp>(const UOp_Cache::UOp& uop) {
  // std::cout << "======= HASH ==========\n";
  u64 hash = DEFAULT_FNV_OFFSET;

  hash ^= static_cast<u64>(uop.op);
  // std::cout << hash << "\n";
  hash *= DEFAULT_FNV_PRIME;
  // std::cout << hash << "\n";

  hash ^= static_cast<u64>(uop.args.type);
  // std::cout << hash << "\n";
  hash *= DEFAULT_FNV_PRIME;
  // std::cout << hash << "\n";

  switch (uop.args.type) {
    case (Args::Type::F32): {
        // TODO: loop through each byte with sizeof(T)
        // can sizeof work on union members?
        float d = uop.args.data.const_f32;
        u32 x = *reinterpret_cast<const u32*>(&d);

        hash ^= x;
        hash *= DEFAULT_FNV_OFFSET;
      } break;
    default: break;

  }
  // std::cout << hash << "\n";

  for (u32 i = 0; i < uop.srcs.size; i++) {
    hash ^= uop.srcs.at(i);
    // std::cout << hash << "\n";
    hash *= DEFAULT_FNV_PRIME;
    // std::cout << hash << "\n";
  }

  return hash;
}

// TODO: this isn't great.
#define UOP(uop) (UOp::cache.cache.at(uop.cache_index))

struct UOp {

  u32 cache_index;

  inline static UOp_Cache cache;

  static u32 new_uop(const Ops& op = Ops::NOOP, const Args& args = Args(), const Sources& srcs = {}) {
    const UOp_Cache::UOp uop { op, args, srcs };
    return cache.insert(uop);
  }

  UOp() : cache_index(0) {}

  UOp(const Ops& op, const Args& args, const Sources& srcs) {
    this->cache_index = new_uop(op, args, srcs);
  }

  UOp(const UOp& other) : cache_index(other.cache_index) {}

  UOp(u32 cache_index) : cache_index(cache_index) {}

  UOp(f32 f) {
    const Ops op    { Ops::CONST };
    const Args args ( f );
    this->cache_index = new_uop(op, args);
  }

  UOp operator+(const UOp& other) const {
    const Ops op       { Ops::ADD };
    const Sources srcs { this->cache_index, other.cache_index };
    return UOp(op, Args(), srcs);
  }

  bool operator==(const UOp& other) const = default;

  friend std::ostream& operator<<(std::ostream& os, const UOp& uop) {
    const UOp_Cache::UOp& _uop = UOP(uop);
    os << uop.cache_index << ": op: " << _uop.op << ", args: " << _uop.args << ", srcs: " << _uop.srcs;
    return os;
  }
};

using Rewrite_Context = Hash_Map<UOp, UOp, UOp_Cache::Size>;

struct Pattern {
  Ops op;
  Args args;

  Pattern(Ops op = Ops::NOOP, Args args = Args()) : op(op), args(args) {}

  bool match(const UOp& uop) const {
    const UOp_Cache::UOp& _uop = UOP(uop);
    return (op == _uop.op) && (args == _uop.args);
  }
};


// Q: how to do cvar vs const
struct UPat {

  // TODO: need to be able to do the same ops you can do on a UOp
  // that's why they do the mixin shit.
  Pattern pat;
  Array<Pattern, 3> srcs;

  UPat(Ops op = Ops::NOOP, Args args = Args(), const Array<Pattern, 3> srcs = {}) : pat(op, args), srcs(srcs) {}

  // TODO: currently only goes one level deep.
  bool match(const UOp& uop) const {
    if (!pat.match(uop)) return false;

    const UOp_Cache::UOp& _uop = UOP(uop);
    if (srcs.size != _uop.srcs.size) return false;

    for (u32 i = 0; i < srcs.size; i++) {
      if (!srcs.at(i).match(_uop.srcs.at(i))) return false;
    }
    return true;
  }
};

using Patterns = Array<std::pair<UPat, std::function<UOp(UOp)>>, 10>;

inline const UOp rewrite(const UOp& uop, Rewrite_Context& ctx, const Patterns& patterns) {

  UOp current = uop;

  for (u32 i = 0; i < patterns.size; i++) {
    if (patterns.at(i).first.match(uop)) {
      current = patterns.at(i).second(current);
      i = 0; // TODO: need to better understand the pdict/caching stuff
    }
  }

  return current;
}

inline void debug_graph(const UOp& sink) {
  using Stack = Array<UOp, UOp_Cache::Size>;
  Stack explored   = { };
  Stack unexplored = { sink };

  Rewrite_Context ctx;

  while (unexplored.size > 0) {
    const UOp uop = unexplored.pop();
    explored.push(uop);

    // Don't rewrite the same uop twice.
    if (ctx.contains(uop)) continue;

    ctx.insert(uop, uop);

    Sources& srcs = UOP(uop).srcs;
    for (u32 i = 0; i < srcs.size; i++) {
      unexplored.push(srcs.at(i));
    }
  }

  if (explored.size == 0) {
    std::cout << "<empty stack>\n";
    return;
  }
  for (u32 i = explored.size - 1; i > 0; i--) {
    const UOp current = explored.pop();
    std::cout << current << "\n";
  }

  const UOp last = explored.pop();
  std::cout << last << "\n";
}

inline const UOp walk_rewrite(const UOp& sink, const Patterns& pm) {

  using Stack = Array<UOp, UOp_Cache::Size>;
  Stack explored   = { };
  Stack unexplored = { sink };

  Rewrite_Context ctx;

  while (unexplored.size > 0) {
    const UOp uop = unexplored.pop();
    explored.push(uop);

    // Don't rewrite the same uop twice.
    if (ctx.contains(uop)) continue;

    ctx.insert(uop, uop);

    const Sources& srcs = UOP(uop).srcs;
    for (u32 i = 0; i < srcs.size; i++) {
      unexplored.push(srcs.at(i));
    }
  }

  while (explored.size > 1) {
    const UOp uop = explored.pop();
    // TODO: not using ctx, are srcs getting fixed
    const UOp rewritten_uop = rewrite(uop, ctx, pm);
    ctx.insert(uop, rewritten_uop);
  }

  return rewrite(sink, ctx, pm);
}

#endif
