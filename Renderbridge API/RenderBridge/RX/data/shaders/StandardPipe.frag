#version 450

#define WRITE_GBUFFER

#include <RendererUniforms.h>

in vec4 v_position;
in vec4 v_normal;
in vec2 v_texcoord;
in vec4 v_color;

layout(location = 0) out vec4 o_color;
layout(location = 1) out vec4 o_position;
layout(location = 2) out vec4 o_normal;
layout(location = 3) out vec4 o_emissive;
layout(location = 4) out vec4 o_material;

#if !defined(user0_MISSING)
in vec4 v_user0;
#endif
#if !defined(user1_MISSING)
in vec4 v_user1;
#endif
#if !defined(user2_MISSING)
in vec4 v_user2;
#endif

//@side_effects
void main_frag(sampler2D sampler, float alpha, float premultiply) {
  
#if defined(WRITE_GBUFFER)
  o_position = v_position / v_position.w;
  o_normal = v_normal;
#endif
  
  vec4 color = texture(sampler, v_texcoord);
  color.a = clamp(color.a * u_alpha * alpha, 0, 1);
  color.rgb *= mix(u_alpha * alpha, color.a, premultiply); // Premultiplied Alpha
  o_color = color;
}
