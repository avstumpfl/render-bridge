
//@implements: sampler2D
struct WarpBlendDynamicPlanar {
  sampler2D sampler;
  sampler2D blend_texture;
  mat4 parameters;
};

// source: http://alex.vlachos.com/graphics/Alex_Vlachos_Advanced_VR_Rendering_GDC2015.pdf
vec3 ScreenSpaceDither(vec2 vScreenPos, float time) {
  vec3 vDither = vec3(dot(vec2(171.0, 231.0), vScreenPos.xy + time));
  vDither.rgb = fract(vDither.rgb / vec3(103.0, 71.0, 97.0)) - vec3(0.5);
  return (vDither.rgb / 255.0); // * 0.375;
}

vec3 linear(vec3 v)    { return pow(v, vec3(2.2)); }
vec3 nonlinear(vec3 v) { return pow(v, vec3(1.0 / 2.2)); }

vec4 texture(WarpBlendDynamicPlanar s, vec2 tex_coords) {
  vec3 position = v_position.xyz / v_position.w;

  // blend map should contain the projectors' positions, the normals of their
  // frustum planes and values for controlling the blend and limit.
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
    
    float edge_smoothness = blend.r;
    float share = clamp(dist, 0.0, edge_smoothness) / edge_smoothness;
    
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
  
  // planar mapping
  vec3 eye = s.parameters[0].xyz;
  vec3 dir = position - eye;
  if (dot(normalize(dir), normalize(v_normal)) < 0)
    return vec4(0);    
  vec2 warp_coords = dir.xy / 2 + vec2(0.5);
  
  vec4 color = clamp(texture(s.sampler, warp_coords), 0.0, 1.0);
  color.rgb = nonlinear(linear(color.rgb) * own_share);
  color.rgb += ScreenSpaceDither(gl_FragCoord.xy, 0.0);
  
  return color;
}
