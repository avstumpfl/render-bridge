#version 330

out vec2 v_texcoord;

void main() {
  gl_Position = vec4(
    gl_VertexID == 1 ? 3.0 : -1.0, 
		gl_VertexID == 2 ? 3.0 : -1.0,
		0.0,
		1.0);
  v_texcoord = vec2(
    gl_VertexID == 1 ? 2.0 : 0.0, 
		gl_VertexID == 2 ? 2.0 : 0.0);
}
