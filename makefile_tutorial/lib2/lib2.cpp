#include <iostream>
#include <vector>
#include "lib2.h"

static auto fib(unsigned n){
  static std::vector<unsigned long> cache{0, 1};

  if(n < cache.size()){
    if(n == 0 || cache[n] != 0)
      return cache[n];
  }else
    cache.resize(n + 1);

  if(cache[n] == 0)
    cache[n] = fib(n - 1) + fib(n - 2);

  return cache[n];
}

void print_fib(unsigned n){
  std::cout << "f[" << n << "] = " << fib(n) << '\n';
}

