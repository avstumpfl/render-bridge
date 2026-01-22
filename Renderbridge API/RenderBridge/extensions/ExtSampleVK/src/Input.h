#pragma once

#include "rxext_client.h"
#include "linalg.h"
#include "piglit/vk.h"

namespace rxext::sample_vk {

using ParameterMatrix4 = ParameterT<linalg::aliases::double4x4, ParameterType::Matrix4>;

class Input : public rxext::InputStream {
public:
  Input(vk_ctx* vk_ctx, ValueSet settings);
  ~Input();

  bool initialize() noexcept override;
  ValueSet get_state() noexcept override;
  bool update() noexcept;
  SyncDesc before_render() noexcept override;
  RenderResult render() noexcept override;
  SyncDesc after_render() noexcept override;

private:
  void initialize_renderer();
  void destroy_renderer();

  ValueSet m_settings;
  ParameterTexture* m_sampler{ };
  ParameterValue* m_time{ };
  ParameterMatrix4* m_view_matrix{ };
  ParameterMatrix4* m_projection_matrix{ };
  bool m_low_resolution{ };

  vk_ctx& vk_core;
  vk_image_att vk_color_att{ };
  vk_image_att vk_depth_att{ };
  vk_renderer vk_rnd{ };
  vk_semaphores vk_sem{ };
};

} // namespace
