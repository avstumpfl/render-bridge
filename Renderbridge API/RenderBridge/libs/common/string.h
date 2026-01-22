#pragma once

#include <cctype>
#include <string>
#include <algorithm>
#include <vector>
#include <type_traits>
#include <stdexcept>
#include <chrono>
#include <filesystem>
#include "string.inl.h"

namespace common {

std::string wide_to_utf8(const std::wstring& str);
std::string multibyte_to_utf8(const std::string& str);
std::wstring utf8_to_wide(const std::string& str);
std::u32string utf8_to_utf32(const std::string& str);
bool is_valid_utf8(std::string_view str);
std::filesystem::path utf8_to_path(std::string_view utf8_string);
std::filesystem::path utf8_to_path(const std::string& utf8_string);
std::filesystem::path utf8_to_path(const std::filesystem::path&) = delete;
std::string path_to_utf8(const std::filesystem::path& path);
std::string path_to_utf8(const std::string&) = delete;

std::vector<std::string> split(const std::string& s, const std::string& delim, bool keep_empty = false);
std::vector<std::string> split_by_delims(const std::string& s, const char* delims);

void replace_all(std::string* subject, std::string_view search, std::string_view replace);
void replace_all(std::wstring* subject, std::wstring_view search, std::wstring_view replace);

template<typename S>
S replace_all(S&& string, 
    std::basic_string_view<typename S::value_type> search, 
    std::basic_string_view<typename S::value_type> replace) {
  replace_all(&string, search, replace);
  return string;
}

template<typename S>
S replace_all(const S& string, 
    std::basic_string_view<typename S::value_type> search, 
    std::basic_string_view<typename S::value_type> replace) {
  return replace_all(S(string), search, replace);
}

void replace_all_identifiers(std::string* subject, const std::string& search, const std::string& replace);
bool starts_with(std::string_view str, std::string_view pattern);
bool ends_with(std::string_view str, std::string_view pattern);
std::string get_extension(const std::string& filename);

std::string escape_percent_hex(std::string string, const char* symbols = "/\\:*?<>|");
std::string unescape_percent_hex(std::string string);

template<typename Str>
auto to_lower(Str&& str) {
  auto s = std::decay_t<Str>(std::forward<Str>(str));
  for (auto& c : s)
    c = static_cast<typename std::decay_t<Str>::value_type>(std::tolower(c));
  return s;
}

template<typename Str>
auto to_upper(Str&& str) {
  auto s = std::decay_t<Str>(std::forward<Str>(str));
  for (auto& c : s)
    c = static_cast<typename std::decay_t<Str>::value_type>(std::toupper(c));
  return s;
}

template<typename Str>
auto ltrim(Str&& str) {
  auto s = std::decay_t<Str>(std::forward<Str>(str));
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), 
    [](auto ch) { return !std::isspace(static_cast<unsigned char>(ch)); }));
  return s;
}

template<typename Str>
auto rtrim(Str&& str) {
  auto s = std::decay_t<Str>(std::forward<Str>(str));
  s.erase(std::find_if(s.rbegin(), s.rend(), 
    [](auto ch) { return !std::isspace(static_cast<unsigned char>(ch)); }).base(), s.end());
  return s;
}

inline std::string_view ltrim(std::string_view str) {
  while (!str.empty() && std::isspace(static_cast<unsigned char>(str.front())))
    str.remove_prefix(1);
  return str;
}

inline std::string_view rtrim(std::string_view str) {
  while (!str.empty() && std::isspace(static_cast<unsigned char>(str.back())))
    str.remove_suffix(1);
  return str;
}

template<typename Str>
auto trim(Str&& str) {
  return ltrim(rtrim(std::forward<Str>(str)));
}

int compare_case_insensitive(const char* a, const char* b);
inline int compare_case_insensitive(const std::string& a, const std::string& b) {
  return compare_case_insensitive(a.c_str(), b.c_str());
}
inline int compare_case_insensitive(const std::string& a, const char* b) {
  return compare_case_insensitive(a.c_str(), b);
}
inline int compare_case_insensitive(const char* a, const std::string& b) {
  return compare_case_insensitive(a, b.c_str());
}

bool contains_case_insensitive(std::string_view haystack, std::string_view needle);

std::string hex_dump(const void* begin, const void* end, 
  size_t word_size = 4, size_t column_count = 8, bool big_endian = false);

std::string to_hex_string(const void* data, size_t size);

bool from_hex_string(std::string_view string, void* data, size_t size);

template<typename T>
std::string to_hex_string(const T& value) {
  return to_hex_string(&value, sizeof(T));
}

template<typename T>
T from_hex_string(std::string_view s) {
  auto result = T{ };
  from_hex_string(s, &result, sizeof(T));
  return result;
}

template<typename T>
std::string to_bit_string(const T& value) {
  const auto bytes = reinterpret_cast<const unsigned char*>(&value);
  auto result = std::string(sizeof(T) * 8, '0');
  auto i = result.size();
  for (auto& c : result) {
    --i;
    if (bytes[i / 8] & (1 << (i % 8)))
      c = '1';
  }
  return result;
}

template<size_t N, typename... Args>
inline void format(std::string& buffer, const char(&format_string)[N], const Args&... args) {
  const auto size = detail::check_snprintf(nullptr, 0, format_string, args...);
  buffer.resize(size);
  detail::check_snprintf(buffer.data(), buffer.size() + 1, format_string, args...);
}

template<size_t N, typename... Args>
inline std::string format(const char(&format_string)[N], const Args&... args) {
  auto buffer = std::string();
  format(buffer, format_string, args...);  
  return buffer;
}

std::string format_time(const std::chrono::duration<double>& time);

template <typename... Args>
inline void parse(const char* string, const char* format_string, Args&&... args) {
  detail::check_sscanf(string, format_string, std::forward<Args>(args)...);
}

} // namespace
