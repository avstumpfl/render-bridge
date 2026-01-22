
#include "Device.h"
#include "Input.h"
#include "Output.h"
#include <functional>
#include <array>

namespace rxext::spout {

Device::Device(ValueSet settings)
  : m_spout(std::make_unique<spoutSenderNames>()) {
}

bool Device::initialize() noexcept {
  update_inputs_callchain();
  return true;
}

void Device::update_inputs_callchain() noexcept {
  update_current_inputs();

  host().set_timeout(std::chrono::seconds(1), 
    std::bind(&Device::update_inputs_callchain, this));
}

void Device::update_current_inputs() {
  auto name = std::array<char, 256>{ };
  auto inputs = vector<ValueSet>();
  for (auto i = 0; i < m_spout->GetSenderCount(); i++)
    if (m_spout->GetSender(i, name.data(), static_cast<int>(name.size()))) {
      auto& input = inputs.emplace_back();
      input.set(SettingNames::name, name.data());
      input.set(SettingNames::handle, name.data());
    }

  const auto lock = std::lock_guard(m_mutex);
  if (m_current_inputs != inputs) {
    m_current_inputs = std::move(inputs);
    host().send_event(EventCategory::StreamsChanged);
  }
}

string Device::get_property(string_view name) noexcept {
  if (name == PropertyNames::name)
    return string("Spout");
  return { };
}

vector<ValueSet> Device::enumerate_stream_settings() noexcept {
  const auto lock = std::lock_guard(m_mutex);
  return m_current_inputs;
}

InputStream* Device::create_input_stream(ValueSet settings) noexcept try {
  auto d3d_device = get_d3d11_device(settings.get(SettingNames::adapter_luid));
  return new Input(d3d_device, settings);
}
catch (const std::exception& ex) {
  host().log_error(ex.what());
  return nullptr;
}

OutputStream* Device::create_output_stream(ValueSet settings) noexcept try {
  auto d3d_device = get_d3d11_device(settings.get(SettingNames::adapter_luid));
  return new Output(d3d_device, settings);
}
catch (const std::exception& ex) {
  host().log_error(ex.what());
  return nullptr;
}

std::shared_ptr<d3d11::Device> Device::get_d3d11_device(
    const std::string& adapter_luid) {
  if (adapter_luid.empty())
    throw std::runtime_error("adapter luid not set");
  
  const auto lock = std::lock_guard(m_mutex);
  auto it = m_d3d11_devices.find(adapter_luid);
  if (it == m_d3d11_devices.end())
    it = m_d3d11_devices.emplace(adapter_luid, 
      std::make_shared<d3d11::Device>(adapter_luid)).first;
  return it->second;
}

} // namespace
