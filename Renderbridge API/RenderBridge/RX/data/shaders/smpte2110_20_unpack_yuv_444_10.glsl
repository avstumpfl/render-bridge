#version 460

// input 480 x 4 uints
// output 512 x 4 rgb pixels
layout(local_size_x = 32, local_size_y = 4, local_size_z = 1) in;

layout(binding = 0, std430) restrict readonly buffer ssbo_src {
  uint data[];
};

// integer image
layout(binding = 0, rgb10_a2) uniform restrict writeonly image2D u_destination;

layout(location = 0) uniform mat4x4 u_color_model_matrix;
layout(location = 1) uniform uint u_src_pitch;

shared uint packed_data[480][gl_WorkGroupSize.y];

uint swap_byte_order(uint value, bool do_swap) {
  return do_swap ? 
    (((value >> 24)) |
    ((value >>  8) & 0x0000ff00) |
    ((value <<  8) & 0x00ff0000) |
    ((value << 24))) : value;
}

uvec3 fetch_components(uint bit_pos) {
  const uint bit_end = bit_pos + 30;
  const uint comp_0 = bit_pos >> 5;
  const uint comp_1 = min(bit_end >> 5, 480 -1);
  
  const uint shift_0 = bit_end & 0x1f;
  const uint shift_1 = 32 - shift_0;
  
  const uint value_0 = packed_data[comp_0][gl_LocalInvocationID.y];
  const uint value_1 = packed_data[comp_1][gl_LocalInvocationID.y];
  
  const uint value =
    (value_0 << shift_0) | ((value_1 >> shift_1) & ((uint(1) << shift_0) - 1));
  
  
  return (value.xxx >> uvec3(10, 20, 0)) & uvec3(0x3ff);
}

void store_data(uint offset) {
  const uint bit_pos = (gl_LocalInvocationID.x + offset) * 30;
  const vec3 yuv = vec3(fetch_components(bit_pos) / vec3(1023.0));
  // do color_transform here
  const uvec2 outpos = gl_WorkGroupID.xy * uvec2(512, gl_WorkGroupSize.y) + gl_LocalInvocationID.xy + uvec2(offset, 0);
  
  imageStore(u_destination, ivec2(outpos), vec4(yuv, 1.0));
}

void main() {
  // calculate src data pos
  const uint src_pos = 480 * gl_WorkGroupID.x + (gl_WorkGroupSize.y * gl_WorkGroupID.y + gl_LocalInvocationID.y) * u_src_pitch + gl_LocalInvocationID.x;

  // load pixel block into shared memory
  for (uint i = 0; i < 15; ++i)
    packed_data[gl_LocalInvocationID.x + i * 32][gl_LocalInvocationID.y] = swap_byte_order(data[src_pos + i * 32], true);

  memoryBarrierShared();
  barrier();
  
  for (uint i = 0; i < 16; ++i)
    store_data(i * 32);
}