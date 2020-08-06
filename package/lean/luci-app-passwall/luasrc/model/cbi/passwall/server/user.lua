local d = require "luci.dispatcher"
local ipkg = require("luci.model.ipkg")
local uci = require"luci.model.uci".cursor()
local api = require "luci.model.cbi.passwall.api.api"

local function is_finded(e)
    local function get_customed_path(e)
        return api.uci_get_type("global_app", e .. "_file")
    end
    return luci.sys.exec("find /usr/*bin %s -iname %s -type f" % {get_customed_path(e), e}) ~= "" and true or false
end

local function is_installed(e) return ipkg.installed(e) end

local ssr_encrypt_method_list = {
    "none", "table", "rc2-cfb", "rc4", "rc4-md5", "rc4-md5-6", "aes-128-cfb",
    "aes-192-cfb", "aes-256-cfb", "aes-128-ctr", "aes-192-ctr", "aes-256-ctr",
    "bf-cfb", "camellia-128-cfb", "camellia-192-cfb", "camellia-256-cfb",
    "cast5-cfb", "des-cfb", "idea-cfb", "seed-cfb", "salsa20", "chacha20",
    "chacha20-ietf"
}

local ssr_protocol_list = {
    "origin", "verify_simple", "verify_deflate", "verify_sha1", "auth_simple",
    "auth_sha1", "auth_sha1_v2", "auth_sha1_v4", "auth_aes128_md5",
    "auth_aes128_sha1", "auth_chain_a", "auth_chain_b", "auth_chain_c",
    "auth_chain_d", "auth_chain_e", "auth_chain_f"
}
local ssr_obfs_list = {
    "plain", "http_simple", "http_post", "random_head", "tls_simple",
    "tls1.0_session_auth", "tls1.2_ticket_auth"
}

local v_ss_encrypt_method_list = {
    "aes-128-cfb", "aes-256-cfb", "aes-128-gcm", "aes-256-gcm", "chacha20", "chacha20-ietf", "chacha20-poly1305", "chacha20-ietf-poly1305"
}

local header_type_list = {
    "none", "srtp", "utp", "wechat-video", "dtls", "wireguard"
}

local encrypt_methods_ss_aead = {
	"dummy",
	"aead_chacha20_poly1305",
	"aead_aes_128_gcm",
	"aead_aes_256_gcm",
	"chacha20-ietf-poly1305",
	"aes-128-gcm",
	"aes-256-gcm",
}

map = Map("passwall_server", translate("Server Config"))
map.redirect = d.build_url("admin", "services", "passwall", "server")

s = map:section(NamedSection, arg[1], "user", "")
s.addremove = false
s.dynamic = false

share = s:option(DummyValue, "passwall_server", translate("Share Current"))
share.rawhtml  = true
share.template = "passwall/node_list/link_share_man"
share.value = arg[1]

enable = s:option(Flag, "enable", translate("Enable"))
enable.default = "1"
enable.rmempty = false

remarks = s:option(Value, "remarks", translate("Remarks"))
remarks.default = translate("Remarks")
remarks.rmempty = false

type = s:option(ListValue, "type", translate("Type"))
if is_finded("ssr-server") then
    type:value("SSR", translate("ShadowsocksR"))
end
if is_installed("v2ray") or is_finded("v2ray") then
    type:value("V2ray", translate("V2ray"))
end
if is_installed("brook") or is_finded("brook") then
    type:value("Brook", translate("Brook"))
end
if is_installed("trojan") or is_finded("trojan") then
    type:value("Trojan", translate("Trojan-Plus"))
end
if is_installed("trojan-go") or is_finded("trojan-go") then
    type:value("Trojan-Go", translate("Trojan-Go"))
end

protocol = s:option(ListValue, "protocol", translate("Protocol"))
protocol:value("vmess", "Vmess")
protocol:value("http", "HTTP")
protocol:value("socks", "Socks")
protocol:value("shadowsocks", "Shadowsocks")
protocol:value("mtproto", "MTProto")
protocol:depends("type", "V2ray")

-- Brook协议
brook_protocol = s:option(ListValue, "brook_protocol", translate("Brook Protocol"))
brook_protocol:value("server", "Brook")
brook_protocol:value("wsserver", "WebSocket")
brook_protocol:depends("type", "Brook")

brook_tls = s:option(Flag, "brook_tls", translate("Use TLS"))
brook_tls:depends("brook_protocol", "wsserver")

port = s:option(Value, "port", translate("Port"))
port.datatype = "port"
port.rmempty = false

username = s:option(Value, "username", translate("Username"))
username:depends("protocol", "http")
username:depends("protocol", "socks")

password = s:option(Value, "password", translate("Password"))
password.password = true
password:depends("type", "SSR")
password:depends("type", "Brook")
password:depends("type", "Trojan")
password:depends({ type = "V2ray", protocol = "http" })
password:depends({ type = "V2ray", protocol = "socks" })
password:depends({ type = "V2ray", protocol = "shadowsocks" })
password:depends({ type = "V2ray", protocol = "mtproto" })

passwords = s:option(DynamicList, "passwords", translate("Password"))
for i = 1, 3 do
    local uuid = luci.sys.exec("echo -n $(cat /proc/sys/kernel/random/uuid)")
    passwords:value(uuid)
end
passwords:depends("type", "Trojan-Go")

ssr_encrypt_method = s:option(ListValue, "ssr_encrypt_method", translate("Encrypt Method"))
for a, t in ipairs(ssr_encrypt_method_list) do ssr_encrypt_method:value(t) end
ssr_encrypt_method:depends("type", "SSR")

v_ss_encrypt_method = s:option(ListValue, "v_ss_encrypt_method", translate("Encrypt Method"))
for a, t in ipairs(v_ss_encrypt_method_list) do v_ss_encrypt_method:value(t) end
v_ss_encrypt_method:depends("protocol", "shadowsocks")

ss_network = s:option(ListValue, "ss_network", translate("Transport"))
ss_network.default = "tcp,udp"
ss_network:value("tcp", "TCP")
ss_network:value("udp", "UDP")
ss_network:value("tcp,udp", "TCP,UDP")
ss_network:depends("protocol", "shadowsocks")

ss_ota = s:option(Flag, "ss_ota", translate("OTA"), translate("When OTA is enabled, V2Ray will reject connections that are not OTA enabled. This option is invalid when using AEAD encryption."))
ss_ota.default = "0"
ss_ota:depends("protocol", "shadowsocks")

ssr_protocol = s:option(ListValue, "ssr_protocol", translate("Protocol"))
for a, t in ipairs(ssr_protocol_list) do ssr_protocol:value(t) end
ssr_protocol:depends("type", "SSR")

ssr_protocol_param = s:option(Value, "protocol_param", translate("Protocol_param"))
ssr_protocol_param:depends("type", "SSR")

obfs = s:option(ListValue, "obfs", translate("Obfs"))
for a, t in ipairs(ssr_obfs_list) do obfs:value(t) end
obfs:depends("type", "SSR")

obfs_param = s:option(Value, "obfs_param", translate("Obfs_param"))
obfs_param:depends("type", "SSR")

timeout = s:option(Value, "timeout", translate("Connection Timeout"))
timeout.datatype = "uinteger"
timeout.default = 300
timeout:depends("type", "SSR")

tcp_fast_open = s:option(ListValue, "tcp_fast_open", translate("TCP Fast Open"), translate("Need node support required"))
tcp_fast_open:value("false")
tcp_fast_open:value("true")
tcp_fast_open:depends("type", "SSR")
tcp_fast_open:depends("type", "Trojan")
tcp_fast_open:depends("type", "Trojan-Go")

udp_forward = s:option(Flag, "udp_forward", translate("UDP Forward"))
udp_forward.default = "1"
udp_forward.rmempty = false
udp_forward:depends("type", "SSR")

vmess_id = s:option(DynamicList, "vmess_id", translate("ID"))
for i = 1, 3 do
    local uuid = luci.sys.exec("echo -n $(cat /proc/sys/kernel/random/uuid)")
    vmess_id:value(uuid)
end
vmess_id:depends({ type = "V2ray", protocol = "vmess" })

alter_id = s:option(Value, "alter_id", translate("Alter ID"))
alter_id.default = 16
alter_id:depends({ type = "V2ray", protocol = "vmess" })

level = s:option(Value, "level", translate("User Level"))
level.default = 1
level:depends({ type = "V2ray", protocol = "vmess" })
level:depends({ type = "V2ray", protocol = "shadowsocks" })
level:depends({ type = "V2ray", protocol = "mtproto" })

stream_security = s:option(ListValue, "stream_security", translate("Transport Layer Encryption"), translate('Whether or not transport layer encryption is enabled, the supported options are "none" for unencrypted (default) and "TLS" for using TLS.'))
stream_security:value("none", "none")
stream_security:value("tls", "tls")
stream_security.default = "tls"
stream_security:depends({ type = "V2ray", protocol = "vmess", transport = "ws" })
stream_security:depends({ type = "V2ray", protocol = "vmess", transport = "h2" })
stream_security:depends({ type = "V2ray", protocol = "socks" })
stream_security:depends({ type = "V2ray", protocol = "shadowsocks" })
stream_security:depends("type", "Trojan")
stream_security:depends("type", "Trojan-Go")
stream_security.validate = function(self, value)
    if value == "none" and type:formvalue(arg[1]) == "Trojan" then
        return nil, translate("'none' not supported for original Trojan.")
    end
    return value
end
-- [[ TLS部分 ]] --

tls_sessionTicket = s:option(Flag, "tls_sessionTicket", translate("Session Ticket"))
tls_sessionTicket.default = "0"
tls_sessionTicket:depends("stream_security", "tls")

tls_serverName = s:option(Value, "tls_serverName", translate("Domain"))
tls_serverName:depends("stream_security", "tls")

tls_allowInsecure = s:option(Flag, "tls_allowInsecure", translate("allowInsecure"), translate("Whether unsafe connections are allowed. When checked, V2Ray does not check the validity of the TLS certificate provided by the remote host."))
tls_allowInsecure.default = "0"
tls_allowInsecure:depends("stream_security", "tls")

tls_certificateFile = s:option(Value, "tls_certificateFile", translate("Public key absolute path"), translate("as:") .. "/etc/ssl/fullchain.pem")
tls_certificateFile:depends("stream_security", "tls")

tls_keyFile = s:option(Value, "tls_keyFile", translate("Private key absolute path"), translate("as:") .. "/etc/ssl/private.key")
tls_keyFile:depends("stream_security", "tls")

transport = s:option(ListValue, "transport", translate("Transport"))
transport:value("tcp", "TCP")
transport:value("mkcp", "mKCP")
transport:value("ws", "WebSocket")
transport:value("h2", "HTTP/2")
transport:value("ds", "DomainSocket")
transport:value("quic", "QUIC")
transport:depends({ type = "V2ray", protocol = "vmess" })
transport:depends({ type = "V2ray", protocol = "socks" })
transport:depends({ type = "V2ray", protocol = "shadowsocks" })

trojan_transport = s:option(ListValue, "trojan_transport", translate("Transport"))
trojan_transport:value("original", "Original")
trojan_transport:value("ws", "WebSocket")
trojan_transport:value("h2", "HTTP/2")
trojan_transport:value("h2+ws", "HTTP/2 & WebSocket")
trojan_transport.default = "ws"
trojan_transport:depends("type", "Trojan-Go")

trojan_plugin = s:option(ListValue, "plugin_type", translate("Plugin Type"))
trojan_plugin:value("plaintext", "Plain Text")
trojan_plugin:value("shadowsocks", "ShadowSocks")
trojan_plugin:value("other", "Other")
trojan_plugin.default = "plaintext"
trojan_plugin:depends({ stream_security = "none", trojan_transport = "original" })

trojan_plugin_cmd = s:option(Value, "plugin_cmd", translate("Plugin Binary"))
trojan_plugin_cmd.placeholder = "eg: /usr/bin/v2ray-plugin"
trojan_plugin_cmd:depends({ plugin_type = "shadowsocks" })
trojan_plugin_cmd:depends({ plugin_type = "other" })

trojan_plugin_op = s:option(Value, "plugin_option", translate("Plugin Option"))
trojan_plugin_op.placeholder = "eg: obfs=http;obfs-host=www.baidu.com"
trojan_plugin_op:depends({ plugin_type = "shadowsocks" })
trojan_plugin_op:depends({ plugin_type = "other" })

trojan_plugin_arg = s:option(DynamicList, "plugin_arg", translate("Plugin Option Args"))
trojan_plugin_arg.placeholder = "eg: [\"-config\", \"test.json\"]"
trojan_plugin_arg:depends({ plugin_type = "shadowsocks" })
trojan_plugin_arg:depends({ plugin_type = "other" })

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

remote_enable = s:option(Flag, "remote_enable", translate("Enable Remote"), translate("You can forward to Nginx/Caddy/V2ray WebSocket and more."))
remote_enable.default = "1"
remote_enable.rmempty = false
remote_enable:depends("type", "Trojan")
remote_enable:depends("type", "Trojan-Go")

remote_address = s:option(Value, "remote_address", translate("Remote Address"))
remote_address.default = "127.0.0.1"
remote_address:depends("remote_enable", 1)

remote_port = s:option(Value, "remote_port", translate("Remote Port"))
remote_port.datatype = "port"
remote_port.default = "80"
remote_port:depends("remote_enable", 1)

ss_aead = s:option(Flag, "ss_aead", translate("Shadowsocks2"))
ss_aead:depends("type", "Trojan-Go")
ss_aead.default = "0"

ss_aead_method = s:option(ListValue, "ss_aead_method", translate("Encrypt Method"))
for _, v in ipairs(encrypt_methods_ss_aead) do ss_aead_method:value(v, v) end
ss_aead_method.default = "aead_aes_128_gcm"
ss_aead_method.rmempty = false
ss_aead_method:depends("ss_aead", "1")

ss_aead_pwd = s:option(Value, "ss_aead_pwd", translate("Password"))
ss_aead_pwd.password = true
ss_aead_pwd.rmempty = false
ss_aead_pwd:depends("ss_aead", "1")

local nodes_table = {}
uci:foreach("passwall", "nodes", function(e)
    if e.type and e.type == "V2ray" and e.remarks and e.address and e.port then
        nodes_table[#nodes_table + 1] = {
            id = e[".name"],
            remarks = "%s：[%s] %s:%s" % {e.type, e.remarks, e.address, e.port}
        }
    end
end)

transit_node = s:option(ListValue, "transit_node", translate("transit node"))
transit_node:value("nil", translate("Close"))
for k, v in pairs(nodes_table) do transit_node:value(v.id, v.remarks) end
transit_node.default = "nil"
transit_node:depends("type", "V2ray")

bind_local = s:option(Flag, "bind_local", translate("Bind Local"), translate("When selected, it can only be accessed locally,It is recommended to turn on when using reverse proxies."))
bind_local.default = "0"
bind_local:depends("type", "V2ray")

accept_lan = s:option(Flag, "accept_lan", translate("Accept LAN Access"), translate("When selected, it can accessed lan , this will not be safe!"))
accept_lan.default = "0"
accept_lan.rmempty = false
accept_lan:depends("type", "V2ray")

return map
