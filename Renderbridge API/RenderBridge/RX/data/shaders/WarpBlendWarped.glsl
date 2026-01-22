
#include "WarpBlendBlackLevel.glsl"

//@implements: sampler2D
struct WarpBlendWarped {
  sampler2D sampler;
  sampler2D warp_texture;
  sampler2D blend_texture;
  sampler2D black_level_texture;
  bool warp_enabled;
  bool blend_enabled;
  uint black_level_method;
  mat3 warp_transform;
  vec4 black_level_scale;
  vec2 inv_warp_resolution;
};

vec4 texture(WarpBlendWarped s, vec2 tex_coords) {
  vec4 color = vec4(1);
    
  vec2 warp_coords = tex_coords;
  if (s.warp_enabled) {
    warp_coords = warp_coords * (1.0 - s.inv_warp_resolution) + 0.5 * s.inv_warp_resolution;
    warp_coords = texture(s.warp_texture, warp_coords).st;
  }
  warp_coords = (s.warp_transform * vec3(warp_coords.st, 1)).st;  
    
  vec2 blend_coords = warp_coords;
  if (s.blend_enabled) {
    vec3 blend = texture(s.blend_texture, blend_coords).rgb;
    if (s.black_level_method == 0 && blend == vec3(0))
      return vec4(0);
    color.rgb *= blend;
  }

  if (color.rgb != vec3(0))
    color *= texture(s.sampler, warp_coords);
  
  black_level_compensation(color, blend_coords,
    s.black_level_method, s.black_level_texture, s.black_level_scale);
    
  return color;
}