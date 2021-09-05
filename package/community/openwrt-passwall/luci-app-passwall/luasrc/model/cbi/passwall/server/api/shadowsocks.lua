module("luci.model.cbi.passwall.server.api.shadowsocks", package.seeall)
function gen_config(user)
    local config = {}
    config.server = {"[::0]", "0.0.0.0"}
    config.server_port = tonumber(user.port)
    config.password = user.password
    config.timeout = tonumber(user.timeout)
    config.fast_open = (user.tcp_fast_open and user.tcp_fast_open == "1") and true or false
    config.method = user.method

    if user.type == "SSR" then
        config.protocol = user.protocol
        config.protocol_param = user.protocol_param
        config.obfs = user.obfs
        config.obfs_param = user.obfs_param
    end

    return config
end
