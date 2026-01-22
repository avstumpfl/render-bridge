#version 450

in vec4 a_position;
in vec2 a_texcoord;
out vec2 v_texcoord;

//@side_effects
void main_vert() {   
  v_texcoord = a_texcoord;
  gl_Position = a_position;
}
