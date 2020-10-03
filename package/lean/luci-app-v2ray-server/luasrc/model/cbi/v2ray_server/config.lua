local app_name = "v2ray_server"
local d = require "luci.dispatcher"

local header_type = {"none", "srtp", "utp", "wechat-video", "dtls", "wireguard"}

map = Map(app_name, "V2ray " .. translate("Server Config"))
map.redirect = d.build_url("admin", "vpn", "v2ray_server")

t = map:section(NamedSection, arg[1], "user", "")
t.addremove = false
t.dynamic = false

enable = t:option(Flag, "enable", translate("Enable"))
enable.default = "1"
enable.rmempty = false

remarks = t:option(Value, "remarks", translate("Remarks"))
remarks.default = translate("Remarks")
remarks.rmempty = false

bind_local = t:option(Flag, "bind_local", translate("Bind Local"), translate(
                          "When selected, it can only be accessed locally,It is recommended to turn on when using reverse proxies."))
bind_local.default = "0"
bind_local.rmempty = false

port = t:option(Value, "port", translate("Port"))
port.datatype = "port"
port.rmempty = false

protocol = t:option(ListValue, "protocol", translate("Protocol"))
protocol:value("vmess", translate("Vmess"))
protocol:value("socks", translate("Socks"))
protocol:value("http",translate("Http"))
protocol:value("shadowsocks", translate("Shadowsocks"))

socks_username = t:option(Value, "socks_username", translate("User name"))
socks_username.rmempty = true
socks_username:depends("protocol", "socks")

socks_password = t:option(Value, "socks_password", translate("Password"))
socks_password.rmempty = true
socks_password.password = true
socks_password:depends("protocol", "socks")

http_username = t:option(Value, "http_username", translate("User name"))
http_username.rmempty = true
http_username:depends("protocol", "http")

http_password = t:option(Value, "http_password", translate("Password"))
http_password.rmempty = true
http_password.password = true
http_password:depends("protocol", "http")

ss_method = t:option(ListValue, "ss_method", translate("Encrypt Method"))
ss_method:value("aes-128-cfb")
ss_method:value("aes-256-cfb")
ss_method:value("aes-128-gcm")
ss_method:value("aes-256-gcm")
ss_method:value("chacha20")
ss_method:value("chacha20-ietf")
ss_method:value("chacha20-poly1305")
ss_method:value("chacha20-ietf-poly1305")
ss_method:depends("protocol", "shadowsocks")

ss_password = t:option(Value, "ss_password", translate("Password"))
ss_password:depends("protocol", "shadowsocks")

ss_level = t:option(Value, "ss_level", translate("User Level"))
ss_level.default = 1
ss_level:depends("protocol", "shadowsocks")

ss_network = t:option(ListValue, "ss_network", translate("Transport"))
ss_network.default = "tcp,udp"
ss_network:value("tcp", "TCP")
ss_network:value("udp", "UDP")
ss_network:value("tcp,udp", "TCP,UDP")
ss_network:depends("protocol", "shadowsocks")

ss_ota = t:option(Flag, "ss_ota", translate("OTA"), translate(
                      "When OTA is enabled, V2Ray will reject connections that are not OTA enabled. This option is invalid when using AEAD encryption."))
ss_ota.default = "0"
ss_ota:depends("protocol", "shadowsocks")

VMess_id = t:option(DynamicList, "VMess_id", translate("ID"))
for i = 1, 3 do
    local uuid = luci.sys.exec("cat /proc/sys/kernel/random/uuid")
    VMess_id:value(uuid)
end
VMess_id:depends("protocol", "vmess")

VMess_alterId = t:option(Value, "VMess_alterId", translate("Alter ID"))
VMess_alterId.default = 16
VMess_alterId:depends("protocol", "vmess")

VMess_level = t:option(Value, "VMess_level", translate("User Level"))
VMess_level.default = 1
VMess_level:depends("protocol", "vmess")

transport = t:option(ListValue, "transport", translate("Transport"))
transport:value("tcp", "TCP")
transport:value("mkcp", "mKCP")
transport:value("ws", "WebSocket")
transport:value("h2", "HTTP/2")
transport:value("quic", "QUIC")
transport:depends("protocol", "vmess")

-- [[ TCP部分 ]]--
-- TCP伪装
tcp_guise = t:option(ListValue, "tcp_guise", translate("Camouflage Type"))
tcp_guise:depends("transport", "tcp")
tcp_guise:value("none", "none")
tcp_guise:value("http", "http")

-- HTTP域名
tcp_guise_http_host = t:option(DynamicList, "tcp_guise_http_host",
                               translate("HTTP Host"))
tcp_guise_http_host:depends("tcp_guise", "http")

-- HTTP路径
tcp_guise_http_path = t:option(DynamicList, "tcp_guise_http_path",
                               translate("HTTP Path"))
tcp_guise_http_path:depends("tcp_guise", "http")

-- [[ mKCP部分 ]]--
mkcp_guise = t:option(ListValue, "mkcp_guise", translate("Camouflage Type"),
                      translate(
                          '<br>none: default, no masquerade, data sent is packets with no characteristics.<br>srtp: disguised as an SRTP packet, it will be recognized as video call data (such as FaceTime).<br>utp: packets disguised as uTP will be recognized as bittorrent downloaded data.<br>wechat-video: packets disguised as WeChat video calls.<br>dtls: disguised as DTLS 1.2 packet.<br>wireguard: disguised as a WireGuard packet. (not really WireGuard protocol)'))
for a, t in ipairs(header_type) do mkcp_guise:value(t) end
mkcp_guise:depends("transport", "mkcp")

mkcp_mtu = t:option(Value, "mkcp_mtu", translate("KCP MTU"))
mkcp_mtu:depends("transport", "mkcp")

mkcp_tti = t:option(Value, "mkcp_tti", translate("KCP TTI"))
mkcp_tti:depends("transport", "mkcp")

mkcp_uplinkCapacity = t:option(Value, "mkcp_uplinkCapacity",
                               translate("KCP uplinkCapacity"))
mkcp_uplinkCapacity:depends("transport", "mkcp")

mkcp_downlinkCapacity = t:option(Value, "mkcp_downlinkCapacity",
                                 translate("KCP downlinkCapacity"))
mkcp_downlinkCapacity:depends("transport", "mkcp")

mkcp_congestion = t:option(Flag, "mkcp_congestion", translate("KCP Congestion"))
mkcp_congestion:depends("transport", "mkcp")

mkcp_readBufferSize = t:option(Value, "mkcp_readBufferSize",
                               translate("KCP readBufferSize"))
mkcp_readBufferSize:depends("transport", "mkcp")

mkcp_writeBufferSize = t:option(Value, "mkcp_writeBufferSize",
                                translate("KCP writeBufferSize"))
mkcp_writeBufferSize:depends("transport", "mkcp")

-- [[ WebSocket部分 ]]--
ws_path = t:option(Value, "ws_path", translate("WebSocket Path"))
ws_path:depends("transport", "ws")

ws_host = t:option(Value, "ws_host", translate("WebSocket Host"))
ws_host:depends("transport", "ws")

-- [[ HTTP/2部分 ]]--
h2_path = t:option(Value, "h2_path", translate("HTTP/2 Path"))
h2_path:depends("transport", "h2")

h2_host = t:option(DynamicList, "h2_host", translate("HTTP/2 Host"),
                   translate("Camouflage Domain,you can not fill in"))
h2_host:depends("transport", "h2")

-- [[ QUIC部分 ]]--
quic_security =
    t:option(ListValue, "quic_security", translate("Encrypt Method"))
quic_security:value("none")
quic_security:value("aes-128-gcm")
quic_security:value("chacha20-poly1305")
quic_security:depends("transport", "quic")

quic_key = t:option(Value, "quic_key",
                    translate("Encrypt Method") .. translate("Key"))
quic_key:depends("transport", "quic")

quic_guise = t:option(ListValue, "quic_guise", translate("Camouflage Type"))
for a, t in ipairs(header_type) do quic_guise:value(t) end
quic_guise:depends("transport", "quic")

-- [[ TLS部分 ]] --
tls_enable = t:option(Flag, "tls_enable", translate("Use HTTPS"))
tls_enable:depends("transport", "ws")
tls_enable:depends("transport", "h2")
tls_enable.default = "1"
tls_enable.rmempty = false

-- tls_serverName = t:option(Value, "tls_serverName", translate("Domain"))
-- tls_serverName:depends("transport", "ws")
-- tls_serverName:depends("transport", "h2")

tls_certificateFile = t:option(Value, "tls_certificateFile",
                               translate("Public key absolute path"),
                               translate("as:") .. "/etc/ssl/fullchain.pem")
tls_certificateFile:depends("tls_enable", 1)

tls_keyFile = t:option(Value, "tls_keyFile",
                       translate("Private key absolute path"),
                       translate("as:") .. "/etc/ssl/private.key")
tls_keyFile:depends("tls_enable", 1)

accept_lan = t:option(Flag, "accept_lan", translate("Accept LAN Access"),
                      translate(
                          "When selected, it can accessed lan , this will not be safe!"))
accept_lan.default = "0"
accept_lan.rmempty = false

function rmempty_restore()
    VMess_id.rmempty = true
    VMess_alterId.rmempty = true
    socks_username.rmempty = true
    socks_password.rmempty = true
    ss_password.rmempty = true
    ss_ota.rmempty = true
end

protocol.validate = function(self, value)
    rmempty_restore()
    if value == "vmess" then
        VMess_id.rmempty = false
        VMess_alterId.rmempty = false
    elseif value == "socks" then
        socks_username.rmempty = true
        socks_password.rmempty = true
    elseif value == "shadowsocks" then
        ss_password.rmempty = false
        ss_ota.rmempty = false
    end
    return value
end

return map
