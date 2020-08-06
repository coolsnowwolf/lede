module("luci.model.cbi.passwall.server.api.ssr", package.seeall)
function gen_config(user)
    local config = {}
    config.server = {"[::0]", "0.0.0.0"}
    config.server_port = tonumber(user.port)
    config.password = user.password
    config.timeout = tonumber(user.timeout)
    config.fast_open = (user.tcp_fast_open and user.tcp_fast_open == "true") and true or false
    config.method = user.ssr_encrypt_method
    config.protocol = user.ssr_protocol
    config.protocol_param = user.protocol_param
    config.obfs = user.obfs
    config.obfs_param = user.obfs_param
    return config
end
