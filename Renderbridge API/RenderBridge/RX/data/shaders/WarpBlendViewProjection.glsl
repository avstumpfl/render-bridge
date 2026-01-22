
#include "WarpBlendBlackLevel.glsl"

//@implements: sampler2D
struct WarpBlendViewProjection {
  sampler2D sampler;
  sampler2D warp_texture;
  sampler2D blend_texture;
  sampler2D black_level_texture;
  bool warp_enabled;
  bool blend_enabled;
  uint black_level_method;
  mat3 warp_transform;
  mat3 blend_transform;
  vec4 black_level_scale;
  mat4 warp_coord_transform;
  mat4 parameters2;
};

vec4 texture(WarpBlendViewProjection s, vec2 tex_coords) {
  vec4 color = vec4(1);
  
  vec2 blend_coords = (s.blend_transform * vec3(gl_FragCoord.xy, 1)).st;
  if (s.blend_enabled) {
    vec3 blend = texture(s.blend_texture, blend_coords).rgb;
    if (s.black_level_method == 0 && blend == vec3(0))
      return vec4(0);
    color.rgb *= blend;
  }
  
  vec2 warp_coords = tex_coords;
  if (s.warp_enabled) {
  	ivec2 texSize = textureSize(s.warp_texture, 0);
  	vec2 texCoordScale = (texSize - 2.0)/ texSize;
  	warp_coords = (warp_coords - 0.5) * texCoordScale + 0.5;
  	
    vec3 world_pos = texture(s.warp_texture, warp_coords).xyz;
    vec4 pos_projected = s.warp_coord_transform * vec4(world_pos, 1);
    
    float distance = dot(vec3(s.parameters2[2].xyz), world_pos) + s.parameters2[2].w;
    if(distance <= 0.0)
    {
      color = vec4(0);
    }
    
    float overscan = max(s.parameters2[1].x, 1.0);
    warp_coords = vec2(pos_projected.x / pos_projected.w / 2.0 / overscan + 0.5, pos_projected.y / pos_projected.w / 2.0 / overscan + 0.5);
    
    color *= smoothstep(0.0, s.parameters2[1].y, warp_coords.x) * (1.0 - smoothstep(1.0 - s.parameters2[1].y, 1.0, warp_coords.x));
    color *= smoothstep(0.0, s.parameters2[1].z, warp_coords.y) * (1.0 - smoothstep(1.0 - s.parameters2[1].z, 1.0, warp_coords.y));
  }

  warp_coords = (s.warp_transform * vec3(warp_coords.st, 1)).st;
  if(warp_coords.x >= 0.0 && warp_coords.x <= 1.0
    && warp_coords.y >= 0.0 && warp_coords.y <= 1.0)
  {
    color *= texture(s.sampler, warp_coords);
  }
  else
  {
    color = vec4(0);
  }
  
  black_level_compensation(color, blend_coords,
    s.black_level_method, s.black_level_texture, s.black_level_scale);

  return color;
}
