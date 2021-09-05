module("luci.model.cbi.passwall.server.api.hysteria", package.seeall)
function gen_config(user)
    local config = {
        listen = ":" .. user.port,
        obfs = user.hysteria_obfs,
        cert = user.tls_certificateFile,
        key = user.tls_keyFile,
        auth = (user.hysteria_auth_type == "string") and {
            mode = "password",
            config = {
                password = user.hysteria_auth_password
            }
        } or nil,
        disable_udp = (user.hysteria_udp == "0") and true or false,
        up_mbps = tonumber(user.hysteria_up_mbps) or 10,
        down_mbps = tonumber(user.hysteria_down_mbps) or 50
    }
    return config
end

