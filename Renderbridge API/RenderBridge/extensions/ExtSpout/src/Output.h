#pragma once

#include "rxext_client.h"
#include "spout.h"

namespace rxext::spout {

class Output : public rxext::OutputStream {
public:
  Output(std::shared_ptr<d3d11::Device> device, const ValueSet& settings);
  ~Output();

  bool initialize() noexcept override;
  ValueSet get_state() noexcept override;
  TextureRef get_target() noexcept override;
  SyncDesc before_render() noexcept override { return get_sync_desc(); }
  SyncDesc after_render() noexcept override { return get_sync_desc(); }
  void present() noexcept;
  void swap() noexcept;

private:
  SyncDesc get_sync_desc() noexcept;

  const std::shared_ptr<d3d11::Device> m_device;
  const std::string m_handle;
  const int m_resolution_x;
  const int m_resolution_y;
  const Format m_format;
  const int m_frame_rate;
  const bool m_sync_video;
  spoutSenderNames m_spout_sender_names;
  spoutDirectX m_spout_dx;
  spoutFrameCount m_spout_frame_count;
  ComPointer<ID3D11Texture2D> m_spout_texture;
  std::unique_ptr<d3d11::Texture> m_shared_texture;
  std::unique_ptr<d3d11::Fence> m_d3d11_fence;
  uint64_t m_fence_counter{ };
  TextureRef m_target_texture;
};

} // namespace
