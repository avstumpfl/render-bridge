#pragma once

#include "rxext_util.h"
#include <array>
#include <mutex>
#include <map>

namespace rxext {

using SendVideoFrame = function<void(const VideoFrame&, OnComplete) noexcept>;
using SendAudioFrame = function<void(const AudioFrame&, OnComplete) noexcept>;
  
class Parameter : public ParameterP {
public:
  static const Parameter* cast(const ParameterP* self) { return static_cast<const Parameter*>(self); }
  static Parameter* cast(ParameterP* self) { return static_cast<Parameter*>(self); }

  Parameter() 
    : ParameterP{
      [](const ParameterP* p) noexcept { return cast(p)->type(); },
      [](const ParameterP* p) noexcept { return cast(p)->name(); },
      [](ParameterP* p, const void* data, size_t size) noexcept { cast(p)->set_value(data, size); },
      [](const ParameterP* p, void* data, size_t* size) noexcept { cast(p)->get_value(data, size); },
      [](ParameterP* p, string_view name, string value) noexcept { return cast(p)->set_property(name, std::move(value)); },
      [](const ParameterP* p, string_view name) noexcept { return cast(p)->get_property(name); },
    } { }
  virtual ~Parameter() = default;
  virtual ParameterType type() const noexcept = 0;
  virtual string_view name() const noexcept = 0;
  virtual void set_value(const void* data, size_t size) noexcept = 0;
  virtual void get_value(void* data, size_t* size) const noexcept = 0;
  virtual bool set_property(string_view name, string value) noexcept { return false; }
  virtual string get_property(string_view name) const noexcept { return { }; }

  template<typename T>
  bool set_property(string_view name, T&& value) noexcept { 
    return set_property(name, value_to_string(std::forward<T>(value)));
  }
};

class HostContext final {
public:
  HostContext() = default;
  explicit HostContext(HostContextP* p) : p(p) { }
  explicit operator bool() const { return (p != nullptr); }

  void send_event(EventSeverity severity, EventCategory category, string_view message = { }) noexcept { 
    p->send_event(p, severity, category, message); 
  }
  void send_event(EventCategory category, string_view message = { }) noexcept { 
    send_event(EventSeverity::Info, category, message); 
  }
  void log_message(EventSeverity severity, string_view message) noexcept {
    send_event(severity, EventCategory::Message, message);
  }
  void log_verbose(string_view message) noexcept { log_message(EventSeverity::Verbose, message); }
  void log_info(string_view message) noexcept { log_message(EventSeverity::Info, message); }
  void log_warning(string_view message) noexcept  { log_message(EventSeverity::Warning, message); }
  void log_error(string_view message) noexcept  { log_message(EventSeverity::Error, message); }
  void monitor_value(const char* name, double value, bool average = true) noexcept {
    p->monitor_value(p, name, value, average);
  }
  string resolve_storage_filename(string_view storage_filename) noexcept { 
    return p->resolve_storage_filename(p, storage_filename); 
  }
  string get_userdata_path(string_view path) noexcept {
    return p->get_userdata_path(p, path); 
  }
  void async(OnComplete&& callback) noexcept {
    p->async(p, AsyncPolicy::Default, 0, std::move(callback)); 
  }
  void async(AsyncPolicy policy, OnComplete&& callback) noexcept {
    p->async(p, policy, 0, std::move(callback)); 
  }
  void set_timeout(std::chrono::duration<double> delay, OnComplete&& callback) noexcept {
    p->async(p, AsyncPolicy::Default, delay.count(), std::move(callback)); 
  }
  TextureRef create_texture(const TextureDesc& desc) noexcept {
    return TextureRef(p->create_texture(p, &desc));
  }
  void download_texture(TextureRef texture, OnTextureDownloaded callback) noexcept {
    p->download_texture(p, texture.release(), std::move(callback)); 
  }
  void upload_texture(TextureRef texture, const BufferDesc& buffer, bool upload_copy, OnComplete callback) noexcept {
    p->upload_texture(p, texture.release(), &buffer, upload_copy, std::move(callback));
  }
  void unpack_video_frame(const VideoFrame& frame, OnComplete on_data_read, OnVideoFrameUnpacked on_unpacked) noexcept {
    p->unpack_video_frame(p, &frame, std::move(on_data_read), 
      [on_unpacked = std::move(on_unpacked)](TextureP** textures, size_t texture_count) mutable noexcept {
        on_unpacked({ textures, textures + texture_count });
      });
  }
  void unpack_video_frame(const VideoFrame& frame, OnVideoFrameUnpacked on_unpacked) noexcept {
    unpack_video_frame(frame, []() noexcept { }, std::move(on_unpacked));
  }
  void send_audio_frame(const AudioFrame& frame, OnComplete callback) noexcept {
    p->send_audio_frame(p, &frame, std::move(callback));
  }

private:
  HostContextP* p{ };
};

class InputStream : public InputStreamP {
public:
  static InputStream* cast(InputStreamP* self) { return static_cast<InputStream*>(self); }

  InputStream()
    : InputStreamP{
      [](InputStreamP* p) noexcept { delete cast(p); },
      [](InputStreamP* p, HostContextP* host) noexcept { 
        cast(p)->m_host_context = HostContext(host);
        return cast(p)->initialize();
      },
      [](InputStreamP* p, ValueSet settings) noexcept { return cast(p)->update_settings(std::move(settings)); },
      [](InputStreamP* p, string_view name) noexcept { return cast(p)->get_property(name); },
      [](InputStreamP* p, string_view name, string value) noexcept { return cast(p)->set_property(name, std::move(value)); },
      [](InputStreamP* p) noexcept { return cast(p)->get_state(); },
      [](InputStreamP* p) noexcept { return cast(p)->get_parameter_count(); },
      [](InputStreamP* p, size_t index) noexcept -> ParameterP* { return cast(p)->get_parameter(index); },
      [](InputStreamP* p, bool requested) noexcept { cast(p)->set_video_requested(requested); },
      [](InputStreamP* p, bool requested) noexcept { cast(p)->set_audio_requested(requested); },
      [](InputStreamP* p) noexcept { return cast(p)->update(); },
      [](InputStreamP* p) noexcept { return cast(p)->before_render(); },
      [](InputStreamP* p) noexcept { return cast(p)->render(); },
      [](InputStreamP* p) noexcept { return cast(p)->after_render(); },
    } { }
  virtual ~InputStream() = default;
  virtual string get_property(string_view name) noexcept { return { }; }
  virtual bool set_property(string_view name, string value) noexcept { return false; }
  virtual bool initialize() noexcept { return true; }
  virtual bool update_settings(ValueSet settings) noexcept { return false; }
  virtual ValueSet get_state() noexcept { return { }; }
  virtual void set_video_requested(bool requested) noexcept { }
  virtual void set_audio_callback(SendAudioFrame&& send_audio_frame) noexcept { }
  virtual bool update() noexcept { return true; }
  virtual SyncDesc before_render() noexcept { return { }; }
  virtual RenderResult render() noexcept { return RenderResult::Succeeded; }
  virtual SyncDesc after_render() noexcept { return { }; }
  virtual size_t get_parameter_count() noexcept { return m_parameters.size(); }
  virtual Parameter* get_parameter(size_t index) noexcept { return m_parameters[index].get(); }

  Parameter* find_parameter(string_view name) noexcept {
    const auto count = get_parameter_count();
    for (auto i = size_t{ }; i < count; ++i)
      if (auto parameter = get_parameter(i))
        if (parameter->name() == name)
          return parameter;
    return nullptr;
  }

  Parameter* find_parameter(string_view property, string_view value) noexcept {
    const auto count = get_parameter_count();
    for (auto i = size_t{ }; i < count; ++i)
      if (auto parameter = get_parameter(i))
        if (parameter->get_property(property) == value)
          return parameter;
    return nullptr;
  }

protected:
  HostContext& host() noexcept { return m_host_context; }

  template<typename T, typename... Args>
  T* add_parameter(Args&&... args) {
    return static_cast<T*>(m_parameters.emplace_back(
      std::make_unique<T>(std::forward<Args>(args)...)).get());
  }

  template<typename T, typename... Args>
  T* add_output_parameter(Args&&... args) {
    auto parameter = add_parameter<T>(std::forward<Args>(args)...);
    parameter->set_property("direction", "out");
    return parameter;
  }

private:
  HostContext m_host_context;
  std::vector<std::unique_ptr<Parameter>> m_parameters;

  void set_audio_requested(bool requested) noexcept {
    set_audio_callback(!requested ? SendAudioFrame() :
      [this](const AudioFrame& audio_frame, OnComplete on_complete) noexcept {
        host().send_audio_frame(audio_frame, 
          [this, on_complete = std::move(on_complete)]() mutable noexcept {
            on_complete();
          });
      });
  }
};

class OutputStream : public OutputStreamP {
public:
  static OutputStream* cast(OutputStreamP* self) { return static_cast<OutputStream*>(self); }

  OutputStream()
    : OutputStreamP{
      [](OutputStreamP* p) noexcept { delete cast(p); },
      [](OutputStreamP* p, HostContextP* host) noexcept {
        cast(p)->m_host_context = HostContext(host);
        return cast(p)->initialize();
      },
      [](OutputStreamP* p, ValueSet settings) noexcept { return cast(p)->update_settings(std::move(settings)); },
      [](OutputStreamP* p, string_view name) noexcept { return cast(p)->get_property(name); },
      [](OutputStreamP* p, string_view name, string value) noexcept { return cast(p)->set_property(name, std::move(value)); },
      [](OutputStreamP* p) noexcept { return cast(p)->get_state(); },
      [](OutputStreamP* p, const AudioFrame* audio_frame, OnComplete on_complete) noexcept { 
        cast(p)->send_audio_frame(*audio_frame, std::move(on_complete));
      },
      [](OutputStreamP* p) noexcept { return cast(p)->get_target().release(); },
      [](OutputStreamP* p) noexcept { return cast(p)->before_render(); },
      [](OutputStreamP* p) noexcept { return cast(p)->after_render(); },
      [](OutputStreamP* p) noexcept { cast(p)->present(); },
      [](OutputStreamP* p) noexcept { cast(p)->swap(); },
    } { }
  virtual ~OutputStream() = default;
  virtual string get_property(string_view name) noexcept { return { }; }
  virtual bool set_property(string_view name, string value) noexcept { return false; }
  virtual bool initialize() noexcept { return true; }
  virtual bool update_settings(ValueSet settings) noexcept { return false; }
  virtual ValueSet get_state() noexcept { return { }; }
  virtual void send_audio_frame(const AudioFrame& audio_frame, OnComplete on_complete) noexcept { on_complete(); }
  virtual TextureRef get_target() noexcept { return { }; }
  virtual SyncDesc before_render() noexcept { return { }; }
  virtual SyncDesc after_render() noexcept { return { }; }
  virtual void present() noexcept { }
  virtual void swap() noexcept { }

protected:
  HostContext& host() noexcept { return m_host_context; }

private:
  HostContext m_host_context;
};

class StreamDevice : public StreamDeviceP {
public:
  static StreamDevice* cast(StreamDeviceP* self) { return static_cast<StreamDevice*>(self); }

  StreamDevice()
    : StreamDeviceP{
      [](StreamDeviceP* p) noexcept { delete cast(p); },
      [](StreamDeviceP* p, HostContextP* host) noexcept { 
        cast(p)->m_host_context = HostContext(host);
        return cast(p)->initialize(); 
      },
      [](StreamDeviceP* p, ValueSet settings) noexcept { return cast(p)->update_settings(std::move(settings)); },
      [](StreamDeviceP* p, string_view name) noexcept { return cast(p)->get_property(name); },
      [](StreamDeviceP* p, string_view name, string value) noexcept { return cast(p)->set_property(name, std::move(value)); },
      [](StreamDeviceP* p) noexcept { return cast(p)->enumerate_stream_settings(); },
      [](StreamDeviceP* p, ValueSet settings) noexcept -> InputStreamP* { return cast(p)->create_input_stream(std::move(settings)); },
      [](StreamDeviceP* p, ValueSet settings) noexcept -> OutputStreamP* { return cast(p)->create_output_stream(std::move(settings)); },
      [](StreamDeviceP* p, 
          InputStreamP* const* input_streams, size_t input_stream_count, 
          OutputStreamP* const* output_streams, size_t output_stream_count) noexcept { 
        auto is = vector<InputStream*>();
        for (auto i = size_t{ }; i < input_stream_count; ++i)
          is.push_back(InputStream::cast(input_streams[i]));
        auto os = vector<OutputStream*>();
        for (auto i = size_t{ }; i < output_stream_count; ++i)
          os.push_back(OutputStream::cast(output_streams[i]));
        return cast(p)->set_active_streams(std::move(is), std::move(os));
      },
      [](StreamDeviceP* p) noexcept { return cast(p)->update(); },
      [](StreamDeviceP* p) noexcept { return cast(p)->before_render(); },
      [](StreamDeviceP* p) noexcept { return cast(p)->render(); },
      [](StreamDeviceP* p) noexcept { return cast(p)->after_render(); },
    } { }
  virtual ~StreamDevice() = default;
  virtual string get_property(string_view name) noexcept { 
    // device name must always be readable, otherwise it is considered lost and recreated
    if (name == PropertyNames::name)
      return string("Device");
    return { };
  }
  virtual bool set_property(string_view name, string value) noexcept { return false; }
  virtual bool initialize() noexcept { return true; }
  virtual bool update_settings(ValueSet settings) noexcept { return false; }
  virtual vector<ValueSet> enumerate_stream_settings() noexcept { return { }; }
  virtual InputStream* create_input_stream(ValueSet settings) noexcept { return nullptr; }
  virtual OutputStream* create_output_stream(ValueSet settings) noexcept { return nullptr; }
  virtual bool set_active_streams(const vector<InputStream*>& input_streams, 
    const vector<OutputStream*>& output_streams) noexcept { return true; };  
  virtual bool update() noexcept { return true; }
  virtual SyncDesc before_render() noexcept { return { }; }
  virtual void render() noexcept { }
  virtual SyncDesc after_render() noexcept { return { }; }

protected:
  HostContext& host() noexcept { return m_host_context; }

  template<typename T, typename... Args>
  T* add_parameter(Args&&... args) {
    return static_cast<T*>(m_parameters.emplace_back(
      std::make_unique<T>(std::forward<Args>(args)...)).get());
  }

private:
  HostContext m_host_context;
  std::vector<std::unique_ptr<Parameter>> m_parameters;
};

class Extension : public ExtensionP {
public:
  static Extension* cast(ExtensionP* self) { return static_cast<Extension*>(self); }

  Extension()
    : ExtensionP{
      [](ExtensionP* p, HostContextP* host) noexcept {
        cast(p)->m_host_context = HostContext(host);
        return cast(p)->initialize(); 
      },
      [](ExtensionP* p) noexcept { cast(p)->shutdown(); },
      [](ExtensionP* p, string_view name) noexcept { 
        if (name == PropertyNames::api_version)
          return string(api_version);
        if (name == PropertyNames::build_date)
          return string(__DATE__);
        return cast(p)->get_property(name); 
      },
      [](ExtensionP* p, string_view name, string value) noexcept { return cast(p)->set_property(name, std::move(value)); },
      [](ExtensionP* p) noexcept { return cast(p)->enumerate_stream_device_settings(); },
      [](ExtensionP* p, ValueSet settings) noexcept -> StreamDeviceP* { return cast(p)->create_stream_device(std::move(settings)); },
    } { }
  virtual ~Extension() = default;
  virtual bool initialize() noexcept { return true; }
  virtual void shutdown() noexcept { }
  virtual string get_property(string_view name) noexcept { return { }; }
  virtual bool set_property(string_view name, string value) noexcept { return false; }
  virtual vector<ValueSet> enumerate_stream_device_settings() noexcept { return { }; }
  virtual StreamDevice* create_stream_device(ValueSet settings) noexcept { return nullptr; }

protected:
  HostContext& host() noexcept { return m_host_context; }

private:
  HostContext m_host_context;
};

//-------------------------------------------------------------------------

class ParameterBase : public rxext::Parameter {
public:
  using Parameter::set_property;

  ParameterType type() const noexcept override { return m_type; }
  string_view name() const noexcept override { return m_name; }

  string get_property(string_view name) const noexcept override {
    const auto lock = std::lock_guard(mutex());
    const auto it = m_properties.find(name);
    return string(it != m_properties.end() ? it->second : string());
  }

  bool set_property(string_view name, string value) noexcept override {
    const auto lock = std::lock_guard(mutex());
    m_properties[string(name)] = std::move(value);
    return true;
  }

  template<typename T>
  T get_property(string_view name) const noexcept {
    return string_to_value<T>(get_property(name));
  }

protected:
  ParameterBase(ParameterType type, std::string name) 
    : m_type(type), m_name(std::move(name)) {
  }

  std::mutex& mutex() const {
    return m_mutex;
  }

private:
  const ParameterType m_type;
  const std::string m_name;
  mutable std::mutex m_mutex;
  std::map<string, string, std::less<>> m_properties;
};

template<typename T, ParameterType Type>
class ParameterT : public ParameterBase {
public:
  ParameterT(std::string name, const T& default_value = T{ }) 
    : ParameterBase(Type, std::move(name)),
      m_value(default_value) {
  }
  void set_value(const void* data, size_t size) noexcept override {
    const auto lock = std::lock_guard(mutex());
    m_value = *static_cast<const T*>(data);
  }
  void get_value(void* data, size_t* size) const noexcept override {
    const auto lock = std::lock_guard(mutex());
    if (size) {
      if (*size >= sizeof(T))
        std::memcpy(data, &m_value, sizeof(T));
      *size = sizeof(T);
    }
  }
  void set_value(const T& value) {
    const auto lock = std::lock_guard(mutex());
    m_value = value; 
  }
  T value() const {
    const auto lock = std::lock_guard(mutex());
    return m_value; 
  }

private:
  T m_value{ };
};

using ParameterBool = ParameterT<bool, ParameterType::Bool>;
using ParameterInt = ParameterT<int, ParameterType::Int>;
using ParameterValue = ParameterT<double, ParameterType::Value>;
using ParameterVector2 = ParameterT<std::array<double, 2>, ParameterType::Vector2>;
using ParameterVector3 = ParameterT<std::array<double, 3>, ParameterType::Vector3>;
using ParameterVector4 = ParameterT<std::array<double, 4>, ParameterType::Vector4>;
using ParameterMatrix3 = ParameterT<std::array<double, 9>, ParameterType::Matrix3>;
using ParameterMatrix4 = ParameterT<std::array<double, 16>, ParameterType::Matrix4>;

class ParameterString : public ParameterBase {
public:
  ParameterString(std::string name, std::string default_value = "") 
    : ParameterBase(ParameterType::String, std::move(name)),
      m_value(std::move(default_value)) {
  }
  void set_value(const void* data, size_t size) noexcept override {
    const auto lock = std::lock_guard(mutex());
    const auto begin = static_cast<const char*>(data);
    const auto end = begin + size;
    m_value = std::string(begin, end);
  }
  void get_value(void* data, size_t* size) const noexcept override {
    const auto lock = std::lock_guard(mutex());
    if (size) {
      if (*size >= m_value.size())
        std::memcpy(data, m_value.data(), m_value.size());
      *size = m_value.size();
    }
  }
  void set_value(std::string string) {
    const auto lock = std::lock_guard(mutex());
    m_value = std::move(string);
  }
  std::string value() const {
    const auto lock = std::lock_guard(mutex());
    return m_value; 
  }

private:
  std::string m_value;
};

class ParameterData : public ParameterBase {
public:
  explicit ParameterData(std::string name)
    : ParameterBase(ParameterType::Data, std::move(name)) {
  }
  void set_value(const void* data, size_t size) noexcept override {
    const auto lock = std::lock_guard(mutex());
    m_value.resize(size);
    std::memcpy(m_value.data(), data, size);
  }
  void get_value(void* data, size_t* size) const noexcept override {
    const auto lock = std::lock_guard(mutex());
    if (size) {
      if (*size >= m_value.size())
        std::memcpy(data, m_value.data(), m_value.size());
      *size = m_value.size();
    }
  }
  template<typename T = std::byte>
  std::vector<T> value() const {
    const auto lock = std::lock_guard(mutex());
    const auto begin = reinterpret_cast<const T*>(m_value.data());
    const auto end = begin + (m_value.size() / sizeof(T));
    return std::vector<T>(begin, end); 
  }

private:
  std::vector<std::byte> m_value;
};

class ParameterTexture : public ParameterBase {
public:
  explicit ParameterTexture(std::string name) 
    : ParameterBase(ParameterType::Texture, std::move(name)) {
  }
  void set_value(const void* data, size_t size) noexcept override {
    // not settable by host
  }
  void get_value(void* data, size_t* size) const noexcept override {
    const auto lock = std::lock_guard(mutex());
    if (size) {
      if (*size >= sizeof(TextureP*)) {
        // acquire reference for caller
        *static_cast<TextureP**>(data) = TextureRef(m_texture).release();
      }
      *size = sizeof(TextureP*);
    }
  }
  void set_texture(TextureRef texture) {
    const auto lock = std::lock_guard(mutex());
    m_texture = std::move(texture);
  }
  const TextureRef& texture() const {
    // no lock needed since it is not settable by host
    return m_texture;
  }

private:
  TextureRef m_texture;
};

class ParameterTextureSet : public ParameterBase {
public:
  explicit ParameterTextureSet(std::string name) 
    : ParameterBase(ParameterType::Texture, std::move(name)) {
  }
  void set_value(const void* data, size_t size) noexcept override {
    // not settable by host
  }
  void get_value(void* data, size_t* size) const noexcept override {
    const auto lock = std::lock_guard(mutex());
    if (size) {
      const auto handles_size = sizeof(TextureP*) * m_textures.size();
      if (*size >= handles_size) {
        auto pointers = static_cast<TextureP**>(data);
        for (const auto& texture : m_textures) {
          // acquire reference for caller
          *pointers++ = TextureRef(texture).release();
        }
      }
      *size = handles_size;
    }
  }

  void set_textures(vector<TextureRef> textures) {
    const auto lock = std::lock_guard(mutex());
    m_textures = std::move(textures);
  }
  const vector<TextureRef>& textures() const { 
    // no lock needed since it is not settable by host
    return m_textures;
  }

private:
  vector<TextureRef> m_textures;
};

template<typename CreateTexture>
void update_input_texture(const ParameterTexture& parameter, 
    CreateTexture&& create_texture) {
  auto desc = TextureDesc{ };
  desc.width = parameter.get_property<int>(SettingNames::resolution_x);
  desc.height = parameter.get_property<int>(SettingNames::resolution_y);
  desc.format = get_format_by_name(parameter.get_property(SettingNames::format), Format::None);

  const auto current = parameter.texture().desc();
  if (desc.width && desc.height && desc.format != Format::None &&
      std::tie(desc.width, desc.height, desc.format) != 
      std::tie(current.width, current.height, current.format))
    create_texture(desc);
}

//-------------------------------------------------------------------------

class MemoryInputStream : public InputStream {
protected:
  MemoryInputStream() 
    : m_sampler(*add_output_parameter<ParameterTextureSet>("sampler")) {
  }

  void set_video_requested(bool requested) noexcept override {
    set_video_callback(!requested ? SendVideoFrame() :
      [this](const VideoFrame& video_frame, OnComplete on_complete) noexcept {
        host().unpack_video_frame(video_frame, 
          [this, on_complete = std::move(on_complete)](vector<TextureRef> textures) mutable noexcept {
            on_frame_unpacked(std::move(textures));
            on_complete();
          });
      });
  }

  bool update() noexcept override {
    const auto lock = std::lock_guard(m_mutex);
    if (!m_frame_textures.empty()) {
      m_sampler.set_textures(m_frame_textures.front());
      m_frame_textures.erase(m_frame_textures.begin());
    }
    return true;
  }

  virtual void set_video_callback(SendVideoFrame&& send_video_frame) noexcept = 0;

private:
  using FrameTextures = vector<TextureRef>;

  void on_frame_unpacked(vector<TextureRef> textures) noexcept {
    if (textures.empty())
      return;
    const auto lock = std::lock_guard(m_mutex);
    if (m_frame_textures.size() < 4)
      m_frame_textures.push_back(textures);
  }

  ParameterTextureSet& m_sampler;
  std::mutex m_mutex;
  std::vector<FrameTextures> m_frame_textures;
};

//-------------------------------------------------------------------------

class MemoryOutputStream : public OutputStream {
protected:
  explicit MemoryOutputStream(TextureDesc target_desc) 
    : m_target_desc(target_desc) {
  }

  TextureRef get_target() noexcept override {
    auto lock = std::lock_guard(m_mutex);
    if (!m_video_requested)
      return { };

    if (m_targets.empty()) {
      if (m_targets_allocated >= 4)
        return { };
      m_targets.emplace_back(host().create_texture(m_target_desc));
      ++m_targets_allocated;
    }
    return m_targets.front();
  }
  
  void present() noexcept override {
    auto lock = std::unique_lock(m_mutex);
    auto target = std::move(m_targets.front());
    m_targets.erase(m_targets.begin());
    lock.unlock();

    host().download_texture(target,
      [this, target](BufferDesc data) mutable noexcept {
        auto lock = std::lock_guard(m_mutex);
        send_texture_data(data);
        m_targets.push_back(std::move(target));
      });
  }

  virtual bool send_texture_data(const BufferDesc& plane) noexcept = 0;

  const TextureDesc& target_desc() const { return m_target_desc; }

  void set_video_requested(bool video_requested) { 
    auto lock = std::lock_guard(m_mutex);
    m_video_requested = video_requested; 
  }

private:
  std::mutex m_mutex;
  const TextureDesc m_target_desc;
  std::vector<TextureRef> m_targets;
  size_t m_targets_allocated{ };
  bool m_video_requested{ true };
};

} // namespace
