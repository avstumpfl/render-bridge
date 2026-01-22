#pragma once

#include "rxext_client.h"
#include "Instance.h"
#include <map>
#include <mutex>

namespace rxext::notch {

class Device : public rxext::StreamDevice {
public:
  Device(HostContext& host, ValueSet settings);
  InputStream* create_input_stream(ValueSet settings) noexcept override;

private:
  ValueSet m_settings;
  std::shared_ptr<Block> m_block;
};

} // namespace
