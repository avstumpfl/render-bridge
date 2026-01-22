
#include "Input2.h"
#include <cmath>

namespace rxext::sample_cpu {

Input2::Input2(const ValueSet& settings) {
}

void Input2::set_video_callback(SendVideoFrame&& send_video_frame) noexcept {
  auto lock = std::lock_guard(m_mutex);
  m_send_video_frame = std::move(send_video_frame);
}

bool Input2::initialize() noexcept try {
  receive_frame_callchain();
  return true;
}
catch (const std::exception& ex) {
  host().log_error(std::string("initializing input failed: ") + ex.what());
  return false;
}

ValueSet Input2::get_state() noexcept {
  auto state = ValueSet();
  state.set(StateNames::resolution_x, m_resolution_x);
  state.set(StateNames::resolution_y, m_resolution_y);
  state.set(StateNames::pixel_format, m_pixel_format);
  return state;
}

void Input2::receive_frame_callchain() noexcept {
  auto lock = std::lock_guard(m_mutex);
  if (m_send_video_frame)
    write_video_frame(m_frame_index++);

  const auto delay = std::chrono::milliseconds(20);
  host().set_timeout(delay, [this]() noexcept { receive_frame_callchain(); });
}

void Input2::write_video_frame(int index) noexcept {
  auto frame = rxext::VideoFrame{ };
  frame.resolution_x = m_resolution_x;
  frame.resolution_y = m_resolution_y;
  frame.pixel_format = m_pixel_format;

  struct RGBA8 { uint8_t r,g,b,a; };
  auto buffer = BufferDesc{ };
  auto data = std::make_unique<RGBA8[]>(m_resolution_x * m_resolution_y);
  for (auto y = 0; y < m_resolution_y; ++y)
    for (auto x = 0; x < m_resolution_x; ++x) {
      auto& color = data[y * m_resolution_x + x];
      color.r = static_cast<uint8_t>((x * 255) / (m_resolution_x - 1));
      color.g = static_cast<uint8_t>((y * 255) / (m_resolution_y - 1));
      color.b = static_cast<uint8_t>(127);
      color.a = static_cast<uint8_t>((std::sin(index * 0.1) + 1) * 127);
    }
  buffer.data = data.get();
  buffer.pitch = m_resolution_x * sizeof(RGBA8);
  buffer.size = m_resolution_y * buffer.pitch;
  frame.planes.push_back(buffer);

  m_send_video_frame(frame, [data = std::move(data)]() noexcept { });
}

} // namespace
