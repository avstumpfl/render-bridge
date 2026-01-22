
EXTENSION_NAME = "ExtNotch"

DEVICE_SETTINGS = {
  filename = [[C:\Test\TestSuite\Notch\ExposedExample_64bit.dfxdll]],
}

STREAM_SETTINGS = {
  handle = 1,  
  --layer_name = [['Basic Exposed Params']],  
  resolution_x = 1920 / 2,
  resolution_y = 1080 / 2,
  use_property_ids = false,
}

SET_STREAM_INPUTS = function(stream)
  clock = host.create_node("SyncClock")
  clock.set_channel(2)  
  eval_clock = host.create_node("EvalClock")
  eval_clock.clock = clock  
  stream.time = eval_clock.value
  
  displacement = host.create_node("Arithmetic")
  displacement.set_op(8)
  displacement.a = eval_clock.value
  stream['Displacement (float)'] = displacement.value
  
  texture = host.create_node("TextureSampler")
  texture.set_filename('data/test/Farbkreis.jpg')
  stream['Input Texture (image)'] = resolve_sampler(texture, 256, 256)  
end

dofile('data/scripts/extension_in.lua')
