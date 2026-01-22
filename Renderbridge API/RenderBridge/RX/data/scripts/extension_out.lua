
--RUN_IN_SUBPROCESS = RUN_IN_SUBPROCESS or true

dofile('data/scripts/standalone.lua')

host.load_module("ModExtension")
extension_manager = host.create_node("ExtensionManager")

if RUN_IN_SUBPROCESS then
  SUB_EXTENSION_NAME = EXTENSION_NAME
  EXTENSION_NAME = "ExtProcess"
end
extension = extension_manager.load_extension(EXTENSION_NAME, "")
if RUN_IN_SUBPROCESS then
  if not extension.set_property("extension", SUB_EXTENSION_NAME) then
    error('loading extension failed')
  end
end

device_manager = extension.stream_device_manager()

graphics_device_index = GRAPHICS_DEVICE_INDEX or 0
device_settings = DEVICE_SETTINGS or { }
stream_settings = STREAM_SETTINGS or { }
stream_settings.id = 1

initialize_render_device(graphics_device_index)

-- evaluate in next graph update so adapter_luid is available
set_timeout(function()
  local adapter_luid = g_display_manager.get_adapter_luid(graphics_device_index)
  device_settings.adapter_luid = adapter_luid
  stream_settings.adapter_luid = adapter_luid
  
  if EXTENSION_DEVICE_INDEX then
    device_manager.configure_device(EXTENSION_DEVICE_INDEX, settings_to_string(device_settings), { }, { })
    device = device_manager.get_device(EXTENSION_DEVICE_INDEX)  
  else
    device = device_manager.create_device(settings_to_string(device_settings))
  end
  
  device.configure_streams({ settings_to_string(stream_settings) })
  
  stream = device.get_stream(stream_settings.id)
  
  display = open_custom_display{
    device_index = graphics_device_index,
    stream = stream,
  }
  display.render = create_quad_render_pipe{
    sampler = create_video_sampler{
      settings = {
        filename = VIDEO_FILE,
        frame_rate = FRAME_RATE,
        loop = true
      }
    }
  }
  
  if AUDIO_FILE then
    audio = create_audio_object{
      settings = {
        filename = AUDIO_FILE,
        loop = true
      }
    }
    stream.audio_stream = audio.stream
  end
end, 1)