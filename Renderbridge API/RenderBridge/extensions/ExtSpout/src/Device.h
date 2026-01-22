#pragma once

#include "rxext_client.h"
#include "spout.h"
#include <mutex>
#include <map>

namespace rxext::spout {

class Device : public rxext::StreamDevice {
public:
  explicit Device(ValueSet settings);

  bool initialize() noexcept override;
  string get_property(string_view name) noexcept override;
  vector<ValueSet> enumerate_stream_settings() noexcept override;
  InputStream* create_input_stream(ValueSet settings) noexcept override;
  OutputStream* create_output_stream(ValueSet settings) noexcept override;

private:
  void update_inputs_callchain() noexcept;
  void update_current_inputs();
  std::shared_ptr<d3d11::Device> get_d3d11_device(
    const std::string& adapter_luid);

  std::mutex m_mutex;
  const std::unique_ptr<spoutSenderNames> m_spout;
  vector<ValueSet> m_current_inputs;
  std::map<std::string, std::shared_ptr<d3d11::Device>> m_d3d11_devices;
};

} // namespace
