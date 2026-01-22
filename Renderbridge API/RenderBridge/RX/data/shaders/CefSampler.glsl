
//@implements: sampler2D
struct CefSampler {
  sampler2D sampler;
  float scale_y;
};

vec4 texture(CefSampler s, vec2 tex_coords) {
  tex_coords.y = 1 - tex_coords.y;
  tex_coords.y *= s.scale_y;
  return texture(s.sampler, tex_coords).bgra;
}

