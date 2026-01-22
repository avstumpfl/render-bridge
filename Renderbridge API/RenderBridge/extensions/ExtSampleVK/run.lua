
EXTENSION_NAME = "ExtSampleVK"

DEVICE_SETTINGS = {
  filename = [[C:\Path\to\file]],
}

STREAM_SETTINGS = {
  handle = "handle_0",
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
end

dofile('data/scripts/extension_in.lua')
