module("luci.model.cbi.passwall.server.api.trojan", package.seeall)
function gen_config(user)
    local cipher = "ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-CHACHA20-POLY1305:ECDHE-RSA-CHACHA20-POLY1305:ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-SHA:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES128-SHA:ECDHE-RSA-AES256-SHA:DHE-RSA-AES128-SHA:DHE-RSA-AES256-SHA:AES128-SHA:AES256-SHA:DES-CBC3-SHA"
    local cipher13 = "TLS_AES_128_GCM_SHA256:TLS_CHACHA20_POLY1305_SHA256:TLS_AES_256_GCM_SHA384"
    local config = {
        run_type = "server",
        local_addr = "::",
        local_port = tonumber(user.port),
        remote_addr = (user.remote_enable == "1" and user.remote_address) and user.remote_address or nil,
        remote_port = (user.remote_enable == "1" and user.remote_port) and tonumber(user.remote_port) or nil,
        password = user.uuid,
        log_level = (user.log and user.log == "1") and tonumber(user.loglevel) or 5,
        ssl = {
            cert = user.tls_certificateFile,
            key = user.tls_keyFile,
            key_password = "",
            cipher = cipher,
            cipher_tls13 = cipher13,
            prefer_server_cipher = true,
            reuse_session = true,
            session_ticket = (user.tls_sessionTicket == "1") and true or false,
            session_timeout = 600,
            plain_http_response = "",
            curves = "",
            dhparam = ""
        },
        tcp = {
            prefer_ipv4 = false,
            no_delay = true,
            keep_alive = true,
            reuse_port = false,
            fast_open = (user.tcp_fast_open and user.tcp_fast_open == "1") and true or false,
            fast_open_qlen = 20
        }
    }
    if user.type == "Trojan-Go" then
        config.ssl.cipher = nil
        config.ssl.cipher_tls13 = nil
        config.udp_timeout = 60
        config.disable_http_check = true
        config.transport_plugin = ((user.tls == nil or user.tls ~= "1") and user.trojan_transport == "original") and {
            enabled = user.plugin_type ~= nil,
            type = user.plugin_type or "plaintext",
            command = user.plugin_type ~= "plaintext" and user.plugin_cmd or nil,
            option = user.plugin_type ~= "plaintext" and user.plugin_option or nil,
            arg = user.plugin_type ~= "plaintext" and { user.plugin_arg } or nil,
            env = {}
        } or nil
        config.websocket = (user.trojan_transport == 'ws') and {
            enabled = true,
            path = user.ws_path or "/",
            host = user.ws_host or ""
        } or nil
        config.shadowsocks = (user.ss_aead == "1") and {
            enabled = true,
            method = user.ss_aead_method or "aes_128_gcm",
            password = user.ss_aead_pwd or ""
        } or nil
    end
    return config
end