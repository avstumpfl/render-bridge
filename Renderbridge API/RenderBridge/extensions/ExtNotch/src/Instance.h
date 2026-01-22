#pragma once

#include "Block.h"
#include <vector>

class NotchInstance;

namespace d3d11 {
  class Fence;
  class TextureRef;
}

namespace rxext::notch {

class Input;
class ParameterTexture;

class Instance {
public:
  explicit Instance(std::shared_ptr<Block> block);
  Instance(const Instance&) = delete;
  Instance& operator=(const Instance&) = delete;
  ~Instance();
  d3d11::Device& d3d11_device() { return m_block->d3d11_device(); }

  std::vector<std::string> get_layer_names() const;
  std::vector<std::string> get_layer_ids() const;
  NotchInstance& notch_instance() { return *m_notch_instance; }
  void register_input(const Input* input);
  void deregister_input(const Input* input);
  bool is_first_input(const Input* input);
  SyncDesc get_sync_desc();
  RenderResult render(const ParameterTexture& target);

private:
  std::shared_ptr<Block> m_block;
  NotchInstance* m_notch_instance{ };
  std::vector<const Input*> m_inputs;
  std::unique_ptr<d3d11::Fence> m_d3d11_fence;
  uint64_t m_fence_counter{ };
};

} // namespace
