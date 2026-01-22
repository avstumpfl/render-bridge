
//@implements: sampler2D
struct ColorTransform_identity {
  sampler2D sampler;
  float scale_factor;
};

vec4 texture(ColorTransform_identity s, vec2 tex_coords) {
  vec4 color = texture(s.sampler, tex_coords);
  color.rgb *= s.scale_factor;
  return color;
}

ivec2 textureSize(ColorTransform_identity s, int level) {
  return textureSize(s.sampler, level);
}

vec4 textureLod(ColorTransform_identity s, vec2 tex_coords, float lod) {
  vec4 color = textureLod(s.sampler, tex_coords, lod);
  color.rgb *= s.scale_factor;
  return color;
}

//@implements: sampler2D
struct ColorTransform_clut {
  sampler2D sampler;
  sampler3D clut_texture;
  float clut_coord_offset;
  float clut_coord_scale;  
  float clut_indent;
  float scale_factor;
};

vec4 texture(ColorTransform_clut s, vec2 tex_coords) {
  vec4 color = texture(s.sampler, tex_coords);
  
  // lookup = map(0, 1, half_entry_size, 1 - half_entry_size, 
  //              pow(map(clut_minimum, clut_maximum, 0, 1, color), inv_gamma);
  vec3 clut_coords = color.rgb * s.clut_coord_scale + vec3(s.clut_coord_offset);
  clut_coords = sqrt(clut_coords);
  vec3 lookup_coords = clut_coords * (1.0 - 2.0 * s.clut_indent) + s.clut_indent;
  
  color.rgb = texture(s.clut_texture, lookup_coords).rgb;
  color.rgb *= s.scale_factor;
  return color;
}

vec4 textureLod(ColorTransform_clut s, vec2 tex_coords, float lod) {
  vec4 color = textureLod(s.sampler, tex_coords, lod);
  
  // lookup = map(0, 1, half_entry_size, 1 - half_entry_size, 
  //              pow(map(clut_minimum, clut_maximum, 0, 1, color), inv_gamma);
  vec3 clut_coords = color.rgb * s.clut_coord_scale + vec3(s.clut_coord_offset);
  clut_coords = sqrt(clut_coords);
  vec3 lookup_coords = clut_coords * (1.0 - 2.0 * s.clut_indent) + s.clut_indent;
  
  color.rgb = texture(s.clut_texture, lookup_coords).rgb;
  color.rgb *= s.scale_factor;
  return color;
}
