
dofile('data/scripts/common.lua')
dofile('data/scripts/settings.lua')

host.load_module('ModGraphics')
host.load_module('ModDisplay')
host.load_module('ModAudio')
host.load_module('ModVideo')
host.load_module('ModFrame')
host.load_module('ModRender')
host.load_module('ModService')

g_graphics_manager = host.create_node("GraphicsManager")
g_display_manager = host.create_node("DisplayManager")
g_display_manager.set_show_display_info(true)
host.running = g_display_manager.refresh

g_system_clock = host.create_node("SyncClock")
g_system_clock.set_channel(1) -- channel 1 is the system clock

g_render_devices = {}
g_next_display_index = 100

function initialize_render_device(device_index)
  g_display_manager.initialize_render_devices({ device_index })
  g_render_devices[device_index] =
    g_display_manager.get_render_device(device_index)
end

-- open_predicate(device, output): bool
-- returns list of Displays
function open_displays(open_predicate)
  local graphics_device_indices = {}
  local displays = {}

  for device_index = 0, g_graphics_manager.device_count() - 1 do
    local device = g_graphics_manager.get_device(device_index)  
    
    for output_index = 0, g_graphics_manager.get_output_count(device_index) - 1 do
      local output = g_graphics_manager.get_output(device_index, output_index)
      if output.enabled() then
        context_settings.desktop_x = output.desktop_x()
        context_settings.desktop_y = output.desktop_y()
        break
      end
    end  
    
    display_settings.device_name = device.name()
    display_settings.device_profile = display_settings.device_profile or "high"
  
    g_display_manager.set_device_settings(device_index, 
      settings_to_string(display_settings)) 
  
    for output_index = 0, g_graphics_manager.get_output_count(device_index) - 1 do
      local output = g_graphics_manager.get_output(device_index, output_index)
      if output.enabled() and open_predicate(device, output) then
        display = g_display_manager.create_display(device_index, output.index())
        display.on_closed.attach(g_display_manager.quit)
        display.desktop_x = output.desktop_x
        display.desktop_y = output.desktop_y
        display.resolution_x = output.resolution_x
        display.resolution_y = output.resolution_y
        display.bits_per_component = output.bits_per_component
        display.refresh_rate = output.refresh_rate
        display.capability_state = output.capability_state
        display.device_info = device.info
        display.set_settings(settings_to_string(display_settings))
        display.set_name('Display')
        table.insert(displays, display)
        table.insert(graphics_device_indices, device_index)
        g_render_devices[device_index] =
          g_display_manager.get_render_device(device_index)
      end
    end
  end

  g_display_manager.initialize_render_devices(graphics_device_indices)
  return displays
end

function open_custom_display(args)
  local display_index = g_next_display_index
  g_next_display_index = g_next_display_index + 1
  local display = g_display_manager.create_custom_display(args.device_index or 0, display_index)
  display.set_name(args.name or 'Extension Output')
  display.render = args.render
  display.begin_present_callback = args.stream.begin_present
  display.end_present_callback = args.stream.end_present
  display.swap_callback = args.stream.swap
  
  initialize_render_device(args.device_index)
  return display
end

function set_upload_devices(item)
  for _, render_device in pairs(g_render_devices) do
    item.add_upload_device(render_device)
  end
end

function create_video_sampler(args)
  local video = host.create_node("VideoSource")
  local frame_loader = host.create_node("FrameLoader")
  local frame_blender = host.create_node("FrameBlender")  

  local frame_durtion_set = false
  if args.settings.frame_rate then
    frame_loader.set_frame_duration(Flicks(1 / args.settings.frame_rate))
    frame_durtion_set = true
    args.settings.frame_rate = nil
  end
  video.set_settings(settings_to_string(args.settings))
  video.on_frame_ready.attach(frame_loader.frame_processed)
  set_upload_devices(video)
  frame_loader.set_max_frame_count(16)
  frame_loader.process_callback = video.process
 
  frame_blender.get_frames_callback = frame_loader.get_frames
  frame_blender.set_ranges_callback = frame_loader.set_ranges
  frame_blender.frame_duration = frame_loader.frame_duration
  frame_blender.clock = args.clock or g_system_clock
  
  if not frame_durtion_set then
    -- to detect frame duration
    video.load_first()
  end
  return frame_blender
end

function create_quad_render_pipe(args)
  local mesh = host.create_node("Quad")
  mesh.set_top_down(args.topdown or false)
  local render_pipe_factory = host.create_node("RenderPipeFactory")
  local render_pipe = render_pipe_factory.get(
    'data/shaders/MinimalPipe.vert',
    'data/shaders/MinimalPipe.frag')
  render_pipe.a_position = mesh.positions
  render_pipe.a_texcoord = mesh.texcoords
  render_pipe.sampler = args.sampler
  return render_pipe.render
end

function resolve_sampler(sampler, resolution_x, resolution_y)
  local render = create_quad_render_pipe{ sampler = sampler }
  local render_sampler = host.create_node('RenderSampler')
  render_sampler.render = render
  render_sampler.set_resolution_x(resolution_x)
  render_sampler.set_resolution_y(resolution_y)
  return render_sampler
end

---------------------------------------------------------------------------

g_audio_manager = host.create_node("AudioManager")

if AUDIO_API then
  g_audio_manager.update_devices()
end

-- open_predicate(device, channel): bool
-- returns list of AudioOutputLinks
function create_audio_output_links(open_predicate)
  local links = {}
  for device_index = 0, g_audio_manager.device_count() - 1 do
    local device = g_audio_manager.get_device(device_index)
    for channel_index = 0, g_audio_manager.get_channel_count(device_index) - 1 do
      local channel = g_audio_manager.get_channel(device_index, channel_index)
      if channel.type() == "output" and open_predicate(device, channel) then
        local link = host.create_node("AudioOutputLink")
        link.device_channel = channel
        link.stream_clock = g_system_clock
        table.insert(links, link)
      end
    end
  end
  return links
end

function create_audio_object(args)
  local stream = args.stream
  if not stream then
    stream = host.create_node("AudioSource")
    stream.set_settings(settings_to_string(args.settings))
    stream.start()
  end
  local object = host.create_node("AudioObject")
  object.stream = stream
  object.stream_clock = args.clock or g_system_clock
  return object
end
