
//@implements: sampler2D
struct ColorTransform_dynamic {
  sampler2D sampler;
  float scale_factor;
};

vec4 texture(ColorTransform_dynamic s, vec2 tex_coords) {
  vec4 color = texture(s.sampler, tex_coords);
  //DYNAMIC
  color.rgb *= s.scale_factor;
  return color;
}
