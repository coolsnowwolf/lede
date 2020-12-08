local ucursor = require 'luci.model.uci'.cursor()
local name = 'vssr'
local json = require 'luci.jsonc'
local server_section = arg[1]
local proto = arg[2]
local local_port = arg[3]
local outbounds_table = {}
local rules_table = {}

function read_conf(file)
    local rfile = io.open(file, "r")
    local ltable = {}
    for line in rfile:lines() do
        local re = string.gsub(line, "\r", "")
        table.insert(ltable,re)
    end
    local rtable = next(ltable) ~= nil and ltable or nil
    return rtable
end

local v2ray_flow = ucursor:get_first(name, 'global', 'v2ray_flow', '0')

local custom_domain = read_conf("/etc/vssr/custom_domain.list")
local youtube_domain = read_conf("/etc/vssr/youtube_domain.list")
local tw_video_domain = read_conf("/etc/vssr/tw_video_domain.list")
local netflix_domain = read_conf("/etc/vssr/netflix_domain.list")
local disney_domain = read_conf("/etc/vssr/disney_domain.list")
local prime_domain = read_conf("/etc/vssr/prime_domain.list")
local tvb_domain = read_conf("/etc/vssr/tvb_domain.list")

local flow_table = {
    yotube = {
        name = 'youtube',
        port = 2081,
        rules = {
            type = 'field',
            domain = youtube_domain,
            outboundTag = 'youtube'
        }
    },
    tw_video = {
        name = 'tw_video',
        port = 2082,
        rules = {
            type = 'field',
            domain = tw_video_domain,
            outboundTag = 'tw_video'
        }
    },
    netflix = {
        name = 'netflix',
        port = 2083,
        rules = {
            type = 'field',
            domain = netflix_domain,
            outboundTag = 'netflix'
        }
    },
    disney = {
        name = 'disney',
        port = 2084,
        rules = {
            type = 'field',
            domain = disney_domain,
            outboundTag = 'disney'
        }
    },
    prime = {
        name = 'prime',
        port = 2085,
        rules = {
            type = 'field',
            domain = prime_domain,
            outboundTag = 'prime'
        }
    },
    tvb = {
        name = 'tvb',
        port = 2086,
        rules = {
            type = 'field',
            domain = tvb_domain,
            outboundTag = 'tvb'
        }
    },
    custom = {
        name = 'custom',
        port = 2087,
        rules = {
            type = 'field',
            domain = custom_domain,
            outboundTag = 'custom'
        }
    }
}

local bt_rules = {
    type = 'field',
    outboundTag = 'bt',
    protocol = {
        'bittorrent'
    }
}
local bt_rules1 = {
    type = 'field',
    outboundTag = 'bt',
    domain = {
        'torrent',
        'peer_id=',
        'info_hash',
        'get_peers',
        'find_node',
        'BitTorrent',
        'announce_peer',
        'announce.php?passkey='
    },
}

function gen_outbound(server_node, tags, local_ports)
    local bound = {}
    if server_node == nil or server_node == 'nil' then
        bound = nil
    else
        local server = ucursor:get_all(name, server_node)
        local outbound_security = "none"
        if (server.xtls == '1') then
            outbound_security = "xtls"
        elseif (server.tls == '1') then
            outbound_security = "tls"
        elseif (server.tls == "0") then
            outbound_security = "none"
        end
        local node_type = server.type == "vless" and "vless" or "vmess"

        if server.type ~= 'v2ray' and server.type ~= 'vless' then
            bound = {
                tag = tags,
                protocol = 'socks',
                settings = {
                    servers = {
                        {address = '127.0.0.1', port = tonumber(local_ports)}
                    }
                }
            }
        else
            bound = {
                tag = tags,
                protocol = node_type,
                settings = {
                    vnext = {
                        {
                            address = server.server,
                            port = tonumber(server.server_port),
                            users = {
                                {
                                    id = server.vmess_id,
                                    alterId = server.type == "v2ray" and tonumber(server.alter_id) or nil,
                                    security =  server.type == "v2ray" and server.security or nil,
                                    flow = (server.xtls == '1') and (server.vless_flow and server.vless_flow or "xtls-rprx-origin") or nil,
                                    encryption = server.type == "vless" and server.vless_encryption or nil
                                }
                            }
                        }
                    }
                },
                -- 底层传输配置
                streamSettings = {
                    network = server.transport,
                    security = outbound_security,
                    tlsSettings = (outbound_security == "tls") and {allowInsecure = (server.insecure ~= "0") and true or false,serverName=server.tls_host,} or nil,
                    xtlsSettings = (outbound_security == "xtls") and {allowInsecure = (server.insecure ~= "0") and true or false,serverName=server.tls_host,} or nil,
                    kcpSettings = (server.transport == 'kcp') and
                        {
                            mtu = tonumber(server.mtu),
                            tti = tonumber(server.tti),
                            uplinkCapacity = tonumber(server.uplink_capacity),
                            downlinkCapacity = tonumber(server.downlink_capacity),
                            congestion = (server.congestion == '1') and true or false,
                            readBufferSize = tonumber(server.read_buffer_size),
                            writeBufferSize = tonumber(server.write_buffer_size),
                            header = {type = server.kcp_guise}
                        } or
                        nil,
                    wsSettings = (server.transport == 'ws') and (server.ws_path ~= nil or server.ws_host ~= nil) and
                        {
                            path = server.ws_path,
                            headers = (server.ws_host ~= nil) and {Host = server.ws_host} or nil
                        } or
                        nil,
                    httpSettings = (server.transport == 'h2') and {path = server.h2_path, host = server.h2_host} or nil,
                    quicSettings = (server.transport == 'quic') and
                        {
                            security = server.quic_security,
                            key = server.quic_key,
                            header = {type = server.quic_guise}
                        } or
                        nil
                },
                mux = {
                    enabled = (server.mux == '1') and true or false,
                    concurrency = tonumber(server.concurrency)
                }
            }
        end
    end
    return bound
end

function gen_bt_outbounds()
    local bound = {
        tag = 'bt',
        protocol = 'freedom',
        settings = {
            a = 1
        }
    }
    return bound
end

if v2ray_flow == '1' then
    table.insert(outbounds_table, gen_outbound(server_section, 'global', 2080))
    for _, v in pairs(flow_table) do
        if(v.rules.domain ~= nil) then
            local server = ucursor:get_first(name, 'global', v.name .. '_server')
            table.insert(outbounds_table, gen_outbound(server, v.name, v.port))
            table.insert(rules_table, (server ~= nil and server ~= 'nil' ) and v.rules or nil)
        end
    end
else
    table.insert(outbounds_table, gen_outbound(server_section, 'main', local_port))
end

table.insert(outbounds_table, gen_bt_outbounds())
table.insert(rules_table, bt_rules)
table.insert(rules_table, bt_rules1)

local v2ray = {
    log = {
        -- error = "/var/vssrsss.log",
        -- access = "/var/v2rays.log",
        loglevel = 'warning'
    },
    -- 传入连接
    inbounds = {
        {
            port = tonumber(local_port),
            protocol = 'dokodemo-door',
            settings = {network = proto, followRedirect = true},
            sniffing = {enabled = true, destOverride = {'http', 'tls'}},
            streamSettings = {
                sockopt = {tproxy = (proto == 'tcp') and 'redirect' or 'tproxy'}
            }
        }
    },
    -- 传出连接
    outbounds = outbounds_table,
    routing = {domainStrategy = 'AsIs', rules = rules_table}
}
print(json.stringify(v2ray, 1))
