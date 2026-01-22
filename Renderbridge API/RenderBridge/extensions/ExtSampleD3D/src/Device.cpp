
#include "Device.h"
#include "Input.h"
#include <format>

namespace rxext::sample_d3d {

Device::Device(ValueSet settings) 
  : m_settings(std::move(settings)) {
}

bool Device::initialize() noexcept {
  m_d3d11_device = std::make_shared<d3d11::Device>(
    m_settings.get(SettingNames::adapter_luid));

  const auto filename = m_settings.get(SettingNames::filename);
  host().log_info(std::format("loading file '{}'", filename));
  return true;
}

vector<ValueSet> Device::enumerate_stream_settings() noexcept {
  // return identification of streams found in file
  auto streams = vector<ValueSet>();
  for (auto i = 0; i < 3; ++i) {
    auto& stream = streams.emplace_back();
    stream.set(SettingNames::name, std::format("Stream {}", i));
    stream.set(SettingNames::handle, std::format("handle_{}", i));
    stream.set(SettingNames::resolution_x, 1920);
    stream.set(SettingNames::resolution_y, 1080);

    auto settings_desc = SettingsDescBuilder();
    settings_desc.add_int(SettingNames::resolution_x, "Resolution X", 128, 8192);
    settings_desc.add_int(SettingNames::resolution_y, "Resolution Y", 128, 8192);
    settings_desc.add_bool("red_channel_only", "Red channel only");
    settings_desc.add_double("unused_double", "Unused double", 0.0, 1.0);
    settings_desc.add_string("unused_string", "Unused string");
    settings_desc.add_enum("unused_enum", "Unused enum", { "Option 1", "Option 2", "Option 3" });
    stream.set(SettingNames::settings_desc, settings_desc.str());
  }
  return streams;
}

InputStream* Device::create_input_stream(ValueSet settings) noexcept try {
  return new Input(m_d3d11_device, std::move(settings));
}
catch (const std::exception& ex) {
  host().log_error(ex.what());
  return nullptr;
}

} // namespace 
