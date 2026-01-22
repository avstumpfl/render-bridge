
#include "Extension.h"
#include "Device.h"

namespace rxext::sample_cpu {
  
string Extension::get_property(string_view name) noexcept { 
  if (name == PropertyNames::name)
    return string("SampleCPU");
  return { }; 
}

StreamDevice* Extension::create_stream_device(ValueSet settings) noexcept {
  return new Device();
}

} // namespace

//-------------------------------------------------------------------------

rxext::ExtensionP* rxext_open() {
  return new rxext::sample_cpu::Extension();
}

void rxext_close(rxext::ExtensionP* extension) {
  delete static_cast<rxext::sample_cpu::Extension*>(extension);
}
