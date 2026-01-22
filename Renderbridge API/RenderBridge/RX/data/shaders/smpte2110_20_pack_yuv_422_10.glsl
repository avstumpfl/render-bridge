#version 460

#extension GL_ARB_gpu_shader_int64 : enable

#if !defined(FILTER)
# define FILTER 1
#endif

// input 256 x 8 444 pixels
// ouput 160 x 8 uints
layout(local_size_x = 32, local_size_y = 8, local_size_z = 1) in;

layout(binding = 0, std430) restrict writeonly buffer ssbo_dst {
  uint data[];
};

// integer image
layout(binding = 0, rgb10_a2) uniform restrict readonly image2D u_source;

layout(location = 0) uniform mat4x4 u_color_model_matrix;
layout(location = 1) uniform uint u_dst_pitch;
//layout(location = 2) uniform uint u_field_parity;

shared uint64_t packed_data[128][gl_WorkGroupSize.y];

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

vec3 rgb_to_yuv(vec3 rgb) {
  return (u_color_model_matrix * vec4(rgb, 1.0)).rgb;
}

vec4 filter_444_to_422(vec3 c0, vec3 c1) {
#if (FILTER)  
  vec2 uv = mix(c0.yz, c1.yz, 0.5);
#else
  vec2 uv = c0.yz;
#endif
  return vec4(uv.x, c0.x, uv.y, c1.x);
}

uint64_t pack_422_to_uint64(vec4 color) {
  color = max(vec4(0.0), min(vec4(1.0), color));
  u64vec4 tmp = u64vec4(color * vec4(1023, 1023, 1023, 1023));
  tmp &= uvec4(0x3ff);
  tmp <<= uvec4(30, 20, 10, 0);
  return tmp.x | tmp.y | tmp.z | tmp.w;
}

vec3 load_data(uvec2 block_pos, uvec2 local_pos) {
  const vec3 color = imageLoad(u_source, ivec2(transform_coords(block_pos + local_pos, true))).rgb;
  return color;
} 

void store_data(uint block_pos, uvec2 local_pos) {
  // fetch data
  const uint pixel_id = ((local_pos.x + 1) * 4) / 5;
  const uint pixel_id_0 = (pixel_id == 0) ? 0 : pixel_id - 1;
  const uint pixel_id_1 = min(pixel_id, 127);
  
  const uint64_t data_0 = packed_data[pixel_id_0][local_pos.y];
  const uint64_t data_1 = packed_data[pixel_id_1][local_pos.y];
  

  // calculate shift values
  const uint shift_index = local_pos.x % 5 + 1;
  const uint shift_0 = 40 - shift_index * 8;
  const uint shift_1 =  0 + shift_index * 8;
  
  // store data
  data[block_pos + local_pos.x] = swap_byte_order(uint((data_0 << shift_0) | (data_1 >> shift_1)), true);
}

void main() {
  const uvec2 in_block_pos = gl_WorkGroupSize.xy * uvec2(8, 1) * gl_WorkGroupID.xy + gl_LocalInvocationID.xy * uvec2(2, 1);
  // load pixels
  packed_data[gl_LocalInvocationID.x +  0][gl_LocalInvocationID.y] =
    pack_422_to_uint64(
      filter_444_to_422(
        rgb_to_yuv(load_data(in_block_pos, uvec2(  0, 0))),
        rgb_to_yuv(load_data(in_block_pos, uvec2(  1, 0)))));
  packed_data[gl_LocalInvocationID.x + 32][gl_LocalInvocationID.y] =
    pack_422_to_uint64(
      filter_444_to_422(
        rgb_to_yuv(load_data(in_block_pos, uvec2( 64, 0))),
        rgb_to_yuv(load_data(in_block_pos, uvec2( 65, 0)))));
  packed_data[gl_LocalInvocationID.x + 64][gl_LocalInvocationID.y] =
    pack_422_to_uint64(
      filter_444_to_422(
        rgb_to_yuv(load_data(in_block_pos, uvec2(128, 0))),
        rgb_to_yuv(load_data(in_block_pos, uvec2(129, 0)))));
  packed_data[gl_LocalInvocationID.x + 96][gl_LocalInvocationID.y] =
    pack_422_to_uint64(
      filter_444_to_422(
        rgb_to_yuv(load_data(in_block_pos, uvec2(192, 0))),
        rgb_to_yuv(load_data(in_block_pos, uvec2(193, 0)))));

  memoryBarrierShared();
  barrier();
  
  // calculate buffer position
  uvec2 out_pos = gl_WorkGroupSize.xy * uvec2(5, 1) * gl_WorkGroupID.xy;
  uint out_block_pos = out_pos.x + (out_pos.y + gl_LocalInvocationID.y) * u_dst_pitch;
  store_data(out_block_pos, gl_LocalInvocationID.xy + uvec2(  0, 0));
  store_data(out_block_pos, gl_LocalInvocationID.xy + uvec2( 32, 0));
  store_data(out_block_pos, gl_LocalInvocationID.xy + uvec2( 64, 0));
  store_data(out_block_pos, gl_LocalInvocationID.xy + uvec2( 96, 0));
  store_data(out_block_pos, gl_LocalInvocationID.xy + uvec2(128, 0));
}