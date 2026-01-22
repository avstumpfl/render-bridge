#version 460

// input 128 x 4 rgb pixels
// ouput 96 x 4 uints
layout(local_size_x = 32, local_size_y = 4, local_size_z = 1) in;

layout(binding = 0, std430) restrict writeonly buffer ssbo_dst {
  uint data[];
};

// integer image
layout(binding = 0, rgba8ui) uniform restrict readonly uimage2D u_source;

//layout(location = 0) uniform mat4x4 u_color_model_matrix;
layout(location = 1) uniform uint u_dst_pitch;
//layout(location = 2) uniform uint u_field_parity;

shared uint unpacked_data[128][gl_WorkGroupSize.y];

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

void load_data(uvec2 block_pos, uvec2 local_pos) {
  uvec4 color = imageLoad(u_source, ivec2(transform_coords(block_pos + local_pos, true)));
  // calculate output value
  unpacked_data[local_pos.x][local_pos.y] =
    (color.b << 16) |
    (color.g <<  8) |
    (color.r <<  0);
} 

void store_data(uint block_pos, uvec2 local_pos) {
  // fetch data
  const uint pixel_id = local_pos.x + local_pos.x / 3;
  const uint data_0 = unpacked_data[pixel_id + 0][local_pos.y];
  const uint data_1 = unpacked_data[pixel_id + 1][local_pos.y];
  
  // calculate shift values
  const uint shift_index = local_pos.x % 3;
  const uint shift_0 =  0 + shift_index * 8;
  const uint shift_1 = 24 - shift_index * 8;
  
  // store data
  data[block_pos + local_pos.x] = swap_byte_order((data_0 >> shift_0) | (data_1 << shift_1), false);
}

void main() {
  const uvec2 in_block_pos = gl_WorkGroupSize.xy * uvec2(4, 1) * gl_WorkGroupID.xy;
  // load pixels
  load_data(in_block_pos, gl_LocalInvocationID.xy + uvec2( 0, 0)); 
  load_data(in_block_pos, gl_LocalInvocationID.xy + uvec2(32, 0)); 
  load_data(in_block_pos, gl_LocalInvocationID.xy + uvec2(64, 0)); 
  load_data(in_block_pos, gl_LocalInvocationID.xy + uvec2(96, 0)); 
  
  memoryBarrierShared();
  barrier();
  
  // calculate buffer position
  uvec2 out_pos = gl_WorkGroupSize.xy * uvec2(3, 1) * gl_WorkGroupID.xy;
  uint out_block_pos = out_pos.x + (out_pos.y + gl_LocalInvocationID.y) * u_dst_pitch;
  store_data(out_block_pos, gl_LocalInvocationID.xy + uvec2( 0, 0));
  store_data(out_block_pos, gl_LocalInvocationID.xy + uvec2(32, 0));
  store_data(out_block_pos, gl_LocalInvocationID.xy + uvec2(64, 0));
}