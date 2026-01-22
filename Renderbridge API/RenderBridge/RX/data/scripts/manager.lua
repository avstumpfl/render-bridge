
dofile('data/scripts/common.lua')
dofile('data/scripts/settings.lua')

host.set_process_priority("high")

local local_interface = host.get_argument('local_interface')
if local_interface ~= "" then
  manager_settings.local_interface = local_interface
else
  -- spawn engine processes when not started by Pixera
  manager_settings.spawn_service_min = 27103
  manager_settings.spawn_service_max = 27199
end

local port = host.get_argument('port')
if port ~= "" then
  manager_settings.client_listen_port = port
end

host.load_module('ModManager')
local factory = host.create_node('ManagerFactory')
manager = factory.get(settings_to_string(manager_settings))
host.running = manager.refresh

local webserver_args = ""
if local_interface ~= "" then
  webserver_args = "--manager-address " .. local_interface
end
host.spawn("RXWeb", webserver_args)
