local ucursor = require 'luci.model.uci'.cursor()
local name = 'vssr'
local json = require 'luci.jsonc'
local server_section = arg[1]
local proto = arg[2]
local local_port = arg[3]
local outbounds_table = {}
local rules_table = {}

local v2ray_flow = ucursor:get_first(name, 'global', 'v2ray_flow', '0')
local proxy_domain_name = ucursor:get_list(name, '@access_control[0]', 'proxy_domain_name')
local flow_table = {
    yotube = {
        name = 'youtube',
        port = 2081,
        rules = {
            type = 'field',
            domain = {
                'youtube',
                'ggpht.com',
                'googlevideo.com',
                'withyoutube.com',
                'youtu.be',
                'youtube-nocookie.com',
                'youtube.com',
                'youtubeeducation.com',
                'youtubegaming.com',
                'youtubei.googleapis.com',
                'youtubekids.com',
                'youtubemobilesupport.com',
                'yt.be',
                'ytimg.com'
            },
            outboundTag = 'youtube'
        }
    },
    tw_video = {
        name = 'tw_video',
        port = 2082,
        rules = {
            type = 'field',
            domain = {
                'vidol.tv',
                'hinet.net',
                'books.com',
                'litv.tv',
                'pstatic.net',
                'app-measurement.com',
                'kktv.com.tw',
                'gamer.com.tw',
                'wetv.vip',
                'kktv.me',
                'myvideo.net.tw'
            },
            outboundTag = 'tw_video'
        }
    },
    netflix = {
        name = 'netflix',
        port = 2083,
        rules = {
            type = 'field',
            domain = {
                'fast.com',
                'netflix.ca',
                'netflix.com',
                'netflix.net',
                'netflixinvestor.com',
                'netflixtechblog.com',
                'nflxext.com',
                'nflximg.com',
                'nflximg.net',
                'nflxsearch.net',
                'nflxso.net',
                'nflxvideo.net',
                'netflixdnstest0.com',
                'netflixdnstest1.com',
                'netflixdnstest2.com',
                'netflixdnstest3.com',
                'netflixdnstest4.com',
                'netflixdnstest5.com',
                'netflixdnstest6.com',
                'netflixdnstest7.com',
                'netflixdnstest8.com',
                'netflixdnstest9.com'
            },
            outboundTag = 'netflix'
        }
    },
    disney = {
        name = 'disney',
        port = 2084,
        rules = {
            type = 'field',
            domain = {
                'cdn.registerdisney.go.com',
                'disneyplus.com',
                'disney-plus.net',
                'dssott.com',
                'bamgrid.com',
                'execute-api.us-east-1.amazonaws.com'
            },
            outboundTag = 'disney'
        }
    },
    prime = {
        name = 'prime',
        port = 2085,
        rules = {
            type = 'field',
            domain = {
                'aiv-cdn.net',
                'amazonaws.com',
                'amazonvideo.com',
                'llnwd.net',
                'amazonprimevideos.com',
                'amazonvideo.cc',
                'prime-video.com',
                'primevideo.cc',
                'primevideo.com',
                'primevideo.info',
                'primevideo.org',
                'primevideo.tv'
            },
            outboundTag = 'prime'
        }
    },
    tvb = {
        name = 'tvb',
        port = 2086,
        rules = {
            type = 'field',
            domain = {'tvsuper.com', 'tvb.com'},
            outboundTag = 'tvb'
        }
    },
    custom = {
        name = 'custom',
        port = 2087,
        rules = {
            type = 'field',
            domain = proxy_domain_name,
            outboundTag = 'custom'
        }
    }
}

function gen_outbound(server_node, tags, local_ports)
    local bound = {}
    if server_node == nil or server_node == 'nil' then
        bound = nil
    else
        local server = ucursor:get_all(name, server_node)
        if server.type ~= 'v2ray' then
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
                protocol = 'vmess',
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
                    security = (server.tls == '1') and 'tls' or 'none',
                    tlsSettings = {
                        allowInsecure = (server.insecure == '1') and true or false,
                        serverName = server.ws_host
                    },
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

if v2ray_flow == '1' then
    table.insert(outbounds_table, gen_outbound(server_section, 'global', 2080))
    for i, v in pairs(flow_table) do
        local server = ucursor:get_first(name, 'global', v.name .. '_server')
        table.insert(outbounds_table, gen_outbound(server, v.name, v.port))
        table.insert(rules_table, (server ~= nil and server ~= 'nil') and v.rules or nil)
    end
else
    table.insert(outbounds_table, gen_outbound(server_section, 'main', local_port))
end

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
