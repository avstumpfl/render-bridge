
//@implements: sampler2D
struct UnitySampler {
  sampler2D color;
  sampler2D depth;
  bool discard_background;
};

vec4 texture(UnitySampler s, vec2 tex_coords) {
  float depth = texture(s.depth, tex_coords).r;
  if (s.discard_background && depth == 0)
    discard;
  
  gl_FragDepth = 1.0 - depth;
  return vec4(texture(s.color, tex_coords).rgb, 1);
}
