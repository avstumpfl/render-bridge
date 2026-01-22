
#include "Extension.h"
#include "Device.h"
#include "ndi.h"

namespace rxext::ndi {

bool Extension::initialize() noexcept {
  return NDIlib_initialize();
}

void Extension::shutdown() noexcept {
  NDIlib_destroy();
}

string Extension::get_property(string_view name) noexcept { 
  if (name == PropertyNames::name)
    return string("NDI");
  return { }; 
}

vector<ValueSet> Extension::enumerate_stream_device_settings() noexcept { 
  // one device without settings
  return { {} };
}

StreamDevice* Extension::create_stream_device(ValueSet settings) noexcept {
  return new Device();
}

} // namespace

//-------------------------------------------------------------------------

rxext::ExtensionP* rxext_open() {
  return new rxext::ndi::Extension();
}

void rxext_close(rxext::ExtensionP* extension) {
  delete static_cast<rxext::ndi::Extension*>(extension);
}
