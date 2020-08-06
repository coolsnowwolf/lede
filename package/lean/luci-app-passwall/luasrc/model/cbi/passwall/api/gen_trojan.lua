local ucursor = require"luci.model.uci".cursor()
local json = require "luci.jsonc"
local node_section = arg[1]
local run_type = arg[2]
local local_addr = arg[3]
local local_port = arg[4]
local node = ucursor:get_all("passwall", node_section)

local cipher = "ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-CHACHA20-POLY1305:ECDHE-RSA-CHACHA20-POLY1305:ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-SHA:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES128-SHA:ECDHE-RSA-AES256-SHA:DHE-RSA-AES128-SHA:DHE-RSA-AES256-SHA:AES128-SHA:AES256-SHA:DES-CBC3-SHA"
local cipher13 = "TLS_AES_128_GCM_SHA256:TLS_CHACHA20_POLY1305_SHA256:TLS_AES_256_GCM_SHA384"
local trojan = {
    run_type = run_type,
    local_addr = local_addr,
    local_port = tonumber(local_port),
    remote_addr = node.address,
    remote_port = tonumber(node.port),
    password = {node.password},
    log_level = 1,
    ssl = {
        verify = (node.tls_allowInsecure ~= "1") and true or false,
        verify_hostname = true,
        cert = node.trojan_cert_path,
        cipher = cipher,
        cipher_tls13 = cipher13,
        sni = node.tls_serverName,
        alpn = {"h2", "http/1.1"},
        reuse_session = true,
        session_ticket = (node.tls_sessionTicket == "1") and true or false,
        curves = ""
    },
    udp_timeout = 60,
    mux = (node.mux == "1") and {
        enabled = true,
        concurrency = tonumber(node.mux_concurrency),
        idle_timeout = 60,
        } or nil,
    tcp = {
        no_delay = true,
        keep_alive = true,
        reuse_port = true,
        fast_open = (node.tcp_fast_open == "true") and true or false,
        fast_open_qlen = 20
    }
}
if node.type == "Trojan-Go" then
    trojan.ssl.cipher = node.fingerprint == nil and cipher or (node.fingerprint == "disable" and cipher13 .. ":" .. cipher or "")
    trojan.ssl.cipher_tls13 = node.fingerprint == nil and cipher13 or nil
    trojan.ssl.fingerprint = (node.fingerprint ~= nil and node.fingerprint ~= "disable" ) and node.fingerprint or ""
    trojan.ssl.alpn = node.trojan_transport == 'ws' and {} or {"h2", "http/1.1"}
    if node.stream_security ~= "tls" and node.trojan_transport == "original" then trojan.ssl = nil end
    trojan.transport_plugin = node.stream_security == "none" and node.trojan_transport == "original" and {
        enabled = node.plugin_type ~= nil,
        type = node.plugin_type or "plaintext",
        command = node.plugin_type ~= "plaintext" and node.plugin_cmd or nil,
        option = node.plugin_type ~= "plaintext" and node.plugin_option or nil,
        arg = node.plugin_type ~= "plaintext" and { node.plugin_arg } or nil,
        env = {}
    } or nil
    trojan.websocket = node.trojan_transport and node.trojan_transport:find('ws') and {
        enabled = true,
        path = node.ws_path or "/",
        host = node.ws_host or (node.tls_serverName or node.address)
    } or nil
    trojan.shadowsocks = (node.ss_aead == "1") and {
        enabled = true,
        method = node.ss_aead_method or "aead_aes_128_gcm",
        password = node.ss_aead_pwd or ""
    } or nil
end
print(json.stringify(trojan, 1))
