
layout(std140, binding = 0)
uniform RendererUniforms {
  mat4 u_view_matrix;
  mat4 u_projection_matrix;
  mat4 u_clip_matrix;
  mat4 u_view_parameters_matrix;
  vec4 u_view_position;
  vec4 u_clip_planes[4];
  vec2 u_target_size;
  bool u_use_clip_planes;
  uint u_view_id;
  uint u_target_id;
  float u_alpha;
  float u_sync_time;
};
