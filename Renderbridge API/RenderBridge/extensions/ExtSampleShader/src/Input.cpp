
#include "Input.h"
#include <array>

namespace rxext::sample_shader {

namespace {
  // source: https://www.shadertoy.com/view/MtXBWn
  const auto shader_source = R"glsl(
    //@implements: sampler2D
    struct Sampler {
      sampler2D texture;
      float time;
    };

    vec4 texture(Sampler s, vec2 tex_coords) {
      vec4 O;
      O.xy = asin(2.* tex_coords - 1.),
      O.zw = - O.xy;
      O = O.xxzz / 2. - O.ywyw / 3. + s.time;
      O = fwidth(O) / abs( fract( O * 1.9 ) -.5 ); O += O.a;
      return texture(s.texture, tex_coords) + O;
    }

    ivec2 textureSize(Sampler s, int level) {
      return textureSize(s.texture, level);
    }
  )glsl";
} // namespace

Input::Input(const ValueSet& settings) {
  m_texture = add_parameter<ParameterTexture>("texture");
  add_parameter<ParameterValue>("time")
    ->set_property(PropertyNames::purpose, PurposeNames::TimelineTime);
}

string Input::get_property(string_view name) noexcept { 
  if (name == PropertyNames::shader_export)
    return string("Sampler");
  if (name == PropertyNames::shader_source)
    return string(shader_source);
  return { }; 
}

bool Input::update() noexcept try {
  // allocate input texture when dimensions changed
  update_input_texture(*m_texture, 
    [&](const TextureDesc& desc) {
      m_texture->set_texture(host().create_texture(desc));
    });
  return true;
}
catch (const std::exception& ex) {
  host().log_error(std::string("updating input failed: ") + ex.what());
  return false;
}

} // namespace
