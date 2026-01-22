
#include "Instance.h"
#include "Parameter.h"
#include "notch/NotchBlock.h"
#include "d3d11/d3d11.h"
#include "Extension.h"

namespace rxext::notch {

Instance::Instance(std::shared_ptr<Block> block) 
  : m_block(std::move(block)) {

  m_notch_instance = m_block->create_instance();

  m_d3d11_fence = std::make_unique<d3d11::Fence>(
      m_block->d3d11_device());
}

Instance::~Instance() {
  m_block->release_instance(m_notch_instance);
}

std::vector<std::string> Instance::get_layer_names() const {
  const auto unquote = [](std::string string) {
    if (string.size() >= 2 && string.front() == '\'' && string.back() == '\'')
      return string.substr(1, string.size() - 2);
    return string;
  };
  auto result = std::vector<std::string>();
  for (auto layer : m_notch_instance->GetLayers())
    result.push_back(unquote(layer->m_name));
  return result;
}

std::vector<std::string> Instance::get_layer_ids() const {
  auto result = std::vector<std::string>();
  for (auto layer : m_notch_instance->GetLayers())
    result.push_back(layer->m_guid);
  return result;
}

void Instance::register_input(const Input* input) {
  m_inputs.push_back(input);
}

void Instance::deregister_input(const Input* input) {
  auto it = std::find(m_inputs.begin(), m_inputs.end(), input);
  if (it != m_inputs.end())
    m_inputs.erase(it);
}

bool Instance::is_first_input(const Input* input) {
  return (!m_inputs.empty() && m_inputs.front() == input);
}

SyncDesc Instance::get_sync_desc() {
  return {
    SyncStrategy::TimelineSemaphore,
    ShareHandle{
      HandleType::D3D_FENCE,
      m_d3d11_fence->share_handle(),
    },
    m_fence_counter,
  };
}

RenderResult Instance::render(const ParameterTexture& target) {
  // synchronize with end of usage
  m_d3d11_fence->wait(m_block->d3d11_device(), ++m_fence_counter);

  m_block->d3d11_device().clear_target(target.d3d11_render_target_view());
  if (!m_notch_instance->Render(target.d3d11_texture(), target.d3d11_render_target_view()))
    return RenderResult::Failed;

  // signal end of update
  m_d3d11_fence->signal(m_block->d3d11_device(), ++m_fence_counter);

  return RenderResult::Succeeded;
}

} // namespace
