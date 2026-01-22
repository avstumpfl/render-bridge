
#include "Extension.h"
#include "Device.h"

namespace rxext::sample_d3d {

bool Extension::initialize() noexcept {
  host().log_info("Hello from the SampleD3D extension!");
  return true;
}
  
string Extension::get_property(string_view name) noexcept { 
  if (name == PropertyNames::name)
    return string("SampleD3D");
  return { }; 
}

StreamDevice* Extension::create_stream_device(ValueSet settings) noexcept try {
  return new Device(std::move(settings));
}
catch (const std::exception& ex) {
  host().log_error(ex.what());
  return nullptr;
}

} // namespace

//-------------------------------------------------------------------------

rxext::ExtensionP* rxext_open() {
  return new rxext::sample_d3d::Extension();
}

void rxext_close(rxext::ExtensionP* extension) {
  delete static_cast<rxext::sample_d3d::Extension*>(extension);
}
