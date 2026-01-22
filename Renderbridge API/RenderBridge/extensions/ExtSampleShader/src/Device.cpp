
#include "Device.h"
#include "Input.h"

namespace rxext::sample_shader {

InputStream* Device::create_input_stream(ValueSet settings) noexcept try {
  return new Input(settings);
}
catch (const std::exception& ex) {
  host().log_error(ex.what());
  return nullptr;
}

} // namespace
