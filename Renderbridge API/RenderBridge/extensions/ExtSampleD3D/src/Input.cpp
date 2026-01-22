
#include "Input.h"
#include <d3d11.h>
#include <format>

namespace rxext::sample_d3d {

namespace {
  DXGI_FORMAT get_dxgi_format(Format format) {
    switch (format) {
      case Format::None: break;
      case Format::R8_UNORM: return DXGI_FORMAT_R8_UNORM;
      case Format::R8G8_UNORM: return DXGI_FORMAT_R8G8_UNORM;
      case Format::R8G8B8A8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM;
      case Format::B8G8R8A8_UNORM: return DXGI_FORMAT_B8G8R8A8_UNORM;
      case Format::R16G16B16A16_SFLOAT: return DXGI_FORMAT_R16G16B16A16_FLOAT;
    }
    throw std::runtime_error("unhandled format");
  }
} // namespace

Input::Input(std::shared_ptr<d3d11::Device> device, ValueSet settings) 
    : m_device(std::move(device)),
      m_settings(std::move(settings)),
      m_d3d_fence(std::make_unique<d3d11::Fence>(m_device->device_5())),
      m_d3d_quad(std::make_unique<d3d11::Quad>(m_device->device(), 2/3.f)) {
}

Input::~Input() = default;

bool Input::initialize() noexcept try {
  host().log_info(std::format("initializing stream '{}'", 
    m_settings.get(SettingNames::handle)));
  
  // share target texture with host as parameter "sampler"
  m_target.parameter = add_output_parameter<ParameterTexture>(ParameterNames::sampler);

  // create some parameters
  m_time = add_parameter<ParameterValue>("time");
  m_time->set_property(PropertyNames::purpose, PurposeNames::TimelineTime);

  m_color = add_parameter<ParameterVector4>("color", 
    std::array<double, 4>{ 1, 0.5, 0, 1 });
  m_color->set_property(PropertyNames::name, "Tint");
  m_color->set_property(PropertyNames::purpose, PurposeNames::Color);

  m_text = add_parameter<ParameterString>("text");

  m_data = add_parameter<ParameterData>("data");

  m_float = add_parameter<ParameterValue>("float", 3.141593);
  m_float->set_property(PropertyNames::purpose, PurposeNames::Rotation);
  m_float->set_property(PropertyNames::min_value, -3.141593 * 2);
  m_float->set_property(PropertyNames::max_value, 3.141593 * 2);
  m_float->set_property(PropertyNames::min_value_ui, -360);
  m_float->set_property(PropertyNames::max_value_ui, 360);

  m_int = add_parameter<ParameterInt>("int", 16);
  m_int->set_property(PropertyNames::min_value, -1);
  m_int->set_property(PropertyNames::max_value, 64);

  m_bool = add_parameter<ParameterBool>("bool", true);

  m_enum = add_parameter<ParameterInt>("enum", 1);
  m_enum->set_property<std::vector<std::string>>(
    PropertyNames::enum_names, { "Option 0", "Option 1 (default)", "Option 2" });

  m_view_matrix = add_parameter<ParameterMatrix4>("view_matrix");
  m_view_matrix->set_property(PropertyNames::purpose, PurposeNames::ViewMatrix);

  m_texture.parameter = add_parameter<ParameterTexture>("texture");

  return update_settings(std::move(m_settings));
}
catch (const std::exception& ex) {
  host().log_error(std::string("initializing input failed: ") + ex.what());
  return false;
}

bool Input::update_settings(ValueSet settings) noexcept {
  m_settings = std::move(settings);

  auto desc = TextureDesc{ };
  desc.width = m_settings.get<int>(SettingNames::resolution_x, 128);
  desc.height = m_settings.get<int>(SettingNames::resolution_y, 128);
  desc.format = (m_settings.get<bool>("red_channel_only") ? 
    Format::R8_UNORM : Format::R8G8B8A8_UNORM);
  desc.is_target = true;
  m_target.d3d_texture = create_shared_texture(*m_target.parameter, desc);

  return true;
}

std::unique_ptr<d3d11::Texture> Input::create_shared_texture(
    ParameterTexture& parameter, TextureDesc desc) {
  // create D3D texture and share with host
  auto d3d_texture = std::make_unique<d3d11::Texture>(m_device->device(), 
    desc.width, desc.height, get_dxgi_format(desc.format), desc.is_target);
  desc.share_handle = { HandleType::D3D11_IMAGE, d3d_texture->share_handle() };
  parameter.set_texture(host().create_texture(desc));
  return d3d_texture;
}

ValueSet Input::get_state() noexcept {
  auto state = ValueSet();
  const auto desc = m_target.parameter->texture().desc();
  state.set(StateNames::resolution_x, desc.width);
  state.set(StateNames::resolution_y, desc.height);
  state.set(StateNames::format, get_format_name(desc.format));
  return state;
}

bool Input::update() noexcept try {
  // allocate input texture when dimensions changed
  update_input_texture(*m_texture.parameter,
    [&](const TextureDesc& desc) {
      m_texture.d3d_texture = 
        create_shared_texture(*m_texture.parameter, desc);
    });
  return true;
}
catch (const std::exception& ex) {
  host().log_error(std::string("updating input failed: ") + ex.what());
  return false;
}

SyncDesc Input::get_sync_desc() noexcept {
  return {
    SyncStrategy::TimelineSemaphore,
    ShareHandle{
      HandleType::D3D_FENCE,
      m_d3d_fence->share_handle(),
    },
    m_fence_counter,
  };
}

RenderResult Input::render() noexcept try {  
  auto d3d_context = m_device->device_context();
  auto d3d_context_4 = m_device->device_context_4();

  // synchronize with end of usage
  m_d3d_fence->wait(d3d_context_4, ++m_fence_counter);

  // set target and viewport
  auto render_target_view = m_target.d3d_texture->render_target_view();
  d3d_context->OMSetRenderTargets(1, &render_target_view, nullptr);
  const auto desc = m_target.parameter->texture().desc();
  auto viewport = D3D11_VIEWPORT{ 0, 0, static_cast<FLOAT>(desc.width), 
                                        static_cast<FLOAT>(desc.height), 0, 1 };
  d3d_context->RSSetViewports(1, &viewport);

  // clear target using values of parameters "color" and "time"
  const auto time = m_time->value();
  const auto color = m_color->value();
  const auto alpha = 0.5 + std::sin(m_time->value() * 5) / 2;
  float fcolor[4] = { 
    static_cast<float>(color[0]),
    static_cast<float>(color[1]), 
    static_cast<float>(color[2]), 
    static_cast<float>(color[3] * alpha)
  };
  d3d_context->ClearRenderTargetView(render_target_view, fcolor);

  // do nothing with other parameters
  [[maybe_unused]] const auto text = m_text->value();
  [[maybe_unused]] const auto data = m_data->value<uint8_t>();
  [[maybe_unused]] const auto vfloat = m_float->value();
  [[maybe_unused]] const auto vint = m_int->value();
  [[maybe_unused]] const auto vbool = m_bool->value();
  [[maybe_unused]] const auto entry = m_enum->value();
  [[maybe_unused]] const auto view = m_view_matrix->value();

  // render input texture to target
  if (m_texture.d3d_texture)
    m_d3d_quad->draw(d3d_context, m_texture.d3d_texture->shader_resource_view());

  // signal end of update
  m_d3d_fence->signal(d3d_context_4, ++m_fence_counter);

  return RenderResult::Succeeded;
}
catch (const std::exception& ex) {
  host().log_error(std::string("rendering input failed: ") + ex.what());
  return RenderResult::Failed;
}

} // namespace
