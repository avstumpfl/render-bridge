
//@implements: sampler2D
struct SolidColor {
  vec4 color;
};

vec4 texture(SolidColor s, vec2 tex_coords) {
  return s.color;
}
