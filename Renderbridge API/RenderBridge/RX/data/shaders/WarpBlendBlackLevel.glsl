
vec3 linear(vec3 v)    { return pow(v, vec3(2.2)); }
vec3 nonlinear(vec3 v) { return pow(v, vec3(1.0 / 2.2)); }

void black_level_compensation(inout vec4 color, vec2 blend_coords,
    uint method, sampler2D black_level_texture, vec4 scale) {
  switch (method) {
    case 1: {
      color = clamp(color, 0.0, 1.0);
      vec3 black = linear(texture(black_level_texture, blend_coords, 0).rgb);
      color.rgb = nonlinear(linear(color.rgb) * (vec3(1) - black) + black);      
    }
    break;
  
    case 2: {
      vec3 tmp = scale.x * texture(black_level_texture, blend_coords).rgb;
      color.rgb = clamp(scale.y * tmp + (vec3(1) - scale.z * tmp) * color.rgb, tmp, vec3(1.0));
    }
    break;
  }
}
