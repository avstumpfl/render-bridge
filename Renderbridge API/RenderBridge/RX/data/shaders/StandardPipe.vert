#version 450

#include <RendererUniforms.h>

in vec4 a_position;
in vec4 a_normal;
in vec2 a_texcoord;
in vec4 a_color;
out vec4 v_position;
out vec4 v_normal;
out vec2 v_texcoord;
out vec4 v_color;

#if !defined(pointsize_MISSING)
in float a_pointsize;
#endif
#if !defined(user0_MISSING)
in vec4 a_user0;
out vec4 v_user0;
#endif
#if !defined(user1_MISSING)
in vec4 a_user1;
out vec4 v_user1;
#endif
#if !defined(user2_MISSING)
in vec4 a_user2;
out vec4 v_user2;
#endif

//@side_effects
void main_vert(mat4 model_matrix) {
  vec4 position = model_matrix * a_position;
  v_position = position;
  v_normal = model_matrix * a_normal;
  v_texcoord = a_texcoord;
  v_color = a_color;

  position = u_clip_matrix * u_projection_matrix * u_view_matrix * position;
  
  if (u_use_clip_planes) {
    gl_ClipDistance[0] = dot(position, u_clip_planes[0]);
    gl_ClipDistance[1] = dot(position, u_clip_planes[1]);
    gl_ClipDistance[2] = dot(position, u_clip_planes[2]);
    gl_ClipDistance[3] = dot(position, u_clip_planes[3]);
  }
  
  gl_Position = position;

#if !defined(pointsize_MISSING)
  gl_PointSize = a_pointsize;
#endif
#if !defined(user0_MISSING)
  v_user0 = a_user0;
#endif
#if !defined(user1_MISSING)
  v_user1 = a_user1;
#endif
#if !defined(user2_MISSING)
  v_user2 = a_user2;
#endif
}
