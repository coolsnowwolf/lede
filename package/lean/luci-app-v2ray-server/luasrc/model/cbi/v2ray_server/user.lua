local d = require "luci.dispatcher"
local appname = "v2ray_server"

local v_ss_encrypt_method_list = {
    "aes-128-cfb", "aes-256-cfb", "aes-128-gcm", "aes-256-gcm", "chacha20", "chacha20-ietf", "chacha20-poly1305", "chacha20-ietf-poly1305"
}

local header_type_list = {
    "none", "srtp", "utp", "wechat-video", "dtls", "wireguard"
}

m = Map(appname, translate("Server Config"))
m.redirect = d.build_url("admin", "services", appname)

s = m:section(NamedSection, arg[1], "user", "")
s.addremove = false
s.dynamic = false

enable = s:option(Flag, "enable", translate("Enable"))
enable.default = "1"
enable.rmempty = false

remarks = s:option(Value, "remarks", translate("Remarks"))
remarks.default = translate("Remarks")
remarks.rmempty = false

protocol = s:option(ListValue, "protocol", translate("Protocol"))
protocol:value("vmess", "Vmess")
protocol:value("vless", "VLESS")
protocol:value("http", "HTTP")
protocol:value("socks", "Socks")
protocol:value("shadowsocks", "Shadowsocks")
protocol:value("trojan", "Trojan")
protocol:value("mtproto", "MTProto")

port = s:option(Value, "port", translate("Port"))
port.datatype = "port"
port.rmempty = false

auth = s:option(Flag, "auth", translate("Auth"))
auth.validate = function(self, value, t)
    if value and value == "1" then
        local user_v = username:formvalue(t) or ""
        local pass_v = password:formvalue(t) or ""
        if user_v == "" or pass_v == "" then
            return nil, translate("Username and Password must be used together!")
        end
    end
    return value
end
auth:depends({ protocol = "socks" })
auth:depends({ protocol = "http" })

username = s:option(Value, "username", translate("Username"))
username:depends("auth", "1")

password = s:option(Value, "password", translate("Password"))
password.password = true
password:depends("auth", "1")
password:depends({ protocol = "shadowsocks" })

mtproto_password = s:option(Value, "mtproto_password", translate("Password"), translate("The MTProto protocol must be 32 characters and can only contain characters from 0 to 9 and a to f."))
mtproto_password:depends({ protocol = "mtproto" })
mtproto_password.default = arg[1]
function mtproto_password.cfgvalue(self, section)
	return m:get(section, "password")
end
function mtproto_password.write(self, section, value)
	m:set(section, "password", value)
end

decryption = s:option(Value, "decryption", translate("Encrypt Method"))
decryption.default = "none"
decryption:depends({ protocol = "vless" })

v_ss_encrypt_method = s:option(ListValue, "v_ss_encrypt_method", translate("Encrypt Method"))
for a, t in ipairs(v_ss_encrypt_method_list) do v_ss_encrypt_method:value(t) end
v_ss_encrypt_method:depends({ protocol = "shadowsocks" })
function v_ss_encrypt_method.cfgvalue(self, section)
	return m:get(section, "method")
end
function v_ss_encrypt_method.write(self, section, value)
	m:set(section, "method", value)
end

ss_network = s:option(ListValue, "ss_network", translate("Transport"))
ss_network.default = "tcp,udp"
ss_network:value("tcp", "TCP")
ss_network:value("udp", "UDP")
ss_network:value("tcp,udp", "TCP,UDP")
ss_network:depends({ protocol = "shadowsocks" })

uuid = s:option(DynamicList, "uuid", translate("ID") .. "/" .. translate("Password"))
for i = 1, 3 do
    uuid:value(luci.sys.exec("echo -n $(cat /proc/sys/kernel/random/uuid)"))
end
uuid:depends({ protocol = "vmess" })
uuid:depends({ protocol = "vless" })
uuid:depends({ protocol = "trojan" })

alter_id = s:option(Value, "alter_id", translate("Alter ID"))
alter_id.default = 16
alter_id:depends({ protocol = "vmess" })

level = s:option(Value, "level", translate("User Level"))
level.default = 1
level:depends({ protocol = "vmess" })
level:depends({ protocol = "vless" })
level:depends({ protocol = "shadowsocks" })
level:depends({ protocol = "trojan" })
level:depends({ protocol = "mtproto" })

tls = s:option(Flag, "tls", translate("TLS"))
tls.default = 0
tls.validate = function(self, value, t)
    if value then
        if value == "1" then
            local ca = tls_certificateFile:formvalue(t) or ""
            local key = tls_keyFile:formvalue(t) or ""
            if ca == "" or key == "" then
                return nil, translate("Public key and Private key path can not be empty!")
            end
        end
        return value
    end
end
tls:depends({ protocol = "vmess" })
tls:depends({ protocol = "vless" })
tls:depends({ protocol = "socks" })
tls:depends({ protocol = "shadowsocks" })

xtls = s:option(Flag, "xtls", translate("XTLS"))
xtls.default = 0
xtls:depends({ protocol = "vless", tls = "1" })

flow = s:option(Value, "flow", translate("flow"))
flow.default = "xtls-rprx-direct"
flow:value("xtls-rprx-origin")
flow:value("xtls-rprx-origin-udp443")
flow:value("xtls-rprx-direct")
flow:value("xtls-rprx-direct-udp443")
flow:value("xtls-rprx-splice")
flow:value("xtls-rprx-splice-udp443")
flow:depends("xtls", "1")

-- [[ TLS部分 ]] --

tls_serverName = s:option(Value, "tls_serverName", translate("Domain"))
tls_serverName:depends("tls", "1")

tls_certificateFile = s:option(Value, "tls_certificateFile", translate("Public key absolute path"), translate("as:") .. "/etc/ssl/fullchain.pem")
tls_certificateFile.validate = function(self, value, t)
    if value and value ~= "" then
        if not nixio.fs.access(value) then
            return nil, translate("Can't find this file!")
        else
            return value
        end
    end
    return nil
end
tls_certificateFile:depends("tls", "1")

tls_keyFile = s:option(Value, "tls_keyFile", translate("Private key absolute path"), translate("as:") .. "/etc/ssl/private.key")
tls_keyFile.validate = function(self, value, t)
    if value and value ~= "" then
        if not nixio.fs.access(value) then
            return nil, translate("Can't find this file!")
        else
            return value
        end
    end
    return nil
end
tls_keyFile:depends("tls", "1")

transport = s:option(ListValue, "transport", translate("Transport"))
transport:value("tcp", "TCP")
transport:value("mkcp", "mKCP")
transport:value("ws", "WebSocket")
transport:value("h2", "HTTP/2")
transport:value("ds", "DomainSocket")
transport:value("quic", "QUIC")
transport:depends({ protocol = "vmess" })
transport:depends({ protocol = "vless" })
transport:depends({ protocol = "socks" })
transport:depends({ protocol = "shadowsocks" })
transport:depends({ protocol = "trojan" })

-- [[ WebSocket部分 ]]--

ws_host = s:option(Value, "ws_host", translate("WebSocket Host"))
ws_host:depends("transport", "ws")
ws_host:depends("ss_transport", "ws")
ws_host:depends("trojan_transport", "h2+ws")
ws_host:depends("trojan_transport", "ws")

ws_path = s:option(Value, "ws_path", translate("WebSocket Path"))
ws_path:depends("transport", "ws")
ws_path:depends("ss_transport", "ws")
ws_path:depends("trojan_transport", "h2+ws")
ws_path:depends("trojan_transport", "ws")

-- [[ HTTP/2部分 ]]--

h2_host = s:option(Value, "h2_host", translate("HTTP/2 Host"))
h2_host:depends("transport", "h2")
h2_host:depends("ss_transport", "h2")
h2_host:depends("trojan_transport", "h2+ws")
h2_host:depends("trojan_transport", "h2")

h2_path = s:option(Value, "h2_path", translate("HTTP/2 Path"))
h2_path:depends("transport", "h2")
h2_path:depends("ss_transport", "h2")
h2_path:depends("trojan_transport", "h2+ws")
h2_path:depends("trojan_transport", "h2")

-- [[ TCP部分 ]]--

-- TCP伪装
tcp_guise = s:option(ListValue, "tcp_guise", translate("Camouflage Type"))
tcp_guise:value("none", "none")
tcp_guise:value("http", "http")
tcp_guise:depends("transport", "tcp")

-- HTTP域名
tcp_guise_http_host = s:option(DynamicList, "tcp_guise_http_host", translate("HTTP Host"))
tcp_guise_http_host:depends("tcp_guise", "http")

-- HTTP路径
tcp_guise_http_path = s:option(DynamicList, "tcp_guise_http_path", translate("HTTP Path"))
tcp_guise_http_path:depends("tcp_guise", "http")

-- [[ mKCP部分 ]]--

mkcp_guise = s:option(ListValue, "mkcp_guise", translate("Camouflage Type"), translate('<br />none: default, no masquerade, data sent is packets with no characteristics.<br />srtp: disguised as an SRTP packet, it will be recognized as video call data (such as FaceTime).<br />utp: packets disguised as uTP will be recognized as bittorrent downloaded data.<br />wechat-video: packets disguised as WeChat video calls.<br />dtls: disguised as DTLS 1.2 packet.<br />wireguard: disguised as a WireGuard packet. (not really WireGuard protocol)'))
for a, t in ipairs(header_type_list) do mkcp_guise:value(t) end
mkcp_guise:depends("transport", "mkcp")

mkcp_mtu = s:option(Value, "mkcp_mtu", translate("KCP MTU"))
mkcp_mtu.default = "1350"
mkcp_mtu:depends("transport", "mkcp")

mkcp_tti = s:option(Value, "mkcp_tti", translate("KCP TTI"))
mkcp_tti.default = "20"
mkcp_tti:depends("transport", "mkcp")

mkcp_uplinkCapacity = s:option(Value, "mkcp_uplinkCapacity", translate("KCP uplinkCapacity"))
mkcp_uplinkCapacity.default = "5"
mkcp_uplinkCapacity:depends("transport", "mkcp")

mkcp_downlinkCapacity = s:option(Value, "mkcp_downlinkCapacity", translate("KCP downlinkCapacity"))
mkcp_downlinkCapacity.default = "20"
mkcp_downlinkCapacity:depends("transport", "mkcp")

mkcp_congestion = s:option(Flag, "mkcp_congestion", translate("KCP Congestion"))
mkcp_congestion:depends("transport", "mkcp")

mkcp_readBufferSize = s:option(Value, "mkcp_readBufferSize", translate("KCP readBufferSize"))
mkcp_readBufferSize.default = "1"
mkcp_readBufferSize:depends("transport", "mkcp")

mkcp_writeBufferSize = s:option(Value, "mkcp_writeBufferSize", translate("KCP writeBufferSize"))
mkcp_writeBufferSize.default = "1"
mkcp_writeBufferSize:depends("transport", "mkcp")

mkcp_seed = s:option(Value, "mkcp_seed", translate("KCP Seed"))
mkcp_seed:depends("transport", "mkcp")

-- [[ DomainSocket部分 ]]--

ds_path = s:option(Value, "ds_path", "Path", translate("A legal file path. This file must not exist before running V2Ray."))
ds_path:depends("transport", "ds")

-- [[ QUIC部分 ]]--
quic_security = s:option(ListValue, "quic_security", translate("Encrypt Method"))
quic_security:value("none")
quic_security:value("aes-128-gcm")
quic_security:value("chacha20-poly1305")
quic_security:depends("transport", "quic")

quic_key = s:option(Value, "quic_key", translate("Encrypt Method") .. translate("Key"))
quic_key:depends("transport", "quic")

quic_guise = s:option(ListValue, "quic_guise", translate("Camouflage Type"))
for a, t in ipairs(header_type_list) do quic_guise:value(t) end
quic_guise:depends("transport", "quic")

-- [[ VLESS Fallback部分 ]]--
--[[
fallback = s:option(Flag, "fallback", translate("Fallback"))
fallback:depends({ protocol = "vless", transport = "tcp", tls = "1" })

fallback_alpn = s:option(Value, "fallback_alpn", "Fallback alpn")
fallback_alpn:depends("fallback", "1")

fallback_path = s:option(Value, "fallback_path", "Fallback path")
fallback_path:depends("fallback", "1")

fallback_dest = s:option(Value, "fallback_dest", "Fallback dest")
fallback_dest:depends("fallback", "1")

fallback_xver = s:option(Value, "fallback_xver", "Fallback xver")
fallback_xver.default = 0
fallback_xver:depends("fallback", "1")
]]--

bind_local = s:option(Flag, "bind_local", translate("Bind Local"), translate("When selected, it can only be accessed locally,It is recommended to turn on when using reverse proxies."))
bind_local.default = "0"

accept_lan = s:option(Flag, "accept_lan", translate("Accept LAN Access"), translate("When selected, it can accessed lan , this will not be safe!"))
accept_lan.default = "0"
accept_lan.rmempty = false

return m
