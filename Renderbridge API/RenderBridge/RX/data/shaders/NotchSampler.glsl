
//@implements: sampler2D
struct NotchSampler {
  sampler2D sampler;
  float alpha;
};

vec4 texture(NotchSampler s, vec2 tex_coords) {
  vec4 color = texture(s.sampler, vec2(tex_coords.x, 1 - tex_coords.y));
  color.a *= s.alpha;
  return color;
}

vec4 textureLod(NotchSampler s, vec2 tex_coords, float lod) {
  return textureLod(s.sampler, vec2(tex_coords.x, 1 - tex_coords.y), lod);
}
