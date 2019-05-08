
#include <iostream>
#include <limits>

#include <easy_iterator.h>

using namespace easy_iterator;

using integer = unsigned long long;

struct Fibonacci {
  integer current = 0;
  integer next = 1;

  bool advance() {
    if (std::numeric_limits<unsigned>::max() - current < next) {
      // abort before integer overflow
      return false;
    }
    auto tmp = next;
    next += current;
    current = tmp;
    return true;
  }
  
  integer value() {
    return current;
  }
  
};

int main(){
  for (auto [i,v]: enumerate(MakeIterable<Fibonacci>())){
    std::cout << "Fib_" << i << "\t= " << v << std::endl;
  }
  std::cout << "done" << std::endl;
  return 0;
}
