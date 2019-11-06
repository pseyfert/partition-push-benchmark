#include <benchmark/benchmark.h>
#include <boost/container/small_vector.hpp>
#include <boost/random/taus88.hpp>
#include <vector>

boost::random::taus88 random_gen;

std::vector<std::pair<int, float>> gen() {
  std::vector<std::pair<int, float>> retval;
  auto size = 60 + random_gen() % 40;

  for (std::size_t i = 0; i < size; ++i) {
    retval.emplace_back((int)random_gen(),
                        (float)random_gen() / (float)random_gen());
  }
  return retval;
}

namespace detail {
std::tuple<std::vector<float>, std::vector<int>>
reference(std::vector<std::pair<int, float>> const input) {
  std::vector<float> floats;
  std::vector<int> ints;
  floats.reserve(input.size());
  ints.reserve(input.size());

  for (auto pair : input) {
    floats.push_back(pair.second);
    ints.push_back(pair.first);
  }

  return {floats, ints};
}

std::tuple<boost::container::small_vector<float, 45>,
           boost::container::small_vector<int, 45>,
           boost::container::small_vector<float, 45>,
           boost::container::small_vector<int, 45>>
boring(std::vector<std::pair<int, float>> const input) {
  boost::container::small_vector<float, 45> floats_f;
  boost::container::small_vector<int, 45> ints_f;
  boost::container::small_vector<float, 45> floats_b;
  boost::container::small_vector<int, 45> ints_b;

  for (auto pair : input) {
    if (pair.first < 0) {
      floats_b.push_back(pair.second);
      ints_b.push_back(pair.first);
    } else {
      floats_f.push_back(pair.second);
      ints_f.push_back(pair.first);
    }
  }

  return {floats_f, ints_f, floats_b, ints_b};
}

std::tuple<boost::container::small_vector<std::tuple<float, int>, 45>,
           boost::container::small_vector<std::tuple<float, int>, 45>>
boring_aos(std::vector<std::pair<int, float>> const input) {
  boost::container::small_vector<std::tuple<float, int>, 45> out_f;
  boost::container::small_vector<std::tuple<float, int>, 45> out_b;

  for (auto pair : input) {
    if (pair.first < 0) {
      out_b.push_back({pair.second, pair.first});
    } else {
      out_f.push_back({pair.second, pair.first});
    }
  }

  return {out_f, out_b};
}

std::tuple<boost::container::small_vector<std::tuple<float, int>, 45>,
           boost::container::small_vector<std::tuple<float, int>, 45>>
smart_aos(std::vector<std::pair<int, float>> input) {
  boost::container::small_vector<std::tuple<float, int>, 45> out_f;
  boost::container::small_vector<std::tuple<float, int>, 45> out_b;

  auto pivot = std::partition(input.begin(), input.end(),
                              [](auto pair) { return pair.first < 0; });

  for (auto it = input.begin(); it != pivot; ++it) {
    out_b.push_back({it->second, it->first});
  }
  for (auto it = pivot; it != input.end(); ++it) {
    out_f.push_back({it->second, it->first});
  }

  return {out_f, out_b};
}

std::tuple<boost::container::small_vector<float, 45>,
           boost::container::small_vector<int, 45>,
           boost::container::small_vector<float, 45>,
           boost::container::small_vector<int, 45>>
smart(std::vector<std::pair<int, float>> input) {
  boost::container::small_vector<float, 45> floats_f;
  boost::container::small_vector<int, 45> ints_f;
  boost::container::small_vector<float, 45> floats_b;
  boost::container::small_vector<int, 45> ints_b;

  auto pivot = std::partition(input.begin(), input.end(),
                              [](auto pair) { return pair.first < 0; });

  for (auto it = input.begin(); it != pivot; ++it) {
    floats_b.push_back(it->second);
    ints_b.push_back(it->first);
  }
  for (auto it = pivot; it != input.end(); ++it) {
    floats_f.push_back(it->second);
    ints_f.push_back(it->first);
  }

  return {floats_f, ints_f, floats_b, ints_b};
}

std::tuple<boost::container::small_vector<float, 45>,
           boost::container::small_vector<int, 45>,
           boost::container::small_vector<float, 45>,
           boost::container::small_vector<int, 45>>
reserve_assign(std::vector<std::pair<int, float>> input) {
  boost::container::small_vector<float, 45> floats_f;
  boost::container::small_vector<int, 45> ints_f;
  boost::container::small_vector<float, 45> floats_b;
  boost::container::small_vector<int, 45> ints_b;

  auto pivot = std::partition(input.begin(), input.end(),
                              [](auto pair) { return pair.first < 0; });

  floats_b.resize(pivot - input.begin());
  ints_b.resize(pivot - input.begin());
  floats_f.resize(input.end() - pivot);
  ints_f.resize(input.end() - pivot);

  for (auto it = input.begin(); it != pivot; ++it) {
    floats_b[it - input.begin()] = it->second;
    ints_b[it - input.begin()] = it->first;
  }
  for (auto it = pivot; it != input.end(); ++it) {
    floats_f[it - pivot] = it->second;
    ints_f[it - pivot] = it->first;
  }

  return {floats_f, ints_f, floats_b, ints_b};
}

std::tuple<boost::container::small_vector<float, 45>,
           boost::container::small_vector<int, 45>,
           boost::container::small_vector<float, 45>,
           boost::container::small_vector<int, 45>>
stable_emplace(std::vector<std::pair<int, float>> input) {
  boost::container::small_vector<float, 45> floats_f;
  boost::container::small_vector<int, 45> ints_f;
  boost::container::small_vector<float, 45> floats_b;
  boost::container::small_vector<int, 45> ints_b;

  auto pivot = std::partition(input.begin(), input.end(),
                              [](auto pair) { return pair.first < 0; });

  floats_b.reserve(pivot - input.begin());
  ints_b.reserve(pivot - input.begin());
  floats_f.reserve(input.end() - pivot);
  ints_f.reserve(input.end() - pivot);

  for (auto it = input.begin(); it != pivot; ++it) {
    floats_b.stable_emplace_back(it->second);
    ints_b.stable_emplace_back(it->first);
  }
  for (auto it = pivot; it != input.end(); ++it) {
    floats_f.stable_emplace_back(it->second);
    ints_f.stable_emplace_back(it->first);
  }

  return {floats_f, ints_f, floats_b, ints_b};
}

std::tuple<boost::container::small_vector<float, 45>,
           boost::container::small_vector<int, 45>,
           boost::container::small_vector<float, 45>,
           boost::container::small_vector<int, 45>>
reserve(std::vector<std::pair<int, float>> input) {
  boost::container::small_vector<float, 45> floats_f;
  boost::container::small_vector<int, 45> ints_f;
  boost::container::small_vector<float, 45> floats_b;
  boost::container::small_vector<int, 45> ints_b;

  auto pivot = std::partition(input.begin(), input.end(),
                              [](auto pair) { return pair.first < 0; });

  floats_b.reserve(pivot - input.begin());
  ints_b.reserve(pivot - input.begin());
  floats_f.reserve(input.end() - pivot);
  ints_f.reserve(input.end() - pivot);

  for (auto it = input.begin(); it != pivot; ++it) {
    floats_b.push_back(it->second);
    ints_b.push_back(it->first);
  }
  for (auto it = pivot; it != input.end(); ++it) {
    floats_f.push_back(it->second);
    ints_f.push_back(it->first);
  }

  return {floats_f, ints_f, floats_b, ints_b};
}
} // namespace detail

static void boring_aos_main(benchmark::State &state) {
  auto store = gen();
  for (auto _ : state) {
    benchmark::DoNotOptimize(detail::boring_aos(store));
  }
}
static void smart_aos_main(benchmark::State &state) {
  auto store = gen();
  for (auto _ : state) {
    benchmark::DoNotOptimize(detail::smart_aos(store));
  }
}
static void boring_main(benchmark::State &state) {
  auto store = gen();
  for (auto _ : state) {
    benchmark::DoNotOptimize(detail::boring(store));
  }
}
static void smart_main(benchmark::State &state) {
  auto store = gen();
  for (auto _ : state) {
    benchmark::DoNotOptimize(detail::smart(store));
  }
}
static void stable_emplace_main(benchmark::State &state) {
  auto store = gen();
  for (auto _ : state) {
    benchmark::DoNotOptimize(detail::stable_emplace(store));
  }
}
static void reserve_assign_main(benchmark::State &state) {
  auto store = gen();
  for (auto _ : state) {
    benchmark::DoNotOptimize(detail::reserve_assign(store));
  }
}
static void reserve_main(benchmark::State &state) {
  auto store = gen();
  for (auto _ : state) {
    benchmark::DoNotOptimize(detail::reserve(store));
  }
}
static void reference_main(benchmark::State &state) {
  auto store = gen();
  for (auto _ : state) {
    benchmark::DoNotOptimize(detail::reference(store));
  }
}

auto compute_min = [](const std::vector<double> &v) -> double {
  return *(std::min_element(std::begin(v), std::end(v)));
};

BENCHMARK(boring_main)
    ->ComputeStatistics("min", compute_min)
    ->UseRealTime()
    ->ThreadRange(1, 4);
BENCHMARK(smart_main)
    ->ComputeStatistics("min", compute_min)
    ->UseRealTime()
    ->ThreadRange(1, 4);
BENCHMARK(boring_aos_main)
    ->ComputeStatistics("min", compute_min)
    ->UseRealTime()
    ->ThreadRange(1, 4);
BENCHMARK(smart_aos_main)
    ->ComputeStatistics("min", compute_min)
    ->UseRealTime()
    ->ThreadRange(1, 4);
BENCHMARK(stable_emplace_main)
    ->ComputeStatistics("min", compute_min)
    ->UseRealTime()
    ->ThreadRange(1, 4);
BENCHMARK(reserve_assign_main)
    ->ComputeStatistics("min", compute_min)
    ->UseRealTime()
    ->ThreadRange(1, 4);
BENCHMARK(reserve_main)
    ->ComputeStatistics("min", compute_min)
    ->UseRealTime()
    ->ThreadRange(1, 4);
BENCHMARK(reference_main)
    ->ComputeStatistics("min", compute_min)
    ->UseRealTime()
    ->ThreadRange(1, 4);

BENCHMARK_MAIN();
