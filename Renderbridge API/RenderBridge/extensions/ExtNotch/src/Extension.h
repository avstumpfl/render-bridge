#pragma once

#include "rxext_client.h"

namespace rxext::notch {
  
void send_event(EventSeverity severity, EventCategory category, string_view message);
void log_verbose(string_view message);

class Extension : public rxext::Extension {
public:
  bool initialize() noexcept override;
  void shutdown() noexcept override;
  string get_property(string_view name) noexcept override;
  StreamDevice* create_stream_device(ValueSet settings) noexcept override;
};

} // namespace
