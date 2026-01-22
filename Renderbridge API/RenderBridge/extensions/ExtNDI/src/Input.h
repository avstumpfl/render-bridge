#pragma once

#include "rxext_client.h"
#include "ndi.h"
#include <mutex>

namespace rxext::ndi {

class Input : public rxext::MemoryInputStream {
public:
  Input(const ValueSet& settings);

  bool initialize() noexcept override;
  void set_video_callback(SendVideoFrame&& send_video_frame) noexcept override;
  void set_audio_callback(SendAudioFrame&& send_audio_frame) noexcept override;
  ValueSet get_state() noexcept override;

private:
  using ReceivePtr = std::shared_ptr<NDIlib_recv_instance_type>;
  struct VideoFrame {
    ReceivePtr receive_ptr;
    NDIlib_video_frame_v2_t ndi_frame;
  };
  struct AudioFrame {
    ReceivePtr receive_ptr;
    NDIlib_audio_frame_v2_t ndi_frame;
  };
  struct FreeVideoFrame { void operator()(VideoFrame*) const; };
  using VideoFramePtr = std::unique_ptr<VideoFrame, FreeVideoFrame>;
  struct FreeAudioFrame { void operator()(AudioFrame*) const; };
  using AudioFramePtr = std::unique_ptr<AudioFrame, FreeAudioFrame>;

  void receive_frame_callchain() noexcept;
  void write_video_frame(VideoFramePtr video_frame) noexcept;
  void write_audio_frame(AudioFramePtr audio_frame) noexcept;
  void update_stream_mode(const NDIlib_video_frame_v2_t* video, 
    const NDIlib_audio_frame_v2_t* audio) noexcept;

  const std::string m_handle;
  std::mutex m_mutex;
  SendVideoFrame m_send_video_frame;
  SendAudioFrame m_send_audio_frame;
  ReceivePtr m_ndi_receive;

  int m_resolution_x{ };
  int m_resolution_y{ };
  std::string m_pixel_format;
  NDIlib_FourCC_video_type_e m_video_type{ };
  double m_frame_rate{ };
  int m_audio_channel_count{ };
  int m_audio_sample_rate{ };
};

} // namespace
