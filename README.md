[![Build Status](https://travis-ci.com/TheLartians/EasyIterator.svg?branch=master)](https://travis-ci.com/TheLartians/EasyIterator)
[![codecov](https://codecov.io/gh/TheLartians/EasyIterator/branch/master/graph/badge.svg)](https://codecov.io/gh/TheLartians/EasyIterator)

# EasyIterator

EasyIterator aims to be a zero-cost C++17 iterator library that simplifies iterator creation.
It adds well-known iterators from other languages to C++, such as `range`, `zip` and `enumerate`. 

## Example

### Iteration

```cpp
using namespace easy_iterator;

std::vector<int> integers(10);
std::vector<std::string> strings(integers.size());

for (auto i: range(integers.size())) {
  integers[i] = i*i;
}

for (auto [i, v, s]: zip(range(integers.size()), integers, strings)) {
  s = std::to_string(i) + "^2 = " + std::to_string(v);
}

for (auto [i, s]: enumerate(strings)) {
  std::cout << "strings[" << i << "] = \"" << s << "\"" << std::endl;
}
```

### Simple iterator creation

```cpp
struct Fibonacci {
  unsigned current = 0;
  unsigned next = 1;

  void advance() {
    auto tmp = next;
    next += current;
    current = tmp;
    return true;
  }
  
  unsigned value() {
    return current;
  }
};

for (auto [i,v]: enumerate(MakeIterable<Fibonacci>())){
  std::cout << "Fib_" << i << "\t= " << v << std::endl;
  if (i > 10) break;
}
```

## Installation and usage

EasyIterator is a single-header library, so you can simply download and copy the header into your project, or use the Cmake scripe to install it gloablly.
Using the [CPM](https://github.com/TheLartians/CPM) dependency manager the easiest way to include EasyIterator is to add with the following lines in your projects' `CMakeLists.txt`.

```cmake
CPMAddPackage(
  NAME EasyIterator
  VERSION 1.0
  GIT_REPOSITORY https://github.com/TheLartians/EasyIterator.git
)
```
