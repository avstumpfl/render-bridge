
#include "common/string.h"

#if defined(_WIN32)

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <array>

namespace util {

std::string get_temp_filename() {
  std::array<wchar_t, MAX_PATH> path;
  std::array<wchar_t, MAX_PATH + 1> tempfile;
  GetTempPathW(static_cast<DWORD>(path.size()), path.data());
  GetTempFileNameW(path.data(), L"", 0, tempfile.data());
  return common::wide_to_utf8(tempfile.data());
}

} // namespace

#else // !_WIN32

#include <filesystem>
#include <random>

namespace util {

std::string get_temp_filename() {
  const auto filename = std::to_string(std::random_device()());
  auto path = (std::filesystem::temp_directory_path() / filename).string();
  std::fclose(std::fopen(path.c_str(), "wb"));
  return path;
}

} // namespace

#endif // !_WIN32
