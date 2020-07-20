local ucursor = require"luci.model.uci".cursor()
local json = require "luci.jsonc"
local server_section = arg[1]
local proto = arg[2]
local local_port = arg[3]
local host = arg[4]

local server = ucursor:get_all("vssr", server_section)

local v2ray = {
    log = {
        -- error = "/var/ssrplus.log",
        loglevel = "info"
    },
    -- 传入连接
    inbound = {

        port = local_port,
        protocol = "dokodemo-door",
        settings = {network = proto, followRedirect = true},
        sniffing = {enabled = true, destOverride = {"http", "tls"}}

    },
    -- 传出连接
    outbounds = {
        {
            tag = "protocol_layer",
            protocol = "shadowsocks",
            settings = {
                servers = {
                    {
                        address = host,
                        port = tonumber(server.server_port),
                        method = server.encrypt_method_ss,
                        password = server.password
                    }
                }
            },
            proxySettings = {tag = "transport_layer"}
        }, {
            tag = "transport_layer",
            protocol = "freedom",
            settings = (server.obfs_host ~= nil) and{
                redirect =  server.obfs_host .. ":" ..
                    tonumber(server.server_port) 
            } or nil,
            streamSettings = (server.obfs_transport ~= nil) and{
                network = server.obfs_transport,
                security = (server.obfs_opts == '1') and "tls" or "none",
                wsSettings = {
                    path = server.obfs_path,
                    headers = (server.obfs_host ~= nil) and {host = server.obfs_host} or nil
                }
            } or nil,
            mux = {enabled = (server.mux == "1") and true or false}
        }

    }

}
print(json.stringify(v2ray, 1))
