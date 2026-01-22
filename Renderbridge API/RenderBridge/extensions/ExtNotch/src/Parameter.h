#pragma once

#include "rxext_client.h"
#include "glm/glm.hpp"

struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;
struct ID3D11RenderTargetView;

namespace d3d11 {
  class Device;
  class Texture;
}

namespace rxext::notch {

using ParameterVector2 = ParameterT<glm::dvec2, ParameterType::Vector2>;
using ParameterVector3 = ParameterT<glm::dvec3, ParameterType::Vector3>;
using ParameterVector4 = ParameterT<glm::dvec4, ParameterType::Vector4>;
using ParameterMatrix3 = ParameterT<glm::dmat3, ParameterType::Matrix3>;
using ParameterMatrix4 = ParameterT<glm::dmat4, ParameterType::Matrix4>;

class ParameterTexture : public rxext::ParameterTexture {
public:
  explicit ParameterTexture(std::string name);
  ParameterTexture(const ParameterTexture&) = delete;
  ParameterTexture& operator=(const ParameterTexture&) = delete;
  ~ParameterTexture();

  void create(rxext::HostContext& host, d3d11::Device& d3d11_device, 
    rxext::TextureDesc desc);

  ID3D11Texture2D* d3d11_texture() const;
  ID3D11ShaderResourceView* d3d11_shader_resource_view() const;
  ID3D11RenderTargetView* d3d11_render_target_view() const;

private:
  std::unique_ptr<d3d11::Texture> m_d3d11_texture;
};

} // namespace