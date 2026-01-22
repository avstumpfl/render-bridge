
#include "Extension.h"
#include "Device.h"

namespace rxext::notch {

HostContext g_log_host_context;

void send_event(EventSeverity severity, EventCategory category, string_view message) {
  if (g_log_host_context)
    g_log_host_context.send_event(severity, category, message);
}

void log_verbose(string_view message) {
  if (g_log_host_context)
    g_log_host_context.log_verbose(message);
}

bool Extension::initialize() noexcept {
  g_log_host_context = host();
  return true;
}

void Extension::shutdown() noexcept {
  g_log_host_context = { };
}

string Extension::get_property(string_view name) noexcept { 
  if (name == PropertyNames::name)
    return string("Notch");
  return { }; 
}

StreamDevice* Extension::create_stream_device(ValueSet settings) noexcept try {
  return new Device(host(), std::move(settings));
}
catch (const std::exception& ex) {
  host().log_error(ex.what());
  return nullptr;
}

} // namespace

//-------------------------------------------------------------------------

rxext::ExtensionP* rxext_open() {
  return new rxext::notch::Extension();
}

void rxext_close(rxext::ExtensionP* extension) {
  delete static_cast<rxext::notch::Extension*>(extension);
}
