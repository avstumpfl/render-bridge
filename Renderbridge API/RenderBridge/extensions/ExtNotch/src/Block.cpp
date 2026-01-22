
#include "Block.h"
#include "Extension.h"
#include "notch/NotchBlock.h"
#include "d3d11/d3d11.h"
#include "util/get_temp_filename.h"
#include "common/string.h"
#include <filesystem>

namespace rxext::notch {

namespace {
  void log_callback(int message_type, const char* file, int line_no, 
      const char* expression, const char* text) {
	  const auto severity = [&]() {
		  switch (message_type) {
			  case LogMessageType::DebugType_DebugText: return EventSeverity::Verbose;
			  case LogMessageType::DebugType_Info: return EventSeverity::Verbose;
			  case LogMessageType::DebugType_Warning: return EventSeverity::Warning;
			  case LogMessageType::DebugType_Assert: break;
		  }
		  return EventSeverity::Error;
	  }();
	  send_event(severity, EventCategory::Message, common::multibyte_to_utf8(text));
  }
} // namespace

Block::Block(const std::string& filename, const std::string& adapter_luid, bool copy_to_temp) 
  : m_notch_block(std::make_unique<NotchBlock>()),
    m_d3d11_device(std::make_unique<d3d11::Device>(adapter_luid)),
    m_filename(filename) {
  
  if (copy_to_temp)
    create_temporary_file();

  log_verbose(common::format("loading block '%s'", m_filename.c_str()));

  if (!m_notch_block->LoadBlock(m_filename.c_str(), 
        m_d3d11_device->device(), m_d3d11_device->device_context(),
        log_callback)) {
    release();
    throw std::runtime_error("loading block failed");
  }
}

Block::~Block() {
  release();
}

void Block::release() noexcept {
  log_verbose(common::format("unloading block '%s'", m_filename.c_str()));
  m_notch_block.reset();

  if (m_created_temporary_file)
    free_temporary_file();
}

NotchInstance* Block::create_instance() {
  auto instance = m_notch_block->CreateInstance();
  if (!instance)
    throw std::runtime_error("creating block instance failed");
  return instance;
}

void Block::release_instance(NotchInstance* instance) {
  m_notch_block->ReleaseInstance(instance);
}

void Block::create_temporary_file() {
  auto temp_filename = util::get_temp_filename();
  std::filesystem::copy_file(m_filename, temp_filename,
    std::filesystem::copy_options::overwrite_existing);
  m_filename = temp_filename;
  m_created_temporary_file = true;
}

void Block::free_temporary_file() noexcept {
  auto error = std::error_code{ };
  if (m_created_temporary_file)
    std::filesystem::remove(m_filename, error);
  if (error)
    log_verbose(common::format("removing temporary '%s' failed", m_filename.c_str()));
}

} // namespace

