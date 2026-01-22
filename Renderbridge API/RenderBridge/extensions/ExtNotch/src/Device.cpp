#pragma once

#include "Device.h"
#include "Input.h"
#include "common/string.h"

namespace rxext::notch {

Device::Device(HostContext& host, ValueSet settings) 
  : m_settings(std::move(settings)) {
  m_block = std::make_shared<notch::Block>(
    std::string(host.resolve_storage_filename(m_settings.get(SettingNames::filename))),
    m_settings.get(SettingNames::adapter_luid),
    m_settings.get<bool>(SettingNames::preview));
}

InputStream* Device::create_input_stream(ValueSet settings) noexcept try {
  host().log_verbose(common::format("created instance '%s'", 
    m_settings.get(SettingNames::filename).c_str()));
  return new Input(std::make_shared<notch::Instance>(m_block), std::move(settings));
}
catch (const std::exception& ex) {
  host().log_error(ex.what());
  return nullptr;
}

} // namespace
