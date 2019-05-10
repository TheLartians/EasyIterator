#include <benchmark/benchmark.h>
#include <easy_iterator.h>

#ifdef COMPARE_WITH_ITERTOOLS
#include <range.hpp>
#include <enumerate.hpp>
#include <zip.hpp>
#endif

#include <vector>
#include <iostream>

using Integer = unsigned long long;


template <class A, class B> void AssertEqual(const A &a, const B & b){
  if (a != b) {
    throw std::runtime_error("assertion failed: " + std::to_string(a) + " != " + std::to_string(b));
  }
}

Integer __attribute__((noinline)) easyRangeLoop(Integer max){
  Integer result = 0;
  for (auto i: easy_iterator::range(max+1)) {
    result += i;
  }
  return result;
}

void EasyRangeLoop(benchmark::State& state) {
  Integer max = 10000;
  for (auto _ : state) {
    benchmark::DoNotOptimize(max);
    AssertEqual(easyRangeLoop(max),max*(max+1)/2);
  }
}

BENCHMARK(EasyRangeLoop);

Integer __attribute__((noinline)) easyCustomRangeLoop(Integer max){
  
  struct CustomRangeIterator: public easy_iterator::InitializedIterable {
    Integer current, max, step;
    
    CustomRangeIterator(Integer start,Integer end,Integer increment):
      current(start),
      max(end - ((end - start) % increment)),
      step(increment){
    }
    
    CustomRangeIterator(Integer start,Integer end):CustomRangeIterator(start,end,1){ }
    explicit CustomRangeIterator(Integer max):CustomRangeIterator(0,max,1){ }
    
    bool init(){ return current != max; }
    bool advance(){ current += step; return current != max; }
    Integer value(){ return current; }
  };
  
  Integer result = 0;
  for (auto i: easy_iterator::MakeIterable<CustomRangeIterator>(max+1)) {
    result += i;
  }
  
  return result;
}

void EasyCustomRangeLoop(benchmark::State& state) {
  Integer max = 10000;
  for (auto _ : state) {
    benchmark::DoNotOptimize(max);
    AssertEqual(easyCustomRangeLoop(max),max*(max+1)/2);
  }
}

BENCHMARK(EasyCustomRangeLoop);

#ifdef COMPARE_WITH_ITERTOOLS

Integer __attribute__((noinline)) iterRangeLoop(Integer max){
  Integer result = 0;
  for (auto i: iter::range(max+1)) {
    result += i;
  }
  return result;
}

void IterRangeLoop(benchmark::State& state) {
  Integer max = 10000;
  for (auto _ : state) {
    benchmark::DoNotOptimize(max);
    AssertEqual(easyRangeLoop(max),max*(max+1)/2);
  }
}

BENCHMARK(IterRangeLoop);

#endif

Integer __attribute__((noinline)) forLoop(Integer max){
  Integer result = 0;
  for (auto i=0; i<max+1; ++i) {
    result += i;
  }
  return result;
}

void ForLoop(benchmark::State& state) {
  Integer max = 10000;
  for (auto _ : state) {
    benchmark::DoNotOptimize(max);
    AssertEqual(forLoop(max),max*(max+1)/2);
  }
}

BENCHMARK(ForLoop);

Integer __attribute__((noinline)) easyArrayIteration(const std::vector<int> &values){
  using namespace easy_iterator;
  Integer result = 0;
  for (auto &i: valuesBetween(values.data(),values.data() + values.size())) {
    result += i;
  }
  return result;
}

void EasyArrayIteration(benchmark::State& state) {
  Integer max = 10000;
  std::vector<int> values(max+1);
  easy_iterator::copy(easy_iterator::range(max+1), values);
  for (auto _ : state) {
    benchmark::DoNotOptimize(values);
    AssertEqual(easyArrayIteration(values) ,max*(max+1)/2);
  }
}


BENCHMARK(EasyArrayIteration);

Integer __attribute__((noinline)) stdArrayIteration(const std::vector<int> &values){
  Integer result = 0;
  for (auto &i: values) {
    result += i;
  }
  return result;
}

void StdArrayIteration(benchmark::State& state) {
  Integer max = 10000;
  std::vector<int> values(max+1);
  easy_iterator::copy(easy_iterator::range(max+1), values);
  for (auto _ : state) {
    benchmark::DoNotOptimize(values);
    AssertEqual(stdArrayIteration(values),max*(max+1)/2);
  }
}

BENCHMARK(StdArrayIteration);

void __attribute__((noinline)) easyZipIteration(const std::vector<int> &integers, const std::vector<double> &doubles){
  for (auto [i,d]: easy_iterator::zip(integers, doubles)){
    AssertEqual(i, d);
  }
}

void EasyZipIteration(benchmark::State& state) {
  Integer size = 10000;
  std::vector<int> integers(size);
  std::vector<double> doubles(size);
  easy_iterator::copy(easy_iterator::range(size), integers);
  easy_iterator::copy(easy_iterator::range(size), doubles);
  for (auto _ : state) {
    benchmark::DoNotOptimize(integers);
    benchmark::DoNotOptimize(doubles);
    easyZipIteration(integers, doubles);
  }
}

BENCHMARK(EasyZipIteration);

#ifdef COMPARE_WITH_ITERTOOLS

void __attribute__((noinline)) iterZipIteration(const std::vector<int> &integers, const std::vector<double> &doubles){
  for (auto [i,d]: iter::zip(integers, doubles)){
    AssertEqual(i, d);
  }
}

void IterZipIteration(benchmark::State& state) {
  Integer size = 10000;
  std::vector<int> integers(size);
  std::vector<double> doubles(size);
  easy_iterator::copy(easy_iterator::range(size), integers);
  easy_iterator::copy(easy_iterator::range(size), doubles);
  for (auto _ : state) {
    benchmark::DoNotOptimize(integers);
    benchmark::DoNotOptimize(doubles);
    iterZipIteration(integers, doubles);
  }
}

BENCHMARK(IterZipIteration);

#endif


void __attribute__((noinline)) stdZipIteration(const std::vector<int> &integers, const std::vector<double> &doubles){
  auto i = integers.begin(), ie = integers.end();
  auto d = doubles.begin();
  while (i != ie){
    AssertEqual(*i, *d);
    ++i;
    ++d;
  }
}

void StdZipIteration(benchmark::State& state) {
  Integer size = 10000;
  std::vector<int> integers(size);
  std::vector<double> doubles(size);
  easy_iterator::copy(easy_iterator::range(size), integers);
  easy_iterator::copy(easy_iterator::range(size), doubles);
  for (auto _ : state) {
    benchmark::DoNotOptimize(integers);
    benchmark::DoNotOptimize(doubles);
    stdZipIteration(integers, doubles);
  }
}

BENCHMARK(StdZipIteration);

void __attribute__((noinline)) easyEnumerateIteration(const std::vector<int> &values) {
  for (auto [i, v]: easy_iterator::enumerate(values)) {
    AssertEqual(i, v);
  }
}

void EasyEnumerateIteration(benchmark::State& state) {
  Integer max = 10000;
  std::vector<int> values(max);
  easy_iterator::copy(easy_iterator::range(max), values);
  for (auto _ : state) {
    benchmark::DoNotOptimize(values);
    easyEnumerateIteration(values);
  }
}

BENCHMARK(EasyEnumerateIteration);

#ifdef COMPARE_WITH_ITERTOOLS

void __attribute__((noinline)) iterEnumerateIteration(const std::vector<int> &values) {
  for (auto [i, v]: iter::enumerate(values)) {
    AssertEqual(i, v);
  }
}

void IterEnumerateIteration(benchmark::State& state) {
  Integer max = 10000;
  std::vector<int> values(max);
  easy_iterator::copy(easy_iterator::range(max), values);
  for (auto _ : state) {
    benchmark::DoNotOptimize(values);
    iterEnumerateIteration(values);
  }
}

BENCHMARK(IterEnumerateIteration);

#endif

void __attribute__((noinline)) manualEnumerateIteration(const std::vector<int> &values) {
  auto i = 0;
  for (auto &v: values) {
    AssertEqual(i, v);
    ++i;
  }
}

void ManualEnumerateIteration(benchmark::State& state) {
  Integer max = 10000;
  std::vector<int> values(max);
  easy_iterator::copy(easy_iterator::range(max), values);
  for (auto _ : state) {
    benchmark::DoNotOptimize(values);
    manualEnumerateIteration(values);
  }
}

BENCHMARK(ManualEnumerateIteration);


BENCHMARK_MAIN();
