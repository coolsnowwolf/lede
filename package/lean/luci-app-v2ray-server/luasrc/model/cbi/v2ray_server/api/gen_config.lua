module("luci.model.cbi.v2ray_server.api.gen_config", package.seeall)

function gen_config(user)
    local settings = nil
    local routing = nil
    local outbounds = {
        {protocol = "freedom", tag = "direct"}, {protocol = "blackhole", tag = "blocked"}
    }

    if user.protocol == "vmess" or user.protocol == "vless" then
        if user.uuid then
            local clients = {}
            for i = 1, #user.uuid do
                clients[i] = {
                    id = user.uuid[i],
                    flow = (user.xtls and user.xtls == "1") and user.flow or nil,
                    level = tonumber(user.level),
                    alterId = tonumber(user.alter_id)
                }
            end
            settings = {
                clients = clients,
                decryption = user.decryption or "none"
            }
        end
    elseif user.protocol == "socks" then
        settings = {
            auth = (user.auth and user.auth == "1") and "password" or "noauth",
            accounts = (user.auth and user.auth == "1") and {
                {
                    user = user.username,
                    pass = user.password
                }
            }
        }
    elseif user.protocol == "http" then
        settings = {
            allowTransparent = false,
            accounts = (user.auth and user.auth == "1") and {
                {
                    user = user.username,
                    pass = user.password
                }
            }
        }
        user.transport = "tcp"
        user.tcp_guise = "none"
    elseif user.protocol == "shadowsocks" then
        settings = {
            method = user.method,
            password = user.password,
            level = tonumber(user.level) or 1,
            network = user.ss_network or "TCP,UDP"
        }
    elseif user.protocol == "trojan" then
        if user.uuid then
            local clients = {}
            for i = 1, #user.uuid do
                clients[i] = {
                    password = user.uuid[i],
                    level = tonumber(user.level)
                }
            end
            settings = {
                clients = clients
            }
        end
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

    local config = {
        log = {
            -- error = "/var/etc/v2ray_server/log/" .. user[".name"] .. ".log",
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
                    security = "none",
                    xtlsSettings = (user.tls and user.tls == "1" and user.xtls and user.xtls == "1") and {
                        --alpn = {"http/1.1"},
                        disableSystemRoot = false,
                        certificates = {
                            {
                                certificateFile = user.tls_certificateFile,
                                keyFile = user.tls_keyFile
                            }
                        }
                    } or nil,
                    tlsSettings = (user.tls and user.tls == "1") and {
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
                            request = (user.tcp_guise == "http") and {
                                path = {user.tcp_guise_http_path} or {"/"},
                                headers = {
                                    Host = {user.tcp_guise_http_host} or {}
                                }
                            } or nil
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
                        acceptProxyProtocol = false,
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

    if user.tls and user.tls == "1" then
        config.inbounds[1].streamSettings.security = "tls"
        if user.xtls and user.xtls == "1" then
            config.inbounds[1].streamSettings.security = "xtls"
            config.inbounds[1].streamSettings.tlsSettings = nil
        end
    end

    if user.transport == "mkcp" or user.transport == "quic" then
        config.inbounds[1].streamSettings.security = "none"
        config.inbounds[1].streamSettings.tlsSettings = nil
    end

    return config
end
