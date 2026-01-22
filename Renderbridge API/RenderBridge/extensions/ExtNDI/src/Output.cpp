
#include "Output.h"
#include <functional>

namespace rxext::ndi {

Output::Output(const ValueSet& settings)
    : rxext::MemoryOutputStream({
        settings.get<size_t>(SettingNames::resolution_x, 1920),
        settings.get<size_t>(SettingNames::resolution_y, 1080),
        Format::B8G8R8A8_UNORM
      }),
      m_handle(settings.get(SettingNames::handle)),
      m_frame_rate(settings.get<double>(SettingNames::frame_rate, 60)),
      m_sync_video(settings.get<int>(SettingNames::sync_group, -1) >= 0) {
}

bool Output::initialize() noexcept {
  auto send_settings = NDIlib_send_create_t{ };
  send_settings.p_ndi_name = m_handle.c_str();
  send_settings.clock_video = m_sync_video;
  send_settings.clock_audio = true;

  m_ndi_send.reset(NDIlib_send_create(&send_settings));
  if (!m_ndi_send)
    return false;

  detect_video_request_callchain();
  return true;
}

void Output::detect_video_request_callchain() noexcept {
  set_video_requested(NDIlib_send_get_no_connections(m_ndi_send.get(), 0) > 0);

  host().set_timeout(std::chrono::seconds(1), 
    std::bind(&Output::detect_video_request_callchain, this));
} 

bool Output::send_texture_data(const BufferDesc& plane) noexcept {
  const auto lock = std::lock_guard(m_mutex);
  const auto& desc = target_desc();
  if (plane.size != static_cast<size_t>(desc.width * desc.height * 4u))
    return false;

  // allocate/reuse frame in queue
  auto& queue = m_send_video_queue;
  auto it = std::find_if(queue.begin(), queue.end(), 
    [&](const auto& frame) { return !frame.ndi_frame.p_data; });
  if (it == queue.end())
    it = queue.emplace(queue.end());

  it->buffer.resize(plane.size);
  std::memcpy(it->buffer.data(), plane.data, plane.size);

  auto& ndi_frame = it->ndi_frame;
  ndi_frame.xres = static_cast<int>(desc.width);
  ndi_frame.yres = static_cast<int>(desc.height);
  ndi_frame.FourCC = NDIlib_FourCC_type_BGRA;
  ndi_frame.frame_format_type = NDIlib_frame_format_type_progressive;
  ndi_frame.p_data = it->buffer.data();
  ndi_frame.line_stride_in_bytes = static_cast<int>(plane.pitch);
  ndi_frame.frame_rate_N = static_cast<int>(m_frame_rate * 1000);
  ndi_frame.frame_rate_D = 1000;
  ndi_frame.timecode = NDIlib_send_timecode_synthesize;
  return true;
}

void Output::swap() noexcept {
  const auto lock = std::lock_guard(m_mutex);
  auto& queue = m_send_video_queue;
  if (queue.empty())
    return;

  if (queue.size() < 2 || queue[1].ndi_frame.p_data) {
    // queue[0] is the frame which is currently being sent
    if (queue.size() >= 2) {
      queue[0].ndi_frame = { };
      std::rotate(queue.begin(), queue.begin() + 1, queue.end());
    }
    // sending asynchronously, keep data available until next synchronizing call
    NDIlib_send_send_video_async_v2(m_ndi_send.get(), &queue[0].ndi_frame);
  }
}

ValueSet Output::get_state() noexcept {
  const auto& desc = target_desc();
  auto state = MemoryOutputStream::get_state();
  state.set(StateNames::resolution_x, desc.width);
  state.set(StateNames::resolution_y, desc.height);
  state.set(StateNames::format, rxext::get_format_name(desc.format));
  state.set(StateNames::frame_rate, m_frame_rate);
  state.set(StateNames::scale_y, -1);
  return state;
}

void Output::send_audio_frame(const AudioFrame& frame, OnComplete on_complete) noexcept { 
  const auto channel_count = static_cast<int>(frame.channels.size());
  const auto sample_count = static_cast<int>(frame.channels[0].size / sizeof(float));
  m_send_audio_buffer.reserve(channel_count * sample_count);
  auto data = m_send_audio_buffer.data();
  for (const auto& channel : frame.channels) {
    std::memcpy(data, channel.data, channel.size);
    data += sample_count;
  }

  auto audio_frame = NDIlib_audio_frame_v2_t{ };
  audio_frame.timecode = NDIlib_send_timecode_synthesize;
  audio_frame.no_channels = channel_count;
  audio_frame.no_samples = sample_count;
  audio_frame.channel_stride_in_bytes = sample_count * sizeof(float);
  audio_frame.sample_rate = static_cast<int>(frame.sample_rate);
  audio_frame.p_data = m_send_audio_buffer.data();
  NDIlib_send_send_audio_v2(m_ndi_send.get(), &audio_frame);

  on_complete();
}

} // namespace
