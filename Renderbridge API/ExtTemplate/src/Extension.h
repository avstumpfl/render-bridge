#pragma once

#include "rxext_client.h"

namespace rxext::exttemplate {

class Extension : public rxext::Extension {
public:
  string get_property(string_view name) noexcept override;
};

} // namespace
