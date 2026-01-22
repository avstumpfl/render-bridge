#version 430

#if !defined(FILTER)
# define FILTER 1
#endif

// 96 x 8 RGB pixels in ==> 96 x 8 packed YUV pixels out
layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;


layout(binding = 0, rgba8ui) uniform readonly uimage2D u_source;
// ssbo: lines pitch must be padded to 128 bytes ==> 32 pixels
// ssbo: line count must be padded to 32
layout(binding = 0, std430) writeonly  buffer ssbo {
  uint data[];
};

layout(location = 0) uniform mat4x4 u_color_model_matrix;

shared uint tmp[32][32];

uint pack_rgb(uvec4 color) {
  return
    color.r << 16 |
    color.g <<  8 |
    color.b;
}

uint load_packed(ivec2 pos) {
  ivec2 t0 = (pos * ivec2(4, 1)) / ivec2(3, 1);
  ivec2 t1 = t0 + ivec2(1, 0);
  uint c0 = pack_rgb(imageLoad(u_source, t0));
  uint c1 = pack_rgb(imageLoad(u_source, t1));
  uint s0;
  uint s1;
  return (c0 >> s0) | (c1 << s1);
}


void main() {
  const uvec2 block_id = gl_WorkGroupID.xy * uvec2(32, 32);
  const uvec2 local_id = gl_LocalInvocationID.xy;
  const uvec2 global_id = block_id + local_id;

  tmp[local_id.y][local_id.x] = pack_rgb(imageLoad(u_source, ivec2(global_id) + ivec2( 0,  0)));
  
  // sync
  memoryBarrierShared();
  barrier();
  
  const uint pitch = gl_NumWorkGroups.x * 32;

  uint addr = global_id.y * pitch + global_id.x;
  if (addr < data.length())
    data[addr] = tmp[local_id.y][local_id.x];
}
