
dofile('data/scripts/common.lua')
dofile('data/scripts/settings.lua')

host.set_process_priority("high")

host.load_module('ModService')
service = host.create_node('Service')

local local_interface = host.get_argument('local_interface')
if local_interface ~= "" then
  service.set_local_interface(local_interface)
end

service.set_port(host.get_argument('port'))
service.set_default_storage_location(file_cache_settings.cache_location)
host.running = service.refresh


local port = tonumber(host.get_argument('port'))
if port == 27110 or port == 27111 then
  host.load_module('ModStorage')
  storage_server = host.create_node("StorageServer")
  storage_server.add_endpoint(local_interface, manager_settings.storage_server_port)
end
