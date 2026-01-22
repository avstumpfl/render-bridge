#version 460

// input 512 x 4 444 pixels
// ouput 480 x 4 uints
layout(local_size_x = 32, local_size_y = 4, local_size_z = 1) in;

layout(binding = 0, std430) restrict writeonly buffer ssbo_dst {
  uint data[];
};

// integer image
layout(binding = 0, rgb10_a2ui) uniform restrict readonly uimage2D u_source;

//layout(location = 0) uniform mat4x4 u_color_model_matrix;
layout(location = 1) uniform uint u_dst_pitch;
//layout(location = 2) uniform uint u_field_parity;

shared uint unpacked_data[512][gl_WorkGroupSize.y];

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

uint pack_rgb_to_uint(uvec3 color) {
  return (color.x << 0) | (color.y << 10) | (color.z << 20);  
}

uvec3 load_data(uvec2 block_pos, uvec2 local_pos) {
  const uvec3 color = imageLoad(u_source, ivec2(transform_coords(block_pos + local_pos, true))).rgb;
  return color.bgr;
} 

void store_data(uint block_pos, uvec2 local_pos) {
  // fetch data
  const uint pixel_id = local_pos.x + local_pos.x / 15;
  const uint data_0 = unpacked_data[pixel_id + 0][local_pos.y];
  const uint data_1 = unpacked_data[pixel_id + 1][local_pos.y];
  
  // calculate shift values
  const uint shift_index = local_pos.x % 15;
  const uint shift_0 =  2 + shift_index * 2;
  const uint shift_1 = 28 - shift_index * 2;
  // store data
  data[block_pos + local_pos.x] = swap_byte_order((data_0 << shift_0) | (data_1 >> shift_1), true);
}

void main() {
  const uvec2 in_block_pos = gl_WorkGroupID.xy * uvec2(512, gl_WorkGroupSize.y) + gl_LocalInvocationID.xy;
  // load pixels
  
  for (uint i = 0; i < 16; ++i)
    unpacked_data[gl_LocalInvocationID.x + i * 32][gl_LocalInvocationID.y] =
      pack_rgb_to_uint(load_data(in_block_pos, uvec2(i * 32, 0)));

  memoryBarrierShared();
  barrier();
  
  // calculate buffer position
  uvec2 out_pos = gl_WorkGroupID.xy * uvec2(480, gl_WorkGroupSize.y);
  uint out_block_pos = out_pos.x + (out_pos.y + gl_LocalInvocationID.y) * u_dst_pitch;
  
  for (uint i = 0; i < 15; ++i)
    store_data(out_block_pos, gl_LocalInvocationID.xy + uvec2(i * 32, 0));
}