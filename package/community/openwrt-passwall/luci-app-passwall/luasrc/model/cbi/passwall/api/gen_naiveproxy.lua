local api = require "luci.model.cbi.passwall.api.api"
local uci = api.uci
local jsonc = api.jsonc

local var = api.get_args(arg)
local node_section = var["-node"]
if not node_section then
    print("-node 不能为空")
    return
end
local run_type = var["-run_type"]
local local_addr = var["-local_addr"]
local local_port = var["-local_port"]
local server_host = var["-server_host"]
local server_port = var["-server_port"]
local node = uci:get_all("passwall", node_section)

local config = {
    listen = run_type .. "://" .. local_addr .. ":" .. local_port,
    proxy = node.protocol .. "://" .. node.username .. ":" .. node.password .. "@" .. (server_host or node.address) .. ":" .. (server_port or node.port)
}

print(jsonc.stringify(config, 1))
