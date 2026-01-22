
#include "Extension.h"
#include "Device.h"
#include "spout.h"

namespace rxext::spout {
  
string Extension::get_property(string_view name) noexcept { 
  if (name == PropertyNames::name)
    return string("Spout");
  return { }; 
}

vector<ValueSet> Extension::enumerate_stream_device_settings() noexcept { 
  // one device without settings
  return { {} };
}

StreamDevice* Extension::create_stream_device(ValueSet settings) noexcept {
  return new Device(std::move(settings));
}

} // namespace

//-------------------------------------------------------------------------

rxext::ExtensionP* rxext_open() {
  return new rxext::spout::Extension();
}

void rxext_close(rxext::ExtensionP* extension) {
  delete static_cast<rxext::spout::Extension*>(extension);
}
