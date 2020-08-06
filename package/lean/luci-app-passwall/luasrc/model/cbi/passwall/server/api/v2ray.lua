module("luci.model.cbi.passwall.server.api.v2ray", package.seeall)
local ucic = require"luci.model.uci".cursor()

function gen_config(user)
    local settings = nil
    local routing = nil
    local outbounds = {
        {protocol = "freedom", tag = "direct"}, {protocol = "blackhole", tag = "blocked"}
    }

    if user.protocol == "vmess" then
        if user.vmess_id then
            local clients = {}
            for i = 1, #user.vmess_id do
                clients[i] = {
                    id = user.vmess_id[i],
                    level = tonumber(user.level),
                    alterId = tonumber(user.alter_id)
                }
            end
            settings = {clients = clients}
        end
    elseif user.protocol == "socks" then
        settings = {
            auth = (user.username == nil and user.password == nil) and "noauth" or "password",
            accounts = {
                {
                    user = (user.username == nil) and "" or user.username,
                    pass = (user.password == nil) and "" or user.password
                }
            }
        }
    elseif user.protocol == "http" then
        settings = {
            allowTransparent = false,
            accounts = {
                {
                    user = (user.username == nil) and "" or user.username,
                    pass = (user.password == nil) and "" or user.password
                }
            }
        }
    elseif user.protocol == "shadowsocks" then
        settings = {
            method = user.v_ss_encrypt_method,
            password = user.password,
            level = tonumber(user.level) or 1,
            network = user.ss_network or "TCP,UDP",
            ota = (user.ss_ota == '1') and true or false
        }
    elseif user.protocol == "mtproto" then
        settings = {
            users = {
                {
                    level = tonumber(user.level) or 1,
                    secret = (user.password == nil) and "" or user.password
                }
            }
        }
    end

    routing = {
        domainStrategy = "IPOnDemand",
        rules = {
            {
                type = "field",
                ip = {"10.0.0.0/8", "172.16.0.0/12", "192.168.0.0/16"},
                outboundTag = (user.accept_lan == nil or user.accept_lan == "0") and "blocked" or "direct"
            }
        }
    }

    if user.transit_node and user.transit_node ~= "nil" then
        local node = ucic:get_all("passwall", user.transit_node)
        if node and node ~= "nil" and node.type and node.type == "V2ray" then
            local transit_node = {
                tag = "transit",
                protocol = node.protocol or "vmess",
                mux = {
                    enabled = (node.mux == "1") and true or false,
                    concurrency = (node.mux_concurrency) and tonumber(node.mux_concurrency) or 8
                },
                -- 底层传输配置
                streamSettings = {
                    network = node.transport,
                    security = node.stream_security,
                    tlsSettings = (node.stream_security == "tls") and {
                        disableSessionResumption = node.sessionTicket ~= "1" and true or false,
                        serverName = node.tls_serverName,
                        allowInsecure = (node.tls_allowInsecure == "1") and true or false
                    } or nil,
                    tcpSettings = (node.transport == "tcp") and {
                        header = {
                            type = node.tcp_guise,
                            request = {
                                path = node.tcp_guise_http_path or {"/"},
                                headers = {
                                    Host = node.tcp_guise_http_host or {}
                                }
                            } or {}
                        }
                    } or nil,
                    kcpSettings = (node.transport == "mkcp") and {
                        mtu = tonumber(node.mkcp_mtu),
                        tti = tonumber(node.mkcp_tti),
                        uplinkCapacity = tonumber(node.mkcp_uplinkCapacity),
                        downlinkCapacity = tonumber(node.mkcp_downlinkCapacity),
                        congestion = (node.mkcp_congestion == "1") and true or false,
                        readBufferSize = tonumber(node.mkcp_readBufferSize),
                        writeBufferSize = tonumber(node.mkcp_writeBufferSize),
                        seed = (node.mkcp_seed and node.mkcp_seed ~= "") and node.mkcp_seed or nil,
                        header = {type = node.mkcp_guise}
                    } or nil,
                    wsSettings = (node.transport == "ws") and {
                        path = node.ws_path or "",
                        headers = (node.ws_host ~= nil) and {Host = node.ws_host} or nil
                    } or nil,
                    httpSettings = (node.transport == "h2") and {
                        path = node.h2_path, host = node.h2_host
                    } or nil,
                    dsSettings = (node.transport == "ds") and {
                        path = node.ds_path
                    } or nil,
                    quicSettings = (node.transport == "quic") and {
                        security = node.quic_security,
                        key = node.quic_key,
                        header = {type = node.quic_guise}
                    } or nil
                },
                settings = {
                    vnext = (node.protocol == "vmess") and {
                        {
                            address = node.address,
                            port = tonumber(node.port),
                            users = {
                                {
                                    id = node.vmess_id,
                                    alterId = tonumber(node.alter_id),
                                    level = tonumber(node.level),
                                    security = node.security
                                }
                            }
                        }
                    } or nil,
                    servers = (node.protocol == "http" or node.protocol == "socks" or node.protocol == "shadowsocks") and {
                        {
                            address = node.address,
                            port = tonumber(node.port),
                            method = node.v_ss_encrypt_method,
                            password = node.password or "",
                            ota = (node.ss_ota == '1') and true or false,
                            users = (node.username and node.password) and
                                {
                                    {
                                        user = node.username or "",
                                        pass = node.password or ""
                                    }
                                } or nil
                        }
                    } or nil
                }
            }

            if node.transport == "mkcp" or node.transport == "ds" or node.transport == "quic" then
                transit_node.streamSettings.security = "none"
                transit_node.streamSettings.tlsSettings = nil
            end

            table.insert(outbounds, 1, transit_node)
        end
    end

    local config = {
        log = {
            -- error = "/var/etc/passwall_server/log/" .. user[".name"] .. ".log",
            loglevel = "warning"
        },
        -- 传入连接
        inbounds = {
            {
                listen = (user.bind_local == "1") and "127.0.0.1" or nil,
                port = tonumber(user.port),
                protocol = user.protocol,
                settings = settings,
                streamSettings = {
                    network = user.transport,
                    security = (user.stream_security == 'tls') and "tls" or "none",
                    tlsSettings = (user.stream_security == 'tls') and {
                        disableSessionResumption = user.sessionTicket ~= "1" and true or false,
                        allowInsecure = false,
                        disableSystemRoot = false,
                        certificates = {
                            {
                                certificateFile = user.tls_certificateFile,
                                keyFile = user.tls_keyFile
                            }
                        }
                    } or nil,
                    tcpSettings = (user.transport == "tcp") and {
                        header = {
                            type = user.tcp_guise,
                            request = {
                                path = user.tcp_guise_http_path or {"/"},
                                headers = {
                                    Host = user.tcp_guise_http_host or {}
                                }
                            } or {}
                        }
                    } or nil,
                    kcpSettings = (user.transport == "mkcp") and {
                        mtu = tonumber(user.mkcp_mtu),
                        tti = tonumber(user.mkcp_tti),
                        uplinkCapacity = tonumber(user.mkcp_uplinkCapacity),
                        downlinkCapacity = tonumber(user.mkcp_downlinkCapacity),
                        congestion = (user.mkcp_congestion == "1") and true or false,
                        readBufferSize = tonumber(user.mkcp_readBufferSize),
                        writeBufferSize = tonumber(user.mkcp_writeBufferSize),
                        seed = (user.mkcp_seed and user.mkcp_seed ~= "") and user.mkcp_seed or nil,
                        header = {type = user.mkcp_guise}
                    } or nil,
                    wsSettings = (user.transport == "ws") and {
                        headers = (user.ws_host) and {Host = user.ws_host} or nil,
                        path = user.ws_path
                    } or nil,
                    httpSettings = (user.transport == "h2") and {
                        path = user.h2_path, host = user.h2_host
                    } or nil,
                    dsSettings = (user.transport == "ds") and {
                        path = user.ds_path
                    } or nil,
                    quicSettings = (user.transport == "quic") and {
                        security = user.quic_security,
                        key = user.quic_key,
                        header = {type = user.quic_guise}
                    } or nil
                }
            }
        },
        -- 传出连接
        outbounds = outbounds,
        routing = routing
    }

    if user.transport == "mkcp" or user.transport == "ds" or user.transport == "quic" then
        config.inbounds[1].streamSettings.security = "none"
        config.inbounds[1].streamSettings.tlsSettings = nil
    end

    return config
end
