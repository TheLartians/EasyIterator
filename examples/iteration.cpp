
#include <easy_iterator.h>

#include <vector>
#include <string>
#include <iostream>


int main(){
  using namespace easy_iterator;
  
  std::vector<int> integers(10);

  for (auto i: range(integers.size())) {
    integers[i] = i*i;
  }
  
  std::vector<std::string> strings(integers.size());
  for (auto [i, v, s]: zip(range(integers.size()), integers, strings)) {
    s = std::to_string(i) + "^2 = " + std::to_string(v);
  }
  
  for (auto [i, s]: enumerate(strings)) {
    std::cout << "strings[" << i << "] = \"" << s << "\"" << std::endl;
  }
  
  return 0;
}
