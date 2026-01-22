#pragma once

#include "rxext_client.h"

namespace rxext::sample_cpu {

class Input : public rxext::InputStream {
public:
  explicit Input(const ValueSet& settings);

  bool initialize() noexcept override;
  ValueSet get_state() noexcept override;

private:
  ParameterTexture& m_sampler;
};

} // namespace
