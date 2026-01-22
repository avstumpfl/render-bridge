
//@implements: sampler2D
struct MultiSampler {
  sampler2D sampler0;
  sampler2D sampler1;
  sampler2D sampler2;
  sampler2D sampler3;
  sampler2D sampler4;
  sampler2D sampler5;
  sampler2D sampler6;
  sampler2D sampler7;
  float blend0;
  float blend1;
  float blend2;
  float blend3;
  float blend4;
  float blend5;
  float blend6;
  float blend7;
  mat3x3 transform0;
  mat3x3 transform1;
  mat3x3 transform2;
  mat3x3 transform3;
  mat3x3 transform4;
  mat3x3 transform5;
  mat3x3 transform6;
  mat3x3 transform7;
};

vec2 transform(vec2 tc, mat3x3 mat) {
   return (mat * vec3(tc, 1)).xy;
}

float clamp_factor(vec2 tc) {
  return step(0, tc.x) * step(0, tc.y) * (1 - step(1, tc.x)) * (1 - step(1, tc.y));
}

vec4 texture(MultiSampler s, vec2 tex_coords) {
  vec4 color = vec4(0);
  
  vec2 tc = transform(tex_coords, s.transform0);
  float blend = s.blend0 * clamp_factor(tc);
  if (blend > 0) {
    vec4 map = texture(s.sampler0, tc);
    color = mix(color, map, blend * map.a);
  }
  
#if !defined(sampler1_MISSING)
  tc = transform(tex_coords, s.transform1);
  blend = s.blend1 * clamp_factor(tc);
  if (blend > 0) {
    vec4 map = texture(s.sampler1, tc);
    color = mix(color, map, blend * map.a);
  }
#endif
  
#if !defined(sampler2_MISSING)
  tc = transform(tex_coords, s.transform2);
  blend = s.blend2 * clamp_factor(tc);
  if (blend > 0) {
    vec4 map = texture(s.sampler2, tc);
    color = mix(color, map, blend * map.a);
  }
#endif

#if !defined(sampler3_MISSING)
  tc = transform(tex_coords, s.transform3);
  blend = s.blend3 * clamp_factor(tc);
  if (blend > 0) {
    vec4 map = texture(s.sampler3, tc);
    color = mix(color, map, blend * map.a);
  }
#endif

#if !defined(sampler4_MISSING)
  tc = transform(tex_coords, s.transform4);
  blend = s.blend4 * clamp_factor(tc);
  if (blend > 0) {
    vec4 map = texture(s.sampler4, tc);
    color = mix(color, map, blend * map.a);
  }
#endif

#if !defined(sampler5_MISSING)
  tc = transform(tex_coords, s.transform5);
  blend = s.blend5 * clamp_factor(tc);
  if (blend > 0) {
    vec4 map = texture(s.sampler5, tc);
    color = mix(color, map, blend * map.a);
  }
#endif

#if !defined(sampler6_MISSING)
  tc = transform(tex_coords, s.transform6);
  blend = s.blend6 * clamp_factor(tc);
  if (blend > 0) {
    vec4 map = texture(s.sampler6, tc);
    color = mix(color, map, blend * map.a);
  }
#endif

#if !defined(sampler7_MISSING)
  tc = transform(tex_coords, s.transform7);
  blend = s.blend7 * clamp_factor(tc);
  if (blend > 0) {
    vec4 map = texture(s.sampler7, tc);
    color = mix(color, map, blend * map.a);
  }
#endif

  return color;
}
