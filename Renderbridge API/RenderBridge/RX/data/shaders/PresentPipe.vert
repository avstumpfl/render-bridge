#version 450

in vec4 a_position;
in vec2 a_texcoord;
out vec4 v_position;
out vec2 v_texcoord;

#if !defined(normal_MISSING)
in vec3 a_normal;
out vec3 v_normal;
#endif

//@side_effects
void main_vert(
    mat4 world_transform,
    mat4 view_transform,
    mat4 projection_transform,
    bool flip_vertically) {   
  v_position = world_transform * a_position;
  v_texcoord = a_texcoord;
  vec4 position = projection_transform * view_transform * v_position;
  if (flip_vertically)
    position.y *= -1;
  gl_Position = position;
  
#if !defined(normal_MISSING)
  v_normal = (world_transform * vec4(a_normal, 0)).xyz;
#endif  
}
