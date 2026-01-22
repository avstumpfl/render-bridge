#version 460

#extension GL_ARB_gpu_shader_int64 : enable

// input  256 x 8 pixels
// output 288 x 8 uints
layout(local_size_x = 32, local_size_y = 8, local_size_z = 1) in;

layout(binding = 0, std430) restrict writeonly buffer ssbo_dst {
  uint data[];
};

// integer image
layout(binding = 0, rgba16ui) uniform restrict readonly uimage2D u_source;

//layout(location = 0) uniform mat4x4 u_color_model_matrix;
layout(location = 1) uniform uint u_dst_pitch;

shared uint64_t packed_data[256][gl_WorkGroupSize.y];

uint swap_byte_order(uint value, bool do_swap) {
  return do_swap ? 
    (((value >> 24)) |
      ((value >>  8) & 0x0000ff00) |
      ((value <<  8) & 0x00ff0000) |
      ((value << 24))) : value;
}

uvec2 transform_coords(uvec2 coords, bool flip) {
  if (flip)
    coords.y = imageSize(u_source).y - coords.y - 1;
  return coords;
}

uint64_t pack_rgb_to_uint(u64vec3 color) {
  return (color.x << 0) | (color.y << 12) | (color.z << 24);  
}

uvec3 load_data(uvec2 block_pos, uvec2 local_pos) {
  const uvec3 color = imageLoad(u_source, ivec2(transform_coords(block_pos + local_pos, true))).bgr >> 4;
  return color;
}

void store_data(uint block_pos, uvec2 local_pos) {
  const uint pixel_id_1 = local_pos.x - local_pos.x / 9;
  const uint pixel_id_0 = (pixel_id_1 == 0) ? 0 : pixel_id_1 - 1;
  
  const uint64_t data_1 = packed_data[pixel_id_1][local_pos.y];
  const uint64_t data_0 = packed_data[pixel_id_0][local_pos.y];
  
  const uint shift_index = (local_pos.x % 9) << 2;
  const uint shift_0 = 32 - shift_index;
  const uint shift_1 =  4 + shift_index;
  
  data[block_pos + local_pos.x] = swap_byte_order(uint((data_0 << shift_0) | (data_1 >> shift_1)), true);
}

void main() {
  const uvec2 in_block_pos = gl_WorkGroupID.xy * uvec2(256, gl_WorkGroupSize.y) + gl_LocalInvocationID.xy;
  
  // load pixels
  for (uint i = 0; i < 8; ++i)
    packed_data[gl_LocalInvocationID.x + i * 32][gl_LocalInvocationID.y] =
      pack_rgb_to_uint(load_data(in_block_pos, uvec2(i * 32, 0)));
      
  memoryBarrierShared();
  barrier();
  
  // calculate buffer position
  uvec2 out_pos = gl_WorkGroupID.xy * uvec2(288, gl_WorkGroupSize.y);
  uint out_block_pos = out_pos.x + (out_pos.y + gl_LocalInvocationID.y) * u_dst_pitch;
  for (uint i = 0; i < 9; ++i)
    store_data(out_block_pos, gl_LocalInvocationID.xy + uvec2(i * 32, 0));
}