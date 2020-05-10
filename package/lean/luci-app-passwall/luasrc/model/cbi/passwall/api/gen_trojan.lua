local ucursor = require"luci.model.uci".cursor()
local json = require "luci.jsonc"
local node_section = arg[1]
local run_type = arg[2]
local local_addr = arg[3]
local local_port = arg[4]
local node = ucursor:get_all("passwall", node_section)

local trojan = {
    run_type = run_type,
    local_addr = local_addr,
    local_port = local_port,
    remote_addr = node.address,
    remote_port = tonumber(node.port),
    password = {node.password},
    log_level = 1,
    ssl = {
        verify = (node.trojan_verify_cert == "1") and true or false,
        verify_hostname = true,
        cert = node.trojan_cert_path,
        cipher = "ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-CHACHA20-POLY1305:ECDHE-RSA-CHACHA20-POLY1305:ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-SHA:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES128-SHA:ECDHE-RSA-AES256-SHA:DHE-RSA-AES128-SHA:DHE-RSA-AES256-SHA:AES128-SHA:AES256-SHA:DES-CBC3-SHA",
        cipher_tls13 = "TLS_AES_128_GCM_SHA256:TLS_CHACHA20_POLY1305_SHA256:TLS_AES_256_GCM_SHA384",
        sni = node.tls_serverName,
        alpn = {"h2", "http/1.1"},
        reuse_session = true,
        session_ticket = false,
        curves = ""
    },
    tcp = {
        no_delay = true,
        keep_alive = true,
        reuse_port = true,
        fast_open = (node.tcp_fast_open == "true") and true or false,
        fast_open_qlen = 20
    }
}
print(json.stringify(trojan, 1))
