

#include "tiger.hpp"

// TODO: impl these, then clean up everything.
// add (const (0), const (a)) -> add(const (a), const (0))
// add (const (a), const (0)) -> const(a)
// add (const (a), const (a)) -> const(2a)
// add (const (a), const (b)) -> const(a + b)
inline static const Patterns basics = {
  {
    UPat(Ops::ADD, Args(), { Pattern(Ops::CONST), Pattern(Ops::CONST) }),
    [](UOp uop) {
      f32 first = UOp::cache.get((UOP(uop).srcs.at(0))).args.data.const_f32;
      f32 second = UOp::cache.get((UOP(uop).srcs.at(1))).args.data.const_f32;
      // TODO: for some reason, this will not see the matching UOp in the cache.
      std::cout << "Output op\n";
      return UOp(Ops::CONST, Args(first + second), {});
    },
  },
};

int main() {

  const UOp a = 1.0f;
  const UOp b = 0.0f;
  const UOp e = 1.0f;
  const UOp f = -1.0f;
  UOp g = 1.0f;

  UOp c = a + b + f + g;
  // // TODO: asserts for hash cons

  std::cout << "=== old graph ===\n";
  debug_graph(c);
  UOp d = walk_rewrite(c, basics);

  std::cout << "=== new graph ===\n";
  debug_graph(d);

  std::cout << "=== after rewrite ===\n";

  std::cout << "Actual: " << d << "\n";
  std::cout << "Expected: " << e << "\n";
  return 0;
}
