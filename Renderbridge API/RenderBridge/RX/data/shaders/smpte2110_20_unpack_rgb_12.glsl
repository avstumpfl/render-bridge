#version 460

#extension GL_ARB_gpu_shader_int64 : enable

// input  288 x 8 uints
// output 256 x 8 pixels
layout(local_size_x = 32, local_size_y = 8, local_size_z = 1) in;

layout(binding = 0, std430) restrict readonly buffer ssbo_src {
  uint data[];
};

// integer image
layout(binding = 0, rgba16ui) uniform restrict writeonly uimage2D u_destination;

//layout(location = 0) uniform mat4x4 u_color_model_matrix;
layout(location = 1) uniform uint u_src_pitch;

shared uint packed_data[288][gl_WorkGroupSize.y];

uint swap_byte_order(uint value, bool do_swap) {
  return do_swap ? 
    (((value >> 24)) |
    ((value >>  8) & 0x0000ff00) |
    ((value <<  8) & 0x00ff0000) |
    ((value << 24))) : value;
}
#if defined(GL_ARB_gpu_shader_int64) && GL_ARB_gpu_shader_int64
void main() {
  // calculate src data pos
  const uint src_pos = 288 * gl_WorkGroupID.x + (gl_WorkGroupSize.y * gl_WorkGroupID.y + gl_LocalInvocationID.y) * u_src_pitch + gl_LocalInvocationID.x;
  
  // load pixel block into shared memory
  for (uint i = 0; i < 9; ++i)
    packed_data[gl_LocalInvocationID.x + i * 32][gl_LocalInvocationID.y] = swap_byte_order(data[src_pos + i * 32], true);

  memoryBarrierShared();
  barrier();
  
  // calculate ouput position
  const uvec2 out_pos = gl_WorkGroupID.xy * uvec2(256, gl_WorkGroupSize.y) + gl_LocalInvocationID.xy;
  
  // fetch pixel components from shared memory
  // calculate bit position calculate shifts
  for (uint i = 0; i < 8; ++i) {
    const uint id = gl_LocalInvocationID.x + i * 32;
    const uint bit_pos = id * 36;
    
    const uint comp = bit_pos >> 5;
    const uint begin = 28 - ((id & 0x7) << 2); 
    const uint64_t value_0 = uint64_t(packed_data[comp + 0][gl_LocalInvocationID.y]) << 32;
    const uint64_t value_1 = uint64_t(packed_data[comp + 1][gl_LocalInvocationID.y]) <<  0;
    
    const uint64_t value = (value_0 | value_1) >> begin;
    
    // split into components
    // && store image
    const uvec3 color = ((uvec3(value.xxx >> uvec3(24, 12, 0)) & 0xfff) * 0xffff) / 0xfff;
    imageStore(u_destination, ivec2(out_pos + uvec2(i * 32, 0)), uvec4(color.xyz, 0xffff));
  }
}
#else
void main() {
}
#endif