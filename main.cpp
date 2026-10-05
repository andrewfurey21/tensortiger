

#include "tiger.hpp"

int main() {

  Virtual_Memory_Manager::Allocate_Pages(2);

  UOp a = 1.0f;
  UOp b = 1.0f;
  UOp e = 2.0f;
  UOp::assert_same_uop(a, b);

  UOp c = a + b;

  UOp::assert_different_uop(c, a);
  UOp::assert_different_uop(c, b);

  // UOp d = UOp::graph_rewrite(c);
  // UOp::assert_different_uop(c, d);
  // UOp::assert_different_uop(a, d);
  // UOp::assert_different_uop(b, d);
  // UOp::assert_same_uop(d, e);

  std::cout << a << "\n";
  std::cout << b << "\n";
  std::cout << c << "\n";

  // std::cout << "Should output a UOp of const 2 f32\n";
  // std::cout << d << "\n";

  return 0;
}
