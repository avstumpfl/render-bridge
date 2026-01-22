
#include "Input.h"
#include <functional>

namespace rxext::ndi {

namespace {
  std::string_view get_pixel_format(NDIlib_FourCC_video_type_e type) {
    switch (type) {
      case NDIlib_FourCC_video_type_UYVY: return "UYVY422";
      case NDIlib_FourCC_video_type_UYVA: return "UYVY422_ALPHA";
      case NDIlib_FourCC_video_type_YV12: return "YV12";
      case NDIlib_FourCC_video_type_NV12: return "NV12";
      case NDIlib_FourCC_video_type_I420: return "I420";
      case NDIlib_FourCC_video_type_RGBX: return "RGBA";
      case NDIlib_FourCC_video_type_RGBA: return "RGBA";
      case NDIlib_FourCC_video_type_BGRX: return "BGRA";
      case NDIlib_FourCC_video_type_BGRA: return "BGRA";
      case NDIlib_FourCC_video_type_P216: return "P216";
      case NDIlib_FourCC_video_type_PA16: return "PA16";
      case NDIlib_FourCC_video_type_max: break;
    }
    return { };
  }

  bool is_rgb_format(NDIlib_FourCC_video_type_e type) {
    switch (type) {
      case NDIlib_FourCC_video_type_RGBX:
      case NDIlib_FourCC_video_type_RGBA:
      case NDIlib_FourCC_video_type_BGRX:
      case NDIlib_FourCC_video_type_BGRA:
        return true;
      default: break;
    }
    return false;
  }

  bool ignore_alpha(NDIlib_FourCC_video_type_e type) {
    switch (type) {
      case NDIlib_FourCC_video_type_RGBX:
      case NDIlib_FourCC_video_type_BGRX:
        return true;
      default: break;
    }
    return false;
  }
} // namespace

void Input::FreeVideoFrame::operator()(VideoFrame* video_frame) const {
  NDIlib_recv_free_video_v2(video_frame->receive_ptr.get(), 
    &video_frame->ndi_frame);
  delete video_frame;
}

void Input::FreeAudioFrame::operator()(AudioFrame* audio_frame) const {
  NDIlib_recv_free_audio_v2(audio_frame->receive_ptr.get(), 
    &audio_frame->ndi_frame);
  delete audio_frame;
}

Input::Input(const ValueSet& settings) 
    : m_handle(settings.get(SettingNames::handle)) {
}

bool Input::initialize() noexcept {
  auto receive_settings = NDIlib_recv_create_v3_t{ };
  receive_settings.source_to_connect_to.p_ndi_name = m_handle.c_str();
  receive_settings.color_format = NDIlib_recv_color_format_fastest;
  receive_settings.allow_video_fields = false;
	auto ndi_receive = NDIlib_recv_create_v3(&receive_settings);
	if (!ndi_receive) 
    return false;

  m_ndi_receive = ReceivePtr(ndi_receive, &NDIlib_recv_destroy);
  receive_frame_callchain();
  return true;
}

void Input::set_video_callback(SendVideoFrame&& send_video_frame) noexcept {
  auto lock = std::lock_guard(m_mutex);
  m_send_video_frame = std::move(send_video_frame);
}

void Input::set_audio_callback(SendAudioFrame&& send_audio_frame) noexcept {
  auto lock = std::lock_guard(m_mutex);
  m_send_audio_frame = std::move(send_audio_frame);
}

void Input::receive_frame_callchain() noexcept {
  auto lock = std::lock_guard(m_mutex);
  const auto receive_video = static_cast<bool>(m_send_video_frame);
  const auto receive_audio = static_cast<bool>(m_send_audio_frame);
  auto ndi_video_frame = NDIlib_video_frame_v2_t{ };
  auto ndi_audio_frame = NDIlib_audio_frame_v2_t{ };
  const auto result = NDIlib_recv_capture_v2(m_ndi_receive.get(),
    (receive_video || !m_resolution_x ? &ndi_video_frame : nullptr), 
    (receive_audio || !m_audio_channel_count ? &ndi_audio_frame : nullptr), 
    nullptr, 0);

  if (result == NDIlib_frame_type_error) {
    host().log_error("NDI stream failed");
    return;
  }

  if (result == NDIlib_frame_type_video) {
    update_stream_mode(&ndi_video_frame, nullptr);
    if (receive_video)
      write_video_frame(VideoFramePtr(new VideoFrame{ m_ndi_receive, ndi_video_frame }));
  }
  else if (result == NDIlib_frame_type_audio) {
    update_stream_mode(nullptr, &ndi_audio_frame);
    if (receive_audio)
      write_audio_frame(AudioFramePtr(new AudioFrame{ m_ndi_receive, ndi_audio_frame }));
  }

  const auto delay = std::chrono::milliseconds(
    result != NDIlib_frame_type_none ? 0 :
    receive_video || receive_audio ? 1 : 250);
  host().set_timeout(delay, std::bind(&Input::receive_frame_callchain, this));
}

void Input::write_video_frame(VideoFramePtr video_frame) noexcept {
  const auto& ndi_frame = video_frame->ndi_frame;
  auto frame = rxext::VideoFrame{ };
  frame.resolution_x = m_resolution_x;
  frame.resolution_y = m_resolution_y;
  frame.pixel_format = m_pixel_format;

  auto& plane0 = frame.planes.emplace_back();
  plane0.data = ndi_frame.p_data;
  plane0.pitch = ndi_frame.line_stride_in_bytes;
  plane0.size = plane0.pitch * ndi_frame.yres;
  
  // alpha plane directly follows packed YUV
  if (ndi_frame.FourCC == NDIlib_FourCC_video_type_UYVA) {
    auto& plane1 = frame.planes.emplace_back();
    plane1.data = ndi_frame.p_data + plane0.size;
    plane1.pitch = ndi_frame.xres;
    plane1.size = plane1.pitch * ndi_frame.yres;
  }
  m_send_video_frame(frame, [video_frame = std::move(video_frame)]() noexcept { });
}

void Input::write_audio_frame(AudioFramePtr audio_frame) noexcept {
  const auto& ndi_frame = audio_frame->ndi_frame;
  auto frame = rxext::AudioFrame{ };
  frame.sample_rate = ndi_frame.sample_rate;
  auto offset = size_t{ };
  for (auto i = 0; i < ndi_frame.no_channels; ++i) {
    auto& channel = frame.channels.emplace_back();
    channel.data = &ndi_frame.p_data[offset];
    channel.size = ndi_frame.channel_stride_in_bytes;
    channel.pitch = sizeof(float);
    offset += channel.size / channel.pitch;
  }
  m_send_audio_frame(frame, [audio_frame = std::move(audio_frame)]() noexcept { });
}

void Input::update_stream_mode(const NDIlib_video_frame_v2_t* video, 
    const NDIlib_audio_frame_v2_t* audio) noexcept {
  auto changed = false;
  const auto update = [&](auto& property, const auto& value) {
    changed |= (std::exchange(property, value) != value); 
  };
  if (video) {
    update(m_resolution_x, video->xres);
    update(m_resolution_y, video->yres);
    update(m_frame_rate, 1.0 * video->frame_rate_N / video->frame_rate_D);
    update(m_video_type, video->FourCC);
    m_pixel_format = get_pixel_format(m_video_type);
  }
  if (audio) {
    update(m_audio_channel_count, audio->no_channels);
    update(m_audio_sample_rate, audio->sample_rate);
  }
  if (changed)
    host().send_event(EventCategory::StreamsChanged);
}

ValueSet Input::get_state() noexcept {
  auto lock = std::lock_guard(m_mutex);
  auto state = ValueSet();
  state.set(StateNames::resolution_x, m_resolution_x);
  state.set(StateNames::resolution_y, m_resolution_y);
  state.set(StateNames::frame_rate, m_frame_rate);
  state.set(StateNames::pixel_format, m_pixel_format);
  state.set(StateNames::color_space, is_rgb_format(m_video_type) ? "RGB" : "BT709");
  state.set(StateNames::mpeg_range, !is_rgb_format(m_video_type));
  state.set(StateNames::ignore_alpha, ignore_alpha(m_video_type));
  state.set(StateNames::scale_y, -1);
  state.set(StateNames::audio_channel_count, m_audio_channel_count);
  state.set(StateNames::audio_sample_rate, m_audio_sample_rate);
  return state;
}

} // namespace
