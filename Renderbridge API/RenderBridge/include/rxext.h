#pragma once

#include <cstddef>
#pragma push_macro("new")
#undef new
#include <iostream>
#include "ptl/string.hpp"
#include "ptl/vector.hpp"
#include "ptl/function.hpp"
#pragma pop_macro("new")

namespace rxext {

constexpr const char* api_version = "1.2";

template<typename T> 
using vector = ptl::vector<T>;
using string = ptl::string;
using string_view = std::string_view;
template<typename Signature> 
using function = ptl::function<Signature>;
using Handle = void*;
using OnComplete = function<void() noexcept>;
using OnTextureDownloaded = function<void(struct BufferDesc) noexcept>;
using OnVideoFrameUnpackedP = function<void(struct TextureP**, size_t) noexcept>;

struct NamedValue {
  string name;
  string value;

  NamedValue() = default;
  NamedValue(string_view name, string_view value) : name(name), value(value) { }

  friend bool operator<(const NamedValue& a, const NamedValue& b) {
    return std::tie(a.name, a.value) < std::tie(b.name, b.value);
  }
};

struct ValueSet {
  vector<NamedValue> values;

  template<typename T> void set(string_view name, const T& value);
  template<typename T = std::string> T get(string_view name, T default_value = T{ }) const;
  std::string get(string_view name, const char* default_value) const {
    return get<std::string>(name, default_value);
  }
  friend bool operator<(const ValueSet& a, const ValueSet& b) {
    return (a.values < b.values);
  }
};

enum class EventSeverity : size_t {
  Verbose = static_cast<size_t>(-1),
  Info    = 0,
  Warning = 1,
  Error   = 2,
};

enum class EventCategory : size_t {
  Message        = 0,
  DevicesChanged = 4,
  Failed         = 11,
  StreamsChanged = 12,
};

enum class ParameterType : size_t {
  Bool,
  Int,
  Value,
  Vector2, Vector3, Vector4, Matrix3, Matrix4,
  String,
  Texture,
  Data,
};

enum class HandleType : size_t {
  None             = 0,
  OPAQUE_FD        = 0x9586,
  OPAQUE_WIN32     = 0x9587,
  OPAQUE_WIN32_KMT = 0x9588,
  D3D12_TILEPOOL   = 0x9589,
  D3D12_RESOURCE   = 0x958A,
  D3D11_IMAGE      = 0x958B,
  D3D11_IMAGE_KMT  = 0x958C,
  D3D_FENCE        = 0x9594,
  RX_TEXTURE       = 1,
};

struct ShareHandle {
  HandleType type;
  Handle handle;
  uint64_t process_id;
  uint64_t memory_size;
  uint64_t memory_offset;
  bool dedicated;
};

// values correspond to VK_FORMAT
enum class Format : size_t {
  None = 0,
  R8_UNORM = 9,
  R8G8_UNORM = 16,
  R8G8B8A8_UNORM = 37,
  B8G8R8A8_UNORM = 44,
  R16G16B16A16_SFLOAT = 97,
  R32G32B32A32_SFLOAT = 109,
};

enum class RenderResult {
  Succeeded = 1,
  Incomplete = 3,
  Failed = 6,
};

enum class AsyncPolicy : size_t {
  Default,
  MainThread,
};

struct TextureDesc {
  size_t width;
  size_t height;
  Format format;
  bool is_target;
  ShareHandle share_handle;
};

enum class SyncStrategy {
  None,
  BinarySemaphore,
  TimelineSemaphore,
};

struct SyncDesc {
  SyncStrategy sync_strategy;
  ShareHandle share_handle;
  uint64_t value;
};

struct BufferDesc {
  const void* data;
  size_t size;
  size_t pitch;
};

struct VideoFrame {
  size_t resolution_x;
  size_t resolution_y;
  string pixel_format;
  vector<BufferDesc> planes;
};

struct AudioFrame {
  size_t sample_rate;
  vector<BufferDesc> channels;
};

struct TextureP {
  void (*acquire)(TextureP* p) noexcept;
  void (*release)(TextureP* p) noexcept;
  const TextureDesc* (*get_desc)(TextureP* p) noexcept;
};

struct HostContextP {
  void (*send_event)(HostContextP* p, EventSeverity severity, EventCategory category, string_view message) noexcept;
  void (*monitor_value)(HostContextP* p, const char* name, double value, bool average) noexcept;
  string (*resolve_storage_filename)(HostContextP* p, string_view storage_filename) noexcept;
  string (*get_userdata_path)(HostContextP* p, string_view path) noexcept;
  void (*async)(HostContextP* p, AsyncPolicy policy, double delay_seconds, OnComplete callback) noexcept;
  TextureP* (*create_texture)(HostContextP* p, const TextureDesc* desc);
  void (*download_texture)(HostContextP* p, TextureP* texture, OnTextureDownloaded on_downloaded) noexcept;
  void (*upload_texture)(HostContextP* p, TextureP* texture, const BufferDesc* buffer, 
    bool upload_copy, OnComplete callback) noexcept;
  void (*unpack_video_frame)(HostContextP* p, const VideoFrame* video_frame, 
    OnComplete on_data_read, OnVideoFrameUnpackedP on_unpacked) noexcept;
  void (*send_audio_frame)(HostContextP* p, const AudioFrame* frame, OnComplete on_complete) noexcept;
};

struct ParameterP {
  ParameterType (*type)(const ParameterP* p) noexcept;
  string_view (*name)(const ParameterP* p) noexcept;
  void (*set_value)(ParameterP* p, const void* data, size_t size) noexcept;
  void (*get_value)(const ParameterP* p, void* data, size_t* size) noexcept;
  bool (*set_property)(ParameterP* p, string_view name, string value) noexcept;
  string (*get_property)(const ParameterP* p, string_view name) noexcept;
};

struct InputStreamP {
  void (*release)(InputStreamP* p) noexcept;
  bool (*initialize)(InputStreamP* p, HostContextP* host) noexcept;
  bool (*update_settings)(InputStreamP* p, ValueSet settings) noexcept;
  string (*get_property)(InputStreamP* p, string_view name) noexcept;
  bool (*set_property)(InputStreamP* p, string_view name, string value) noexcept;
  ValueSet (*get_state)(InputStreamP* p) noexcept;
  size_t (*get_parameter_count)(InputStreamP* p) noexcept;
  ParameterP* (*get_parameter)(InputStreamP* p, size_t index) noexcept;
  void (*set_video_requested)(InputStreamP* p, bool requested) noexcept;
  void (*set_audio_requested)(InputStreamP* p, bool requested) noexcept;
  bool (*update)(InputStreamP* p) noexcept;
  SyncDesc (*before_render)(InputStreamP* p) noexcept;
  RenderResult (*render)(InputStreamP* p) noexcept;
  SyncDesc (*after_render)(InputStreamP* p) noexcept;
};

struct OutputStreamP {
  void (*release)(OutputStreamP* p) noexcept;
  bool (*initialize)(OutputStreamP* p, HostContextP* host) noexcept;
  bool (*update_settings)(OutputStreamP* p, ValueSet settings) noexcept;
  string (*get_property)(OutputStreamP* p, string_view name) noexcept;
  bool (*set_property)(OutputStreamP* p, string_view name, string value) noexcept;
  ValueSet (*get_state)(OutputStreamP* p) noexcept;
  void (*send_audio_frame)(OutputStreamP* p, const AudioFrame* audio_frame, OnComplete on_complete) noexcept;
  TextureP* (*get_target)(OutputStreamP* p) noexcept;
  SyncDesc (*before_render)(OutputStreamP* p) noexcept;
  SyncDesc (*after_render)(OutputStreamP* p) noexcept;
  void (*present)(OutputStreamP* p) noexcept;
  void (*swap)(OutputStreamP* p) noexcept;
};

struct StreamDeviceP {
  void (*release)(StreamDeviceP* p) noexcept;
  bool (*initialize)(StreamDeviceP* p, HostContextP* host) noexcept;
  bool (*update_settings)(StreamDeviceP* p, ValueSet settings) noexcept;
  string (*get_property)(StreamDeviceP* p, string_view name) noexcept;
  bool (*set_property)(StreamDeviceP* p, string_view name, string value) noexcept;
  vector<ValueSet> (*enumerate_stream_settings)(StreamDeviceP* p) noexcept;
  InputStreamP* (*create_input_stream)(StreamDeviceP* p, ValueSet settings) noexcept;
  OutputStreamP* (*create_output_stream)(StreamDeviceP* p, ValueSet settings) noexcept;
  bool (*set_active_streams)(StreamDeviceP* p, 
    InputStreamP* const* input_streams, size_t input_stream_count, 
    OutputStreamP* const* output_streams, size_t output_stream_count) noexcept;
  bool (*update)(StreamDeviceP* p) noexcept;
  SyncDesc (*before_render)(StreamDeviceP* p) noexcept;
  void (*render)(StreamDeviceP* p) noexcept;
  SyncDesc (*after_render)(StreamDeviceP* p) noexcept;
};

struct ExtensionP {
  bool (*initialize)(ExtensionP* p, HostContextP* host) noexcept;
  void (*shutdown)(ExtensionP* p) noexcept;
  string (*get_property)(ExtensionP* p, string_view name) noexcept;
  bool (*set_property)(ExtensionP* p, string_view name, string value) noexcept;
  vector<ValueSet> (*enumerate_stream_device_settings)(ExtensionP* p) noexcept;
  StreamDeviceP* (*create_stream_device)(ExtensionP* p, ValueSet settings) noexcept;
};

using TextureFormat [[deprecated("renamed to Format")]] = Format;
} // namespace

#if defined(_WIN32)
# define RXEXT_API extern "C" __declspec(dllexport)
#else
# define RXEXT_API extern "C" __attribute__ ((visibility ("default")))
#endif

RXEXT_API rxext::ExtensionP* rxext_open();
RXEXT_API void rxext_close(rxext::ExtensionP* extension);
