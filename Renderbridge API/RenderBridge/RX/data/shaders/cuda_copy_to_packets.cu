
#define COPY_BLOCK_SIZE 1

using uint32_t = unsigned int;
using uint8_t = unsigned char;

// sizes and offsets in uint32_t steps
extern "C"
__global__ void copy_buffer_to_packets_4(const uint32_t* buffer, uint32_t* packets, size_t buffer_pitch, size_t packet_size, size_t packet_stride, size_t packets_per_line, size_t packet_count) {
  const size_t tid_x = blockIdx.x * blockDim.x + threadIdx.x;
  const size_t tid_y = blockIdx.y * blockDim.y + threadIdx.y * COPY_BLOCK_SIZE;
  
  // calculate packet id
  const size_t packet_id_x = tid_x / packet_size;
  if (packet_id_x >= packets_per_line)
    return;

  const size_t offset_in_packet = tid_x - packet_id_x * packet_size;
  #pragma unroll
  for (size_t i = 0; i < COPY_BLOCK_SIZE; ++i) {
    const size_t id_y = tid_y + i;
    const size_t packet_id = packet_id_x + packets_per_line * id_y;
    if (packet_id >= packet_count)
      return;
    
    uint32_t packet_offset = id_y * buffer_pitch + packet_id_x * packet_size;
    // fetch packet data from buffer
    uint32_t value = buffer[packet_offset + offset_in_packet];
    
    // store packet data
    packets[packet_id * packet_stride + offset_in_packet] = value;
  }
}

extern "C"
__global__ void copy_buffer_to_packets_1(const uint8_t* buffer, uint8_t* packets, size_t buffer_pitch, size_t packet_size, size_t packet_stride, size_t packets_per_line, size_t packet_count) {
  const size_t tid_x = blockIdx.x * blockDim.x + threadIdx.x;
  const size_t tid_y = blockIdx.y * blockDim.y + threadIdx.y * COPY_BLOCK_SIZE;

  // calculate packet id
  const size_t packet_id_x = tid_x / packet_size;
  if (packet_id_x >= packets_per_line)
    return;

  const size_t offset_in_packet = tid_x - packet_id_x * packet_size;
#pragma unroll
  for (size_t i = 0; i < COPY_BLOCK_SIZE; ++i) {
    const size_t id_y = tid_y + i;
    const size_t packet_id = packet_id_x + packets_per_line * id_y;
    if (packet_id >= packet_count)
      return;

    uint32_t packet_offset = id_y * buffer_pitch + packet_id_x * packet_size;
    // fetch packet data from buffer
    uint8_t value = buffer[packet_offset + offset_in_packet];

    // store packet data
    packets[packet_id * packet_stride + offset_in_packet] = value;
  }
}