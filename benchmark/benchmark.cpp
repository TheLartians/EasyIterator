#include <benchmark/benchmark.h>
#include <easy_iterator.h>

#include <vector>
#include <iostream>

using Integer = unsigned long long;

Integer __attribute__ ((noinline)) test_range (Integer max) {
  Integer result = 0;
  for (auto i: easy_iterator::range(max+1)) {
    result += i;
  }
  return result;
}

Integer __attribute__ ((noinline)) test_for (Integer max) {
  Integer result = 0;
  for (auto i=0; i<max+1; ++i) {
    result += i;
  }
  return result;
}

Integer __attribute__ ((noinline)) test_ei_array_iteration (Integer max) {
  using namespace easy_iterator;
  Integer result = 0;
  std::vector<int> values(RangeIterator<Integer>(0), RangeIterator<Integer>(max+1));
  for (auto &i: valuesBetween(&values[0],&values[max+1])) {
    result += i;
  }
  return result;
}

Integer __attribute__ ((noinline)) test_std_array_iteration (Integer max) {
  std::vector<int> values(easy_iterator::RangeIterator<Integer>(0), easy_iterator::RangeIterator<Integer>(max+1));
  Integer result = 0;
  for (auto &i: values) {
    result += i;
  }
  return result;
}

template <class A, class B> void AssertEqual(const A &a, const B & b){
  if (a != b) {
    throw std::runtime_error("assertion failed: " + std::to_string(a) + " != " + std::to_string(b));
  }
}

void RangeLoop(benchmark::State& state) {
  Integer max = 100000;
  benchmark::DoNotOptimize(max);
  for (auto _ : state) {
    AssertEqual(test_range(max),max*(max+1)/2);
  }
}

void ForLoop(benchmark::State& state) {
  Integer max = 100000;
  benchmark::DoNotOptimize(max);
  for (auto _ : state) {
    AssertEqual(test_for(max),max*(max+1)/2ll);
  }
}

void EIArrayIteration(benchmark::State& state) {
  Integer max = 100000;
  benchmark::DoNotOptimize(max);
  for (auto _ : state) {
    AssertEqual(test_ei_array_iteration(max),max*(max+1)/2);
  }
}

void StdArrayIteration(benchmark::State& state) {
  Integer max = 100000;
  benchmark::DoNotOptimize(max);
  for (auto _ : state) {
    AssertEqual(test_std_array_iteration(max),max*(max+1)/2);
  }
}

BENCHMARK(RangeLoop);
BENCHMARK(ForLoop);
BENCHMARK(EIArrayIteration);
BENCHMARK(StdArrayIteration);

BENCHMARK_MAIN();
