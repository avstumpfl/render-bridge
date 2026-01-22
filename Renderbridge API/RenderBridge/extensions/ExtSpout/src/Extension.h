#pragma once

#include "rxext_client.h"

namespace rxext::spout {

class Extension : public rxext::Extension {
public:
  string get_property(string_view name) noexcept override;
  vector<ValueSet> enumerate_stream_device_settings() noexcept;
  StreamDevice* create_stream_device(ValueSet settings) noexcept override;
};

} // namespace
