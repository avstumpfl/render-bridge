
#include "Parameter.h"
#include "d3d11/d3d11.h"

namespace rxext::notch {

namespace {
  DXGI_FORMAT get_format(Format format) {
    switch (format) {
      case Format::None: break;
      case Format::R8G8B8A8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM;
      case Format::R16G16B16A16_SFLOAT: return DXGI_FORMAT_R16G16B16A16_FLOAT;
      case Format::R32G32B32A32_SFLOAT: return DXGI_FORMAT_R32G32B32A32_FLOAT;
      default: break;
    }
    return { };
  }
} // namespace

ParameterTexture::ParameterTexture(std::string name) 
  : rxext::ParameterTexture(std::move(name)) {
}

ParameterTexture::~ParameterTexture() = default;

void ParameterTexture::create(rxext::HostContext& host, 
    d3d11::Device& d3d11_device, rxext::TextureDesc desc) {

  m_d3d11_texture = std::make_unique<d3d11::Texture>(d3d11_device, 
    desc.width, desc.height, get_format(desc.format), desc.is_target);

  desc.share_handle = { 
    HandleType::D3D11_IMAGE, 
    m_d3d11_texture->share_handle()
  };
  set_texture(host.create_texture(desc));
}

ID3D11Texture2D* ParameterTexture::d3d11_texture() const {
  return (m_d3d11_texture ? m_d3d11_texture->texture() : nullptr);
}

ID3D11ShaderResourceView* ParameterTexture::d3d11_shader_resource_view() const {
  return (m_d3d11_texture ? m_d3d11_texture->shader_resource_view() : nullptr);
}

ID3D11RenderTargetView* ParameterTexture::d3d11_render_target_view() const {
  return (m_d3d11_texture ? m_d3d11_texture->render_target_view() : nullptr);
}

} /// namespace
