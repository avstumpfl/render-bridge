#pragma once

#include "rxext_client.h"
#include "ndi.h"

namespace rxext::ndi {

class Output : public rxext::MemoryOutputStream {
public:
  explicit Output(const ValueSet& settings);

  bool initialize() noexcept override;
  bool send_texture_data(const BufferDesc& plane) noexcept override;
  void swap() noexcept override;
  ValueSet get_state() noexcept override;
  void send_audio_frame(const AudioFrame& frame, OnComplete on_complete) noexcept override;

private:
  struct FreeSend { void operator()(NDIlib_send_instance_type* send) const { NDIlib_send_destroy(send); } };
  using SendPtr = std::unique_ptr<NDIlib_send_instance_type, FreeSend>;

  struct SendVideoFrame {
    std::vector<uint8_t> buffer;
    NDIlib_video_frame_v2_t ndi_frame;
  };

  void detect_video_request_callchain() noexcept;

  const std::string m_handle;
  const double m_frame_rate;
  const bool m_sync_video;
  std::mutex m_mutex;
  std::vector<SendVideoFrame> m_send_video_queue;
  std::vector<float> m_send_audio_buffer;
  SendPtr m_ndi_send;
};

} // namespace
