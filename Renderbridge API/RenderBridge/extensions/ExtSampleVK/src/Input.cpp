
#define NOMINMAX
#include "Input.h"
#include "piglit/vk_shader.vert.spv.h"
#include "piglit/vk_shader.frag.spv.h"
#include <format>

namespace rxext::sample_vk {

// based on https://github.com/drywolf/vkgl-test
namespace {
#if defined(_WIN32)
  const auto handle_type = HandleType::OPAQUE_WIN32;
#else
  const auto handle_type = HandleType::OPAQUE_FD;
#endif

  void error(const char* message) {
    throw std::runtime_error(message);
  }
} // namespace

Input::Input(vk_ctx* vk_core, ValueSet settings) 
    : m_settings(std::move(settings)),
      vk_core(*vk_core) {

  m_sampler = add_output_parameter<ParameterTexture>(ParameterNames::sampler);

  m_time = add_parameter<ParameterValue>("time");
  m_time->set_property(PropertyNames::purpose, PurposeNames::TimelineTime);

  m_view_matrix = add_parameter<ParameterMatrix4>("view_matrix");
  m_view_matrix->set_property(PropertyNames::purpose, PurposeNames::ViewMatrix);

  m_projection_matrix = add_parameter<ParameterMatrix4>("projection_matrix");
  m_projection_matrix->set_property(PropertyNames::purpose, PurposeNames::ProjectionMatrix);
}

Input::~Input() {
  destroy_renderer();
}

bool Input::initialize() noexcept try {
  host().log_info(std::format("initializing stream '{}'", 
    m_settings.get(SettingNames::handle)));

  initialize_renderer();

  return true;
}
catch (const std::exception& ex) {
  host().log_error(std::format("initializing input failed: {}", ex.what()));
  return false;
}

void Input::initialize_renderer() {
  const auto resolution_divisor = (m_low_resolution ? 10 : 1);
  const auto w = m_settings.get<int>(SettingNames::resolution_x) / resolution_divisor;
  const auto h = m_settings.get<int>(SettingNames::resolution_y) / resolution_divisor;
  const auto d = 1;
  const auto msaa_samples = 1;
  const auto num_levels = 1;
  const auto num_layers = 1;

  const auto color_tiling = VK_IMAGE_TILING_OPTIMAL;
  const auto depth_tiling = VK_IMAGE_TILING_OPTIMAL;
  const auto color_in_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  const auto color_end_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  const auto depth_in_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  const auto depth_end_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  const auto color_format = VK_FORMAT_R8G8B8A8_UNORM;
  const auto depth_format = VK_FORMAT_D32_SFLOAT_S8_UINT;

  if (!vk_fill_ext_image_props(&vk_core,
      w, h, d,
      msaa_samples,
      num_levels,
      num_layers,
      color_format,
      color_tiling,
      color_in_layout,
      color_end_layout,
      true,
      &vk_color_att.props))
    error("Unsupported color image properties");

  if (!vk_create_ext_image(&vk_core, &vk_color_att.props, &vk_color_att.obj))
    error("Failed to create color image");

  if (!vk_fill_ext_image_props(&vk_core,
      w, h, d,
      msaa_samples,
      num_levels,
      num_layers,
      depth_format,
      depth_tiling,
      depth_in_layout,
      depth_end_layout,
      true,
      &vk_depth_att.props))
    error("Unsupported depth image properties");

  if (!vk_create_ext_image(&vk_core, &vk_depth_att.props, &vk_depth_att.obj))
    error("Failed to create depth image");

  // create renderer
  if (!vk_create_renderer(&vk_core, 
        reinterpret_cast<const char*>(vk_shader_vert), sizeof(vk_shader_vert), 
        reinterpret_cast<const char*>(vk_shader_frag), sizeof(vk_shader_frag), 
        true, false, &vk_color_att, &vk_depth_att, 0, &vk_rnd))
    error("Failed to create Vulkan renderer");

  // create semaphores
  if (!vk_create_semaphores(&vk_core, &vk_sem))
    error("Failed to create semaphores");

  // share target texture with host as parameter "sampler"
  const auto desc = TextureDesc{ 
    .width = static_cast<size_t>(w),
    .height = static_cast<size_t>(h),
    .format = static_cast<Format>(color_format),
    .is_target = true,
    .share_handle = {
      .type = handle_type,
      .handle = reinterpret_cast<Handle>(vk_color_att.obj.mobj.fd),
      .memory_size = vk_color_att.obj.mobj.mem_sz,
      .dedicated = vk_color_att.obj.mobj.dedicated,
    }
  };
  m_sampler->set_texture(host().create_texture(desc));
  m_view_matrix->set_value(linalg::lookat_matrix<double>({ 0, -1, -2 }, { 0, 0, 0}, { 0, 1, 0 }));
  m_projection_matrix->set_value(linalg::perspective_matrix<double>(
    45 * 3.14 / 180, static_cast<double>(w) / h, 0.1, 100));
}

void Input::destroy_renderer() {
  vk_destroy_ext_image(&vk_core, &vk_color_att.obj);
  vk_destroy_ext_image(&vk_core, &vk_depth_att.obj);
  vk_destroy_semaphores(&vk_core, &vk_sem);
  vk_destroy_renderer(&vk_core, &vk_rnd);
}

ValueSet Input::get_state() noexcept {
  auto state = ValueSet();
  const auto desc = m_sampler->texture().desc();
  state.set(StateNames::resolution_x, desc.width);
  state.set(StateNames::resolution_y, desc.height);
  state.set(StateNames::format, get_format_name(desc.format));
  state.set(StateNames::scale_y, -1);
  return state;
}

bool Input::update() noexcept try {

  // simulate resolution change every 3 seconds
  const auto time = m_time->value() / 3.0;
  const auto low_resolution = (static_cast<int>(time) % 2 == 1);
  if (std::exchange(m_low_resolution, low_resolution) != low_resolution) {
    destroy_renderer();
    initialize_renderer();
  }
  return true;
}
catch (const std::exception& ex) {
  host().log_error(std::format("updating input failed: {}", ex.what()));
  return false;
}

SyncDesc Input::before_render() noexcept {
  return {
    SyncStrategy::BinarySemaphore,
    ShareHandle{
      handle_type,
      reinterpret_cast<Handle>(vk_sem.fd_gl_frame_done),
    },
  };
}

RenderResult Input::render() noexcept try {  
  const auto desc = m_sampler->texture().desc();
  const auto w = static_cast<float>(desc.width);
  const auto h = static_cast<float>(desc.height);
  const auto t = static_cast<float>(m_time->value() / 2);
  const auto model = linalg::rotation_matrix<double>({ sin(t), 0, -cos(t), 0 });
  const auto view = m_view_matrix->value();
  const auto projection = m_projection_matrix->value();
  const auto mvp = linalg::aliases::float4x4(mul(projection, mul(view, model)));
  
  auto pc = vk_push_constants{ };
  std::memcpy(&pc, &mvp, sizeof(mvp));  
  struct vk_image_att images[] = { vk_color_att, vk_depth_att };
  float vk_fb_color[4] = { 0.4f, 0.5f, 0.6f, 1.0f };
  vk_clear_color(&vk_core, 0, &vk_rnd, vk_fb_color, 4, &vk_sem,
    true, false, images, ARRAY_SIZE(images), 0, 0, w, h);
  vk_draw(&vk_core, 0, &vk_rnd, vk_fb_color, 4, &vk_sem,
    false, true, images, ARRAY_SIZE(images), &pc, 0, 0, w, h);

  return RenderResult::Succeeded;
}
catch (const std::exception& ex) {
  host().log_error(std::format("rendering input failed: {}", ex.what()));
  return RenderResult::Failed;
}

SyncDesc Input::after_render() noexcept {
  return {
    SyncStrategy::BinarySemaphore,
    ShareHandle{
      handle_type,
      reinterpret_cast<Handle>(vk_sem.fd_vk_frame_ready),
    },
  };
}

} // namespace
