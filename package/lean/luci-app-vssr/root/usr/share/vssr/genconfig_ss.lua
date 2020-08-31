local ucursor = require 'luci.model.uci'.cursor()
local json = require 'luci.jsonc'
local server_section = arg[1]
local local_port = arg[3]
local host = arg[4]

local server = ucursor:get_all('vssr', server_section)

local ss = {
    server = host,
    server_port = server.server_port,
    local_address = '0.0.0.0',
    local_port = local_port,
    password = server.password,
    timeout = (server.timeout ~= nil) and server.timeout or 60,
    method = server.encrypt_method_ss,
    reuse_port = true,
    fast_open = (server.fast_open == '1') and true or false,
    plugin = (server.plugin ~= nil) and server.plugin or nil,
    plugin_opts = (server.plugin ~= nil) and server.plugin_opts or nil
}
print(json.stringify(ss, 1))
