
#include "Extension.h"

namespace rxext::exttemplate {
  
string Extension::get_property(string_view name) noexcept { 
  if (name == PropertyNames::name)
    return string("Template");
  return { }; 
}

} // namespace

//-------------------------------------------------------------------------

rxext::ExtensionP* rxext_open() {
  return new rxext::exttemplate::Extension();
}

void rxext_close(rxext::ExtensionP* extension) {
  delete static_cast<rxext::exttemplate::Extension*>(extension);
}
