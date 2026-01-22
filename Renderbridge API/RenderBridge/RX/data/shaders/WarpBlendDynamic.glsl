
//@implements: sampler2D
struct WarpBlendDynamic {
  sampler2D sampler;
  sampler2D warp_texture;
  sampler2D blend_texture;
  bool warp_enabled;
  bool blend_enabled;
  mat3 warp_transform;
  mat3 blend_transform;
  vec2 inv_warp_resolution;
};

// source: http://alex.vlachos.com/graphics/Alex_Vlachos_Advanced_VR_Rendering_GDC2015.pdf
vec3 ScreenSpaceDither(vec2 vScreenPos, float time) {
  vec3 vDither = vec3(dot(vec2(171.0, 231.0), vScreenPos.xy + time));
  vDither.rgb = fract(vDither.rgb / vec3(103.0, 71.0, 97.0)) - vec3(0.5);
  return (vDither.rgb / 255.0); // * 0.375;
}

vec3 linear(vec3 v, float gamma)    { return pow(v, vec3(gamma)); }
vec3 nonlinear(vec3 v, float gamma) { return pow(v, vec3(1.0 / gamma)); }

vec4 texture(WarpBlendDynamic s, vec2 tex_coords) {
  float gamma = 0;
  vec3 position = v_position.xyz / v_position.w;

  // blend map should contain the projectors' positions, the normals of their
  // frustum planes and values for controlling the blend and limit/gamma.
  // so width should be 7, height the projector count and format RGB32F.  
  // the last projector is the one currently rendering (first line in top-down image file).
  float own_share = 0;
  float total_share = 0;
  int projectors = textureSize(s.blend_texture, 0).y;
  for (int i = 0; i < projectors; ++i) {
    vec3 projector_position = texelFetch(s.blend_texture, ivec2(0, i), 0).rgb;
    vec3 blend = texelFetch(s.blend_texture, ivec2(5, i), 0).rgb;
    
    // take distance to frustum planes into account
    float dist = 1.0;
    for (int j = 0; j < 4; ++j) {
      vec3 normal = texelFetch(s.blend_texture, ivec2(j + 1, i), 0).rgb;
      dist = min(dist, dot(normal, position - projector_position));
    }
    
    float share = pow(clamp(dist, 0.0, 1.0), blend.r);
    
    vec3 ray = position - projector_position;
    float rayLength = length(ray);
    float angle = 1;
        
#if !defined(normal_MISSING)
    angle = clamp(dot(normalize(v_normal), ray / rayLength), 0, 1);
#endif

    // take angle of incoming light into account
    share *= pow(angle, blend.g);

    // take distance to projector into account
    share /= pow(rayLength, blend.b);
        
    if (i == projectors - 1)
      own_share = share;
      
    // limit angle of contribution
    vec3 limit = texelFetch(s.blend_texture, ivec2(6, i), 0).rgb;
    float limit_angle = limit.r;
    float limit_power = limit.g;
    gamma = limit.b;
    if (angle < limit_angle) {
      float limited = share * pow(angle / limit_angle, limit_power);
      // others try to compensate
      if (i == projectors - 1)
        own_share = limited;
      else
        share = limited;
    }
    
    total_share += share;
  }

  if (own_share <= 0)
    return vec4(0);

  own_share /= total_share;
  
  vec2 warp_coords = tex_coords;
  if (s.warp_enabled) {
    warp_coords = warp_coords * (1.0 - s.inv_warp_resolution) + 0.5 * s.inv_warp_resolution;
    warp_coords = texture(s.warp_texture, warp_coords).st;
  }
  warp_coords = (s.warp_transform * vec3(warp_coords.st, 1)).st;
    
  vec4 color = texture(s.sampler, warp_coords);
  color.rgb = nonlinear(linear(color.rgb, gamma) * own_share, gamma);
  
  color.rgb += ScreenSpaceDither(gl_FragCoord.xy, 0.0);
  
  return color;
}
