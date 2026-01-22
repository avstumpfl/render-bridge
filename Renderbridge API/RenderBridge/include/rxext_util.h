#pragma once

#include "rxext.h"
#include <string>
#include <vector>
#include <sstream>
#include <chrono>

namespace rxext {

namespace detail {
  template<typename> struct is_vector : std::false_type{ };
  template<typename T> struct is_vector<vector<T>> : std::true_type{ };
  template<typename T, typename A> struct is_vector<std::vector<T, A>> : std::true_type{ };
} // namespace

inline bool operator==(const NamedValue& a, const NamedValue& b) { 
  return a.name == b.name && a.value == b.value; 
}

inline bool operator!=(const NamedValue& a, const NamedValue& b) { 
  return !(a == b); 
}

template<typename T, typename S>
T string_to_value(S&& str) {
  if constexpr (std::is_same_v<T, string>) {
    return std::forward<S>(str);
  }
  else if constexpr (std::is_same_v<T, std::string>) {
    return { str.begin(), str.end() };
  }
  else if constexpr (detail::is_vector<std::decay_t<T>>::value) {
    // vector<E>
    using E = typename T::value_type;
    auto ss = std::istringstream({ str.begin(), str.end() });
    ss >> std::boolalpha;
    auto value = T{ };
    auto element = E{ };
    while (ss.good()) {
      if constexpr (std::is_same_v<E, std::string> || std::is_same_v<E, string>) {
        // vector<string>
        for (;;) {
          const auto c = ss.get();
          if (!ss.good())
            break;
          if (value.empty())
            value.emplace_back();
          if (c == '\v')  
            value.emplace_back();
          else
            value.back().push_back(static_cast<char>(c));
        } 
      }
      else {
        // vector<E>
        ss.peek();
        if (ss.good()) {
          ss >> element;
          value.push_back(element);
        }
        ss.get();
      }
    }
    return value;
  }
  else {
    auto value = T{ };
    auto ss = std::istringstream(str.c_str());
    ss >> std::boolalpha >> value;
    return value;
  }
}

template<typename T>
string value_to_string(T&& value) {
  if constexpr (std::is_same_v<std::decay_t<T>, string>) {
    return std::forward<T>(value);
  }
  else if constexpr (std::is_same_v<std::decay_t<T>, std::string>) {
    return { value.begin(), value.end() };
  }
  else if constexpr (detail::is_vector<std::decay_t<T>>::value) {
    // vector<E>
    auto ss = std::ostringstream();
    ss << std::boolalpha;
    auto first = true;
    for (const auto& element : value) {
      if (!std::exchange(first, false))
        ss.put('\v');
      ss << element;
    }
    const auto str = std::move(ss).str();
    return { str.begin(), str.end() };
  }
  else {
    auto ss = std::ostringstream();
    ss << std::boolalpha << value;
    const auto str = std::move(ss).str();
    return { str.begin(), str.end() };
  }
}

template<typename T> 
void ValueSet::set(string_view name, const T& value) {
  auto it = std::find_if(values.begin(), values.end(), 
    [&](const NamedValue& prop) { return (prop.name == name); });
  if (it == values.end())
    it = values.emplace(values.end(), name, string());
  it->value = value_to_string(value);
}

template<typename T> 
T ValueSet::get(string_view name, T default_value) const {
  const auto it = std::find_if(values.begin(), values.end(), 
    [&](const NamedValue& prop) { return (prop.name == name); });
  if (it == values.end())
    return default_value;
  return string_to_value<T>(it->value);
}

inline bool operator==(const ValueSet& a, const ValueSet& b) { return (a.values == b.values); }
inline bool operator!=(const ValueSet& a, const ValueSet& b) { return (a.values != b.values); }

//-------------------------------------------------------------------------

#define RXEXT_ADD_EACH_FORMAT \
  X(R8_UNORM) \
  X(R8G8_UNORM) \
  X(R8G8B8A8_UNORM) \
  X(B8G8R8A8_UNORM) \
  X(R16G16B16A16_SFLOAT) \
  X(R32G32B32A32_SFLOAT) \

inline Format get_format_by_name(string_view format, 
    Format default_format = Format::R8G8B8A8_UNORM) {
  if (format.empty())
    return default_format;

  if (format == "RGBA8") return Format::R8G8B8A8_UNORM;
  if (format == "BGRA8") return Format::B8G8R8A8_UNORM;
  if (format == "RGBA16F") return Format::R16G16B16A16_SFLOAT;
  if (format == "RGBA32F") return Format::R32G32B32A32_SFLOAT;

#define X(FORMAT) if (format == #FORMAT) return Format::FORMAT;
  RXEXT_ADD_EACH_FORMAT
#undef X
  return { };
}

inline string_view get_format_name(Format format) {
  switch (format) {
    case Format::None: return "";
#define X(FORMAT) case Format::FORMAT: return #FORMAT;
    RXEXT_ADD_EACH_FORMAT
#undef X
  }
  return "";
}

//-------------------------------------------------------------------------

template<typename T>
class PtrBase {
public:
  PtrBase() = default;
  explicit PtrBase(T* p) : p(p) { }
  explicit operator bool() const { return (p != nullptr); }
  explicit operator T*() { return p; }
  bool operator==(const PtrBase& rhs) const { return (p == rhs.p); }
  bool operator!=(const PtrBase& rhs) const { return (p != rhs.p); }

protected:
  T* p{ };
};

//-------------------------------------------------------------------------

template<typename T>
class RefBase {
public:
  RefBase() = default;
  explicit RefBase(T* p) : p(p) { }
  RefBase(const RefBase& rhs) noexcept
    : p(rhs.p) {
    if (p)
      p->acquire(p);
  }
  RefBase& operator=(const RefBase& rhs) noexcept {
    auto tmp = rhs;
    std::swap(p, tmp.p);
    return *this;
  }
  RefBase(RefBase&& rhs) noexcept
    : p(std::exchange(rhs.p, nullptr)) {
  }
  RefBase& operator=(RefBase&& rhs) noexcept {
    auto tmp = std::move(rhs);
    std::swap(p, tmp.p);
    return *this;
  }
  ~RefBase() { 
    reset();
  }
  explicit operator bool() const { return (p != nullptr); }
  bool operator==(const RefBase& rhs) const { return (p == rhs.p); }
  bool operator!=(const RefBase& rhs) const { return (p != rhs.p); }
  T* get() const { return p; }

  void reset() {
    if (p)
      p->release(p); 
    p = { };
  }

  T* release() {
    return std::exchange(p, nullptr);
  }

protected:
  T* p{ };
};

//-------------------------------------------------------------------------

class TextureRef final : public RefBase<TextureP> {
public:
  using RefBase::RefBase;

  TextureDesc desc() const { return (p ? *p->get_desc(p) : TextureDesc{ }); }
};

using OnVideoFrameUnpacked = function<void(vector<TextureRef>) noexcept>;

//-------------------------------------------------------------------------

class SettingsDescBuilder {
private:
  std::ostringstream m_json;
  bool m_first_setting{ true };
  bool m_first_property{ true };

  void begin_property(std::string_view id) {
    m_json << (std::exchange(m_first_property, false) ? " " : ", ") << 
      "\"" << id << "\": ";
  }

  template <typename T>
  void set_string(std::string_view id, T string) {
    begin_property(id);
    m_json << "\"" << string << "\"";
  }

  template <typename T>
  void set_value(std::string_view id, T value) {
    begin_property(id);
    m_json << value;
  }

  void begin_setting(std::string_view id, std::string_view name, std::string_view type) {
    m_json << (std::exchange(m_first_setting, false) ? "" : ",") << "\n";
    m_json << "\"" << id << "\": {";
    set_string("name", name);
    set_string("type", type);
  }

  void end_setting() { 
    m_json << " }"; 
    m_first_property = true;
  }

public:
  SettingsDescBuilder() { 
    m_json << "{";
  }

  SettingsDescBuilder& add_int(std::string_view id, std::string_view name, 
      int min_value, int max_value) {
    begin_setting(id, name, "int");
    set_value("min_value", min_value);
    set_value("max_value", max_value);
    end_setting();
    return *this;
  }

  SettingsDescBuilder& add_double(std::string_view id, std::string_view name, 
      double min_value, double max_value) {
    begin_setting(id, name, "double");
    set_value("min_value", min_value);
    set_value("max_value", max_value);
    end_setting();
    return *this;
  }

  SettingsDescBuilder& add_bool(std::string_view id, std::string_view name) {
    begin_setting(id, name, "bool");
    end_setting();
    return *this;
  }

  SettingsDescBuilder& add_string(std::string_view id, std::string_view name) {
    begin_setting(id, name, "string");
    end_setting();
    return *this;
  }

  SettingsDescBuilder& add_enum(std::string_view id, std::string_view name,
      std::initializer_list<std::string_view> enum_names) {
    begin_setting(id, name, "enum");
    m_json << ", \"enum_names\": \"";
    auto first = true;
    for (const auto& enum_name : enum_names)
      m_json << (std::exchange(first, false) ? "" : ", ") << enum_name;
    m_json << "\"";
    end_setting();
    return *this;
  }

  std::string str() {
    m_json << "\n}";
    return m_json.str();
  }
};

//-------------------------------------------------------------------------

#define RXEXT_ADD(X) constexpr auto X = #X

namespace SettingNames {
  RXEXT_ADD(handle);
  RXEXT_ADD(name);
  RXEXT_ADD(settings_desc);
  RXEXT_ADD(instance_id);
  RXEXT_ADD(logger_id);
  RXEXT_ADD(preview);
  RXEXT_ADD(adapter_luid);
  RXEXT_ADD(filename);
  RXEXT_ADD(resolution_x);
  RXEXT_ADD(resolution_y);
  RXEXT_ADD(frame_rate);
  RXEXT_ADD(format);
  RXEXT_ADD(sync_group);
  RXEXT_ADD(layer_id);
}

namespace StateNames {
  RXEXT_ADD(resolution_x);
  RXEXT_ADD(resolution_y);
  RXEXT_ADD(frame_rate);
  RXEXT_ADD(pixel_format);
  RXEXT_ADD(format);
  RXEXT_ADD(color_space);
  RXEXT_ADD(mpeg_range);
  RXEXT_ADD(ignore_alpha);
  RXEXT_ADD(scale_y);
  RXEXT_ADD(audio_channel_count);
  RXEXT_ADD(audio_sample_rate);
}

namespace ParameterNames {
  RXEXT_ADD(sampler);
}

namespace PropertyNames {
  RXEXT_ADD(name);

  // extension
  RXEXT_ADD(api_version);
  RXEXT_ADD(build_date);
  RXEXT_ADD(dependencies);

  // stream device
  RXEXT_ADD(channel_count);

  // input stream
  RXEXT_ADD(shader_export);
  RXEXT_ADD(shader_file);
  RXEXT_ADD(shader_source);
  RXEXT_ADD(layer_names);
  RXEXT_ADD(layer_ids);

  // parameter
  RXEXT_ADD(purpose);
  RXEXT_ADD(enum_names);
  RXEXT_ADD(min_value);
  RXEXT_ADD(max_value);
  RXEXT_ADD(min_value_ui);
  RXEXT_ADD(max_value_ui);
  RXEXT_ADD(group_name);
  RXEXT_ADD(active_in_layers);
  RXEXT_ADD(direction);
  RXEXT_ADD(internal);
}

namespace PurposeNames {
  RXEXT_ADD(Visible);
  RXEXT_ADD(TimelineTime);
  RXEXT_ADD(Color);
  RXEXT_ADD(Position);
  RXEXT_ADD(Rotation);
  RXEXT_ADD(Scale);
  RXEXT_ADD(WorldMatrix);
  RXEXT_ADD(ViewMatrix);
  RXEXT_ADD(ProjectionMatrix);
  RXEXT_ADD(LayerIndex);
  RXEXT_ADD(CameraIndex);
  RXEXT_ADD(FrameIndex);
  RXEXT_ADD(FrameRate);
}
#undef RXEXT_ADD

} // namespace
