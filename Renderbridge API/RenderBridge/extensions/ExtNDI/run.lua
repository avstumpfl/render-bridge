
EXTENSION_NAME = "ExtNdi"

AUDIO_API = 'MME'
AUDIO_DEVICE = 'Logitech G430'

if true then
  -- input
  SELECT_STREAM = function(info)
    return not string.match(info.name, "Remote Connection")
  end

  dofile('data/scripts/extension_in.lua')
else
  -- output
  GRAPHICS_DEVICE_INDEX = 0
  VIDEO_FILE = [[C:\Test\TestSuite\Videos\action2-60fps.mp4]]
  FRAME_RATE = 60
  AUDIO_FILE = [[C:\Test\TestSuite\Audio\Ctrl-Z - Spy vs Spy.mp3]]

  STREAM_SETTINGS = {
    handle = 'Output 1',
    resolution_x = 1920,
    resolution_y = 1080,
    frame_rate = FRAME_RATE,
    sync_group = 0,
  }
  dofile('data/scripts/extension_out.lua')
end
