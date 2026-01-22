#pragma once

#include <combaseapi.h>

template<typename T>
class ComPointer {
public:
  ComPointer() = default;
  explicit ComPointer(T* pointer)
    : m_pointer(pointer) {
  }
  ComPointer(ComPointer&& rhs) noexcept
    : m_pointer(std::exchange(rhs.m_pointer, nullptr)) {
  }
  ComPointer& operator=(ComPointer&& rhs) noexcept {
    auto tmp = std::move(rhs);
    std::swap(m_pointer, tmp.m_pointer);
    return *this;
  }
  ~ComPointer() {
    release();
  }
  operator T*() const { return m_pointer; }
  T* operator ->() { return m_pointer; }
  T** operator&() {
    *this = { };
    return &m_pointer;
  }
  void release() {
    if (auto pointer = std::exchange(m_pointer, nullptr))
      pointer->Release();
  }

private:
  T* m_pointer{ };
};

template <typename T>
ComPointer<T> com_cast(IUnknown* unknown) {
  auto pointer = std::add_pointer_t<T>{};
  if (unknown && SUCCEEDED(unknown->QueryInterface(IID_PPV_ARGS(&pointer))))
    return ComPointer<T>(pointer);
  return { };
}
