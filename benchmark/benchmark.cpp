#include <benchmark/benchmark.h>
#include <easy_iterator.h>

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
  Integer max = 100000;
  benchmark::DoNotOptimize(max);
  for (auto _ : state) {
    AssertEqual(easyRangeLoop(max),max*(max+1)/2);
  }
}

Integer __attribute__((noinline)) easyForLoop(Integer max){
  Integer result = 0;
  for (auto i=0; i<max+1; ++i) {
    result += i;
  }
  return result;
}

void ForLoop(benchmark::State& state) {
  Integer max = 100000;
  benchmark::DoNotOptimize(max);
  for (auto _ : state) {
    AssertEqual(easyForLoop(max),max*(max+1)/2);
  }
}

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
  benchmark::DoNotOptimize(values);
  for (auto _ : state) {
    AssertEqual(easyArrayIteration(values) ,max*(max+1)/2);
  }
}

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
  benchmark::DoNotOptimize(values);
  for (auto _ : state) {
    AssertEqual(stdArrayIteration(values),max*(max+1)/2);
  }
}

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
    easyZipIteration(integers, doubles);
  }
}

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
    stdZipIteration(integers, doubles);
  }
}

void __attribute__((noinline)) easyEnumerateIteration(const std::vector<int> &values) {
  for (auto [i, v]: easy_iterator::enumerate(values)) {
    AssertEqual(i, v);
  }
}

void EasyEnumerateIteration(benchmark::State& state) {
  Integer max = 10000;
  std::vector<int> values(max);
  easy_iterator::copy(easy_iterator::range(max), values);
  benchmark::DoNotOptimize(values);
  for (auto _ : state) {
    easyEnumerateIteration(values);
  }
}

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
  benchmark::DoNotOptimize(values);
  for (auto _ : state) {
    manualEnumerateIteration(values);
  }
}

BENCHMARK(EasyRangeLoop);
BENCHMARK(ForLoop);

BENCHMARK(EasyArrayIteration);
BENCHMARK(StdArrayIteration);

BENCHMARK(EasyZipIteration);
BENCHMARK(StdZipIteration);

BENCHMARK(EasyEnumerateIteration);
BENCHMARK(ManualEnumerateIteration);


BENCHMARK_MAIN();
