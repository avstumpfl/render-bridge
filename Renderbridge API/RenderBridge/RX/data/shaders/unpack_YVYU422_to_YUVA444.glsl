#version 430

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(binding = 0, std430) readonly buffer ssbo {
  uint data[];
};

layout(binding = 0, rgba8ui) uniform writeonly uimage2D u_output;

void main() {
  uvec2 pos = gl_GlobalInvocationID.xy;
  uint address = pos.x / 2 + pos.y * gl_WorkGroupSize.x * gl_NumWorkGroups.x / 2;
  
  if (address < data.length()) {
    uint p = data[address];
    uvec4 color = {
      bitfieldExtract(p, int(pos.x & 1) << 4, 8),
      bitfieldExtract(p, 24, 8),
      bitfieldExtract(p, 8, 8),
      0xFF
    };
    imageStore(u_output, ivec2(pos), color);    
  }
}
