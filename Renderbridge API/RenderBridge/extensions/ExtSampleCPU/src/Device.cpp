
#include "Device.h"
#include "Input.h"
#include "Input2.h"

namespace rxext::sample_cpu {

bool Device::set_property(string_view name, string value) noexcept {
  return false;
}

vector<ValueSet> Device::enumerate_stream_settings() noexcept { 
  auto streams = vector<ValueSet>();

  auto* stream = &streams.emplace_back();
  stream->set(SettingNames::name, "Layer 2");
  stream->set(SettingNames::handle, 2);

  stream = &streams.emplace_back();
  stream->set(SettingNames::name, "Layer 1");
  stream->set(SettingNames::handle, 1);

  return streams;
}

InputStream* Device::create_input_stream(ValueSet settings) noexcept try {
  auto handle = settings.get<int>(SettingNames::handle);
  switch (handle) { 
    case 1: return new Input(settings);
    case 2: return new Input2(settings);
    default:
      throw std::runtime_error("invalid handle");
  }
}
catch (const std::exception& ex) {
  host().log_error(ex.what());
  return nullptr;
}

} // namespace
