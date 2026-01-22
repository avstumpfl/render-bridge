
//@implements: samplerCube
struct CubeMapSampler {
  samplerCube sampler;
};

vec4 texture(CubeMapSampler s, vec3 tex_coords) {
  return texture(s.sampler, tex_coords);
}

vec4 textureLod(CubeMapSampler s, vec3 tex_coords, float lod) {
  return textureLod(s.sampler, tex_coords, lod);
}

int textureQueryLevels(CubeMapSampler s) {
  return textureQueryLevels(s.sampler);
}
