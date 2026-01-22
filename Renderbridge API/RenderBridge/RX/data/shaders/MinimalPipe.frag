#version 450

in vec2 v_texcoord;
out vec4 o_color;

//@side_effects
void main_frag(sampler2D sampler) {
  o_color = texture(sampler, v_texcoord);
}
