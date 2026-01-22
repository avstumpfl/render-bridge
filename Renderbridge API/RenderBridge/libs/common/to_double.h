#pragma once

#include <chrono>

namespace common {

namespace detail {
  template<typename T>
  struct FromDouble {
    static T convert(double value) {
      return static_cast<T>(value);
    }
  };

  template<typename R, typename P>
  struct FromDouble<std::chrono::duration<R, P>> {
    static std::chrono::duration<R, P> convert(double value) {
      return std::chrono::duration_cast<std::chrono::duration<R, P>>(std::chrono::duration<double>(value));
    }
  };
} // namespace

template<typename T>
double to_double(const T& value) {
  return static_cast<double>(value);
}

template<typename T>
T from_double(double value) {
  return detail::FromDouble<T>::convert(value);
}

template<typename R, typename P>
double to_double(const std::chrono::duration<R, P>& duration) {
  return std::chrono::duration<double>(duration).count();
}

} // namespace
