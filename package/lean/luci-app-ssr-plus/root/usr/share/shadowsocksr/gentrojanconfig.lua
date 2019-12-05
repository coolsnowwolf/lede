local ucursor = require "luci.model.uci".cursor()
local json = require "luci.jsonc"
local server_section = arg[1]
local proto = arg[2] 
local usr_dns = arg[3]
local usr_port = arg[4]
local local_port = arg[5]

local server = ucursor:get_all("shadowsocksr", server_section)

local trojan = {
    -- error = "/var/ssrplus.log",
    log_level = 3,
    run_type = proto,
    local_addr = "0.0.0.0",
    local_port = tonumber(local_port),
    remote_addr = server.server,
    remote_port = tonumber(server.server_port),
    target_addr = usr_dns,
    target_port = tonumber(usr_port),
    udp_timeout = 60,
    -- 传入连接
    password = {server.password},
    -- 传出连接
    ssl = (server.tls) and {
        verify = (server.insecure == "1") and false or true,
        verify_hostname = (server.insecure == "1") and false or true,
        cert = "",
        ciper = "ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-CHACHA20-POLY1305:ECDHE-RSA-CHACHA20-POLY1305:ECDHE-RSA-AES128-SHA:ECDHE-RSA-AES256-SHA:RSA-AES128-GCM-SHA256:RSA-AES256-GCM-SHA384:RSA-AES128-SHA:RSA-AES256-SHA:RSA-3DES-EDE-SHA",
        sni = server.tls_host,
        alpn = {"h2", "http/1.1"},
        curve = "",
        reuse_session = true,
        session_ticket = false,
        },
        tcp = {
            no_delay = true,
            keep_alive = true,
            fast_open = (server.fast_open == "1") and true or false,
            fast_open_qlen = 20
        }
}
print(json.stringify(trojan, 1))
