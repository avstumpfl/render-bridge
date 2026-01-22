
//@implements: sampler2D
struct Sampler {
  sampler2D sampler;
};

vec4 texture(Sampler s, vec2 tex_coords) {
  return texture(s.sampler, tex_coords);
}

vec4 textureLod(Sampler s, vec2 tex_coords, float lod) {
  return textureLod(s.sampler, tex_coords, lod);
}

vec4 texelFetch(Sampler s, ivec2 tex_coords, int level) {
  return texelFetch(s.sampler, tex_coords, level);
}

//@implements: sampler2D
struct TopDownSampler {
  sampler2D sampler;
};

vec4 texture(TopDownSampler s, vec2 tex_coords) {
  return texture(s.sampler, vec2(tex_coords.x, 1 - tex_coords.y));
}

vec4 textureLod(TopDownSampler s, vec2 tex_coords, float lod) {
  return textureLod(s.sampler, vec2(tex_coords.x, 1 - tex_coords.y), lod);
}

//@implements: sampler3D
struct Sampler3D {
  sampler3D sampler;
};

vec4 texture(Sampler3D s, vec3 tex_coords) {
  return texture(s.sampler, tex_coords);
}

vec4 textureLod(Sampler3D s, vec3 tex_coords, float lod) {
  return textureLod(s.sampler, tex_coords, lod);
}

vec4 texelFetch(Sampler3D s, ivec3 tex_coords, int level) {
  return texelFetch(s.sampler, tex_coords, level);
}
