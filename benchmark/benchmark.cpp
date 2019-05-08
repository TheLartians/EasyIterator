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


void RangeLoop(benchmark::State& state) {
  Integer max = 100000;
  benchmark::DoNotOptimize(max);
  for (auto _ : state) {
    Integer result = 0;
    for (auto i: easy_iterator::range(max+1)) {
      result += i;
    }
    AssertEqual(result,max*(max+1)/2);
  }
}

void ForLoop(benchmark::State& state) {
  Integer max = 100000;
  benchmark::DoNotOptimize(max);
  for (auto _ : state) {
    Integer result = 0;
    for (auto i=0; i<max+1; ++i) {
      result += i;
    }
    AssertEqual(result,max*(max+1)/2);
  }
}

void EasyArrayIteration(benchmark::State& state) {
  Integer max = 100000;
  std::vector<int> values(easy_iterator::RangeIterator<Integer>(0), easy_iterator::RangeIterator<Integer>(max+1));
  benchmark::DoNotOptimize(values);
  for (auto _ : state) {
    using namespace easy_iterator;
    Integer result = 0;
    for (auto &i: valuesBetween(&values[0],&values[max+1])) {
      result += i;
    }
    AssertEqual(result,max*(max+1)/2);
  }
}

void StdArrayIteration(benchmark::State& state) {
  Integer max = 100000;
  std::vector<int> values(easy_iterator::RangeIterator<Integer>(0), easy_iterator::RangeIterator<Integer>(max+1));
  benchmark::DoNotOptimize(values);
  for (auto _ : state) {
    Integer result = 0;
    for (auto &i: values) {
      result += i;
    }
    AssertEqual(result,max*(max+1)/2);
  }
}



void EnumerateIteration(benchmark::State& state) {
  Integer max = 100000;
  std::vector<int> values(easy_iterator::RangeIterator<Integer>(0), easy_iterator::RangeIterator<Integer>(max+1));
  benchmark::DoNotOptimize(values);
  for (auto _ : state) {
    for (auto [i, v]: easy_iterator::enumerate(values)) {
      AssertEqual(i, v);
    }
  }
}

void ManualEnumerateIteration(benchmark::State& state) {
  Integer max = 100000;
  std::vector<int> values(easy_iterator::RangeIterator<Integer>(0), easy_iterator::RangeIterator<Integer>(max+1));
  benchmark::DoNotOptimize(values);
  for (auto _ : state) {
    auto i = 0;
    for (auto &v: values) {
      AssertEqual(i, v);
      ++i;
    }
  }
}

BENCHMARK(RangeLoop);
BENCHMARK(ForLoop);

BENCHMARK(EasyArrayIteration);
BENCHMARK(StdArrayIteration);

BENCHMARK(EnumerateIteration);
BENCHMARK(ManualEnumerateIteration);


BENCHMARK_MAIN();
