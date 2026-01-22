#pragma once

#include "rxext_client.h"

namespace rxext::sample_shader {

class Input : public rxext::InputStream {
public:
  explicit Input(const ValueSet& settings);
  string get_property(string_view name) noexcept override;
  bool update() noexcept override;

private:
  ParameterTexture* m_texture{ };
};

} // namespace
