
#include "Device.h"
#include "Input.h"
#include "Output.h"
#include <functional>

namespace rxext::ndi {

namespace {
  // report all inputs that were visible in the last N seconds,
  // since NDIlib_find_get_current_sources for some reason regularly reports
  // 0 sources (even when running the Test Pattern util on the local machine)
  const auto stream_timeout_seconds = 3;

  vector<ValueSet> get_current_inputs(NDIlib_find_instance_type& find) {
    auto num_sources = uint32_t{ };
    auto sources = NDIlib_find_get_current_sources(&find, &num_sources);
    auto inputs = vector<ValueSet>();
    for (auto i = 0u; i < num_sources; i++) {
      auto& input = inputs.emplace_back();
      input.set(SettingNames::name, sources[i].p_ndi_name);
      // also use stream name as handle (instead of url_address)
      input.set(SettingNames::handle, sources[i].p_ndi_name);
    }
    return inputs;
  }

  vector<ValueSet> merge_inputs(const std::vector<vector<ValueSet>>& recent_inputs) {
    auto merged = vector<ValueSet>{ };
    for (const auto& input : recent_inputs)
      merged.insert(merged.end(), input.begin(), input.end());
    std::sort(merged.begin(), merged.end());
    merged.erase(std::unique(merged.begin(), merged.end()), merged.end());
    return merged;
  }
} // namespace

Device::~Device() {
  auto lock = std::unique_lock(m_mutex);
  m_shutdown = true;
  lock.unlock();
  m_signal.notify_one();
  if (m_thread.joinable())
    m_thread.join();
}

bool Device::initialize() noexcept {
  m_thread = std::thread(&Device::thread_func, this);
  return true;
}

void Device::thread_func() noexcept {
  auto show_local = true;
  auto show_groups = std::string();
  auto show_ips = std::string();
  const auto find_settings = NDIlib_find_create_t{
    show_local,
    (show_groups.empty() ? nullptr : show_groups.c_str()),
    (show_ips.empty() ? nullptr : show_ips.c_str()),
  };
  auto ndi_find = std::shared_ptr<NDIlib_find_instance_type>{
    NDIlib_find_create_v2(&find_settings), NDIlib_find_destroy };
	if (!ndi_find) {
    host().log_error("creating NDI finder failed");
    return;
  }

  // TODO: revert when race is fixed - enumeration also requests current streams
  std::this_thread::sleep_for(std::chrono::seconds(5));

  auto recent_inputs = std::vector<vector<ValueSet>>();
  for (;;) {
    recent_inputs.push_back(get_current_inputs(*ndi_find));
    if (recent_inputs.size() > stream_timeout_seconds)
      recent_inputs.erase(recent_inputs.begin());
    auto inputs = merge_inputs(recent_inputs);

    auto lock = std::unique_lock(m_mutex);
    if (m_current_inputs != inputs) {
      m_current_inputs = std::move(inputs);
      host().send_event(EventCategory::StreamsChanged);
    }
    m_signal.wait_for(lock, std::chrono::seconds(1));
    if (m_shutdown)
      break;
  }
}

string Device::get_property(string_view name) noexcept {
  if (name == PropertyNames::name)
    return string("NDI");
  return { };
}

vector<ValueSet> Device::enumerate_stream_settings() noexcept {
  auto lock = std::lock_guard(m_mutex);
  return m_current_inputs;
}

InputStream* Device::create_input_stream(ValueSet settings) noexcept try {
  return new Input(settings);
}
catch (const std::exception& ex) {
  host().log_error(ex.what());
  return nullptr;
}

OutputStream* Device::create_output_stream(ValueSet settings) noexcept try {
  return new Output(settings);
}
catch (const std::exception& ex) {
  host().log_error(ex.what());
  return nullptr;
}

} // namespace
