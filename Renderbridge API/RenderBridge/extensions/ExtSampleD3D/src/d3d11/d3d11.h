#pragma once

#pragma warning(disable: 5219) // implicit conversion from 'UINT' to 'FLOAT'

#include <cstdint>
#include <string>
#include "dxgiformat.h"
#include "ComPointer.h"

struct ID3D11Device;
struct ID3D11Device5;
struct ID3D11DeviceContext;
struct ID3D11DeviceContext4;
struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;
struct ID3D11RenderTargetView;
struct ID3D11Fence;
struct ID3D11Buffer;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11InputLayout;
struct ID3D11SamplerState;

namespace d3d11 {

class Device {
public:
  explicit Device(std::string_view adapter_luid);
  Device(const Device&) = delete;
  Device& operator=(const Device&) = delete;
  ~Device();

  ID3D11Device* device() { return m_device; }
  ID3D11Device5* device_5() { return m_device_5; }
  ID3D11DeviceContext* device_context() { return m_device_context; }
  ID3D11DeviceContext4* device_context_4() { return m_device_context_4; }

  operator ID3D11Device*() { return device(); }
  operator ID3D11Device5*() { return device_5(); }
  operator ID3D11DeviceContext*() { return device_context(); }
  operator ID3D11DeviceContext4*() { return device_context_4(); }

private:
  ComPointer<ID3D11Device> m_device;
  ComPointer<ID3D11Device5> m_device_5;
  ComPointer<ID3D11DeviceContext> m_device_context;
  ComPointer<ID3D11DeviceContext4> m_device_context_4;
};

class Texture {
public:
  Texture(ID3D11Device* device, size_t width, size_t height, DXGI_FORMAT format, bool is_render_target);
  Texture(const Texture&) = delete;
  Texture& operator=(const Texture&) = delete;
  ~Texture();

  ID3D11Texture2D* texture() { return m_texture; }
  ID3D11ShaderResourceView* shader_resource_view() { return m_shader_resource_view; }
  ID3D11RenderTargetView* render_target_view() { return m_render_target_view; }
  size_t width() const { return m_width; }
  size_t height() const { return m_height; }
  DXGI_FORMAT format() const { return m_format; }
  void* share_handle() const { return m_share_handle; }

private:
  const size_t m_width;
  const size_t m_height;
  const DXGI_FORMAT m_format;
  ComPointer<ID3D11Texture2D> m_texture;
  ComPointer<ID3D11ShaderResourceView> m_shader_resource_view;
  ComPointer<ID3D11RenderTargetView> m_render_target_view;
  void* m_share_handle{ };
};

class Fence {
public:
  explicit Fence(ID3D11Device5* device_5);
  Fence(const Fence&) = delete;
  Fence& operator=(const Fence&) = delete;
  ~Fence();

  void signal(ID3D11DeviceContext4* device_context_4, uint64_t value);
  void wait(ID3D11DeviceContext4* device_context_4, uint64_t value);
  void* share_handle() const { return m_share_handle; }

private:
  ComPointer<ID3D11Fence> m_fence;
  void* m_share_handle{ };
};

class Quad {
public:
  explicit Quad(ID3D11Device* device, float size = 1.0);
  Quad(const Quad&) = delete;
  Quad& operator=(const Quad&) = delete;
  ~Quad();
  void draw(ID3D11DeviceContext* context, ID3D11ShaderResourceView* texture);

private:
  struct Vertex { float position[4]; float uv[2]; };
  ComPointer<ID3D11Buffer> m_buffer;
  ComPointer<ID3D11VertexShader> m_vertex_shader;
  ComPointer<ID3D11PixelShader> m_pixel_shader;
  ComPointer<ID3D11InputLayout> m_input_layout;
  ComPointer<ID3D11SamplerState> m_sample_linear;
};

} // namespace