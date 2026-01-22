#pragma once

#include "rxext_client.h"

class NotchBlock;
class NotchInstance;
namespace d3d11 {
  class Device;
}

namespace rxext::notch {

class Block {
public:
  Block(const std::string& filename, const std::string& adapter_luid, bool copy_to_temp);
  Block(const Block&) = delete;
  Block& operator=(const Block&) = delete;
  ~Block();

  NotchInstance* create_instance();
  void release_instance(NotchInstance* instance);
  d3d11::Device& d3d11_device() { return *m_d3d11_device; }

private:
  void release() noexcept;
  void create_temporary_file();
  void free_temporary_file() noexcept;

  std::unique_ptr<NotchBlock> m_notch_block;

  // only one D3D device can be associated with a block
  // the device is already required for creating the instance/reading the property list.
  // creating one device per block, since loading a block can take minutes
  // and would block the device exclusively.
  std::unique_ptr<d3d11::Device> m_d3d11_device;

  std::string m_filename;
  bool m_created_temporary_file{ };
};

} // namespace
