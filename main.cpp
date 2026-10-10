

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

      // need something better, maybe another Args constructor.
      return UOp(Ops::CONST, Args(first + second), {});
    },
  },
};

int main() {

  UOp a = 1.0f;
  UOp b = 2.0f;
  UOp c = 3.0f;
  UOp d = 4.0f;
  UOp e = 5.0f;
  UOp f = 6.0f;
  UOp g = 7.0f;
  UOp h = 8.0f;
  // TODO: asserts for hash cons

  UOp a1 = a + b + c; // 1 + 2 + 3 = 6
  UOp a2 = d + e; // 4 + 5 = 9
  UOp a3 = a2 + f; // 9 + 6 = 15
  UOp a4 = a1 + a2; // 6 + 9 = 15
  UOp a5 = a3 + g + a4 + h; // 15 + 7 + 15 + 8 = 45

  UOp before = a1 + a2 + a3 + a4 + a5; // 6 + 9 + 15 + 15 + 45 = 90
  UOp expected = 90.0f;

  std::cout << "=== old graph ===\n";
  debug_graph(before);
  UOp after = walk_rewrite(before, basics);

  std::cout << "=== new graph ===\n";
  debug_graph(after);

  std::cout << "=== after rewrite ===\n";

  std::cout << "Actual: " << after << "\n";
  std::cout << "Expected: " << expected << "\n";
  return 0;
}
