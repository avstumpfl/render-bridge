#version 450

// output 32 x 32 rgb pixels
layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

// ensure lines are padded to 4 bytes
layout(binding = 0, std430) readonly buffer ssbo {
  uint data[];
};

// integer image
layout(binding = 0, rgb10_a2ui) uniform writeonly uimage2D u_dst_rgb10_a2;
//layout(binding = 1, rgba8) uniform writeonly image2D u_dst_rgb;

void main() {
  const uvec2 block_pos = gl_WorkGroupID.xy * gl_WorkGroupSize.xy;
  const uvec2 pixel_pos = gl_LocalInvocationID.xy + block_pos; 
  const uint dst_pitch = imageSize(u_dst_rgb10_a2).x;

  const uint inpos = pixel_pos.y * dst_pitch + pixel_pos.x;
  uint comp = 0;
  if (inpos < data.length())
    comp = data[inpos];
  
  uvec3 color = uvec3(
    (comp >> 22) & 0x3FF,
    (comp >> 12) & 0x3FF,
    (comp >> 02) & 0x3FF);
  imageStore(u_dst_rgb10_a2, ivec2(pixel_pos), uvec4(color, 3));
  //imageStore(u_dst_rgb, ivec2(pixel_pos), vec4(color / vec3(1024.0), 1.0));
}
