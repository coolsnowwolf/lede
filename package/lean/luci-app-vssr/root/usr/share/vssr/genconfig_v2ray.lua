local ucursor = require"luci.model.uci".cursor()
local name = "vssr"
local json = require "luci.jsonc"
local server_section = arg[1]
local proto = arg[2]
local local_port = arg[3]
local host = arg[4]

local v2ray_flow = ucursor:get_first(name, 'global', 'v2ray_flow', '0')
local youtube_server = ucursor:get_first(name, 'global', 'youtube_server')
local tw_video_server = ucursor:get_first(name, 'global', 'tw_video_server')
local netflix_server = ucursor:get_first(name, 'global', 'netflix_server')
local disney_server = ucursor:get_first(name, 'global', 'disney_server')
local prime_server = ucursor:get_first(name, 'global', 'prime_server')

function gen_outbound(server_node, tags)
    local bound = {}
    if server_node == "nil" then
        bound = nil
    else
        local server = ucursor:get_all(name, server_node)
        bound = {
            tag = tags,
            protocol = "vmess",
            settings = {
                vnext = {
                    {
                        address = server.server,
                        port = tonumber(server.server_port),
                        users = {
                            {
                                id = server.vmess_id,
                                alterId = tonumber(server.alter_id),
                                security = server.security
                            }
                        }
                    }
                }
            },
            -- 底层传输配置
            streamSettings = {
                network = server.transport,
                security = (server.tls == '1') and "tls" or "none",
                tlsSettings = {
                    allowInsecure = (server.insecure == "1") and true or false,
                    serverName = server.ws_host
                },
                kcpSettings = (server.transport == "kcp") and {
                    mtu = tonumber(server.mtu),
                    tti = tonumber(server.tti),
                    uplinkCapacity = tonumber(server.uplink_capacity),
                    downlinkCapacity = tonumber(server.downlink_capacity),
                    congestion = (server.congestion == "1") and true or false,
                    readBufferSize = tonumber(server.read_buffer_size),
                    writeBufferSize = tonumber(server.write_buffer_size),
                    header = {type = server.kcp_guise}
                } or nil,
                wsSettings = (server.transport == "ws") and
                    (server.ws_path ~= nil or server.ws_host ~= nil) and {
                    path = server.ws_path,
                    headers = (server.ws_host ~= nil) and
                        {Host = server.ws_host} or nil
                } or nil,
                httpSettings = (server.transport == "h2") and
                    {path = server.h2_path, host = server.h2_host} or nil,
                quicSettings = (server.transport == "quic") and {
                    security = server.quic_security,
                    key = server.quic_key,
                    header = {type = server.quic_guise}
                } or nil
            },
            mux = {
                enabled = (server.mux == "1") and true or false,
                concurrency = tonumber(server.concurrency)
            }
        }
    end
    return bound
end

local outbounds_table = {}

table.insert(outbounds_table, gen_outbound(server_section, "main"))
if v2ray_flow == "1" then
    table.insert(outbounds_table, gen_outbound(youtube_server, "youtube"))
    table.insert(outbounds_table, gen_outbound(tw_video_server, "twvideo"))
    table.insert(outbounds_table, gen_outbound(netflix_server, "netflix"))
    table.insert(outbounds_table, gen_outbound(disney_server, "disney"))
    table.insert(outbounds_table, gen_outbound(prime_server, "prime"))
end

-- rules gen

local youtube_rule = {
    type = "field",
    domain = {"youtube", "googlevideo.com", "gvt2.com", "youtu.be"},
    outboundTag = "youtube"
}

local tw_video_rule = {
    type = "field",
    domain = {
        "vidol.tv", "hinet.net", "books.com", "litv.tv", "pstatic.net",
        "app-measurement.com", "kktv.com.tw", "gamer.com.tw","wetv.vip"
    },
    outboundTag = "twvideo"
}

local netflix_rule = {
    type = "field",
    domain = {
        "netflix",  "nflxso.net", "nflxext.com",
        "nflximg.com", "nflximg.net", "nflxvideo.net"
    },
    
    outboundTag = "netflix"
}

local disney_rule = {
    type = "field",
    domain = {
        "cdn.registerdisney.go.com", "disneyplus.com", "disney-plus.net",
        "dssott.com", "bamgrid.com", "execute-api.us-east-1.amazonaws.com"
    },
    outboundTag = "disney"
}

local prime_rule = {
    type = "field",
    domain = {"aiv-cdn.net", "amazonaws.com", "amazonvideo.com", "llnwd.net"},
    outboundTag = "prime"
}

local rules_table = {}

if (youtube_server ~= "nil" and v2ray_flow == "1") then
    table.insert(rules_table, youtube_rule)
end

if (tw_video_server ~= "nil" and v2ray_flow == "1") then
    table.insert(rules_table, tw_video_rule)
end

if (netflix_server ~= "nil" and v2ray_flow == "1") then
    table.insert(rules_table, netflix_rule)
end

if (disney_server ~= "nil" and v2ray_flow == "1") then
    table.insert(rules_table, disney_rule)
end

if (prime_server ~= "nil" and v2ray_flow == "1") then
    table.insert(rules_table, prime_rule)
end

local v2ray = {
    log = {
        -- error = "/var/ssrplus.log",
        -- access = "/var/v2rays.log", 
        loglevel = "warning"
    },
    -- 传入连接
    inbounds = {
        {
            port = local_port,
            protocol = "dokodemo-door",
            settings = {network = proto, followRedirect = true},
            sniffing = {enabled = true, destOverride = {"http", "tls"}}
        }

    },
    -- 传出连接
    outbounds = outbounds_table,
    routing = {domainStrategy = "IPIfNonMatch", rules = rules_table}
}
print(json.stringify(v2ray, 1))
