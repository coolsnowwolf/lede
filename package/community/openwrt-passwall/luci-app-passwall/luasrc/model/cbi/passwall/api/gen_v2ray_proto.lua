local api = require "luci.model.cbi.passwall.api.api"
local jsonc = api.jsonc
local inbounds = {}
local outbounds = {}
local routing = nil

local var = api.get_args(arg)
local local_socks_address = var["-local_socks_address"] or "0.0.0.0"
local local_socks_port = var["-local_socks_port"]
local local_socks_username = var["-local_socks_username"]
local local_socks_password = var["-local_socks_password"]
local local_http_address = var["-local_http_address"] or "0.0.0.0"
local local_http_port = var["-local_http_port"]
local local_http_username = var["-local_http_username"]
local local_http_password = var["-local_http_password"]
local server_proto = var["-server_proto"]
local server_address = var["-server_address"]
local server_port = var["-server_port"]
local server_username = var["-server_username"]
local server_password = var["-server_password"]

function gen_outbound(proto, address, port, username, password)
    local result = {
        protocol = proto,
        streamSettings = {
            network = "tcp",
            security = "none"
        },
        settings = {
            servers = {
                {
                    address = address,
                    port = tonumber(port),
                    users = (username and password) and {
                        {
                            user = username,
                            pass = password
                        }
                    } or nil
                }
            }
        }
    }
    return result
end

if local_socks_address and local_socks_port then
    local inbound = {
        listen = local_socks_address,
        port = tonumber(local_socks_port),
        protocol = "socks",
        settings = {
            udp = true,
            auth = "noauth"
        }
    }
    if local_socks_username and local_socks_password and local_socks_username ~= "" and local_socks_password ~= "" then
        inbound.settings.auth = "password"
        inbound.settings.accounts = {
            {
                user = local_socks_username,
                pass = local_socks_password
            }
        }
    end
    table.insert(inbounds, inbound)
end

if local_http_address and local_http_port then
    local inbound = {
        listen = local_http_address,
        port = tonumber(local_http_port),
        protocol = "http",
        settings = {
            allowTransparent = false
        }
    }
    if local_http_username and local_http_password and local_http_username ~= "" and local_http_password ~= "" then
        inbound.settings.accounts = {
            {
                user = local_http_username,
                pass = local_http_password
            }
        }
    end
    table.insert(inbounds, inbound)
end

if server_proto ~= "nil" and server_address ~= "nil" and server_port ~= "nil" then
    local outbound = gen_outbound(server_proto, server_address, server_port, server_username, server_password)
    if outbound then table.insert(outbounds, outbound) end
end

-- 额外传出连接
table.insert(outbounds, {
    protocol = "freedom", tag = "direct", settings = {keep = ""}
})

local config = {
    log = {
        -- error = string.format("/var/etc/passwall/%s.log", node[".name"]),
        loglevel = "warning"
    },
    -- 传入连接
    inbounds = inbounds,
    -- 传出连接
    outbounds = outbounds,
    -- 路由
    routing = routing
}
print(jsonc.stringify(config, 1))
