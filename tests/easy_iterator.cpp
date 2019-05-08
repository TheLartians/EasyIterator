#include <catch2/catch.hpp>
#include <functional>
#include <type_traits>
#include <vector>

#include <iostream>

#include <easy_iterator.h>

using namespace easy_iterator;

TEST_CASE("IteratorPrototype","[iterator]"){
  
  struct CountDownIterator: public IteratorPrototype<int> {
    using IteratorPrototype<int>::IteratorPrototype;
    CountDownIterator & operator++() {
      --value;
      return *this;
    }
  };
  
  SECTION("iteration"){
    CountDownIterator iterator(42);
    REQUIRE(*iterator == 42);
    auto expected = *iterator;
    while (*iterator > 10) {
      REQUIRE(*iterator == expected);
      ++iterator;
      --expected;
    }
    REQUIRE(expected == 10);
  }

  SECTION("wrapper"){
    int expected = 10;
    for (auto i: wrap(CountDownIterator(expected), CountDownIterator(3))) {
      REQUIRE(i == expected);
      --expected;
    }
    REQUIRE(expected == 3);
  }
  
  SECTION("compare"){
    REQUIRE(CountDownIterator(1) == CountDownIterator(1));
    REQUIRE(CountDownIterator(1) != CountDownIterator(2));
  }

}

TEST_CASE("Iterator", "[iterator]"){

  SECTION("values"){
    auto it = Iterator(0, +[](int &v){ v++; return true; });
    REQUIRE(*it == 0);
    ++it;
    REQUIRE(*it == 1);
    for (int i=*it;i<10;++i) {
      REQUIRE(*it == i);
      ++it;
    }
    decltype(it) end(100);
    while (it != end) { ++it; }
    REQUIRE(*it == 100);
  }

  SECTION("array incrementer"){
    std::vector<int> arr(10);
    SECTION("manual iteration"){
      ReferenceIterator<int> it(arr.data());
      REQUIRE(&*it == &arr[0]);
      ++it;
      REQUIRE(&*it == &arr[1]);
    }
    SECTION("iterate to end"){
      ReferenceIterator<int> it(arr.data());
      auto end = Iterator(arr.data() + arr.size());
      REQUIRE(it != end);
      size_t idx = 0;
      while (it != end) {
        REQUIRE(&*it == &arr[idx]);
        ++it;
        ++idx;
      }
      REQUIRE(it == end);
      REQUIRE(idx == 10);
    }
    SECTION("valuesBetween"){
      size_t idx = 0;
      for (auto &v: valuesBetween(arr.data(), arr.data() + arr.size())) {
        static_assert(!std::is_const<std::remove_reference<decltype(v)>::type>::value);
        REQUIRE(&v == &arr[idx]);
        ++idx;
      }
      REQUIRE(idx == 10);
    }
   }

}

TEST_CASE("Range", "[iterator]"){
  SECTION("begin-end-advance"){
    int expected = 3;
    for (auto i: range(3,28,3)) {
      REQUIRE(i == expected);
      expected = expected + 3;
    }
    REQUIRE(expected == 27);
  }

  SECTION("negative advance"){
    int expected = 28;
    for (auto i: range(28,1,-2)) {
      REQUIRE(i == expected);
      expected = expected - 2;
    }
    REQUIRE(expected == 2);
  }

  SECTION("begin-end"){
    int expected = 2;
    for (auto i: range(2,12)) {
      REQUIRE(i == expected);
      expected = expected + 1;
    }
    REQUIRE(expected == 12);
  }

  SECTION("end"){
    int expected = 0;
    for (auto i: range(10)) {
      REQUIRE(i == expected);
      expected = expected + 1;
    }
    REQUIRE(expected == 10);
  }
}

TEST_CASE("Zip","[iterator]"){
  for (auto [i,j,k]: zip(range(10), range(0,20,2), range(0,30,3))) {
    REQUIRE(2*i == j);
    REQUIRE(3*i == k);
  }
}

TEST_CASE("Enumerate","[iterator]"){
  std::vector<int> vec(10);
  int count = 0;
  for (auto [i,v]: enumerate(vec)){
    REQUIRE(i == count);
    REQUIRE(&v == &vec[i]);
    ++count;
  }
  REQUIRE(count == 10);
}

TEST_CASE("Reverse","[iterator]"){
  std::vector<int> vec(rangeValue(0), rangeValue(10));
  int count = 0;
  REQUIRE(vec.size() == 10);
  for (auto [i,v]: enumerate(reverse(vec))){
    REQUIRE(v == 9 - i);
    REQUIRE(i == count);
    ++count;
  }
}

TEST_CASE("array class", "[iterator]"){

  class MyArray {
  private:
    size_t size;
    int * data;
  public:
    using iterator = ReferenceIterator<int>;
    using const_iterator = ReferenceIterator<const int>;

    MyArray(size_t _size):size(_size),data(new int[size]){ }
    MyArray(const MyArray &) = delete;
    ~MyArray(){ delete[] data; }

    int &operator[](size_t idx){ return data[idx]; }
    const int &operator[](size_t idx)const{ return data[idx]; }

    iterator begin() { return iterator(data); }
    iterator end() { return iterator(data + size); }
    const_iterator begin() const { return const_iterator(data); }
    const_iterator end() const { return const_iterator(data + size); }
  };

  MyArray array(10);

  SECTION("iterate"){
    size_t idx = 0;
    for (auto &v: array) {
      REQUIRE(&v == &array[idx]);
      ++idx;
      static_assert(!std::is_const<std::remove_reference<decltype(v)>::type>::value);
    }
    REQUIRE(idx == 10);
  }

  SECTION("const iterate"){
    size_t idx = 0;
    for (auto &v: std::as_const(array)) {
      REQUIRE(&v == &array[idx]);
      ++idx;
      static_assert(std::is_const<std::remove_reference<decltype(v)>::type>::value);
    }
    REQUIRE(idx == 10);
  }

}
 

TEST_CASE("Fibonacci","[iterator]"){
  struct Fibonacci {
    unsigned current = 0;
    unsigned next = 1;

    bool advance() {
      auto tmp = next;
      next += current;
      current = tmp;
      return true;
    }
    
    unsigned value() {
      return current;
    }
  };
  
  MakeIterable<Fibonacci> fibonacci;
  for (auto [i,v]: enumerate(fibonacci)) {
    if (i == 9){ 
      REQUIRE(v == 34);
      break;
    }
  }
  auto it = fibonacci.begin();
  std::advance(it, 10);
  REQUIRE(*it == 55);
}

