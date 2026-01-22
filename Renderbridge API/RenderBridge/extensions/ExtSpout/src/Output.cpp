
#include "Output.h"
#include <functional>

namespace rxext::spout {

// based on SPOUTSDK\SpoutDirectX\Basic\Tutorial04 (using functions directly from Spout SDK classes)

namespace {
  DXGI_FORMAT get_dxgi_format(Format format) {
    switch (format) {
      case Format::None: break;
      case Format::R8_UNORM: return DXGI_FORMAT_R8_UNORM;
      case Format::R8G8_UNORM: return DXGI_FORMAT_R8G8_UNORM;
      case Format::R8G8B8A8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM;
      case Format::B8G8R8A8_UNORM: return DXGI_FORMAT_B8G8R8A8_UNORM;
      case Format::R16G16B16A16_SFLOAT: return DXGI_FORMAT_R16G16B16A16_FLOAT;
      case Format::R32G32B32A32_SFLOAT: return DXGI_FORMAT_R32G32B32A32_FLOAT;
      default: break;
    }
    throw std::runtime_error("unhandled texture format");
  }
} // namespace

Output::Output(std::shared_ptr<d3d11::Device> device, const ValueSet& settings)
    : m_device(std::move(device)), 
      m_handle(settings.get(SettingNames::handle)),
      m_resolution_x(settings.get<int>(SettingNames::resolution_x, 1920)),
      m_resolution_y(settings.get<int>(SettingNames::resolution_y, 1080)),
      m_format(get_format_by_name(settings.get(SettingNames::format))),
      m_frame_rate(settings.get<int>(SettingNames::frame_rate, 60)),
      m_sync_video(settings.get<int>(SettingNames::sync_group, -1) >= 0) {
  m_d3d11_fence = std::make_unique<d3d11::Fence>(m_device->device_5());
}

Output::~Output() {
  m_spout_sender_names.ReleaseSenderName(m_handle.c_str());
  m_spout_frame_count.CloseAccessMutex();
  m_spout_frame_count.CleanupFrameCount();
}

bool Output::initialize() noexcept try {
  // create texture shared with other process
  auto share_handle = HANDLE{ };

  // TODO: could get BGRA only right, by flipping it to RGBA here.
  auto dxgi_format = get_dxgi_format(m_format);
  if (dxgi_format == DXGI_FORMAT_B8G8R8A8_UNORM)
    dxgi_format = DXGI_FORMAT_R8G8B8A8_UNORM;

  if (!m_spout_dx.CreateSharedDX11Texture(*m_device, m_resolution_x, m_resolution_y, 
        dxgi_format, &m_spout_texture, share_handle, true) ||
      !m_spout_sender_names.CreateSender(m_handle.c_str(), 
        m_resolution_x, m_resolution_y, share_handle, dxgi_format) ||
      !m_spout_frame_count.CreateAccessMutex(m_handle.c_str()))
    return false; 

  m_spout_frame_count.EnableFrameCount(m_handle.c_str());

  // create texture shared with host
  m_shared_texture = std::make_unique<d3d11::Texture>(*m_device, 
    m_resolution_x, m_resolution_y, dxgi_format, true);

  // register as target texture
  auto desc = TextureDesc{ };
  desc.width = m_resolution_x;
  desc.height = m_resolution_y;
  desc.format = m_format;
  desc.is_target = true;
  desc.share_handle = { 
    HandleType::D3D11_IMAGE,
    m_shared_texture->share_handle()
  };
  m_target_texture = host().create_texture(desc);
  return true;
}
catch (std::exception& ex) {
  host().log_error(ex.what());
  return false;
}

ValueSet Output::get_state() noexcept {
  auto state = ValueSet();
  state.set(StateNames::resolution_x, m_resolution_x);
  state.set(StateNames::resolution_y, m_resolution_y);
  state.set(StateNames::frame_rate, m_frame_rate);
  state.set(StateNames::format, get_format_name(m_format));
  state.set(StateNames::scale_y, -1);
  return state;
}

SyncDesc Output::get_sync_desc() noexcept {
  return {
    SyncStrategy::TimelineSemaphore, 
    ShareHandle{
      HandleType::D3D_FENCE,
      m_d3d11_fence->share_handle(),
    },
    m_fence_counter,
  };
}

TextureRef Output::get_target() noexcept {
  return m_target_texture;
}

void Output::present() noexcept {
  if (m_spout_frame_count.CheckTextureAccess(m_spout_texture)) {
    // synchronize with end of update
    m_d3d11_fence->wait(m_device->device_context_4(), ++m_fence_counter);

    // copy texture
    m_device->device_context()->CopyResource(m_spout_texture, m_shared_texture->texture());

    // signal end of usage
    m_d3d11_fence->signal(m_device->device_context_4(), ++m_fence_counter);

    m_spout_frame_count.SetNewFrame();
    m_spout_frame_count.AllowTextureAccess(m_spout_texture);
  }
}

void Output::swap() noexcept {
  // also needs to be implemented on the sender side, otherwise this does not block
  if (m_sync_video)
    m_spout_frame_count.WaitFrameSync(m_handle.c_str(), 500);
}

} // namespace
