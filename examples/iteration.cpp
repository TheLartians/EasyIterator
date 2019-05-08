
#include <easy_iterator.h>

#include <vector>
#include <iostream>

unsigned __attribute__ ((noinline)) test_range () {
  using namespace easy_iterator;
  unsigned result = 0;
  for (auto i: range(10)) {
    result += i;
  }
  return result;
}

unsigned __attribute__ ((noinline)) test_for () {
  unsigned result = 0;
  for (auto i=0; i<10; ++i) {
    result += i;
  }
  return result;
}
unsigned __attribute__ ((noinline)) test_array () {
  using namespace easy_iterator;
  unsigned result = 0;
  std::vector<int> values(RangeIterator(0), RangeIterator(10));
  for (auto i: values) {
    result += i;
  }
  return result;
}

unsigned __attribute__ ((noinline)) test_array_for () {
  unsigned result = 0;
  for (auto i=0; i<10; ++i) {
    result += i;
  }
  return result;
}


int main(){

  std::cout << test_range() << std::endl;
  std::cout << test_for() << std::endl;


  return 0;
}
