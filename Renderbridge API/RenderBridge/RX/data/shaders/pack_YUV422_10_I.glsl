#version 430

#if !defined(FILTER)
# define FILTER 1
#endif

// 48 x 32 RGB pixels in ==> 32 x 32 packed YUV pixels out
layout(local_size_x = 256, local_size_y = 1, local_size_z = 1) in;


layout(binding = 0, rgb10_a2) uniform readonly image2D u_source;
// ssbo: lines pitch must be padded to 128 bytes ==> 32 pixels
// ssbo: line count must be padded to 32
layout(binding = 0, std430) writeonly  buffer ssbo {
  uint data[];
};

layout(location = 0) uniform mat4x4 u_color_model_matrix;
layout(location = 1) uniform uint u_field_parity;

// TODO: use interger conversion??
vec3 rgb_to_yuv(vec3 rgb) {
  return (u_color_model_matrix * vec4(rgb, 1.0)).rgb;
}

// storage for 24 UYVY pixels per line ==> 32 output pixels
// 12k shared storage
shared float tmp[4][32][24];

void filter_444_422(vec3 c0, vec3 c1, uvec2 pos) {
#if (FILTER)  
  vec2 uv = mix(c0.yz, c1.yz, 0.5);
#else
  vec2 uv = c0.yz;
#endif
  

  tmp[0][pos.y][pos.x] = uv.x;
  tmp[1][pos.y][pos.x] = c0.x;
  tmp[2][pos.y][pos.x] = uv.y;
  tmp[3][pos.y][pos.x] = c1.x;
}

// output stream :
// group 4 pixels (32bit)
// 30         20         10          0 
// ## VVVVVVVVVV YYYYYYYYYY UUUUUUUUUU
// ## YYYYYYYYYY UUUUUUUUUU yyyyyyyyyy
// ## UUUUUUUUUU yyyyyyyyyy VVVVVVVVVV
// ## yyyyyyyyyy VVVVVVVVVV YYYYYYYYYY
vec4 select_components(uvec2 pos) {
  uvec3 s = (pos.x * 3).xxx + uvec3(0, 1, 2);
  
  vec4 tmp = {
    tmp[s.x & 0x3][pos.y][s.x >> 2],
    tmp[s.y & 0x3][pos.y][s.y >> 2],
    tmp[s.z & 0x3][pos.y][s.z >> 2],
    0,
  };
  
  return tmp.xyzw;
}

uint select_components_3(uvec2 pos) {
  uvec4 tmp = uvec4(select_components(pos) * vec4(1023, 1023, 1023, 3));
  return (tmp.x << 0) | (tmp.y << 10) | (tmp.z << 20);
}

ivec2 transform_coords(ivec2 coords) {
  coords.y = imageSize(u_source).y - coords.y - 1 + int(u_field_parity);
  return coords;
}

void main() {
  {
    // decimate here
    // devide work group into  8 x 32 blocks
    const uvec2 input_block_id = gl_WorkGroupID.xy * uvec2(48, 64);
    const uvec2 local_input_id = uvec2(gl_LocalInvocationID.x & 0x7, gl_LocalInvocationID.x >> 3);
    const uvec2 input_id = input_block_id + local_input_id * uvec2(2, 2);

    filter_444_422(
      rgb_to_yuv(imageLoad(u_source, transform_coords(ivec2(input_id) + ivec2( 0, 0))).rgb),
      rgb_to_yuv(imageLoad(u_source, transform_coords(ivec2(input_id) + ivec2( 1, 0))).rgb), local_input_id + uvec2( 0, 0));
    filter_444_422(
      rgb_to_yuv(imageLoad(u_source, transform_coords(ivec2(input_id) + ivec2(16, 0))).rgb),
      rgb_to_yuv(imageLoad(u_source, transform_coords(ivec2(input_id) + ivec2(17, 0))).rgb), local_input_id + uvec2( 8, 0));
    filter_444_422(
      rgb_to_yuv(imageLoad(u_source, transform_coords(ivec2(input_id) + ivec2(32, 0))).rgb),
      rgb_to_yuv(imageLoad(u_source, transform_coords(ivec2(input_id) + ivec2(33, 0))).rgb), local_input_id + uvec2(16, 0));
  }
  // sync
  memoryBarrierShared();
  barrier();
  {
    // devide work group into 32 * 8 blocks
    const uvec2 output_block_id = gl_WorkGroupID.xy * uvec2(32);
    const uvec2 local_output_id = uvec2(gl_LocalInvocationID.x & 0x1F, gl_LocalInvocationID.x >> 5);
    const uvec2 output_id = output_block_id + local_output_id.xy;

    const uint pitch = gl_NumWorkGroups.x * 32;
    
    uvec4 addr = (output_id.yyyy + uvec4(0, 8, 16, 24)) * pitch + output_id.xxxx;
    bvec4 select = lessThan(addr, data.length().xxxx);    
        
    if (select.x)    
      data[addr.x] = select_components_3(local_output_id + uvec2(0,  0));
    if (select.y)    
      data[addr.y] = select_components_3(local_output_id + uvec2(0,  8));
    if (select.z)    
      data[addr.z] = select_components_3(local_output_id + uvec2(0, 16));
    if (select.w)    
      data[addr.w] = select_components_3(local_output_id + uvec2(0, 24));
  }
}
