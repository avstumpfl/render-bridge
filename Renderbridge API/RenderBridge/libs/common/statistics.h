#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <numeric>
#include <tuple>
#include <vector>
#include <cfloat>
#include <ostream>
#include <iterator>
#include "common/to_double.h"

namespace common {

template<typename T>
class SampleCache {
public:
  using Buffer = std::vector<T>;

  explicit SampleCache(size_t capacity) {
    m_buffer.reserve(std::max(capacity, size_t(1)));
  }
  size_t size() const { return m_buffer.size(); }
  size_t capacity() const { return m_buffer.capacity(); }
  bool empty() const { return m_buffer.empty(); }
  bool filled() const { return size() == capacity(); }

  template<typename... Args>
  T push(Args&&... args) {
    if (!filled()) {
      m_buffer.emplace_back(std::forward<Args>(args)...);
      ++m_index;
      return T();
    }

    T replaced{ std::forward<Args>(args)... };
    std::swap(m_buffer[m_index % m_buffer.size()], replaced);
    ++m_index;
    return replaced;
  }

  void reset() {
    m_buffer.clear();
  }

  void reset(size_t capacity) {
    m_buffer = Buffer();
    m_buffer.reserve(capacity);
  }

  auto begin() -> typename Buffer::iterator { return m_buffer.begin(); }
  auto end() -> typename Buffer::iterator { return  m_buffer.end(); }
  auto begin() const -> typename Buffer::const_iterator { return m_buffer.begin(); }
  auto end() const -> typename Buffer::const_iterator { return  m_buffer.end(); }

private:
  Buffer m_buffer;
  size_t m_index{ };
};
  
template<typename T>
class WindowedStatistic {
public:
  explicit WindowedStatistic(size_t capacity) 
    : m_samples(capacity) {
  }
  bool initialized() const { 
    return m_samples.filled(); 
  }
  void reset() {
    *this = WindowedStatistic(m_samples.capacity());
  }
  size_t capacity() const { return m_samples.capacity(); }
  void push(const T& value) {
    if (!m_samples.filled()) {
      m_samples.push(value);
      update(value, 1);
    } 
    else {
      auto removed = m_samples.push(value);
      update(removed, -1);
      update(value, 1);
    }
  }
  T mean() const { 
    return from_double<T>(m_mean);
  }
  T variance() const {
    if (m_samples.size() <= 1)
      return T(0.0);
    const auto variance_correction = 
      static_cast<double>(m_samples.size()) / static_cast<double>(m_samples.size() - 1);      
    return T(m_variance * variance_correction / m_weight); 
  }
  T std_deviation() const {
    using namespace common;
    return T(!m_samples.empty() ? T(std::sqrt(to_double(variance()))) : T(0.0));
  }
  T skewness() const {
    auto variance = to_double(m_variance);
    return T(variance != 0 ? m_skew * sqrt(m_weight / (variance * variance *  variance)) : 0.0);
  }
  T min() const {
    return (!m_samples.empty() ? *std::min_element(m_samples.begin(), m_samples.end()) : T());
  }
  T max() const {
    return (!m_samples.empty() ? *std::max_element(m_samples.begin(), m_samples.end()) : T());
  }

private:
  void update(const T& value, const double weight) {
    const auto data = to_double(value);

    // sum weights
    const auto tw = m_weight + weight;

    // (x - mu_n)
    const auto delta = data - m_mean;

    // weight factor
    const auto wf = weight / tw;

    // mean
    m_mean += wf * delta;

    // (x - mu_n+1)
    const auto delta_u = data - m_mean;

    // skew
    m_skew += wf * delta * ((m_weight - weight) * 
      to_double(delta) * to_double(delta_u) - 3.0 * to_double(m_variance));

    // variance
    m_variance += weight * to_double(delta) * delta_u;

    // update weight
    m_weight = tw;
  }

  SampleCache<T> m_samples;
  double m_mean{ };
  double m_skew{ };
  double m_variance{ };
  double m_weight{ };
};

template<typename T>
class ExponentialStatistic {
public:
  explicit ExponentialStatistic(const T& init = T(), double default_weight = 0.5) 
    : m_mean(to_double(init)), m_min(m_mean), m_max(m_mean), m_default_weight{ default_weight } {
  }
  void reset(const T& init) {
    *this = ExponentialStatistic(init, m_default_weight);
  }
  void push(const T& data, double weight) {
    update(data, weight);
  }
  void push(const T& value) {
    update(value, m_default_weight);
  }
  T mean() const { 
    return T(m_mean);
  }
  T variance() const { 
    return T(m_variance);
  }
  T std_deviation() const {
    return T(std::sqrt(to_double(variance())));
  }
  T skewness() const {
    auto variance = to_double(m_variance);
    return T((variance != 0) ? m_skewness * sqrt(1.0 / (variance * variance *  variance)) : 0.0);
  }
  T min() const {
    return T(m_min);
  }
  T max() const {
    return T(m_max);
  }

private:
  void update(const T& value, const double weight) {
    const auto data = to_double(value);

    // sum weights
    // always 1.0

    // min && max
    const auto midpoint = (m_max - m_min) / 2;
    m_min += midpoint * weight;
    m_max -= midpoint * weight;
    m_min = (data < m_min) ? data : m_min;
    m_max = (data > m_max) ? data : m_max;

    // (x - mu_n)
    const auto delta = data - m_mean;

    // mean
    m_mean += weight * delta;

    // (x - mu_n+1)
    const auto delta_u = data - m_mean;

    // skewness
    m_skewness += weight * delta *((1.0 - 2.0 * weight) * delta * delta_u - 3.0 * m_variance);

    // variance
    m_variance += weight * delta * delta_u;
  }

  double m_mean{ };
  double m_variance{ };
  double m_skewness{ };
  double m_min{ };
  double m_max{ };
  double m_default_weight{ };
};

template<typename T>
class OnlineStatistic {
public:
  T mean() const {
    return T((m_n > 0) ? m_mean : 0.0);
  }
  T variance() const {
    return T((m_n > 1) ? m_variance / static_cast<double>(m_n - 1) : 0.0);
  }
  T std_dev() const {
    return T(std::sqrt(to_double(variance())));
  }
  T min() const {
    return T(m_min);
  }
  T max() const {
    return T(m_max);
  }
  T skewness() const {
    return T((m_n > 2) ? m_skew * sqrt(static_cast<double>(m_n) / (m_variance * m_variance *  m_variance)) : 0.0);
  }
  size_t samples() const {
    return m_n;
  }

  // use compensated summation??
  void push(const T& value) {
    const auto data = to_double(value);
    ++m_n;

    // min && max
    m_min = (data < m_min) ? data : m_min;
    m_max = (data > m_max) ? data : m_max;

    const auto delta = data - m_mean;
    // mean
    m_mean += delta / static_cast<double>(m_n);

    const auto delta_n = data - m_mean;
    const auto delta_prod = delta * delta_n;

    // skew
    m_skew += (delta_prod * delta * static_cast<double>(m_n - 2) - 3 * delta * m_variance) / static_cast<double>(m_n);

    // variance
    m_variance += delta_prod;
  }

  template <typename ITER>
  void push(ITER begin, ITER end) {
    std::for_each(begin, end, [this](const double& elem) {
      this->push(elem);
    });
  }

  template <typename ITER>
  void push_n(ITER begin, size_t n) {
    ITER end = std::advance(begin, n);
    push(begin, end);
  }

private:
  double m_mean{ };
  double m_variance{ };
  double m_skew{ };
  double m_min{ std::numeric_limits<double>::max() };
  double m_max{ std::numeric_limits<double>::lowest() };
  size_t m_n{ };
};

template<typename It> // requires a sorted range
auto median(It begin, It end) -> typename std::iterator_traits<It>::value_type {
  using Value = typename std::iterator_traits<It>::value_type;
  if (begin == end)
    return Value(); 

  auto count = std::distance(begin, end);
  if (count % 2 != 0) { 
    // uneven: use midpoint of distribution
    return *std::next(begin, count / 2);
  } 
  else { 
    // even: use mean
    std::advance(begin, count / 2 - 1);
    return (*begin + *std::next(begin)) / 2;
  }
}

// requires sorted range
template <typename It>
auto quartiles(It begin, It end) {
  using Value = typename std::iterator_traits<It>::value_type;
  
  auto q1 = Value{};
  auto q2 = Value{};
  auto q3 = Value{};
  
  if (begin == end)
    return std::make_tuple(q1, q2, q3); 

  auto count = std::distance(begin, end);

  // uneven count ==> use midpoint
  auto begin_q1 = begin;
  auto end_q1 = It{ };

  auto begin_q3 = It{ };
  auto end_q3 = end;
  if (count % 2 != 0) {
    auto median_pos = std::next(begin, count/2);

    q1 = median(begin, median_pos);
    q2 = *median_pos;
    q3 = median(std::next(median_pos), end);
  }
  else {
    auto median_pos_0 = std::next(begin, count / 2 - 1);
    auto median_pos_1 = std::next(median_pos_0);

    q1 = median(begin, median_pos_1);
    q2 = (*median_pos_0 + *median_pos_1) / 2;
    q3 = median(median_pos_1, end);
  }

  return std::make_tuple(q1, q2, q3);
}


template <typename ITERATOR>
auto midhinge(ITERATOR begin, ITERATOR end) -> typename std::iterator_traits<ITERATOR>::value_type {
  auto size = std::distance(begin, end);
  return (
    median(begin, std::next(begin, size >> 1)) +
    median(std::next(begin, (size >> 1) + (size & 1)), end)
  ) / 2;
}

template <typename ITERATOR>
auto trimean(ITERATOR begin, ITERATOR end) -> typename std::iterator_traits<ITERATOR>::value_type {
  return (
    median(begin, end) +
    midhinge(begin, end)
  ) / 2;
}

 } // namespace