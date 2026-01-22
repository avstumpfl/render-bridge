#pragma once

#include "rxext_client.h"
#include "d3d11/d3d11.h"

namespace rxext::sample_d3d {

class Input : public rxext::InputStream {
public:
  Input(std::shared_ptr<d3d11::Device> device, ValueSet settings);
  ~Input();

  bool initialize() noexcept override;
  bool update_settings(ValueSet settings) noexcept override;
  ValueSet get_state() noexcept override;
  bool update() noexcept override;
  SyncDesc before_render() noexcept override { return get_sync_desc(); }
  RenderResult render() noexcept override;
  SyncDesc after_render() noexcept override { return get_sync_desc(); }

private:
  struct SharedTexture {
    ParameterTexture* parameter;
    std::unique_ptr<d3d11::Texture> d3d_texture;
  };

  SyncDesc get_sync_desc() noexcept;
  std::unique_ptr<d3d11::Texture> create_shared_texture(
    ParameterTexture& parameter, TextureDesc desc);

  const std::shared_ptr<d3d11::Device> m_device;
  ValueSet m_settings;
  std::unique_ptr<d3d11::Fence> m_d3d_fence;
  uint64_t m_fence_counter{ };
  std::unique_ptr<d3d11::Quad> m_d3d_quad;
  SharedTexture m_target{ };
  
  ParameterValue* m_time{ };
  ParameterVector4* m_color{ };
  ParameterString* m_text{ };
  ParameterData* m_data{ };
  ParameterValue* m_float{ };
  ParameterInt* m_int{ };
  ParameterBool* m_bool{ };
  ParameterInt* m_enum{ };
  ParameterMatrix4* m_view_matrix{ };
  SharedTexture m_texture{ };
};

} // namespace
