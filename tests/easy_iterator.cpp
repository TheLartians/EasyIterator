#include <catch2/catch.hpp>
#include <functional>
#include <type_traits>

#include <easy_iterator.h>

using namespace easy_iterator;

TEST_CASE("Iterator","[iterator]"){
  
  struct CountDownIterator: public Iterator<int> {
    using Iterator<int>::Iterator;
    void advance(std::optional<int> &value) final override {
      if (*value == 0) {
        value.reset();
      } else {
        value = *value - 1;
      }
    }
  };

  SECTION("invalid iterator"){
    CountDownIterator iterator;
    REQUIRE(!iterator);
    REQUIRE_THROWS_AS(*iterator, UndefinedIteratorException);
    REQUIRE_THROWS_WITH(*iterator, "attempt to dereference an undefined iterator");
  }
  
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

  SECTION("abort"){
    CountDownIterator iterator(10);
    auto expected = *iterator;
    while (iterator) {
      REQUIRE(*iterator == expected);
      ++iterator;
      --expected;
    }
    REQUIRE(expected == -1);
    REQUIRE(!iterator);
    REQUIRE_THROWS_AS(*iterator, UndefinedIteratorException);
  }
  
  SECTION("compare"){
    REQUIRE(CountDownIterator(1) == CountDownIterator(1));
    REQUIRE(CountDownIterator() == CountDownIterator());
    REQUIRE(CountDownIterator(1) != CountDownIterator(2));
    REQUIRE(CountDownIterator(1) != CountDownIterator());
  }

}

TEST_CASE("CallbackIterator", "[iterator]"){

  SECTION("values"){
    auto it = CallbackIterator(0, +[](std::optional<int> &v){ (*v)++; });
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
    auto it = IncrementPtrIterator<int>(arr.data());
    REQUIRE(&*it == arr.data());
    decltype(it) end(arr.data() + arr.size());
    static_assert(std::is_same<decltype(it),decltype(end)>::value);
    REQUIRE(it != end);
    size_t idx = 0;
    while (it != end) { 
      REQUIRE(&static_cast<int &>(*it) == &arr[idx]);
      ++it;
      ++idx;
    }
    REQUIRE(it == end);
    REQUIRE(idx == 10);
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

TEST_CASE("array class", "[iterator]"){

  class MyArray {
  private:
    size_t size;
    int * data;
  public:
    using iterator = IncrementPtrIterator<int>;
    using const_iterator = IncrementPtrIterator<const int>;

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
  
  struct Fibonacci: public Iterator<unsigned> {
    unsigned next;
    
    void advance(std::optional<unsigned> &value) final override {
      auto current = next;
      next = next + *value;
      *value = current;
    }
    
    Fibonacci():Iterator(0), next(1){}
  };
  
  Fibonacci fibonacci;
  
  std::advance(fibonacci, 10);
  REQUIRE(*fibonacci == 55);
  
}
