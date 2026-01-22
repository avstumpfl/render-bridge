#pragma once

#include "rxext_client.h"

namespace rxext::sample_shader {

class Device : public rxext::StreamDevice {
public:
  InputStream* create_input_stream(ValueSet settings) noexcept override;
};

} // namespace
