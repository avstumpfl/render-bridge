#version 450

in vec4 v_position;
in vec2 v_texcoord;
out vec4 o_color;

#if !defined(normal_MISSING)
in vec3 v_normal;
#endif

//@side_effects
void main_frag(sampler2D sampler) {
  o_color = texture(sampler, v_texcoord);
}
