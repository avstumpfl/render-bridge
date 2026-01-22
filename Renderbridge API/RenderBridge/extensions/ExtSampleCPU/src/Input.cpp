
#include "Input.h"
#include <array>

namespace rxext::sample_cpu {

Input::Input(const ValueSet& settings) 
    : m_sampler(*add_output_parameter<ParameterTexture>(ParameterNames::sampler)) {
}

bool Input::initialize() noexcept try {
  auto desc = TextureDesc{ };
  desc.width = 2;
  desc.height = 2;
  desc.format = Format::B8G8R8A8_UNORM;
  m_sampler.set_texture(host().create_texture(desc));

  auto buffer = BufferDesc{ };
  auto colors = std::array<uint32_t, 4>{ 0xFF330000, 0xFF330000, 0xFFCC9933, 0xFFCC9933 };
  buffer.data = colors.data();
  buffer.pitch = 2 * sizeof(uint32_t);
  buffer.size = 2 * buffer.pitch;
  host().upload_texture(m_sampler.texture(), buffer, true, []() noexcept { });
  return true;
}
catch (const std::exception& ex) {
  host().log_error(std::string("initializing input failed: ") + ex.what());
  return false;
}

ValueSet Input::get_state() noexcept {
  auto state = ValueSet();
  const auto& desc = m_sampler.texture().desc();
  state.set(StateNames::resolution_x, desc.width);
  state.set(StateNames::resolution_y, desc.height);
  state.set(StateNames::format, get_format_name(desc.format));
  return state;
}

} // namespace
