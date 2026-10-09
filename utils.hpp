
#ifndef TENSOR_TIGER_UTILS
#define TENSOR_TIGER_UTILS

#include <sys/mman.h>

#include <cstring>
#include <cassert>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <initializer_list>
#include <new>
#include <functional>

using i32 = signed int;
using i64 = signed long long;

using u8 = unsigned char;
using u32 = unsigned int;
using u64 = unsigned long long;

using f32 = float;
using f64 = double;


inline void panic_if(bool expr, const char *msg) {
  if (expr) {
    std::cout << msg << "\n";
    std::terminate();
  }
}

#endif
