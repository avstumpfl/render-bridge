
#include "Input.h"
#include <array>
#include <functional>

namespace rxext::spout {

// based on SPOUTSDK\SpoutDirectX\Basic\Tutorial07 (using functions directly from Spout SDK classes)

namespace {
  Format get_format(DXGI_FORMAT format) { 
    switch (format) {
      case DXGI_FORMAT_R8G8B8A8_UNORM: return Format::R8G8B8A8_UNORM;
      case DXGI_FORMAT_B8G8R8A8_UNORM: return Format::B8G8R8A8_UNORM;
      case DXGI_FORMAT_R16G16B16A16_FLOAT: return Format::R16G16B16A16_SFLOAT;
      case DXGI_FORMAT_R32G32B32A32_FLOAT: return Format::R32G32B32A32_SFLOAT;
      default: 
        throw std::runtime_error("unsupported format");
    }
  }
} // namespace

Input::Input(std::shared_ptr<d3d11::Device> device, const ValueSet& settings) 
    : m_device(std::move(device)), 
      m_handle(settings.get(SettingNames::handle)),
      m_sampler(*add_parameter<ParameterTexture>(ParameterNames::sampler)) {

  m_d3d11_fence = std::make_unique<d3d11::Fence>(m_device->device_5());
}

Input::~Input() {
  stop();
}

ValueSet Input::get_state() noexcept {
  const auto lock = std::lock_guard(m_mutex);
  auto state = ValueSet();
  if (m_sampler.texture()) {
    const auto& desc = m_sampler.texture().desc();
    state.set(StateNames::resolution_x, desc.width);
    state.set(StateNames::resolution_y, desc.height);
    state.set(StateNames::frame_rate, m_frame_rate);
    state.set(StateNames::format, get_format_name(desc.format));
    state.set(StateNames::scale_y, -1);
  }
  return state;
}

bool Input::update() noexcept try {
  auto width = 0u;
  auto height = 0u;
  auto share_handle = HANDLE{ };
  auto format = DWORD{ };
  auto name = std::array<char, SpoutMaxSenderNameLen>();
  std::copy_n(m_handle.data(), m_handle.size() + 1, name.data());
	m_spout_sender_names.FindSender(name.data(), width, height, share_handle, format);

  if (m_spout_share_handle != share_handle) {
    m_shared_texture.reset();

    // import texture shared with other process
    if (!m_spout_dx.OpenDX11shareHandle(m_device->device(), &m_spout_texture, share_handle))
      return false;

    // create texture shared with host
    m_shared_texture = std::make_unique<d3d11::Texture>(
      m_device->device(), width, height, static_cast<DXGI_FORMAT>(format), false);

    // register as sampler texture
    auto desc = TextureDesc{ };
    desc.width = width;
    desc.height = height;
    desc.format = get_format(static_cast<DXGI_FORMAT>(format));
    desc.share_handle = {
      HandleType::D3D11_IMAGE,
      m_shared_texture->share_handle()
    };
    m_sampler.set_texture(host().create_texture(desc));

    m_spout_share_handle = share_handle;
    host().send_event(EventCategory::StreamsChanged);
  }
  return true;
}
catch (const std::exception& ex) {
  host().log_error(std::string("initializing Spout failed: ") + ex.what());
  return false;
}

void Input::set_video_requested(bool requested) noexcept {
  if (requested)
    start();
  else
    stop();
}

void Input::start() {
  if (std::exchange(m_started, true))
    return;

	m_spout_frame_count.CreateAccessMutex(m_handle.c_str());
	m_spout_frame_count.EnableFrameCount(m_handle.c_str());
}

void Input::stop() {
  if (!std::exchange(m_started, false))
    return;

  m_spout_frame_count.CloseAccessMutex();
  m_spout_frame_count.CleanupFrameCount();
}

SyncDesc Input::get_sync_desc() noexcept {
  return {
    SyncStrategy::TimelineSemaphore,
    ShareHandle{
      HandleType::D3D_FENCE,
      m_d3d11_fence->share_handle(),
    },
    m_fence_counter,
  };
}

RenderResult Input::render() noexcept try {
  if (m_shared_texture)
	  if (m_spout_frame_count.CheckTextureAccess(m_spout_texture)) {
		  if (m_spout_frame_count.GetNewFrame()) {
        // synchronize with end of usage
        m_d3d11_fence->wait(m_device->device_context_4(), ++m_fence_counter);

        // copy texture
        m_device->device_context()->CopyResource(m_shared_texture->texture(), m_spout_texture);

        // signal end of update
        m_d3d11_fence->signal(m_device->device_context_4(), ++m_fence_counter);
      }
		  m_spout_frame_count.AllowTextureAccess(m_spout_texture);
    }

  // allow source to synchronize to output
  m_spout_frame_count.SetFrameSync(m_handle.c_str());
  m_frame_rate = m_spout_frame_count.GetSenderFps();

  return (m_shared_texture ? RenderResult::Succeeded : RenderResult::Failed);
}
catch (const std::exception& ex) {
  host().log_error(std::string("receiving Spout frame failed: ") + ex.what());
  return RenderResult::Failed;
}

} // namespace
