#include <catch2/catch.hpp>
#include <functional>
#include <type_traits>
#include <vector>
#include <string>
#include <map>

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
    auto it = makeIterator(0, +[](int &v){ v++; return true; });
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
      auto end = makeIterator(arr.data() + arr.size());
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
  
  SECTION("modifiers"){
    auto a = range(5,20,3);
    int expected = 5;
    SECTION("copy"){
      auto b = a;
      for (auto i: b) {
        REQUIRE(i == expected);
        expected = expected + 3;
      }
    }
    SECTION("const"){
      for (auto i: std::as_const(a)) {
        REQUIRE(i == expected);
        expected = expected + 3;
      }
    }
    REQUIRE(expected == 20);
  }
}

TEST_CASE("Zip","[iterator]"){
  SECTION("with ranges"){
    unsigned expected = 0;
    for (auto [i,j,k]: zip(range(10), range(0,20,2), range(0,30,3))) {
      REQUIRE(i == expected);
      REQUIRE(2*i == j);
      REQUIRE(3*i == k);
      expected++;
    }
    REQUIRE(expected == 10);
  }
  
  SECTION("with arrays"){
    std::vector<int> integers(10);
    unsigned expected = 0;
    for (auto [i,v]: zip(range(10), integers)) {
      REQUIRE(i == expected);
      REQUIRE(&integers[i] == &v);
      v = i;
      REQUIRE(integers[i] == i);
      expected++;
    }
    REQUIRE(expected == 10);
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

TEST_CASE("fill","[iterator]"){
  std::vector<int> vec(10);
  fill(vec, 42);
  for(auto v: vec){ REQUIRE(v == 42); }
}

TEST_CASE("copy","[iterator]"){
  std::vector<int> vec(10);
  SECTION("value"){
    copy(range(10), vec);
    for(auto [i, v]: enumerate(vec)){ REQUIRE(v == i); }
  }
  SECTION("transformed value"){
    copy(range(10), vec, [](auto v){ return 2*v; });
    for(auto [i, v]: enumerate(vec)){ REQUIRE(v == 2*i); }
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

    explicit MyArray(size_t _size):size(_size),data(new int[size]){ }
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
 

TEST_CASE("MakeIterable","[iterator]"){
  
  struct Countdown {
    unsigned current;
    
    explicit Countdown(unsigned start): current(start) {}
    
    bool advance() {
      if (current == 0) { return false; }
      current--;
      return true;
    }
    
    unsigned value() {
      return current;
    }
  };
  
  SECTION("iterate"){
    auto it = MakeIterable<Countdown>(1).begin();
    REQUIRE(it);
    REQUIRE(it != IterationEnd());
    REQUIRE(*it == 1);
    ++it;
    REQUIRE(it);
    REQUIRE(it != IterationEnd());
    REQUIRE(*it == 0);
    ++it;
    REQUIRE(!it);
    REQUIRE_THROWS_AS(*it, UndefinedIteratorException);
    REQUIRE_THROWS_WITH(*it, "attempt to dereference an undefined iterator");
    REQUIRE(it == IterationEnd());
  }
  
  SECTION("iterate"){
    unsigned count = 0;
    for (auto v: MakeIterable<Countdown>(10)) {
      REQUIRE(v == 10-count);
      count++;
    }
    REQUIRE(count == 11);
  }
  
  SECTION("initialized") {
    struct Invalid:InitializedIterable {
      bool init() { return false; }
      int value() { REQUIRE(false); return 0; }
      bool advance() { REQUIRE(false); return true; }
    };
    
    auto it = MakeIterable<Invalid>().begin();
    REQUIRE(!it);
    REQUIRE_THROWS_AS(*it, UndefinedIteratorException);
  }
  
}

TEST_CASE("eraseIfFound") {
  std::map<std::string, int> map;
  map["a"] = 1;
  map["b"] = 2;
  REQUIRE(eraseIfFound(map.find("a"), map));
  REQUIRE(!eraseIfFound(map.find("c"), map));
  REQUIRE(map.find("a") == map.end());
  REQUIRE(map.size() == 1);
}

TEST_CASE("found") {
  std::map<std::string, int> map;
  map["a"] = 1;
  map["b"] = 2;
  REQUIRE(found(map.find("a"), map));
  REQUIRE(&found(map.find("a"), map)->second == &map["a"]);
  REQUIRE(!found(map.find("c"), map));
}
