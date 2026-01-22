#pragma once

#include "rxext_client.h"
#include "ndi.h"
#include <mutex>

namespace rxext::ndi {

class Device : public rxext::StreamDevice {
public:
  ~Device() noexcept override;
  bool initialize() noexcept override;
  string get_property(string_view name) noexcept override;
  vector<ValueSet> enumerate_stream_settings() noexcept override;
  InputStream* create_input_stream(ValueSet settings) noexcept override;
  OutputStream* create_output_stream(ValueSet settings) noexcept override;

private:
  void thread_func() noexcept;

  std::thread m_thread;
  std::mutex m_mutex;
  std::condition_variable m_signal;
  bool m_shutdown{ false };
  vector<ValueSet> m_current_inputs;
};

} // namespace
