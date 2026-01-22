#version 460

// input 32 x 32 uints
// output 128 x 32 rgb pixels
layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(binding = 0, std430) restrict readonly buffer ssbo_src {
  uint data[];
};

// integer image
layout(binding = 0, rgba8ui) uniform restrict writeonly uimage2D u_destination;

//layout(location = 0) uniform mat4x4 u_color_model_matrix;
layout(location = 1) uniform uint u_src_pitch;

shared uint packed_data[gl_WorkGroupSize.x][gl_WorkGroupSize.y];

uint fetch_component(uint pos) {
  const uint shift = (pos & 0x3) << 3;
  const uint value = packed_data[pos >> 2][gl_LocalInvocationID.y];
  
  return (value >> shift) & 0xff;
}

void store_data(uint offset) {
  const uint pos = (gl_LocalInvocationID.x + offset);
  const uint alpha = fetch_component(pos);
  
  const uvec2 outpos = gl_WorkGroupID.xy * uvec2(128, gl_WorkGroupSize.y) + gl_LocalInvocationID.xy + uvec2(offset, 0);
  
  imageStore(u_destination, ivec2(outpos), uvec4(255, 255, 255, alpha));
}

void main() {
  // calculate src data pos
  const uint src_pos = gl_WorkGroupSize.x * gl_WorkGroupID.x + (gl_WorkGroupSize.y * gl_WorkGroupID.y + gl_LocalInvocationID.y) * u_src_pitch + gl_LocalInvocationID.x;
  
  // load pixel block into shared memory
  packed_data[gl_LocalInvocationID.x][gl_LocalInvocationID.y] = data[src_pos];
  
  memoryBarrierShared();
  barrier();
  
  store_data( 0);
  store_data(32);
  store_data(64);
  store_data(96);
}