
//@implements: sampler2D
struct UnrealSampler {
  sampler2D color;
  vec4 viewport_rect;
  bool ignore_alpha;
  bool invert_alpha;
};

vec4 texture(UnrealSampler s, vec2 tex_coords) {
  tex_coords.y = 1.0 - tex_coords.y;
  
  tex_coords = tex_coords * s.viewport_rect.zw + s.viewport_rect.xy;
  vec4 color = texture(s.color, tex_coords);
  if (s.ignore_alpha) {
    color.a = 1.0;
  }
  else if (s.invert_alpha) {
    color.a = 1.0 - color.a;
  }
  return color;
}
