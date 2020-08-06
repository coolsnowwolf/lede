local d = require "luci.dispatcher"
local ipkg = require("luci.model.ipkg")
local uci = require"luci.model.uci".cursor()
local api = require "luci.model.cbi.passwall.api.api"

local appname = "passwall"

local function get_customed_path(e)
    return api.uci_get_type("global_app", e .. "_file")
end

local function is_finded(e)
    return luci.sys.exec("find /usr/*bin %s -iname %s -type f" % {get_customed_path(e), e}) ~= "" and true or false
end

local function is_installed(e) return ipkg.installed(e) end

local ss_encrypt_method_list = {
    "rc4-md5", "aes-128-cfb", "aes-192-cfb", "aes-256-cfb", "aes-128-ctr",
    "aes-192-ctr", "aes-256-ctr", "bf-cfb", "camellia-128-cfb",
    "camellia-192-cfb", "camellia-256-cfb", "salsa20", "chacha20",
    "chacha20-ietf", -- aead
    "aes-128-gcm", "aes-192-gcm", "aes-256-gcm", "chacha20-ietf-poly1305",
    "xchacha20-ietf-poly1305"
}

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

local security_list = {"none", "auto", "aes-128-gcm", "chacha20-poly1305"}

local header_type_list = {
    "none", "srtp", "utp", "wechat-video", "dtls", "wireguard"
}
local force_fp = {
    "disable", "firefox", "chrome", "ios"
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

m = Map(appname, translate("Node Config"))
m.redirect = d.build_url("admin", "services", appname)

s = m:section(NamedSection, arg[1], "nodes", "")
s.addremove = false
s.dynamic = false

share = s:option(DummyValue, "passwall", translate("Share Current"))
share.rawhtml  = true
share.template = "passwall/node_list/link_share_man"
share.value = arg[1]

remarks = s:option(Value, "remarks", translate("Node Remarks"))
remarks.default = translate("Remarks")
remarks.rmempty = false

type = s:option(ListValue, "type", translate("Type"))
if is_installed("ipt2socks") or is_finded("ipt2socks") then
    type:value("Socks", translate("Socks"))
end
if is_finded("ss-redir") then
    type:value("SS", translate("Shadowsocks"))
end
if is_finded("ssr-redir") then
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
protocol:value("vmess", translate("Vmess"))
protocol:value("http", translate("HTTP"))
protocol:value("socks", translate("Socks"))
protocol:value("shadowsocks", translate("Shadowsocks"))
protocol:value("_balancing", translate("Balancing"))
protocol:value("_shunt", translate("Shunt"))
protocol:depends("type", "V2ray")

local nodes_table = {}
uci:foreach(appname, "nodes", function(e)
    if e.type and e.remarks and e.port then
        if e.address:match("[\u4e00-\u9fa5]") and e.address:find("%.") and e.address:sub(#e.address) ~= "." then
            nodes_table[#nodes_table + 1] = {
                id = e[".name"],
                remarks = "%s：[%s] %s:%s" % {e.type, e.remarks, e.address, e.port}
            }
        end
    end
end)

-- 负载均衡列表
balancing_node = s:option(DynamicList, "balancing_node", translate("Load balancing node list"), translate("Load balancing node list, <a target='_blank' href='https://toutyrater.github.io/routing/balance2.html'>document</a>"))
for k, v in pairs(nodes_table) do balancing_node:value(v.id, v.remarks) end
balancing_node:depends("protocol", "_balancing")

-- 分流
uci:foreach(appname, "shunt_rules", function(e)
    o = s:option(ListValue, e[".name"], translate(e.remarks))
    o:value("nil", translate("Close"))
    for k, v in pairs(nodes_table) do o:value(v.id, v.remarks) end
    o:depends("protocol", "_shunt")

    o = s:option(Flag, e[".name"] .. "_proxy", translate(e.remarks) .. translate("Preproxy"), translate("Use the default node for the transit."))
    o.default = 0
    o:depends("protocol", "_shunt")
end)

default_node = s:option(ListValue, "default_node", translate("Default") .. " " .. translate("Node"))
default_node:value("nil", translate("Close"))
for k, v in pairs(nodes_table) do default_node:value(v.id, v.remarks) end
default_node:depends("protocol", "_shunt")

-- Brook协议
brook_protocol = s:option(ListValue, "brook_protocol", translate("Brook Protocol"))
brook_protocol:value("client", translate("Brook"))
brook_protocol:value("wsclient", translate("WebSocket"))
brook_protocol:depends("type", "Brook")

brook_tls = s:option(Flag, "brook_tls", translate("Use TLS"))
brook_tls:depends("brook_protocol", "wsclient")

address = s:option(Value, "address", translate("Address (Support Domain Name)"))
address.rmempty = false
address:depends("type", "Socks")
address:depends("type", "SS")
address:depends("type", "SSR")
address:depends({ type = "V2ray", protocol = "vmess" })
address:depends({ type = "V2ray", protocol = "http" })
address:depends({ type = "V2ray", protocol = "socks" })
address:depends({ type = "V2ray", protocol = "shadowsocks" })
address:depends("type", "Brook")
address:depends("type", "Trojan")
address:depends("type", "Trojan-Go")

--[[
use_ipv6 = s:option(Flag, "use_ipv6", translate("Use IPv6"))
use_ipv6.default = 0
use_ipv6:depends("type", "Socks")
use_ipv6:depends("type", "SS")
use_ipv6:depends("type", "SSR")
use_ipv6:depends({ type = "V2ray", protocol = "vmess" })
use_ipv6:depends({ type = "V2ray", protocol = "http" })
use_ipv6:depends({ type = "V2ray", protocol = "socks" })
use_ipv6:depends({ type = "V2ray", protocol = "shadowsocks" })
use_ipv6:depends("type", "Brook")
use_ipv6:depends("type", "Trojan")
use_ipv6:depends("type", "Trojan-Go")
--]]

port = s:option(Value, "port", translate("Port"))
port.datatype = "port"
port.rmempty = false
port:depends("type", "Socks")
port:depends("type", "SS")
port:depends("type", "SSR")
port:depends({ type = "V2ray", protocol = "vmess" })
port:depends({ type = "V2ray", protocol = "http" })
port:depends({ type = "V2ray", protocol = "socks" })
port:depends({ type = "V2ray", protocol = "shadowsocks" })
port:depends("type", "Brook")
port:depends("type", "Trojan")
port:depends("type", "Trojan-Go")

username = s:option(Value, "username", translate("Username"))
username:depends("type", "Socks")
username:depends("protocol", "http")
username:depends("protocol", "socks")

password = s:option(Value, "password", translate("Password"))
password.password = true
password:depends("type", "Socks")
password:depends("type", "SS")
password:depends("type", "SSR")
password:depends("type", "Brook")
password:depends("type", "Trojan")
password:depends("type", "Trojan-Go")
password:depends("protocol", "http")
password:depends("protocol", "socks")
password:depends("protocol", "shadowsocks")

ss_encrypt_method = s:option(ListValue, "ss_encrypt_method", translate("Encrypt Method"))
for a, t in ipairs(ss_encrypt_method_list) do ss_encrypt_method:value(t) end
ss_encrypt_method:depends("type", "SS")

ssr_encrypt_method = s:option(ListValue, "ssr_encrypt_method", translate("Encrypt Method"))
for a, t in ipairs(ssr_encrypt_method_list) do ssr_encrypt_method:value(t) end
ssr_encrypt_method:depends("type", "SSR")

security = s:option(ListValue, "security", translate("Encrypt Method"))
for a, t in ipairs(security_list) do security:value(t) end
security:depends("protocol", "vmess")

v_ss_encrypt_method = s:option(ListValue, "v_ss_encrypt_method", translate("Encrypt Method"))
for a, t in ipairs(v_ss_encrypt_method_list) do v_ss_encrypt_method:value(t) end
v_ss_encrypt_method:depends("protocol", "shadowsocks")

ss_ota = s:option(Flag, "ss_ota", translate("OTA"), translate("When OTA is enabled, V2Ray will reject connections that are not OTA enabled. This option is invalid when using AEAD encryption."))
ss_ota.default = "0"
ss_ota:depends("protocol", "shadowsocks")

ssr_protocol = s:option(ListValue, "ssr_protocol", translate("Protocol"))
for a, t in ipairs(ssr_protocol_list) do ssr_protocol:value(t) end
ssr_protocol:depends("type", "SSR")

protocol_param = s:option(Value, "protocol_param", translate("Protocol_param"))
protocol_param:depends("type", "SSR")

obfs = s:option(ListValue, "obfs", translate("Obfs"))
for a, t in ipairs(ssr_obfs_list) do obfs:value(t) end
obfs:depends("type", "SSR")

obfs_param = s:option(Value, "obfs_param", translate("Obfs_param"))
obfs_param:depends("type", "SSR")

timeout = s:option(Value, "timeout", translate("Connection Timeout"))
timeout.datatype = "uinteger"
timeout.default = 300
timeout:depends("type", "SS")
timeout:depends("type", "SSR")

tcp_fast_open = s:option(ListValue, "tcp_fast_open", translate("TCP Fast Open"), translate("Need node support required"))
tcp_fast_open:value("false")
tcp_fast_open:value("true")
tcp_fast_open:depends("type", "SS")
tcp_fast_open:depends("type", "SSR")
tcp_fast_open:depends("type", "Trojan")
tcp_fast_open:depends("type", "Trojan-Go")

ss_plugin = s:option(ListValue, "ss_plugin", translate("plugin"))
ss_plugin:value("none", translate("none"))
if is_finded("v2ray-plugin") then ss_plugin:value("v2ray-plugin") end
if is_finded("obfs-local") then ss_plugin:value("obfs-local") end
ss_plugin:depends("type", "SS")

ss_plugin_opts = s:option(Value, "ss_plugin_opts", translate("opts"))
ss_plugin_opts:depends("ss_plugin", "v2ray-plugin")
ss_plugin_opts:depends("ss_plugin", "obfs-local")

use_kcp = s:option(Flag, "use_kcp", translate("Use Kcptun"),
                   "<span style='color:red'>" .. translate("Please confirm whether the Kcptun is installed. If not, please go to Rule Update download installation.") .. "</span>")
use_kcp.default = 0
use_kcp:depends("type", "SS")
use_kcp:depends("type", "SSR")
use_kcp:depends("type", "Brook")

kcp_server = s:option(Value, "kcp_server", translate("Kcptun Server"))
kcp_server.placeholder = translate("Default:Current Server")
kcp_server:depends("use_kcp", "1")

kcp_port = s:option(Value, "kcp_port", translate("Kcptun Port"))
kcp_port.datatype = "port"
kcp_port:depends("use_kcp", "1")

kcp_opts = s:option(TextValue, "kcp_opts", translate("Kcptun Config"), translate("--crypt aes192 --key abc123 --mtu 1350 --sndwnd 128 --rcvwnd 1024 --mode fast"))
kcp_opts.placeholder = "--crypt aes192 --key abc123 --mtu 1350 --sndwnd 128 --rcvwnd 1024 --mode fast"
kcp_opts:depends("use_kcp", "1")

vmess_id = s:option(Value, "vmess_id", translate("ID"))
vmess_id.password = true
vmess_id:depends("protocol", "vmess")

alter_id = s:option(Value, "alter_id", translate("Alter ID"))
alter_id:depends("protocol", "vmess")

vmess_level = s:option(Value, "vmess_level", translate("User Level"))
vmess_level.default = 1
vmess_level:depends("protocol", "vmess")

stream_security = s:option(ListValue, "stream_security", translate("Transport Layer Encryption"), translate('Whether or not transport layer encryption is enabled, the supported options are "none" for unencrypted (default) and "TLS" for using TLS.'))
stream_security:value("none", "none")
stream_security:value("tls", "tls")
stream_security.default = "tls"
stream_security:depends("protocol", "vmess")
stream_security:depends("protocol", "socks")
stream_security:depends("protocol", "shadowsocks")
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

-- [[ Trojan TLS ]]--
trojan_force_fp = s:option(ListValue, "fingerprint", translate("Finger Print"))
for a, t in ipairs(force_fp) do trojan_force_fp:value(t) end
trojan_force_fp.default = "firefox"
trojan_force_fp:depends({ type = "Trojan-Go", stream_security = "tls" })

tls_serverName = s:option(Value, "tls_serverName", translate("Domain"))
tls_serverName:depends("stream_security", "tls")

tls_allowInsecure = s:option(Flag, "tls_allowInsecure", translate("allowInsecure"), translate("Whether unsafe connections are allowed. When checked, V2Ray does not check the validity of the TLS certificate provided by the remote host."))
tls_allowInsecure.default = "0"
tls_allowInsecure:depends("stream_security", "tls")

-- [[ Trojan Cert ]]--
trojan_cert_path = s:option(Value, "trojan_cert_path", translate("Trojan Cert Path"))
trojan_cert_path.default = ""
trojan_cert_path:depends({ stream_security = "tls", tls_allowInsecure = false })

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

transport = s:option(ListValue, "transport", translate("Transport"))
transport:value("tcp", "TCP")
transport:value("mkcp", "mKCP")
transport:value("ws", "WebSocket")
transport:value("h2", "HTTP/2")
transport:value("ds", "DomainSocket")
transport:value("quic", "QUIC")
transport:depends("protocol", "vmess")
transport:depends("protocol", "socks")
transport:depends("protocol", "shadowsocks")

--[[
ss_transport = s:option(ListValue, "ss_transport", translate("Transport"))
ss_transport:value("ws", "WebSocket")
ss_transport:value("h2", "HTTP/2")
ss_transport:value("h2+ws", "HTTP/2 & WebSocket")
ss_transport:depends("protocol", "shadowsocks")
]]--

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

-- [[ Trojan-Go Shadowsocks2 ]] --
ss_aead = s:option(Flag, "ss_aead", translate("Shadowsocks2"))
ss_aead:depends("type", "Trojan-Go")
ss_aead.default = "0"

ss_aead_method = s:option(ListValue, "ss_aead_method", translate("Encrypt Method"))
for _, v in ipairs(encrypt_methods_ss_aead) do ss_aead_method:value(v, v) end
ss_aead_method.default = "aead_aes_128_gcm"
ss_aead_method:depends("ss_aead", "1")

ss_aead_pwd = s:option(Value, "ss_aead_pwd", translate("Password"))
ss_aead_pwd.password = true
ss_aead_pwd:depends("ss_aead", "1")

-- [[ Mux ]]--
mux = s:option(Flag, "mux", translate("Mux"))
mux:depends({ type = "V2ray", protocol = "vmess" })
mux:depends({ type = "V2ray", protocol = "http" })
mux:depends({ type = "V2ray", protocol = "socks" })
mux:depends({ type = "V2ray", protocol = "shadowsocks" })
mux:depends("type", "Trojan-Go")

mux_concurrency = s:option(Value, "mux_concurrency", translate("Mux Concurrency"))
mux_concurrency.default = 8
mux_concurrency:depends("mux", "1")

-- [[ 当作为TCP节点时，是否同时开启socks代理 ]]--
--[[
tcp_socks = s:option(Flag, "tcp_socks", translate("TCP Open Socks"), translate("When using this TCP node, whether to open the socks proxy at the same time"))
tcp_socks.default = 0
tcp_socks:depends("type", "V2ray")

tcp_socks_port = s:option(Value, "tcp_socks_port", "Socks " .. translate("Port"), translate("Do not conflict with other ports"))
tcp_socks_port.datatype = "port"
tcp_socks_port.default = 1080
tcp_socks_port:depends("tcp_socks", "1")

tcp_socks_auth = s:option(ListValue, "tcp_socks_auth", translate("Socks for authentication"), translate('Socks protocol authentication, support anonymous and password.'))
tcp_socks_auth:value("noauth", translate("anonymous"))
tcp_socks_auth:value("password", translate("User Password"))
tcp_socks_auth:depends("tcp_socks", "1")

tcp_socks_auth_username = s:option(Value, "tcp_socks_auth_username", "Socks " .. translate("Username"))
tcp_socks_auth_username:depends("tcp_socks_auth", "password")

tcp_socks_auth_password = s:option(Value, "tcp_socks_auth_password", "Socks " .. translate("Password"))
tcp_socks_auth_password:depends("tcp_socks_auth", "password")
--]]

protocol.validate = function(self, value)
    if value == "_shunt" or value == "_balancing" then
        address.rmempty = true
        port.rmempty = true
    end
    return value
end

return m
