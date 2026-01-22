
#define NOMINMAX
#include "Device.h"
#include "Input.h"
#include "piglit/vk_interop_helpers.h"
#include "common/string.h"
#include <format>

namespace rxext::sample_vk {

Device::Device(ValueSet settings) 
  : m_settings(std::move(settings)) {
}

bool Device::initialize() noexcept {
  const auto adapter_luid = common::from_hex_string<uint64_t>(
    m_settings.get(SettingNames::adapter_luid));

  if (!vk_init_ctx_for_rendering(&m_vk_ctx, adapter_luid, true) &&
      !vk_init_ctx_for_rendering(&m_vk_ctx, adapter_luid, false))
    return false;

  if (!vk_load_interop_functions(m_vk_ctx.dev))
    return false;

  return true;
}

Device::~Device() {
  vk_cleanup_ctx(&m_vk_ctx);
}

InputStream* Device::create_input_stream(ValueSet settings) noexcept try {
  return new Input(&m_vk_ctx, std::move(settings));
}
catch (const std::exception& ex) {
  host().log_error(ex.what());
  return nullptr;
}

} // namespace
