
--RUN_IN_SUBPROCESS = RUN_IN_SUBPROCESS or true

dofile('data/scripts/standalone.lua')

local opened_displays = 0
local graphics_device_index = 0
displays = open_displays(function(device, output)
  if opened_displays < 1 then
    graphics_device_index = device.index()
    opened_displays = opened_displays + 1
    return true
  end
end)

local opened_streams = 0
links = create_audio_output_links(function(device, channel)
  if device.api() == AUDIO_API and
      string.find(device.name(), AUDIO_DEVICE) and
      opened_streams < 2 then
    opened_streams = opened_streams + 1
    return true
  end  
end)

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

device_settings = DEVICE_SETTINGS or { }
stream_settings = STREAM_SETTINGS or { }
stream_settings.id = 1
stream_settings.is_input = true

-- evaluate in next graph update so adapter_luid is available
set_timeout(function()
  local adapter_luid = g_display_manager.get_adapter_luid(graphics_device_index)
  device_settings.adapter_luid = adapter_luid
  
  if EXTENSION_DEVICE_INDEX then
    device_manager.configure_device(EXTENSION_DEVICE_INDEX, settings_to_string(device_settings), { }, { })
    device = device_manager.get_device(EXTENSION_DEVICE_INDEX)  
  else
    device = device_manager.create_device(settings_to_string(device_settings))
  end

  -- find stream when none was specified
  while not stream_settings.handle do
    local streams = device.get_dynamic_streams()
    for i = 1, #streams do
      local info = string_to_settings(streams[i])
      if not SELECT_STREAM or SELECT_STREAM(info) then
        stream_settings.handle = info.handle
        break
      end
    end
    sleep(1)
  end

  stream_settings.adapter_luid = adapter_luid
  device.configure_streams({ settings_to_string(stream_settings) })
  stream = device.get_stream(stream_settings.id)
  set_upload_devices(stream)
  
  if SET_STREAM_INPUTS then SET_STREAM_INPUTS(stream) end
  
  for _, display in ipairs(displays) do
    display.render = create_quad_render_pipe{
      sampler = stream
    }
  end
  
  audio = create_audio_object{
    stream = stream.audio_stream
  }
  for index, link in ipairs(links) do
    link.stream = audio.stream
    link.set_stream_channel((index - 1) % 2)
  end
end, 0.1)
