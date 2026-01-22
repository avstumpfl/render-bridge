#version 460

// input 128 x 32 alpha pixels
// output 32 x 32 uints
layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(binding = 0, std430) restrict writeonly buffer ssbo_dst {
  uint data[];
};

// integer image
layout(binding = 0, rgba8ui) uniform restrict readonly uimage2D u_source;

//layout(location = 0) uniform mat4x4 u_color_model_matrix;
layout(location = 1) uniform uint u_dst_pitch;
//layout(location = 2) uniform uint u_field_parity;

#if 1
shared uint unpacked_data[32 + 1][4][gl_WorkGroupSize.y];
#else
shared uint unpacked_data[32][4][gl_WorkGroupSize.y];
#endif

uvec2 transform_coords(uvec2 coords, bool flip) {
  if (flip)
    coords.y = imageSize(u_source).y - coords.y - 1;
  return coords;
}

void load_data(uvec2 block_pos, uvec2 local_pos) {
  uint alpha = imageLoad(u_source, ivec2(transform_coords(block_pos + local_pos, true))).a;
  // calculate position in shared memory
  uint x = local_pos.x >> 2;
  uint y = local_pos.x & 3;

  unpacked_data[x][y][local_pos.y] = alpha;
} 

void store_data(uint block_pos, uvec2 local_pos) {
  // fetch data
  uint data_0 = unpacked_data[local_pos.x][0][local_pos.y];
  uint data_1 = unpacked_data[local_pos.x][1][local_pos.y];
  uint data_2 = unpacked_data[local_pos.x][2][local_pos.y];
  uint data_3 = unpacked_data[local_pos.x][3][local_pos.y];
  
  // store data
  data[block_pos + local_pos.x] =
    (data_0 <<  0) |
    (data_1 <<  8) |
    (data_2 << 16) |
    (data_3 << 24);
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
  uvec2 out_pos = gl_WorkGroupSize.xy * gl_WorkGroupID.xy;
  uint out_block_pos = out_pos.x + (out_pos.y + gl_LocalInvocationID.y) * u_dst_pitch;
  store_data(out_block_pos, gl_LocalInvocationID.xy);
}