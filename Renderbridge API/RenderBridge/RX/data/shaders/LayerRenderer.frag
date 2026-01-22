#version 430

layout(location=0) uniform sampler2D u_texture;
layout(location=1) uniform float u_alpha;

in vec2 v_texcoord;
out vec4 o_color;

void main() {
  vec4 color = texture(u_texture, v_texcoord);
  color.a = clamp(color.a, 0, 1) * u_alpha;
  o_color = color;
}
