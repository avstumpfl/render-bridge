
//@implements: sampler2D
struct WarpBlendAlphaMask {
  sampler2D sampler;
};

vec4 texture(WarpBlendAlphaMask s, vec2 tex_coords) {
  vec4 color = texture(s.sampler, tex_coords);
  return vec4(vec3(color.a), 1);
}
