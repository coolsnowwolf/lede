local ucursor = require "luci.model.uci".cursor()
local json = require "luci.jsonc"
local server_section = arg[1]
local proto = arg[2] 
local local_port = arg[3]
local host = arg[4]

local server = ucursor:get_all("vssr", server_section)

local ssr = {
    server = host,
    server_port = server.server_port,
    local_address = "0.0.0.0",
    local_port = local_port,
    password = server.password,
    timeout =  (server.timeout ~= nil) and server.timeout or 60,
    method = server.encrypt_method,
    protocol = server.protocol,
    protocol_param = server.protocol_param,
    obfs = server.obfs,
    obfs_param = server.obfs_param,
    reuse_port = true,
    fast_open = (server.fast_open == "1") and true or false,
}
print(json.stringify(ssr, 1))
