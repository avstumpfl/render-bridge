
vec4 transform(vec4 color, mat4x4 color_model_matrix) {
  vec3 c = mat4x3(color_model_matrix) * vec4(color.rgb, 1);
  float a = color.a * color_model_matrix[3][3] + color_model_matrix[2][3];
  return vec4(c, a);
}

//@implements: sampler2D
struct FrameBlender {
  mat3x3 coordinate_transform;
  sampler2D sampler0;
  sampler2D sampler1;
  sampler2D sampler2;
  float blend_factor0;
  float blend_factor1;
  float blend_factor2;
  mat4x4 color_model_matrix;
};

vec4 texture(FrameBlender s, vec2 tex_coords) {
  tex_coords = (s.coordinate_transform * vec3(tex_coords, 1)).st;

  vec4 color = vec4(0);
  if (s.blend_factor0 > 0)
    color += texture(s.sampler0, tex_coords) * s.blend_factor0;
  if (s.blend_factor1 > 0)
    color += texture(s.sampler1, tex_coords) * s.blend_factor1;
  if (s.blend_factor2 > 0)
    color += texture(s.sampler2, tex_coords) * s.blend_factor2;

  return transform(color, s.color_model_matrix);
}

vec4 textureLod(FrameBlender s, vec2 tex_coords, float lod) {
  tex_coords = (s.coordinate_transform * vec3(tex_coords, 1)).st;

  vec4 color = vec4(0);
  if (s.blend_factor0 > 0)
    color += textureLod(s.sampler0, tex_coords, lod) * s.blend_factor0;
  if (s.blend_factor1 > 0)
    color += textureLod(s.sampler1, tex_coords, lod) * s.blend_factor1;
  if (s.blend_factor2 > 0)
    color += textureLod(s.sampler2, tex_coords, lod) * s.blend_factor2;

  return transform(color, s.color_model_matrix);
}

//@implements: sampler2D
struct FrameBlender_gray {};

vec4 texture(FrameBlender_gray s, vec2 tex_coords) {
  return vec4(vec3(0.5), 1.0);
}

//@implements: sampler2D
struct FrameBlender_transparent {};

vec4 texture(FrameBlender_transparent s, vec2 tex_coords) {
  return vec4(0.0);
}

//@implements: sampler2D
struct FrameBlender_tiled {
  mat3x3 coordinate_transform;
  sampler2DArray sampler0;
  sampler2DArray sampler1;
  sampler2DArray sampler2;
  float blend_factor0;
  float blend_factor1;
  float blend_factor2;
  mat4x4 color_model_matrix;
  vec2 tiles_scale;
  vec2 tiles_indent;
  int tiles_x;
};

vec4 texture(FrameBlender_tiled s, vec2 tex_coords) {
  tex_coords = clamp(tex_coords, 0, 1);
  tex_coords = (s.coordinate_transform * vec3(tex_coords, 1)).st;
  vec2 coord = tex_coords * s.tiles_scale;
  vec2 tile = floor(coord);
  vec2 tile_coord = coord - tile;
  tile_coord = (-2 * s.tiles_indent + vec2(1)) * tile_coord + s.tiles_indent;

  vec3 tc = vec3(tile_coord, tile.y * s.tiles_x + tile.x);
  vec2 dx = dFdx(coord);
  vec2 dy = dFdy(coord);
  vec4 color = vec4(0);
  if (s.blend_factor0 > 0)
    color += textureGrad(s.sampler0, tc, dx, dy) * s.blend_factor0;
  if (s.blend_factor1 > 0)
    color += textureGrad(s.sampler1, tc, dx, dy) * s.blend_factor1;
  if (s.blend_factor2 > 0)
    color += textureGrad(s.sampler2, tc, dx, dy) * s.blend_factor2;

  return transform(color, s.color_model_matrix);
}

ivec2 textureSize(FrameBlender_tiled s, int level) {
  return ivec2(textureSize(s.sampler0, 0).xy * s.tiles_indent);
}

//@implements: sampler2D
struct FrameBlender_planar {
  mat3x3 coordinate_transform;
  sampler2D sampler0[3];
  sampler2D sampler1[3];
  sampler2D sampler2[3];
  float blend_factor0;
  float blend_factor1;
  float blend_factor2;
  mat4x4 color_model_matrix;
};

vec3 sample_planar(sampler2D sampler[3], vec2 tex_coords) {
  vec3 yuv;
  yuv.x = texture(sampler[0], tex_coords).r;
  yuv.y = texture(sampler[1], tex_coords).r;
  yuv.z = texture(sampler[2], tex_coords).r;
  return yuv;
}

vec3 sample_planar_lod(sampler2D sampler[3], vec2 tex_coords, float lod) {
  vec3 yuv;
  yuv.x = textureLod(sampler[0], tex_coords, lod).r;
  yuv.y = textureLod(sampler[1], tex_coords, lod).r;
  yuv.z = textureLod(sampler[2], tex_coords, lod).r;
  return yuv;
}

vec4 texture(FrameBlender_planar s, vec2 tex_coords) {
  tex_coords = (s.coordinate_transform * vec3(tex_coords, 1)).st;

  vec3 color = vec3(0);
  if (s.blend_factor0 > 0)
    color += sample_planar(s.sampler0, tex_coords) * s.blend_factor0;
  if (s.blend_factor1 > 0)
    color += sample_planar(s.sampler1, tex_coords) * s.blend_factor1;
  if (s.blend_factor2 > 0)
    color += sample_planar(s.sampler2, tex_coords) * s.blend_factor2;

  return transform(vec4(color, 1.0), s.color_model_matrix);
}

vec4 textureLod(FrameBlender_planar s, vec2 tex_coords, float lod) {
  tex_coords = (s.coordinate_transform * vec3(tex_coords, 1)).st;

  vec3 color = vec3(0);
  if (s.blend_factor0 > 0)
    color += sample_planar_lod(s.sampler0, tex_coords, lod) * s.blend_factor0;
  if (s.blend_factor1 > 0)
    color += sample_planar_lod(s.sampler1, tex_coords, lod) * s.blend_factor1;
  if (s.blend_factor2 > 0)
    color += sample_planar_lod(s.sampler2, tex_coords, lod) * s.blend_factor2;

  return transform(vec4(color, 1.0), s.color_model_matrix);
}

//@implements: sampler2D
struct FrameBlender_planar_tiled {
  mat3x3 coordinate_transform;
  sampler2DArray sampler0[3];
  sampler2DArray sampler1[3];
  sampler2DArray sampler2[3];
  float blend_factor0;
  float blend_factor1;
  float blend_factor2;
  mat4x4 color_model_matrix;
  vec2 tiles_scale;
  vec2 tiles_indent;
  int tiles_x;
};

vec3 sample_planar_tiled(sampler2DArray sampler[3], vec3 tex_coords, vec2 dx, vec2 dy) {
  vec3 yuv;
  yuv.x = textureGrad(sampler[0], tex_coords, dx, dy).r;
  yuv.y = textureGrad(sampler[1], tex_coords, dx, dy).r;
  yuv.z = textureGrad(sampler[2], tex_coords, dx, dy).r;
  return yuv;
}

vec4 texture(FrameBlender_planar_tiled s, vec2 tex_coords) {
  tex_coords = clamp(tex_coords, 0, 1);
  tex_coords = (s.coordinate_transform * vec3(tex_coords, 1)).st;
  vec2 coord = tex_coords * s.tiles_scale;
  vec2 tile = floor(coord);
  vec2 tile_coord = coord - tile;
  tile_coord = (-2 * s.tiles_indent + vec2(1)) * tile_coord + s.tiles_indent;

  vec3 tc = vec3(tile_coord, tile.y * s.tiles_x + tile.x);
  vec2 dx = dFdx(coord);
  vec2 dy = dFdy(coord);
  vec3 color = vec3(0);
  if (s.blend_factor0 > 0)
    color += sample_planar_tiled(s.sampler0, tc, dx, dy) * s.blend_factor0;
  if (s.blend_factor1 > 0)
    color += sample_planar_tiled(s.sampler1, tc, dx, dy) * s.blend_factor1;
  if (s.blend_factor2 > 0)
    color += sample_planar_tiled(s.sampler2, tc, dx, dy) * s.blend_factor2;

  return transform(vec4(color, 1.0), s.color_model_matrix);
}

ivec2 textureSize(FrameBlender_planar_tiled s, int level) {
  return ivec2(textureSize(s.sampler0[0], 0).xy * s.tiles_indent);
}

//@implements: sampler2D
struct FrameBlender_planar_alpha {
  mat3x3 coordinate_transform;
  sampler2D sampler0[4];
  sampler2D sampler1[4];
  sampler2D sampler2[4];
  float blend_factor0;
  float blend_factor1;
  float blend_factor2;
  mat4x4 color_model_matrix;
};

vec4 sample_planar_alpha(sampler2D sampler[4], vec2 tex_coords) {
  vec4 yuva;
  yuva.x = texture(sampler[0], tex_coords).r;
  yuva.y = texture(sampler[1], tex_coords).r;
  yuva.z = texture(sampler[2], tex_coords).r;
  yuva.a = texture(sampler[3], tex_coords).r;
  return yuva;
}

vec4 texture(FrameBlender_planar_alpha s, vec2 tex_coords) {
  tex_coords = (s.coordinate_transform * vec3(tex_coords, 1)).st;

  vec4 color = vec4(0);
  if (s.blend_factor0 > 0)
    color += sample_planar_alpha(s.sampler0, tex_coords) * s.blend_factor0;
  if (s.blend_factor1 > 0)
    color += sample_planar_alpha(s.sampler1, tex_coords) * s.blend_factor1;
  if (s.blend_factor2 > 0)
    color += sample_planar_alpha(s.sampler2, tex_coords) * s.blend_factor2;

  return transform(color, s.color_model_matrix);
}

//@implements: sampler2D
struct FrameBlender_planar_alpha_tiled {
  mat3x3 coordinate_transform;
  sampler2DArray sampler0[4];
  sampler2DArray sampler1[4];
  sampler2DArray sampler2[4];
  float blend_factor0;
  float blend_factor1;
  float blend_factor2;
  mat4x4 color_model_matrix;
  vec2 tiles_scale;
  vec2 tiles_indent;
  int tiles_x;
};

vec4 sample_planar_alpha_tiled(sampler2DArray sampler[4], vec3 tex_coords, vec2 dx, vec2 dy) {
  vec4 yuva;
  yuva.x = textureGrad(sampler[0], tex_coords, dx, dy).r;
  yuva.y = textureGrad(sampler[1], tex_coords, dx, dy).r;
  yuva.z = textureGrad(sampler[2], tex_coords, dx, dy).r;
  yuva.a = textureGrad(sampler[3], tex_coords, dx, dy).r;
  return yuva;
}

vec4 texture(FrameBlender_planar_alpha_tiled s, vec2 tex_coords) {
  tex_coords = clamp(tex_coords, 0, 1);
  tex_coords = (s.coordinate_transform * vec3(tex_coords, 1)).st;
  vec2 coord = tex_coords * s.tiles_scale;
  vec2 tile = floor(coord);
  vec2 tile_coord = coord - tile;
  tile_coord = (-2 * s.tiles_indent + vec2(1)) * tile_coord + s.tiles_indent;

  vec3 tc = vec3(tile_coord, tile.y * s.tiles_x + tile.x);
  vec2 dx = dFdx(coord);
  vec2 dy = dFdy(coord);
  vec4 color = vec4(0);
  if (s.blend_factor0 > 0)
    color += sample_planar_alpha_tiled(s.sampler0, tc, dx, dy) * s.blend_factor0;
  if (s.blend_factor1 > 0)
    color += sample_planar_alpha_tiled(s.sampler1, tc, dx, dy) * s.blend_factor1;
  if (s.blend_factor2 > 0)
    color += sample_planar_alpha_tiled(s.sampler2, tc, dx, dy) * s.blend_factor2;

  return transform(color, s.color_model_matrix);
}

ivec2 textureSize(FrameBlender_planar_alpha_tiled s, int level) {
  return ivec2(vec2(1) / s.tiles_indent);
}

//@implements: sampler2D
struct FrameBlender_packed_alpha {
  mat3x3 coordinate_transform;
  sampler2D sampler0[2];
  sampler2D sampler1[2];
  sampler2D sampler2[2];
  float blend_factor0;
  float blend_factor1;
  float blend_factor2;
  mat4x4 color_model_matrix;
};

vec4 sample_packed_alpha(sampler2D sampler[2], vec2 tex_coords) {
  return vec4(
      texture(sampler[0], tex_coords).rgb, texture(sampler[1], tex_coords).r);
}

vec4 texture(FrameBlender_packed_alpha s, vec2 tex_coords) {
  tex_coords = (s.coordinate_transform * vec3(tex_coords, 1)).st;

  vec4 color = vec4(0);
  if (s.blend_factor0 > 0)
    color += sample_packed_alpha(s.sampler0, tex_coords) * s.blend_factor0;
  if (s.blend_factor1 > 0)
    color += sample_packed_alpha(s.sampler1, tex_coords) * s.blend_factor1;
  if (s.blend_factor2 > 0)
    color += sample_packed_alpha(s.sampler2, tex_coords) * s.blend_factor2;

  return transform(color, s.color_model_matrix);
}

//@implements: sampler2D
struct FrameBlender_nv12 {
  mat3x3 coordinate_transform;
  sampler2D sampler0[2];
  sampler2D sampler1[2];
  sampler2D sampler2[2];
  float blend_factor0;
  float blend_factor1;
  float blend_factor2;
  mat4x4 color_model_matrix;
};

vec3 sample_nv12(sampler2D sampler[2], vec2 tex_coords) {
  return vec3(texture(sampler[0], tex_coords).r, texture(sampler[1], tex_coords).rg);
}

vec4 texture(FrameBlender_nv12 s, vec2 tex_coords) {
  tex_coords = (s.coordinate_transform * vec3(tex_coords, 1)).st;

  vec3 color = vec3(0);
  if (s.blend_factor0 > 0)
    color += sample_nv12(s.sampler0, tex_coords) * s.blend_factor0;
  if (s.blend_factor1 > 0)
    color += sample_nv12(s.sampler1, tex_coords) * s.blend_factor1;
  if (s.blend_factor2 > 0)
    color += sample_nv12(s.sampler2, tex_coords) * s.blend_factor2;

  return transform(vec4(color, 1.0), s.color_model_matrix);
}

//@implements: sampler2D
struct FrameBlender_NvYCoCg {
  mat3x3 coordinate_transform;
  sampler2D sampler0;
  sampler2D sampler1;
  sampler2D sampler2;
  float blend_factor0;
  float blend_factor1;
  float blend_factor2;
  mat4x4 color_model_matrix;
};

vec3 sample_NvYCoCg(sampler2D sampler, vec2 tex_coords) {
  vec4 color = texture(sampler, tex_coords);
  float scale = (color.z * (255.0 / 8.0)) + 1.0;
  float Co = (color.x - (0.5 * 256.0 / 255.0)) / scale;
  float Cg = (color.y - (0.5 * 256.0 / 255.0)) / scale;
  float Y = color.w;
  return vec3(Y + Co - Cg, Y + Cg, Y - Co - Cg);
}

vec4 texture(FrameBlender_NvYCoCg s, vec2 tex_coords) {
  tex_coords = (s.coordinate_transform * vec3(tex_coords, 1)).st;

  vec3 color = vec3(0);
  if (s.blend_factor0 > 0)
    color += sample_NvYCoCg(s.sampler0, tex_coords) * s.blend_factor0;
  if (s.blend_factor1 > 0)
    color += sample_NvYCoCg(s.sampler1, tex_coords) * s.blend_factor1;
  if (s.blend_factor2 > 0)
    color += sample_NvYCoCg(s.sampler2, tex_coords) * s.blend_factor2;

  return transform(vec4(color, 1.0), s.color_model_matrix);
}

//@implements: sampler2D
struct FrameBlender_NvYCoCg_alpha {
  mat3x3 coordinate_transform;
  sampler2D sampler0[2];
  sampler2D sampler1[2];
  sampler2D sampler2[2];
  float blend_factor0;
  float blend_factor1;
  float blend_factor2;
  mat4x4 color_model_matrix;
};

vec4 sample_NvYCoCg_alpha(sampler2D sampler[2], vec2 tex_coords) {
  float alpha = texture(sampler[1], tex_coords).r;
  return vec4(sample_NvYCoCg(sampler[0], tex_coords), alpha);
}

vec4 texture(FrameBlender_NvYCoCg_alpha s, vec2 tex_coords) {
  tex_coords = (s.coordinate_transform * vec3(tex_coords, 1)).st;

  vec4 color = vec4(0);
  if (s.blend_factor0 > 0)
    color += sample_NvYCoCg_alpha(s.sampler0, tex_coords) * s.blend_factor0;
  if (s.blend_factor1 > 0)
    color += sample_NvYCoCg_alpha(s.sampler1, tex_coords) * s.blend_factor1;
  if (s.blend_factor2 > 0)
    color += sample_NvYCoCg_alpha(s.sampler2, tex_coords) * s.blend_factor2;

  return transform(color, s.color_model_matrix);
}
