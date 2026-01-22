#pragma once

#include "rxext_client.h"
#include <mutex>

namespace rxext::sample_cpu {

class Input2 : public rxext::MemoryInputStream {
public:
  explicit Input2(const ValueSet& settings);

  bool initialize() noexcept override;
  ValueSet get_state() noexcept override;

private:
  void set_video_callback(SendVideoFrame&& send_video_frame) noexcept override;
  void receive_frame_callchain() noexcept;
  void write_video_frame(int index) noexcept;

  std::mutex m_mutex;
  SendVideoFrame m_send_video_frame;
  int m_resolution_x{ 32 };
  int m_resolution_y{ 32 };
  std::string m_pixel_format{ "RGBA" };
  int m_frame_index{ };
};

} // namespace
