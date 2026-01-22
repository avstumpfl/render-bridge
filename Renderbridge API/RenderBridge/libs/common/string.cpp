
#include <cstring>
#include <cassert>
#include <iterator>
#include <array>
#include <bitset>
#include <cmath>
#include "common/string.h"
#include "utf8/utf8.h"

#if defined(_WIN32)
#  define WIN32_LEAN_AND_MEAN
#  define NOMINMAX
#  include <Windows.h>
#endif // _WIN32

namespace common {

std::string wide_to_utf8(const std::wstring& str) {
#if defined(_WIN32)
  auto result = std::string();
  result.resize(WideCharToMultiByte(CP_UTF8, 0, 
    str.data(), static_cast<int>(str.size()), 
    NULL, 0, 
    NULL, 0));
  WideCharToMultiByte(CP_UTF8, 0, 
    str.data(), static_cast<int>(str.size()), 
    result.data(), static_cast<int>(result.size()),
    NULL, 0);
  return result;
#else // !_WIN32
  return std::string(begin(str), end(str));
#endif // !_WIN32
}

std::string multibyte_to_utf8(const std::string& str) {
#if defined(_WIN32)
  auto temp = std::wstring();
  temp.resize(MultiByteToWideChar(CP_ACP, 0, 
    str.data(), static_cast<int>(str.size()),
    NULL, 0));
  MultiByteToWideChar(CP_ACP, 0, 
    str.data(), static_cast<int>(str.size()), 
    temp.data(), static_cast<int>(temp.size()));
  return wide_to_utf8(temp);
#else // !_WIN32
  return str;
#endif // !_WIN32
}

std::wstring utf8_to_wide(const std::string& str) {
#if defined(_WIN32)
  auto result = std::wstring();
  result.resize(MultiByteToWideChar(CP_UTF8, 0, 
    str.data(), static_cast<int>(str.size()), 
    NULL, 0));
  MultiByteToWideChar(CP_UTF8, 0, 
    str.data(), static_cast<int>(str.size()), 
    result.data(), static_cast<int>(result.size()));
  return result;
#else // _WIN32
  return std::wstring(begin(str), end(str));
#endif // _WIN32
}

std::u32string utf8_to_utf32(const std::string& str) {
  auto result = std::u32string();
  utf8::utf8to32(begin(str), end(str), std::back_inserter(result));
  return result;
}

bool is_valid_utf8(std::string_view str) {
  return utf8::is_valid(str.begin(), str.end());
}

std::filesystem::path utf8_to_path(std::string_view utf8_string) {
#if defined(__cpp_char8_t)
  static_assert(sizeof(char) == sizeof(char8_t));
  return std::filesystem::path(
    reinterpret_cast<const char8_t*>(utf8_string.data()),
    reinterpret_cast<const char8_t*>(utf8_string.data() + utf8_string.size()));
#else
  return std::filesystem::u8path(utf8_string);
#endif
}

std::filesystem::path utf8_to_path(const std::string& utf8_string) {
  return utf8_to_path(std::string_view(utf8_string));
}

std::string path_to_utf8(const std::filesystem::path& path) {
#if defined(__cpp_char8_t)
  static_assert(sizeof(char) == sizeof(char8_t));
#endif
  const auto u8string = path.generic_u8string();
  return std::string(
    reinterpret_cast<const char*>(u8string.data()),
    reinterpret_cast<const char*>(u8string.data() + u8string.size()));
}

std::vector<std::string> split(const std::string& s, const std::string& delim, bool keep_empty) {
  auto result = std::vector<std::string>();
  if (delim.empty()) {
    result.push_back(s);
    return result;
  }
  auto substart = s.begin();
  for (;;) {
    auto subend = std::search(substart, s.end(), delim.begin(), delim.end());
    auto temp = std::string(substart, subend);
    if (keep_empty || !temp.empty()) {
      result.push_back(std::move(temp));
    }
    if (subend == s.end()) {
      break;
    }
    substart = subend + delim.size();
  }
  return result;
}

std::vector<std::string> split_by_delims(const std::string& s, const char* delims) {
  auto is_delim = std::bitset<255>();
  for (; *delims; ++delims)
    is_delim[static_cast<unsigned char>(*delims)] = true;

  auto result = std::vector<std::string>();
  auto begin = s.begin();
  auto in_token = false;
  for (auto it = s.begin(), end = s.end(); it != end; ++it) {
    if (is_delim[*it]) {
      if(in_token) {
        result.emplace_back(begin, it);
        in_token = false;
      }
    }
    else if(!in_token) {
      begin = it;
      in_token = true;
    }
  }
  if (in_token)
    result.emplace_back(begin, s.end());
  return result;
}

void replace_all(std::string* subject, 
    std::string_view search, std::string_view replace) {
  auto pos = size_t{ };
  while ((pos = subject->find(search, pos)) != std::string::npos) {
    subject->replace(pos, search.length(), replace);
    pos += replace.length();
  }
}

void replace_all(std::wstring* subject, 
    std::wstring_view search, std::wstring_view replace) {
  auto pos = size_t{ };
  while ((pos = subject->find(search, pos)) != std::wstring::npos) {
    subject->replace(pos, search.length(), replace);
    pos += replace.length();
  }
}

void replace_all_identifiers(std::string* subject, const std::string& search,
          const std::string& replace) {
  auto is_delimiter = [](char c) { return !isalnum(c) && c != '_'; };
  auto word_boundary = [&](const std::string& str, size_t pos, size_t len) {
    if (pos > 0 && !is_delimiter(str[pos - 1]))
      return false;
    if (pos + len < str.length() && !is_delimiter(str[pos + len]))
      return false;
    return true;
  };
  size_t pos = 0;
  while ((pos = subject->find(search, pos)) != std::string::npos) {
    if (word_boundary(*subject, pos, search.length())) {
      subject->replace(pos, search.length(), replace);
      pos += replace.length();
    }
    else {
      pos += search.length();
    }
  }
}

bool starts_with(std::string_view str, std::string_view pattern) {
  return (str.size() >= pattern.size() && 
    std::equal(pattern.begin(), pattern.end(), str.begin()));
}

bool ends_with(std::string_view str, std::string_view pattern) {  
  return (pattern.size() <= str.size() &&
    std::equal(pattern.rbegin(), pattern.rend(), str.rbegin()));
}

std::string get_extension(const std::string& filename) {
  auto it = filename.find_last_of('.');
  const auto slash = std::min(filename.find_last_of('/'), filename.find_last_of('\\'));
  if (it == std::string::npos || it == 0 || it < slash)
    return { };
  return to_lower(filename.substr(it));
}

int compare_case_insensitive(const char* a, const char* b) {
#if defined(_MSC_VER)
  return _strcmpi(a, b);
#else
  return strcasecmp(a, b);
#endif
}

bool contains_case_insensitive(std::string_view haystack, std::string_view needle) {
  return (std::search(haystack.begin(), haystack.end(), needle.begin(), needle.end(),
    [](char ch1, char ch2) { return std::toupper(ch1) == std::toupper(ch2); }) != haystack.end());
}

std::string hex_dump(const void* _begin, const void* _end, 
    size_t word_size, size_t column_count, bool big_endian) {
  assert(_begin <= _end);
  const auto* const begin = static_cast<const uint8_t*>(_begin);
  const auto* const end = static_cast<const uint8_t*>(_end);

  if (!word_size) {
    word_size = end - begin;
    big_endian = true;
  }

  std::string result;
  if (word_size > 0) {
    const size_t word_count = (end - begin) / word_size;
    result.reserve(word_count * (2 * word_size + 1));

    if (!column_count)
      column_count = word_count;

    const char hex[] = "0123456789ABCDEF";
    for (size_t i = 0; i < word_count; ++i) {
      const auto* word = begin + i * word_size;
      for (size_t j = 0; j < word_size; ++j) {
        auto byte = word[big_endian ? j : word_size - 1 - j];
        result += hex[byte >> 4];
        result += hex[byte & 0xF];
      }
      result += (i % column_count == column_count - 1 ? '\n' : ' ');
    }
  }
  return result;
}
std::string to_hex_string(const void* data, size_t size) {
  auto result = std::string();
  result.reserve(size * 2);
  auto begin = static_cast<const uint8_t*>(data);
  auto end = begin + size;
  const char hex[] = "0123456789ABCDEF";
  for (auto c = begin; c != end; ++c) {
    result += hex[*c >> 4];
    result += hex[*c & 0xF];
  }
  return result;
}

bool from_hex_string(std::string_view string, void* data, size_t size) {
  if (!size || size * 2 != string.size())
    return false;
  auto it = static_cast<uint8_t*>(data);
  auto i = size_t{ };
  *it = 0x00;
  for (auto c : string) {
    if (c >= '0' && c <= '9')
      *it += (c - '0');
    else if (c >= 'A' && c <= 'F')
      *it += (c - 'A' + 10);
    else
      return false;

    if (++i % 2) 
      *it <<= 4;
    else if (i < 2 * size)
      *++it = 0x00;
  }
  return true;
}

std::string escape_percent_hex(std::string string, const char* symbols) {
  const char hex[] = "0123456789ABCDEF";

  auto i = size_t();
  for (;;) {
    i = string.find('%', i);
    if (i == std::string::npos)
      break; 
    string.replace(i, 1, "%%");
    i += 2;
  }

  i = size_t();
  for (;;) {
    i = string.find_first_of(symbols, i);
    if (i == std::string::npos)
      break;

    const auto c = string[i];      
    const auto escaped = std::array{
      '%', hex[c / 16], hex[c % 16], '\0'
    };
    string.replace(i, 1, escaped.data());
    i += 2;
  }  
  return string;
}

std::string unescape_percent_hex(std::string string) {
  auto i = size_t();
  for (;;) {
    i = string.find('%', i);
    if (i == std::string::npos)
      break;
    if (i > string.size() - 2)
      break;
    if (string[i + 1] == '%') {
      string.erase(i, 1);
      ++i;
      continue;
    }
    auto parse_hex_symbol = [](char c) {
      return (c <= '9' ? c - '0' : 
              c <= 'Z' ? c - 'A' + 10 : 
              c - 'a' + 10);
    };
    const auto upper = parse_hex_symbol(string[i + 1]);
    const auto lower = parse_hex_symbol(string[i + 2]);
    const auto unescaped = std::array{
      static_cast<char>(upper * 16 + lower), '\0'
    };    
    string.replace(i, 3, unescaped.data());
    ++i;
  }
  return string;
}

std::string format_time(const std::chrono::duration<double>& time) {
  if (!std::isfinite(time.count()))
    return (time.count() < 0 ? "-inf." : "inf.");
  if (time > std::chrono::years(1) || time < -std::chrono::years(1))
    return common::format("%syears", (time.count() < 0 ? "-" : ""));
  if (time.count() && time > std::chrono::milliseconds(-1) && time < std::chrono::milliseconds(1))
    return common::format("%ius", std::chrono::duration_cast<std::chrono::microseconds>(time).count());
  const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(time).count();
  const auto minutes = (std::abs(ms) / 1000) / 60;
  const auto seconds = (std::abs(ms) / 1000) % 60;
  const auto milliseconds = (std::abs(ms) % 1000);
  if (minutes)
    return common::format("%s%02i:%02i:%02i.%03i", 
      (ms < 0 ? "-" : ""), minutes / 60, minutes % 60, seconds, milliseconds);
  if (seconds)
    return common::format("%s%i.%03is", 
      (ms < 0 ? "-" : ""), seconds, milliseconds);
  return common::format("%s%ims", (ms < 0 ? "-" : ""), milliseconds);
}

} // namespace
