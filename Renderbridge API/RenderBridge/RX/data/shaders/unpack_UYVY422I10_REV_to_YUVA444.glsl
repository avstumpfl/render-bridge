#version 450

// output 32 x 32 rgb pixels
layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

// ensure lines are padded to 4 bytes
layout(binding = 0, std430) readonly buffer ssbo {
  uint data[];
};

// integer image
layout(binding = 0, rgb10_a2ui) uniform writeonly uimage2D u_dst_rgb10_a2;


// swap yz of c_mask && c_offsets to swap colors
// mask to select component
// YUV
const bvec3 c_mask[6] = {
  bvec3(false, false, false),
  bvec3( true, false, false),
  
  bvec3(false,  true, false),
  bvec3( true,  true, false),
  
  bvec3( true,  true, false),
  bvec3( true,  true, false),
};

const ivec3 c_offsets[6] = {
  ivec3( 10, 20,  0), //y0u0v0
  ivec3(  0, 20,  0), //y1u0v0
  
  ivec3( 20,  0, 10), //y2u1v1
  ivec3( 10,  0, 10), //y3u1v1
  
  ivec3(  0, 10, 20), //y4u2v2
  ivec3( 20, 10, 20), //y5u2v2
};

uint pack_id(uint lane_id) {
  return lane_id % 6;
}

bvec3 pack_mask(uint pid) {
  return c_mask[pid];
}

ivec3 pack_offsets(uint pid) {
  return c_offsets[pid];
}

uint dst_to_src(uint dst) {
  return (dst >> 1) + dst / 6;
}

uvec2 pack_pos(uvec2 pos) {
  return uvec2(
    dst_to_src(pos.x),
    pos.y);
}

uvec2 pack_address(uvec2 pos, uint src_pitch) {
  const uvec2 ppos = pack_pos(pos);
  return min((ppos.x + ppos.y * src_pitch).xx + uvec2(0, 1), data.length() - 1);
}

uvec2 fetch_packed_components(uvec2 pos, uint src_pitch, ivec3 src_pos) {
  const uvec2 pack_addr = pack_address(pos, src_pitch);
  return uvec2(
    data[pack_addr.x],
    data[pack_addr.y]
  );
}

uint src_pitch(uint dst_pitch) {
  return dst_to_src(dst_pitch);
}

uvec3 unpack_yuv(uvec2 pack, bvec3 mask, ivec3 offsets) {
  uvec3 tmp = mix(uvec3(pack.x), uvec3(pack.y), mask);
  return uvec3(
    bitfieldExtract(tmp.x, offsets.x, 10),
    bitfieldExtract(tmp.y, offsets.y, 10),
    bitfieldExtract(tmp.z, offsets.z, 10));
}

uint align_down(uint v, uint a) {
  return v - (v % a);
}
uint align_up(uint v, uint a) {
  return align_down(v + a - 1, a);
}

void main() {
  const uvec2 block_pos = gl_WorkGroupID.xy * gl_WorkGroupSize.xy;
  const uvec2 pixel_pos = gl_LocalInvocationID.xy + block_pos; 
  const uint dst_pitch = imageSize(u_dst_rgb10_a2).x;
  const uint src_pitch = (align_up(dst_pitch, 48) * 4) / 6;

  const uvec2 comp = fetch_packed_components(pixel_pos, src_pitch, ivec3(20, 10, 0));
  const uint pid = pack_id(pixel_pos.x); 
  const bvec3 mask = pack_mask(pid);
  const ivec3 offsets = pack_offsets(pid);

  const uvec3 yuv = unpack_yuv(comp, mask, offsets);
  imageStore(u_dst_rgb10_a2, ivec2(pixel_pos), uvec4(yuv.xzy, 3));
}