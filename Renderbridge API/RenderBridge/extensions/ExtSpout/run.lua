
EXTENSION_NAME = "ExtSpout"

if true then
  -- input
  dofile('data/scripts/extension_in.lua')
else
  -- output
  GRAPHICS_DEVICE_INDEX = 0
  VIDEO_FILE = [[C:\Test\TestSuite\Videos\action2-60fps.mp4]]
  FRAME_RATE = 30

  STREAM_SETTINGS = {
    handle = 'Output 1',
    resolution_x = 1920,
    resolution_y = 1080,
    format = 'R16G16B16A16_SFLOAT',
  }

  if false then
    -- synchronize to output
    STREAM_SETTINGS.sync_group = 0
  else
    -- output with defined frame rate
    STREAM_SETTINGS.frame_rate = FRAME_RATE
    STREAM_SETTINGS.sync_group = -1
  end

  dofile('data/scripts/extension_out.lua')
end

