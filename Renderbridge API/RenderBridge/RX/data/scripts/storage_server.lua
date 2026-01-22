
dofile('data/scripts/common.lua')
dofile('data/scripts/settings.lua')

--host.set_process_priority("lowest")

host.load_module('ModStorage')
storage_server = host.create_node("StorageServer")
storage_server.add_endpoint(host.get_argument('local_interface'), host.get_argument('port'))

host.running = host.idle
