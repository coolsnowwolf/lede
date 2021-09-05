local api = require "luci.model.cbi.passwall.api.api"
local uci = api.uci
local json = api.jsonc

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
local loglevel = var["-loglevel"] or 2
local node = uci:get_all("passwall", node_section)

local cipher = "ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-CHACHA20-POLY1305:ECDHE-RSA-CHACHA20-POLY1305:ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-SHA:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES128-SHA:ECDHE-RSA-AES256-SHA:DHE-RSA-AES128-SHA:DHE-RSA-AES256-SHA:AES128-SHA:AES256-SHA:DES-CBC3-SHA"
local cipher13 = "TLS_AES_128_GCM_SHA256:TLS_CHACHA20_POLY1305_SHA256:TLS_AES_256_GCM_SHA384"
local trojan = {
    run_type = run_type,
    local_addr = local_addr,
    local_port = tonumber(local_port),
    remote_addr = server_host or node.address,
    remote_port = tonumber(server_port) or tonumber(node.port),
    password = {node.password},
    log_level = tonumber(loglevel),
    ssl = {
        verify = (node.tls_allowInsecure ~= "1") and true or false,
        verify_hostname = true,
        cert = nil,
        cipher = cipher,
        cipher_tls13 = cipher13,
        sni = node.tls_serverName or node.address,
        alpn = {"h2", "http/1.1"},
        reuse_session = true,
        session_ticket = (node.tls_sessionTicket and node.tls_sessionTicket == "1") and true or false,
        curves = ""
    },
    udp_timeout = 60,
    tcp = {
        use_tproxy = (node.type == "Trojan-Plus" and var["-use_tproxy"]) and true or nil,
        no_delay = true,
        keep_alive = true,
        reuse_port = true,
        fast_open = (node.tcp_fast_open == "true") and true or false,
        fast_open_qlen = 20
    }
}
if node.type == "Trojan-Go" then
    trojan.ssl.cipher = nil
    trojan.ssl.cipher_tls13 = nil
    trojan.ssl.fingerprint = (node.fingerprint ~= "disable") and node.fingerprint or ""
    trojan.ssl.alpn = (node.trojan_transport == 'ws') and {} or {"h2", "http/1.1"}
    if node.tls ~= "1" and node.trojan_transport == "original" then trojan.ssl = nil end
    trojan.transport_plugin = ((not node.tls or node.tls ~= "1") and node.trojan_transport == "original") and {
        enabled = node.plugin_type ~= nil,
        type = node.plugin_type or "plaintext",
        command = node.plugin_type ~= "plaintext" and node.plugin_cmd or nil,
        option = node.plugin_type ~= "plaintext" and node.plugin_option or nil,
        arg = node.plugin_type ~= "plaintext" and { node.plugin_arg } or nil,
        env = {}
    } or nil
    trojan.websocket = (node.trojan_transport == 'ws') and {
        enabled = true,
        path = node.ws_path or "/",
        host = node.ws_host or (node.tls_serverName or node.address)
    } or nil
    trojan.shadowsocks = (node.ss_aead == "1") and {
        enabled = true,
        method = node.ss_aead_method or "aes_128_gcm",
        password = node.ss_aead_pwd or ""
    } or nil
    trojan.mux = (node.smux == "1") and {
        enabled = true,
        concurrency = tonumber(node.mux_concurrency),
        idle_timeout = tonumber(node.smux_idle_timeout)
    } or nil
end
print(json.stringify(trojan, 1))
