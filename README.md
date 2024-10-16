[![Actions Status](https://github.com/TheLartians/EasyIterator/workflows/MacOS/badge.svg)](https://github.com/TheLartians/EasyIterator/actions)
[![Actions Status](https://github.com/TheLartians/EasyIterator/workflows/Windows/badge.svg)](https://github.com/TheLartians/EasyIterator/actions)
[![Actions Status](https://github.com/TheLartians/EasyIterator/workflows/Ubuntu/badge.svg)](https://github.com/TheLartians/EasyIterator/actions)
[![Actions Status](https://github.com/TheLartians/EasyIterator/workflows/Style/badge.svg)](https://github.com/TheLartians/EasyIterator/actions)
[![Actions Status](https://github.com/TheLartians/EasyIterator/workflows/Install/badge.svg)](https://github.com/TheLartians/EasyIterator/actions)
[![codecov](https://codecov.io/gh/TheLartians/EasyIterator/branch/master/graph/badge.svg)](https://codecov.io/gh/TheLartians/EasyIterator)

# EasyIterator

C++ iterators and range-based loops are incredibly useful, however defining iterators still requires a large amount of boilerplate code.
The goal of this library is to find alternative and useful ways to use and create C++17 iterators without impacting performance or compiler optimizations.

## Example

### Iteration

EasyIterator adds well-known generators and iterator combinators from other languages to C++, such as `range`, `zip` and `enumerate`. 

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

### Iterator definition

Most iterator boilerplate code is defined in an `easy_iterator::IteratorPrototype` base class type.
A possible implementation of the `range` iterable is below.

```cpp
using namespace easy_iterator;

template <class T> struct RangeIterator: public IteratorPrototype<T, dereference::ByValue> {
  T increment;

  RangeIterator(const T &start):
    IteratorPrototype<T, dereference::ByValue>(start),
    increment(1) {
  }

  RangeIterator &operator++(){ RangeIterator::value += increment; return *this; }
};

template <class T> auto range(T end) {
  return wrap(RangeIterator<T>(begin), RangeIterator<T>(end));
}
```

### Iterable algorithms

Algorithms can be easily wrapped into iterators by defining a class that defines `advance()` and `value()` member functions. The code below shows how to define an iterator over Fibonacci numbers.

```cpp
struct Fibonacci {
  unsigned current = 0;
  unsigned next = 1;

  void advance() {
    auto tmp = next;
    next += current;
    current = tmp;
  }
  
  unsigned value() {
    return current;
  }
};

using namespace easy_iterator;

for (auto [i,v]: enumerate(MakeIterable<Fibonacci>())){
  std::cout << "Fib_" << i << "\t= " << v << std::endl;
  if (i == 10) break;
}
```

Algorithms that have an end state can also be defined by returning a the state in the `advance()` method. If the initial state can also be undefined, the iterator should define a `bool init()` method and inherit from `easy_iterator::InitializedIterable`. The code below shows an alternative `range` implementation.

```cpp
template <class T> struct RangeIterator: public easy_iterator::InitializedIterable {
  T current, max, step;
  RangeIterator(T end): current(0), max(end), step(1) { }
  bool advance(){ current += step; return current != max; }
  bool init(){ return current != max; }
  T value(){ return current; }
};

template <class T> auto range(T end) {
  return easy_iterator::MakeIterable<RangeIterator<T>>(end);
}
```

## Installation and usage

EasyIterator is a single-header library, so you can simply download and copy the header into your project, or use the Cmake script to install it globally.
Using the [CPM](https://github.com/cpm-cmake/CPM.cmake) dependency manager, you can also include EasyIterator simply by adding the following to your projects' `CMakeLists.txt`.

```cmake
CPMAddPackage("gh:thelartians/easyiterator@1.5")

target_link_libraries(myProject EasyIterator)            
set_target_properties(myProject PROPERTIES CXX_STANDARD 17)        
```

## Test suite

You can run the tests suite included in this repo with the following commands.

```bash
cmake -Htest -Bbuild/test
cmake --build build/test
cmake --build build/test --target test
```

## Performance

EasyIterator is designed to come with little or no performance impact compared to handwritten code. For example, using `for(auto i: range(N))` loops create identical assembly compared to regular `for(auto i=0;i<N;++i)` loops (using `clang++ -O2`).
The performance of different methods and approaches can be compared with the included benchmark suite. 
You can build and run the benchmark with the following commands:

```bash
cmake -Hbenchmark -Bbuild/bench -DCMAKE_BUILD_TYPE=Release
cmake --build build/bench -j8
./build/bench/EasyIteratorBenchmark
```