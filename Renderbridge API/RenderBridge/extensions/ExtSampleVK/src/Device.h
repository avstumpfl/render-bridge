#pragma once

#include "rxext_client.h"
#include "piglit/vk.h"
#include <mutex>

namespace rxext::sample_vk {

class Device : public rxext::StreamDevice {
public:
  explicit Device(ValueSet settings);
  ~Device();
  bool initialize() noexcept override;
  InputStream* create_input_stream(ValueSet settings) noexcept override;

private:
  ValueSet m_settings;
  vk_ctx m_vk_ctx{ };
};

} // namespace
