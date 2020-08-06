local ucursor = require "luci.model.uci".cursor()
local jsonc = require "luci.jsonc"
local node_section = arg[1]
local local_port = arg[2]
local server_host = arg[3]
local server_port = arg[4]
local node = ucursor:get_all("passwall", node_section)

local config = {
    server = server_host or node.address,
    server_port = server_port or tonumber(node.port),
    local_address = "0.0.0.0",
    local_port = tonumber(local_port),
    password = node.password,
    timeout = tonumber(node.timeout),
    fast_open = (node.tcp_fast_open and node.tcp_fast_open == "true") and true or false,
    reuse_port = true
}

if node.type == "SS" then
    config.method = node.ss_encrypt_method
    if node.ss_plugin and node.ss_plugin ~= "none" then
        config.plugin = node.ss_plugin
        config.plugin_opts = node.ss_plugin_opts or nil
    end
elseif node.type == "SSR" then
    config.method = node.ssr_encrypt_method
    config.protocol = node.ssr_protocol
    config.protocol_param = node.protocol_param
    config.obfs = node.obfs
    config.obfs_param = node.obfs_param
end

print(jsonc.stringify(config, 1))
