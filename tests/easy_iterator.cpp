#include <catch2/catch.hpp>
#include <functional>

#include <easy_iterator.h>

using namespace easy_iterator;

TEST_CASE("AdvanceIterator", "[iterator]"){

  SECTION("values"){
    auto it = easy_iterator::AdvanceIterator(0);
    REQUIRE(*it == 0);
    for (int i=0;i<10;++i,++it) {
      REQUIRE(*it == i);
    }
    easy_iterator::AdvanceIterator end(100);
    while (it != end) { ++it; }
    REQUIRE(*it == 100);
  }

  SECTION("custom incrementer"){
    auto it = easy_iterator::AdvanceIterator(0, [](int v){ return v + 2; });
    REQUIRE(*it == 0);
    for (int i=0;i<10;++i,++it) {
      REQUIRE(*it == 2*i);
    }
    easy_iterator::AdvanceIterator end(100);
    while (it != end) { ++it; }
    REQUIRE(*it == 100);
  }

  SECTION("array incrementer"){
    std::vector<int> arr(10);
    auto it = easy_iterator::AdvanceIterator(
      std::reference_wrapper(arr[0]),
      easy_iterator::increment::ByAddress<std::reference_wrapper<int>>(),
      easy_iterator::compare::ByAddress<int>()
    );
    auto end = it.copyWithValue(arr[10]);
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
