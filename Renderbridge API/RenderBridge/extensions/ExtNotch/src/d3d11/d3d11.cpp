
#include "d3d11.h"
#include <stdexcept>
#include <d3d11_4.h>
#include <DXGI.h>
#include <vector>
#include "common/string.h"

const SECURITY_ATTRIBUTES& getSecurityAttributes();

namespace d3d11 {

namespace {
  ComPointer<IDXGIAdapter1> get_device_adapter(IDXGIFactory1& factory, LUID adapter_luid) {
    auto adapter = ComPointer<IDXGIAdapter1>{ };
    for (auto i = 0u; factory.EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i) {
      if (adapter_luid.HighPart == 0 && 
          adapter_luid.LowPart == 0)
        return adapter;

      auto desc = DXGI_ADAPTER_DESC1{};
      adapter->GetDesc1(&desc);
      if (desc.AdapterLuid.HighPart == adapter_luid.HighPart &&
          desc.AdapterLuid.LowPart == adapter_luid.LowPart)
        return adapter;
    }
    return { };
  }

  ComPointer<ID3D11Device> create_device(ComPointer<IDXGIAdapter1> adapter) {
    auto device_flags = 0u;

    // Notch v1.0 requires DirectX 11.3 features
    auto feature_levels_requested = D3D_FEATURE_LEVEL_11_1;
    auto options_11_3 = D3D11_FEATURE_DATA_D3D11_OPTIONS2{ };
    auto device = ComPointer<ID3D11Device>();
    if (FAILED(D3D11CreateDevice(adapter, D3D_DRIVER_TYPE_UNKNOWN, NULL, device_flags, 
          &feature_levels_requested, 1, D3D11_SDK_VERSION, &device, NULL, nullptr)) ||
        FAILED(device->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS2, &options_11_3, sizeof(options_11_3)))) {

      // Fallback to allow loading older blocks
      feature_levels_requested = D3D_FEATURE_LEVEL_11_0;
      if (!device && FAILED(D3D11CreateDevice(adapter, D3D_DRIVER_TYPE_UNKNOWN, NULL, device_flags, 
          &feature_levels_requested, 1, D3D11_SDK_VERSION, &device, NULL, nullptr)))
        return { };
    }
    return device;
  }
} // namespace

Device::Device(std::string_view adapter_luid) {
  auto dxgi_factory = ComPointer<IDXGIFactory1>{ };
  if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&dxgi_factory))))
    m_device = create_device(get_device_adapter(*dxgi_factory, 
      common::from_hex_string<LUID>(adapter_luid)));
  if (!m_device)
    throw std::runtime_error("creating D3D11 device failed");

  m_device_5 = com_cast<ID3D11Device5>(m_device);
  if (!m_device_5)
    throw std::runtime_error("obtaining D3D11_5 device failed");

  m_device->GetImmediateContext(&m_device_context);
  m_device_context_4 = com_cast<ID3D11DeviceContext4>(m_device_context);
  if (!m_device_context_4)
    throw std::runtime_error("obtaining D3D11_4 device context failed");
}

Device::~Device() = default;

void Device::clear_target(ID3D11RenderTargetView* render_target_view) {
  float color[4] = { };
  device_context()->ClearRenderTargetView(render_target_view, color);
}

void Device::copy_texture(ID3D11Texture2D* dest, ID3D11Texture2D* source) {
  if (dest != source)
    device_context()->CopyResource(dest, source);
}

Texture::Texture(ID3D11Device* device, size_t width, size_t height, 
    DXGI_FORMAT format, bool is_render_target) 
  : m_width(width), m_height(height), m_format(format) {

  auto desc = D3D11_TEXTURE2D_DESC{ };
  desc.Width = static_cast<UINT>(width);
  desc.Height = static_cast<UINT>(height);
  desc.Format = format;
  desc.ArraySize = 1;
  desc.MipLevels = 1;
  desc.SampleDesc = { 1, 0 };
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.CPUAccessFlags = 0;
  desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  if (is_render_target)
    desc.BindFlags |= D3D11_BIND_RENDER_TARGET;

  // SHARED_NTHANDLE | SHARED_KEYEDMUTEX + keyed mutex synchronization:
  //  - AcquireKeyedMutexWin32EXT fails with "memory has no keyed mutex"
  // SHARED_NTHANDLE | SHARED_KEYEDMUTEX + D3D11 Fence:
  //  - no errors but textures remain black
  // SHARED_NTHANDLE (+ D3D11 Fence):
  //  - CreateTexture2D fails
  // what finally worked:
  desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED | D3D11_RESOURCE_MISC_SHARED_NTHANDLE;

  if (FAILED(device->CreateTexture2D(&desc, nullptr, &m_texture)))
    throw std::runtime_error("creating D3D11 texture failed");

  if (auto dxgi_resource = com_cast<IDXGIResource1>(m_texture))
    dxgi_resource->CreateSharedHandle(&getSecurityAttributes(), 
      GENERIC_ALL, nullptr, &m_share_handle);
  if (!m_share_handle)
    throw std::runtime_error("creating D3D11 texture share handle failed");

  auto srv_desc = D3D11_SHADER_RESOURCE_VIEW_DESC{ };
  srv_desc.Format = desc.Format;
  srv_desc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
  srv_desc.Texture2D.MipLevels = 1;
  srv_desc.Texture2D.MostDetailedMip = 0;
  if (FAILED(device->CreateShaderResourceView(m_texture, &srv_desc, &m_shader_resource_view)))
    throw std::runtime_error("creating D3D11 shader resource view failed");

  if (is_render_target && FAILED(device->CreateRenderTargetView(
      m_texture, NULL, &m_render_target_view)))
    throw std::runtime_error("creating D3D11 render target view failed");
}

Texture::~Texture() = default;

TextureRef::TextureRef(std::nullptr_t) {
}

TextureRef::TextureRef(ID3D11Texture2D* texture) 
  : m_texture(texture){
  if (m_texture)
    m_texture->AddRef();
}

TextureRef::~TextureRef() = default;

Fence::Fence(ID3D11Device5* device) {
  if (FAILED(device->CreateFence(0, D3D11_FENCE_FLAG_SHARED, IID_PPV_ARGS(&m_fence))))
    throw std::runtime_error("creating D3D11 fence failed");

  if (FAILED(m_fence->CreateSharedHandle(&getSecurityAttributes(), 
        GENERIC_ALL, nullptr, &m_share_handle)))
    throw std::runtime_error("creating D3D11 fence share handle failed");
}

Fence::~Fence() = default;

void Fence::signal(ID3D11DeviceContext4* device_context, uint64_t value) {
  device_context->Signal(m_fence, value);
}

void Fence::wait(ID3D11DeviceContext4* device_context, uint64_t value) {
  device_context->Wait(m_fence, value);
}

} // namespace