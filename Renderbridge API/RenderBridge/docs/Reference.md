# RenderBridge - RX Extension Interface

## Overview

The RX Extension Interface allows to write C++ extensions, which provide additional functionality to the RX engine.
It allows extensions to provide:

- [StreamDevices](#Stream-Device-Extension) for streaming video and audio data to and from RX.

## Files

### _rxext.h_

Is the C++ header containing all type definitions of the extension interface.
All types have a defined ABI, so the extension can be compiled with any C++ compiler supporting C++17 and used by a host regardless of the compiler the host was built with.

It includes headers provided by the [PTL](https://github.com/MFHava/PTL) project, which allow convenient interchange of string, vector and function object parameters.

The header also declares the two C functions each extension binary must export. `rxext_open` is called when the extension is loaded and `rxext_close` when it is unloaded.

### _rxext_client.h_

Contains abstract type definitions the extension should derive from. The function pointers defined in _rxext.h_ are automatically bound to methods of the abstract types.

## Stream Device Extension

The [extension](#Extension) provides a list of [devices](#StreamDevice) for which the host creates streams. Depending on the direction, the host or the extension calls a function once per frame to stream the data to the other.

### Creation of streams

- Each [Extension](#Extension) needs to implement `create_stream_device`, which creates a [StreamDevice](#StreamDevice) with some specific settings.

- The settings for the devices can be either provided by the host or by the extension. The extension can report the available devices by implementing `enumerate_stream_device_settings`. When it does so, it should send a `DevicesChanged` [host event](#HostContext_send_event) when the settings change.

- Each [StreamDevice](#StreamDevice) should implement `get_property`, to provide the device name and information about the channels.
  Depending on the type of device, it needs to implement `create_input_stream` and/or `create_output_stream`, which create a stream with some specific settings.

- The settings for the streams can be either provided by the host or by the device. The device can report the available streams by implementing `enumerate_stream_settings`. When it does so, it should send a `StreamsChanged` [host event](#HostContext_send_event) when the settings change.

- The host creates the streams and subsequently calls `set_active_streams`, passing pointers to all active streams (in case the streams are linked and need to be activated together).

- Each device and input stream can register a number of input and output [Parameters](#Parameter) during its initialization.

- All streams need to implement [get_state](#InputStream_get_state), which the host calls to obtain information about the stream's resolution, color format, audio channel count and sample rate.

### Video input

#### Video input from render target

See [InputStream](#InputStream). [WIP]

#### Video input from system memory

See [MemoryInputStream](#MemoryInputStream). [WIP]

### Video output

#### Video output to render target

See [OutputStream](#OutputStream). [WIP]

#### Video output to system memory

See [MemoryOutputStream](#MemoryOutputStream). [WIP]

### Audio input

The host passes a callback to the [InputStream](#InputStream) by calling `set_audio_callback`. The extension should invoke it for each available audio frame. The callback is set and reset depending on whether audio is currently requested.

### Audio output

The host streams out audio frames by calling the [OutputStream's](#OutputStream) `send_audio_frame`.

## Classes defined in _rxext_client.h_

All types are in the `rxext` namespace and since no exception must leave the extension code, all methods are marked as _noexcept_.

### Extension

An object implementing the `Extension` interface should be created and returned from the `rxext_open` function. It allows the host to communicate with the extension.

    class Extension {
      HostContext& host();
      virtual bool initialize();
      virtual void shutdown();
      virtual string get_property(string_view name);
      virtual bool set_property(string_view name, string value);
      virtual vector<ValueSet> enumerate_stream_device_settings();
      virtual StreamDevice* create_stream_device(ValueSet settings);
    };

- `initialize` is called when the host context is available. It should return whether the initialization succeeded.

- `shutdown` is only called when it was successfully initialized.

- `get_property` can provide information about the extension. Common properties are:

  - _name_: The name of the extension.
  - _version_: The version of the extension.

- `set_property` allows to set properties of the extension.

- `enumerate_stream_device_settings` can provide settings of the currently available device.

- `create_stream_device` creates a new stream device using the provided settings. 

### HostContext

A `HostContext` object which provides functions for communicating with the extension host.

    class HostContext {
      void send_event(EventSeverity severity, EventCategory category, 
          string_view message = { });
      void send_event(EventCategory category, string_view message = { });
      void log_message(EventSeverity severity, string_view message);
      void log_verbose(string_view message);
      void log_info(string_view message);
      void log_warning(string_view message);
      void log_error(string_view message);
      string resolve_storage_filename(string_view storage_filename);
      string get_userdata_path(string_view path);
      void async(OnComplete&& callback);
      void async(AsyncPolicy policy, OnComplete&& callback);
      void set_timeout(std::chrono::duration<double> delay, OnComplete&& callback);
      TextureRef create_texture(const TextureDesc& desc);
      void download_texture(TextureRef texture, OnTextureDownloaded callback);
      void upload_texture(TextureRef texture, const BufferDesc& buffer, 
          bool upload_copy, OnTextureUploaded callback);
      void unpack_video_frame(const VideoFrame& frame, OnComplete on_data_read,
          OnVideoFrameUnpacked callback);
      void send_audio_frame(const AudioFrame& frame, OnComplete callback);
    };

- `log_{message,verbose,info,warning,error}` allows the extension to log messages.

- `send_event` <a name="HostContext_send_event"></a>

  - Message
  - StreamsChanged

- `resolve_storage_filename`

- `get_userdata_path`

- `async` and `set_timeout`

- `create_texture`

- `download_texture`

- `upload_texture`

- `unpack_video_frame`

- `send_audio_frame`

### StreamDevice

    class StreamDevice {
      HostContext& host();
      Parameter* find_parameter(string_view name) const;
      T* add_parameter(Args&&... args);

      virtual bool initialize();
      virtual string get_property(string_view name);
      virtual bool set_property(string_view name, string value);
      virtual vector<ValueSet> enumerate_stream_settings();
      virtual InputStream* create_input_stream(ValueSet settings);
      virtual OutputStream* create_output_stream(ValueSet settings);
      virtual bool set_active_streams(
          const vector<InputStream*>& input_streams,
          const vector<OutputStream*>& output_streams);
      virtual bool update();
      virtual SyncDesc before_render();
      virtual void render();
      virtual SyncDesc after_render();
    };

- `host` returns the device's host context.

- `initialize` is called when the host context is available. It should initialize the device, complete registration of all parameters and return whether the initialization succeeded.

- `get_property` should return properties of the device. Common properties are:

  - _name: string_ - the name of the device.
  - _channel_names: vector&lt;string&gt;_ - the channel names.
  - _channel_types: vector&lt;string&gt;_ - the channel types ('input', 'output', 'clock').

- `set_property` allows to set properties of the device.

- `enumerate_stream_settings` can provide settings of the currently available streams. Common settings are:

  - _name: string_ - the name of the stream.
  - _handle: string_ - the identification of the stream.
  - _settings_desc: string_ - a JSON description of the settings, so the user interface can create controls to customize them. The `SettingsDescBuilder` helps with building the description.

- `create_input_stream` creates a new input stream using the provided settings. See [InputStream::update_settings()](#InputStream_settings) for a list of potential settings.

- `create_output_stream` creates a new input stream using the provided settings. See [OutputStream::update_settings()](#OutputStream_settings) for a list of potential settings.

- `set_active_streams` is called when the set of active streams changes.

- `update` is called once per frame.

- `before_render`

- `render`

- `after_render`

### InputStream

    class InputStream {
      HostContext& host();
      Parameter* find_parameter(string_view name) const;
      T* add_parameter(Args&&... args);
      T* add_output_parameter(Args&&... args);

      virtual bool initialize();
      virtual bool update_settings(ValueSet settings);
      virtual ValueSet get_state();
      virtual size_t get_parameter_count();
      virtual Parameter* get_parameter(size_t index);
      virtual string get_property(string_view name);
      virtual bool set_property(string_view name, string value);  
      virtual void set_video_requested(bool requested);
      virtual void set_audio_callback(SendAudioFrame&& send_audio_frame);
      virtual bool update();
      virtual SyncDesc before_render();
      virtual RenderResult render();
      virtual SyncDesc after_render();
    };

- `host` returns the stream's host context.

- `initialize` is called when the host context is available. It should complete the registration of all parameters and return whether the initialization succeeded.

- `update_settings` <a name="InputStream_settings"></a> allows to update stream settings on the fly. The function should return whether the update could be applied successfully. When it failed the stream is recreated with the new settings. Common settings are:

  - _handle_:
  - _channel_index_:
  - _deinterlace_mode_: (weave, bob, even, odd).
  - _sync_group_:

- `get_state` <a name="InputStream_get_state"></a> can provide information about the stream's state. Common states are:

  - _resolution_x: int_ - the horizontal resolution.
  - _resolution_y: int_ - the vertical resolution.
  - _frame_rate: double_ - the frame rate.
  - _pixel_format: string_ - the format of the video frame corresponding to FFMpeg pixel formats (e.g. RGB24, RGBA, YUV444P).
  - _texture_format: string_ - the texture format.
  - _audio_channel_count: int_ - the audio channel count.
  - _audio_sample_rate: int_ - the audio sample rate.

- `add_parameter` adds a new stream parameter. Parameters need to be added before the initialization is complete.

  - Input parameters bound by the engine:
    - _visible: Bool_
  - Output parameters bound by the engine:
    - _sampler: Texture_

- `add_output_parameter` adds a new parameter where the `direction` property is set to `out`.

- `find_parameter` / `get_parameter` / `get_parameter_count` allow to enumerate the stream's parameters.

- `get_property` should return properties of the stream. Common properties are:

  - _shader_export_: string
  - _shader_file_: string
  - _shader_source_: string
  - _layer_names_: vector&lt;string&gt;
  - _layer_ids_: vector&lt;string&gt;

- `set_property` allows to set properties of the stream.

- `set_video_requested`

- `set_audio_callback`

- `update` is called once per frame.

- `before_render`

- `render`

- `after_render`

### OutputStream

    class OutputStream {
      HostContext& host();

      virtual bool initialize();
      virtual bool update_settings(ValueSet settings);
      virtual ValueSet get_state();
      virtual string get_property(string_view name);
      virtual bool set_property(string_view name, string value);
      virtual bool send_audio_frame(const AudioFrame& audio_frame);
      virtual TextureRef get_target();
      virtual SyncDesc before_render();
      virtual SyncDesc after_render();
      virtual void present();
      virtual void swap();
    };

- `host` returns the stream's host context.

- `initialize` is called when the host context is available. It should return whether the initialization succeeded.

- `update_settings` <a name="OutputStream_settings"></a> allows to update stream settings on the fly. The function should return whether the update could be applied successfully. When it failed the stream is recreated with the new settings. Common settings are:

  - _handle_:
  - _channel_index_:
  - _resolution_x_:
  - _resolution_y_:
  - _frame_rate_:
  - _sync_video_:

- `get_state` can provide information about the stream's state. Common states are:

  - _resolution_x: int_ - the horizontal resolution.
  - _resolution_y: int_ - the vertical resolution.
  - _format: string_ - the format of the render target.
  - _frame_rate: double_ - the target frame rate.
  - _sync_video: int_ - whether the extension is synchronizing to the stream's frame rate.

- `set_property` / `get_property` allow to get or set the properties of the stream.

- `send_audio_frame`

- `get_target`

- `before_render`

- `after_render`

- `present`

- `swap`

### MemoryInputStream

Is derived from [InputStream](#InputStream) and simplifies streaming in video frames, which need to be read from system memory.
Extensions need to implement `set_video_callback`, which is called as soon as video data is requested. The passed callback should be called whenever a new video frame arrives.
It also registers a texture parameter _sampler_, which is updated with the provided video data.

    class MemoryInputStream : public InputStream {
      void set_video_requested(bool requested);

      virtual void set_video_callback(SendVideoFrame&& send_video_frame);
    };

- `set_video_requested`

- `set_video_callback`

### MemoryOutputStream

Is derived from [OutputStream](#OutputStream) and simplifies streaming out video frames, which need to be written to system memory.

    class MemoryOutputStream : public OutputStream {
      MemoryOutputStream(TextureDesc target_desc);
      const TextureDesc& target_desc() const;
      void set_video_requested(bool video_requested);

      virtual bool send_texture_data(const BufferDesc& plane);
    };

- `MemoryOutputStream`

- `target_desc`

- `set_video_requested`

- `send_texture_data`

### Parameter

    virtual ParameterType type() const;
    virtual string_view name() const;
    virtual void get_value(void* data, size_t* size) const;
    virtual void set_value(const void* data, size_t size);
    virtual string get_property(string_view name) const;
    virtual bool set_property(string_view name, string value);
    template<typename T> bool set_property(string_view name, T&& value);

  - `type` returns the type of the parameter.

  - `name` returns the name of the parameter.
  
  - `get_value` returns the current value of the parameter.

  - `set_value` sets the current value of the parameter.

  - `get_property` <a name="Parameter_properties"></a> can provide information about the parameter. Common properties are:

    - _name: string_ - a friendly name to display in the user interface.
    - _purpose: string_ - the purpose of the parameter:
      - TimelineTime
      - Color
      - Position
      - Rotation
      - Scale
      - WorldMatrix
      - ViewMatrix
      - ProjectionMatrix
    - _enum_names: vector&lt;string&gt;_
    - _active_in_layers: vector&lt;int&gt;_
    - _min_value: double_
    - _max_value: doulble_
    - _min_value_ui: double_
    - _max_value_ui: doulble_

  - `set_property` sets a property of the parameter.

## Structures defined in _rxext.h_

### ShareHandle

    struct ShareHandle {
      HandleType type;
      Handle handle;
      uint64_t process_id;
      uint64_t memory_size;
      uint64_t memory_offset;
      bool dedicated;
    };

### SyncDesc

    struct SyncDesc {
      SyncStrategy sync_strategy;
      ShareHandle share_handle;
      uint64_t value;
    };

<br/>
<br/>