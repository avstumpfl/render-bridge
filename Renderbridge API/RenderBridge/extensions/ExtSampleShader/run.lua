
EXTENSION_NAME = "ExtSampleShader"

STREAM_SETTINGS = {
  handle = 1,
}

SET_STREAM_INPUTS = function(stream)
  -- time
  clock = host.create_node("SyncClock")
  clock.set_channel(2)  
  eval_clock = host.create_node("EvalClock")
  eval_clock.clock = clock  
  stream.time = eval_clock.value  

  -- texture
  video = create_video_sampler{
    settings = {
      filename = 'data/test/sucher_HD_h264.mp4',
      loop = true,
    }
  }
  stream.texture = resolve_sampler(video, 1280, 720)
end

dofile('data/scripts/extension_in.lua')
