#version 460

// input 96 x 4 uints
// output 128 x 4 rgb pixels
layout(local_size_x = 32, local_size_y = 4, local_size_z = 1) in;

layout(binding = 0, std430) restrict readonly buffer ssbo_src {
  uint data[];
};

// integer image
layout(binding = 0, rgba8ui) uniform restrict writeonly uimage2D u_destination;

//layout(location = 0) uniform mat4x4 u_color_model_matrix;
layout(location = 1) uniform uint u_src_pitch;

shared uint packed_data[96][gl_WorkGroupSize.y];

uint swap_byte_order(uint value, bool do_swap) {
  return do_swap ? 
    (((value >> 24)) |
      ((value >>  8) & 0x0000ff00) |
      ((value <<  8) & 0x00ff0000) |
      ((value << 24))) : value;
}

uvec3 fetch_components(uint bit_pos) {
  const uint comp_0 = bit_pos >> 5;
  const uint comp_1 = min(comp_0 + 1, 95);
  const uint begin = bit_pos & 0x1f;
  const uint value_0 = packed_data[comp_0][gl_LocalInvocationID.y];
  const uint value_1 = packed_data[comp_1][gl_LocalInvocationID.y];

  // avoid ub when begin == 0
  // begin can not be 32
  const uint value =
    (value_0 >> begin) |
    ((value_1 << 1) << (31 - begin));
  
   return (value.xxx >> uvec3(0, 8, 16)) & uvec3(0xff);
}

void store_data(uint offset) {
  const uint bit_pos = (gl_LocalInvocationID.x + offset) * 24;
  const uvec3 rgb = fetch_components(bit_pos);
  
  const uvec2 outpos = gl_WorkGroupID.xy * uvec2(128, gl_WorkGroupSize.y) + gl_LocalInvocationID.xy + uvec2(offset, 0);
  
  imageStore(u_destination, ivec2(outpos), uvec4(rgb, 255));
}

void main() {
  // calculate src data pos
  const uint src_pos = 96 * gl_WorkGroupID.x + (gl_WorkGroupSize.y * gl_WorkGroupID.y + gl_LocalInvocationID.y) * u_src_pitch + gl_LocalInvocationID.x;
  
  // load pixel block into shared memory
  packed_data[gl_LocalInvocationID.x +  0][gl_LocalInvocationID.y] = swap_byte_order(data[src_pos +  0], false);
  packed_data[gl_LocalInvocationID.x + 32][gl_LocalInvocationID.y] = swap_byte_order(data[src_pos + 32], false);
  packed_data[gl_LocalInvocationID.x + 64][gl_LocalInvocationID.y] = swap_byte_order(data[src_pos + 64], false);

  memoryBarrierShared();
  barrier();
  
  store_data( 0);
  store_data(32);
  store_data(64);
  store_data(96);
}
