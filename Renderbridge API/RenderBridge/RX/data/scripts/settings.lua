
window_settings = {
  visible = true,
  topmost = true,
  bring_to_front = true,
  hide_task = true,
  hide_mousecursor = true,
  broadcast_messages = "RX",
  --x = 0,
  --y = 0,
  --width = 0,
  --height = 0,
  --resizeable = false,
  --transparent = false,
  --fullscreen = false,
  --open_in_origin = false,
  --restore_focus = 0.0,
}

context_settings = {
  affinity = 'auto',
  --major_version = 4,
  --minor_version = 5,
  --compatibility_profile = false,
  --no_error = false,
  --no_flush_release = false,
  --debug_context = false,
  --verbose_debug_output = false,
  --dump_formats = false,
  compositing = 'off',
  swap_method = 'exchange',
  --doublebuffer = true,
  --stereo = false,
  --color_bits = -1,
  --alpha_bits = -1,
  depth_bits = 24,
  --stencil_bits = -1,
  --multisample_samples = 0,
}

render_settings_high = {
  prime_copy_engine_mb = 30,
  upload_buffer_size_mb = 1024,
  upload_capacity_mb = 64,
  unpack_buffer_size_mb = 256,
  direct_upload_buffer_size_mb = 384,
  direct_upload_capacity_mb = 384/2,
  direct_upload_buffer_parts = 2,

  download_buffer_size_mb = 128,
  download_capacity_mb = 128,
  direct_download_buffer_size_mb = 0,
  direct_download_capacity_mb = 0,
  direct_download_buffer_parts = 0,
}

render_settings_medium = {
  prime_copy_engine_mb = 30,
  upload_buffer_size_mb = 128,
  upload_capacity_mb = 35,
  unpack_buffer_size_mb = 70,
  direct_upload_buffer_size_mb = 128,
  direct_upload_capacity_mb = 35,
  direct_upload_buffer_parts = 2,

  download_buffer_size_mb = 128,
  download_capacity_mb = 128,
  direct_download_buffer_size_mb = 0,
  direct_download_capacity_mb = 0,
  direct_download_buffer_parts = 0,
}

render_settings_low = {
  upload_buffer_size_mb = 0,
  upload_capacity_mb = 0,
  unpack_buffer_size_mb = 0,
  direct_upload_buffer_size_mb = 64,
  direct_upload_capacity_mb = 35,
  direct_upload_buffer_parts = 2,

  download_buffer_size_mb = 0,
  download_capacity_mb = 0,
  direct_download_buffer_size_mb = 32,
  direct_download_capacity_mb = 32,
  direct_download_buffer_parts = 2,
}

render_settings = {
  --flush_transfer_contexts = false,
  --copy_engine_limit_formats = true,
  --copy_engine_threshold_mb = 0.25,
  --upload_block_alignment = 64,
  --direct_max_pending_tasks = 100,
  --vulkan_allocation_size_mb = 0,
  --prime_vulkan_allocator_mb = 0,
  --texture_pool_capacity = 30,
  --buffer_pool_capacity = 0,
  --purge_pools = true,
}

display_settings = {
  window = window_settings,
  context = context_settings,
  render_high = render_settings_high,
  render_medium = render_settings_medium,
  render_low = render_settings_low,
  render = render_settings,

  --device_profile = 'high',
  --window_display_mode = 'standard',

  --bind_swap_barrier = 0,
  --synchronize_compositor = false,
  --join_swap_group = 1,
  --detect_swap_group_support = true,
  --swap_interval = 'auto',

  --synchronize_present = true,
  --swap_budget = -1,

  --display_info_detail = 2,
  --display_info_scale = 1.0,
  --display_info_x = 0.05,
  --display_info_y = 0.05,

  --monitoring = false,
  --disable_clear = false,
}

playback_settings = {
  video_source = {
    --engine = 'undefined',
    --packet_prefetch = 4,
    --thread_count = 0,
    --seek_threshold = 1.0,
    --generate_mipmaps = false,
    --fast = false,
    --video_acceleration = false,
    --video_acceleration_method = 0,
    --prefer_builtin = false,
    --prefer_slice_threading = false,
    --file_buffer_size = 128 * 1024,
    --file_cache_size = 512 * 1024,
    frame_writer = {
      max_tile_size = 16384,
      --bypass_upload = false,
    },
  },
  --video_frame_count = 16,
  --frame_blending_filter_width = 1.5,
  --frame_blending_filter_alpha = 8.0,
  --frame_blending_snapping_speed = 0.05,
  --benchmark = false,
}

manager_settings = {
  display = display_settings,
  playback = playback_settings,

  --local_interface = "0.0.0.0", 
  client_listen_port = 27101,
  storage_server_port = 27102,
  playback_service_port = 27103,
  --spawn_remote_strategy = "",
  --spawn_service_min = 0,
  --spawn_service_max = 0,

  --monitoring_update_interval = 1.0,
  --preload_time = 5.0,

  --persist_state = false,
  --restore_state = false,

  --module_paths = { },
  --refresh_graphics_device_status = false,
  --default_target_frame_format = "RGB16F",
  --default_target_buffer_count = 3,
  default_target_frames_latency = 1,
  --default_target_clut_size = 32,
  --default_allow_video_acceleration = false,
  --default_audio_sample_rate = 0,
  --default_audio_latency = 0,
  max_audio_input_channels = 8,
  --max_audio_output_channels = -1,
  --sync_audio_pitch = true,
  --empty_target_filter_means_all = false,
  --image_deduplication = true,

  --live_trace_max_rows = 10000,
  --live_trace_german_excel = false

  --broadcast_instance_updates = false,
  --broadcast_control_clock_updates = true,
  --check_clock_updates = false,
  --check_timely_object_activation = 2.0,
  --disable_audio_device_enumeration = false,
  --disable_extension_subprocesses = false,
  --disable_time_sync = false,

  --cef_remote_debugging_port = 0,
}

file_cache_settings = { cache_location = [[C:\RX-Cache]] }

dofile_optional('data/scripts/pixera_settings.lua')
dofile_optional('data/scripts/user_settings.lua')

local program_data = os.getenv('PROGRAMDATA')
local home = os.getenv('HOME')
if not program_data and home then
  program_data = home .. "/.config"
end
if program_data then
  local user_settings_file = program_data .. [[/AV Stumpfl/RX/user_settings.lua]]
  if not file_exists(user_settings_file) then
    copy_text_file('data/scripts/user_settings.lua.new', user_settings_file)
  end
  dofile_optional(user_settings_file)
end

dofile_optional('../user_settings.lua')
