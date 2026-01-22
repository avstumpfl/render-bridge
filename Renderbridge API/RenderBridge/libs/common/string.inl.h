#pragma once

namespace common {

namespace detail {
  template <typename T>
  constexpr void check_format_argument() {
    static_assert(std::is_arithmetic<T>::value || 
                  std::is_pointer<T>::value ||
                  std::is_array<T>::value, "invalid format argument");
  }

  template<typename... Args>
  size_t check_snprintf(char* buffer, size_t size, const char* format, Args... args) {
    auto check = { 0, (check_format_argument<Args>(), 0)... };
    (void)check;

    auto result = std::snprintf(buffer, size, format, args...);
    if (result < 0)
      throw std::invalid_argument("string formatting failed");
    return static_cast<size_t>(result);
  }

  template <typename... Args>
  size_t check_sscanf(const char* buffer, const char* format, Args&&... args) {
    auto result = std::sscanf(buffer, format, std::addressof(std::forward<Args>(args))...);
    if (result != sizeof...(Args))
      throw std::invalid_argument("string formatting failed");
    return static_cast<size_t>(result);
  }
} // namespace

} // namespace
