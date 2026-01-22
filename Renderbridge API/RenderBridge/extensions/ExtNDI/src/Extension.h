#pragma once

#include "rxext_client.h"

namespace rxext::ndi {

class Device;

class Extension : public rxext::Extension {
public:
  bool initialize() noexcept override;
  void shutdown() noexcept override;
  string get_property(string_view name) noexcept override;
  vector<ValueSet> enumerate_stream_device_settings() noexcept override;
  StreamDevice* create_stream_device(ValueSet settings) noexcept override;
};

} // namespace
