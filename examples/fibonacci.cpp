
#include <iostream>
#include <limits>

#include <easy_iterator.h>

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
  
  integer value() const {
    return current;
  }
  
};

using namespace easy_iterator;

int main(){
  for (auto [i,v]: enumerate(MakeIterable<Fibonacci>())){
    std::cout << "Fib_" << i << "\t= " << v << std::endl;
  }
  return 0;
}
