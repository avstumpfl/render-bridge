#pragma once

#include <cmath>
#include <limits>
#include <chrono>
#include <stdexcept>

namespace common {

// see: https://github.com/OculusVR/Flicks
using Flicks = std::chrono::duration<std::chrono::nanoseconds::rep,
                                     std::ratio<1, 705600000>>;

using Speed = double;
using Duration = std::chrono::duration<double>;

constexpr Duration infinite_duration() noexcept { 
  return Duration(std::numeric_limits<double>::infinity()); 
}

inline bool is_finite(const Duration& duration) noexcept {
  return std::isfinite(duration.count());
}

template<class Rep, class Period>
inline Flicks round_to_flicks(const std::chrono::duration<Rep, Period>& duration) {
  return std::chrono::duration_cast<Flicks>(Duration(duration) + Duration(0.5 / 705600000.0));
}

inline Flicks frame_rate_to_flicks(double frame_rate) {
  if (frame_rate <= 0.0)
    throw std::range_error("invalid frame rate");
  return round_to_flicks(Duration(1.0 / frame_rate));
}

} // namespace
