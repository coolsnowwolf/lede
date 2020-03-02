local ucursor = require"luci.model.uci".cursor()
local json = require "luci.jsonc"
local server_section = arg[1]
local server = ucursor:get_all("v2ray_server", server_section)

local settings = nil
local routing = nil

if server.protocol == "vmess" then
    if server.VMess_id then
        local clients = {}
        for i = 1, #server.VMess_id do
            clients[i] = {
                id = server.VMess_id[i],
                level = tonumber(server.VMess_level),
                alterId = tonumber(server.VMess_alterId)
            }
        end
        settings = {clients = clients}
    end
elseif server.protocol == "socks" then
    settings = {
        auth = (server.socks_username == nil and server.socks_password == nil) and
            "password" or "noauth",
        accounts = {
            {
                user = (server.socks_username == nil) and "" or
                    server.socks_username,
                pass = (server.socks_password == nil) and "" or
                    server.socks_password
            }
        }
    }
elseif server.protocol == "http" then
    settings = {
        allowTransparent = false,
        accounts = {
            {
                user = (server.socks_username == nil) and "" or
                    server.socks_username,
                pass = (server.socks_password == nil) and "" or
                    server.socks_password
            }
        }
    }
elseif server.protocol == "shadowsocks" then
    settings = {
        method = server.ss_method,
        password = server.ss_password,
        level = tonumber(server.ss_level),
        network = server.ss_network,
        ota = (server.ss_ota == '1') and true or false
    }
end

if server.accept_lan == nil or server.accept_lan == "0" then
    routing = {
        domainStrategy = "IPOnDemand",
        rules = {
            {
                type = "field",
                ip = {"10.0.0.0/8", "172.16.0.0/12", "192.168.0.0/16"},
                outboundTag = "blocked"
            }
        }
    }
end

local v2ray = {
    log = {
        -- error = "/var/log/v2ray.log",
        loglevel = "warning"
    },
    -- 传入连接
    inbound = {
        listen = (server.bind_local == "1") and "127.0.0.1" or nil,
        port = tonumber(server.port),
        protocol = server.protocol,
        -- 底层传输配置
        settings = settings,
        streamSettings = (server.protocol == "vmess") and {
            network = server.transport,
            security = (server.tls_enable == '1') and "tls" or "none",
            tlsSettings = (server.tls_enable == '1') and {
                -- serverName = (server.tls_serverName),
                allowInsecure = false,
                disableSystemRoot = false,
                certificates = {
                    {
                        certificateFile = server.tls_certificateFile,
                        keyFile = server.tls_keyFile
                    }
                }
            } or nil,
            kcpSettings = (server.transport == "mkcp") and {
                mtu = tonumber(server.mkcp_mtu),
                tti = tonumber(server.mkcp_tti),
                uplinkCapacity = tonumber(server.mkcp_uplinkCapacity),
                downlinkCapacity = tonumber(server.mkcp_downlinkCapacity),
                congestion = (server.mkcp_congestion == "1") and true or false,
                readBufferSize = tonumber(server.mkcp_readBufferSize),
                writeBufferSize = tonumber(server.mkcp_writeBufferSize),
                header = {type = server.mkcp_guise}
            } or nil,
            wsSettings = (server.transport == "ws") and {
                headers = (server.ws_host) and {Host = server.ws_host} or nil,
                path = server.ws_path
            } or nil,
            httpSettings = (server.transport == "h2") and
                {path = server.h2_path, host = server.h2_host} or nil,
            quicSettings = (server.transport == "quic") and {
                security = server.quic_security,
                key = server.quic_key,
                header = {type = server.quic_guise}
            } or nil
        } or nil
    },
    -- 传出连接
    outbound = {protocol = "freedom"},
    -- 额外传出连接
    outboundDetour = {{protocol = "blackhole", tag = "blocked"}},
    routing = routing
}
print(json.stringify(v2ray, 1))
