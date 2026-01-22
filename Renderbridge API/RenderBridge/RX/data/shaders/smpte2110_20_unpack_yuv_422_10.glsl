#version 460

#extension GL_ARB_gpu_shader_int64 : enable

// input 20 x 8 uints
// output 32 x 8 rgb pixels
layout(local_size_x = 32, local_size_y = 8, local_size_z = 1) in;

layout(binding = 0, std430) restrict readonly buffer ssbo_src {
  uint data[];
};

// integer image
layout(binding = 0, rgb10_a2) uniform restrict writeonly image2D u_destination;

layout(location = 0) uniform mat4x4 u_color_model_matrix;
layout(location = 1) uniform uint u_src_pitch;

shared uint packed_data[gl_WorkGroupSize.x][gl_WorkGroupSize.y];

#if defined(GL_ARB_gpu_shader_int64) && GL_ARB_gpu_shader_int64
uint swap_byte_order(uint value, bool do_swap) {
  return do_swap ? 
    (((value >> 24)) |
    ((value >>  8) & 0x0000ff00) |
    ((value <<  8) & 0x00ff0000) |
    ((value << 24))) : value;
}

ivec4 fetch_components(uint bit_pos) {
  const uint comp = bit_pos >> 5;
  const uint begin = 24 - (bit_pos & 0x1f);
  const uint64_t value_0 = uint64_t(packed_data[comp + 0][gl_LocalInvocationID.y]) << 32;
  const uint64_t value_1 = uint64_t(packed_data[comp + 1][gl_LocalInvocationID.y]);
  
  const uint64_t value = (value_0 | value_1) >> begin;
  
  return (ivec4(uvec4(value.xxxx >> uvec4(20, 30, 10, 0)) & uvec4(0x3ff)));
}

void main() {
  // calculate src data pos
  const uint src_pos = 20 * gl_WorkGroupID.x + (gl_WorkGroupSize.y * gl_WorkGroupID.y + gl_LocalInvocationID.y) * u_src_pitch + gl_LocalInvocationID.x;
  
  const bool do_load = src_pos < data.length() && gl_LocalInvocationID.x < 20;

  // load pixel block into shared memory
  packed_data[gl_LocalInvocationID.x][gl_LocalInvocationID.y] = swap_byte_order(do_load ? data[src_pos] : 0, true);

  memoryBarrierShared();
  barrier();
  
  // fetch pixel components from shared memory
  // calculate bit positions
  const uint group = gl_LocalInvocationID.x >> 1;
  const uint id = gl_LocalInvocationID.x & 0x01;
  const uint bit_pos = group * 40;
  
  const ivec4 components = fetch_components(bit_pos);
  const ivec3 yuv = (id == 0) ? components.xyz : components.wyz;

  // transform color
  vec4 rgb = 
    //u_color_model_matrix *
    vec4(vec3(yuv.xyz) / 1023.0, 1.0);
  
  // store image
  imageStore(u_destination, ivec2(gl_GlobalInvocationID.xy), vec4(rgb.xyz, 1.0));
}
#else
void main() {
}
#endif