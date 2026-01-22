#pragma once

#include "rxext_client.h"

namespace rxext::sample_shader {

class Extension : public rxext::Extension {
public:
  string get_property(string_view name) noexcept override;
  StreamDevice* create_stream_device(ValueSet settings) noexcept override;
};

} // namespace
