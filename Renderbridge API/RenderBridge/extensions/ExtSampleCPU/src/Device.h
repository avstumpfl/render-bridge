#pragma once

#include "rxext_client.h"

namespace rxext::sample_cpu {

class Device : public rxext::StreamDevice {
public:
  bool set_property(string_view name, string value) noexcept override;
  InputStream* create_input_stream(ValueSet settings) noexcept override;
  vector<ValueSet> enumerate_stream_settings() noexcept override;
};

} // namespace
