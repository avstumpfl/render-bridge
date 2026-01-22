
EXTENSION_NAME = "ExtSampleD3D"

DEVICE_SETTINGS = {
  filename = [[C:\Path\to\file]],
}

STREAM_SETTINGS = {
  -- when not set, first handle returned by Device::enumerate_stream_settings is used
  -- handle = "handle_0",

  resolution_x = 1920,
  resolution_y = 1080,
}

SET_STREAM_INPUTS = function(stream)
  -- time
  clock = host.create_node("SyncClock")
  clock.set_channel(2)
  eval_clock = host.create_node("EvalClock")
  eval_clock.clock = clock
  stream.time = eval_clock.value
  
  -- color
  color = host.create_node("Vector4")
  color.set_value(Vector4(1.0, 0.5, 0.25, 1.0))
  stream.color = color.value

  -- texture
  video = create_video_sampler{
    settings = {
      filename = 'data/test/sucher_HD_h264.mp4',
      loop = true,
    }
  }
  stream.texture = resolve_sampler(video, 1280, 720)

  -- text
  stream.set_text(Data("String", "Sample text"))

  -- data
  stream.set_data(Data("Uint8", { 1, 2, 3, 4, 5 }))
end

dofile('data/scripts/extension_in.lua')
