local api = require "luci.model.cbi.passwall.api.api"
local uci = api.uci
local jsonc = api.jsonc

local var = api.get_args(arg)
local node_section = var["-node"]
if not node_section then
    print("-node 不能为空")
    return
end
local local_tcp_redir_port = var["-local_tcp_redir_port"]
local local_udp_redir_port = var["-local_udp_redir_port"]
local local_socks_address = var["-local_socks_address"] or "0.0.0.0"
local local_socks_port = var["-local_socks_port"]
local local_socks_username = var["-local_socks_username"]
local local_socks_password = var["-local_socks_password"]
local local_http_address = var["-local_http_address"] or "0.0.0.0"
local local_http_port = var["-local_http_port"]
local local_http_username = var["-local_http_username"]
local local_http_password = var["-local_http_password"]
local node = uci:get_all("passwall", node_section)

local config = {
    server = node.address .. ":" .. node.port,
    obfs = node.hysteria_obfs,
    auth = (node.hysteria_auth_type == "base64") and node.hysteria_auth_password or nil,
    auth_str = (node.hysteria_auth_type == "string") and node.hysteria_auth_password or nil,
    server_name = node.tls_serverName,
    insecure = (node.tls_allowInsecure == "1") and true or false,
    up_mbps = tonumber(node.hysteria_up_mbps) or 10,
    down_mbps = tonumber(node.hysteria_down_mbps) or 50,
    socks5 = (local_socks_address and local_socks_port) and {
        listen = local_socks_address .. ":" .. local_socks_port,
        timeout = 300,
        disable_udp = false,
        user = (local_socks_username and local_socks_password) and local_socks_username,
        password = (local_socks_username and local_socks_password) and local_socks_password,
    } or nil,
    http = (local_http_address and local_http_port) and {
        listen = local_http_address .. ":" .. local_http_port,
        timeout = 300,
        disable_udp = false,
        user = (local_http_username and local_http_password) and local_http_username,
        password = (local_http_username and local_http_password) and local_http_password,
    } or nil,
    tproxy_tcp = (local_tcp_redir_port) and {
        listen = "0.0.0.0:" .. local_tcp_redir_port,
        timeout = 300
    } or nil,
    tproxy_udp = (local_udp_redir_port) and {
        listen = "0.0.0.0:" .. local_udp_redir_port,
        timeout = 60
    } or nil
}

print(jsonc.stringify(config, 1))
