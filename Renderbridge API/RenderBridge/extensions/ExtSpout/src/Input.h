#pragma once

#include "rxext_client.h"
#include "spout.h"
#include <mutex>

namespace rxext::spout {

class Input : public rxext::InputStream {
public:
  Input(std::shared_ptr<d3d11::Device> device, const ValueSet& settings);
  ~Input();

  ValueSet get_state() noexcept override;
  bool update() noexcept;
  void set_video_requested(bool requested) noexcept override;
  SyncDesc before_render() noexcept override { return get_sync_desc(); }
  RenderResult render() noexcept override;
  SyncDesc after_render() noexcept override { return get_sync_desc(); }

private:
  void start();
  void stop();
  SyncDesc get_sync_desc() noexcept;

  const std::shared_ptr<d3d11::Device> m_device;
  const std::string m_handle;
  ParameterTexture& m_sampler;
  std::mutex m_mutex;
  spoutSenderNames m_spout_sender_names;
  spoutDirectX m_spout_dx;
  spoutFrameCount m_spout_frame_count;
  
  bool m_started{ };
  double m_frame_rate{ };
  HANDLE m_spout_share_handle{ };
  ComPointer<ID3D11Texture2D> m_spout_texture;
  std::unique_ptr<d3d11::Texture> m_shared_texture;
  std::unique_ptr<d3d11::Fence> m_d3d11_fence;
  uint64_t m_fence_counter{ };
};

} // namespace
