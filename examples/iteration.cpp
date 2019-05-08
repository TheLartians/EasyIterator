
#include <easy_iterator.h>

#include <vector>
#include <iostream>


int main(){
  using namespace easy_iterator;
  
  auto evenNumbers = Iterator(0, [](auto &i){ i+=2; });
  auto vec = std::vector<int>(evenNumbers, evenNumbers);
  
  
  for (auto i: wrap(evenNumbers, Iterator(10))) {
    std::cout << i << std::endl;
  }
  
  return 0;
}
