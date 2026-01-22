
//@implements: sampler2D
struct RangeSampler {
  sampler2D sampler0;
  sampler2D sampler1;
  float blend;
};

vec4 texture(RangeSampler s, vec2 tex_coords) {
  return mix(texture(s.sampler0, tex_coords),
             texture(s.sampler1, tex_coords), s.blend);
}
