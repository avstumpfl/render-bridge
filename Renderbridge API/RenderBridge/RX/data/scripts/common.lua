
function settings_to_string(settings)
  if type(settings) ~= 'table' then error("missing settings", 2) end
  local str = ""
  for k, v in pairs(settings) do
    if type(v) == 'table' then
      str = str .. k .. ' = {' .. settings_to_string(v) .. '}, '
    else
      str = str .. k .. " = '" .. tostring(v) .. "', "
    end
  end
  return str
end

function string_to_settings(str)
  local func = load("return {" .. string.gsub(str, '\\', '\\\\') .. "}")
  if not func then error("invalid settings format", 2) end
  return func()
end

function file_exists(filename)
  local f = io.open(filename, "r")
  if f ~= nil then
    io.close(f)
    return true
  end
  return false
end

function dofile_optional(filename)
  if file_exists(filename) then
    return pcall(function() dofile(filename) end)
  end
  return false
end

function copy_text_file(source_filename, dest_filename)
  local succeeded = false
  local s = io.open(source_filename, "r")
  if s ~= nil then
    local d = io.open(dest_filename, "w")
    if d ~= nil then
      while true do
        local line = s:read("*L")
        if not line then break end
        d:write(line)
      end
      d:close()
      succeeded = true
    end
    s:close()
  end
  return succeeded
end

function create_file(filename)
  local succeeded = false
  local file = io.open(filename, "a")
  if file ~= nil then
    file:close()
    succeeded = true
  end
  return succeeded
end

function read_text_file(filename)
  local file = io.open(filename, "r")
  local tmp = ""
  if file ~= nil then
    tmp = file:read("*all")
    file:close()
  end  
  return tmp
end
