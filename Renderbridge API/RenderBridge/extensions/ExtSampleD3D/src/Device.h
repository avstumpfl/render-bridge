#pragma once

#include "rxext_client.h"
#include "d3d11/d3d11.h"
#include <mutex>

namespace rxext::sample_d3d {

class Device : public rxext::StreamDevice {
public:
  explicit Device(ValueSet settings);
  bool initialize() noexcept override;
  vector<ValueSet> enumerate_stream_settings() noexcept override;
  InputStream* create_input_stream(ValueSet settings) noexcept override;

private:
  ValueSet m_settings;
  std::shared_ptr<d3d11::Device> m_d3d11_device;
};

} // namespace
