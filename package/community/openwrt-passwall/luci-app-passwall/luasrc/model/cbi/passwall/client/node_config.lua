local api = require "luci.model.cbi.passwall.api.api"
local appname = api.appname
local uci = api.uci

local ss_encrypt_method_list = {
    "rc4-md5", "aes-128-cfb", "aes-192-cfb", "aes-256-cfb", "aes-128-ctr",
    "aes-192-ctr", "aes-256-ctr", "bf-cfb", "salsa20", "chacha20", "chacha20-ietf",
    "aes-128-gcm", "aes-192-gcm", "aes-256-gcm", "chacha20-ietf-poly1305",
    "xchacha20-ietf-poly1305"
}

local ss_rust_encrypt_method_list = {
    "plain", "none",
    "aes-128-gcm", "aes-256-gcm", "chacha20-ietf-poly1305"
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
local encrypt_methods_ss_aead = {
	"chacha20-ietf-poly1305",
	"aes-128-gcm",
	"aes-256-gcm",
}

m = Map(appname, translate("Node Config"))
m.redirect = api.url()

s = m:section(NamedSection, arg[1], "nodes", "")
s.addremove = false
s.dynamic = false

share = s:option(DummyValue, "passwall", " ")
share.rawhtml  = true
share.template = "passwall/node_list/link_share_man"
share.value = arg[1]

remarks = s:option(Value, "remarks", translate("Node Remarks"))
remarks.default = translate("Remarks")
remarks.rmempty = false

type = s:option(ListValue, "type", translate("Type"))
if api.is_finded("ipt2socks") then
    type:value("Socks", translate("Socks"))
end
if api.is_finded("ss-redir") then
    type:value("SS", translate("Shadowsocks Libev"))
end
if api.is_finded("sslocal") then
    type:value("SS-Rust", translate("Shadowsocks Rust"))
end
if api.is_finded("ssr-redir") then
    type:value("SSR", translate("ShadowsocksR Libev"))
end
if api.is_finded("v2ray") then
    type:value("V2ray", translate("V2ray"))
end
if api.is_finded("xray") then
    type:value("Xray", translate("Xray"))
end
if api.is_finded("brook") then
    type:value("Brook", translate("Brook"))
end
--[[
if api.is_finded("trojan-plus") or api.is_finded("trojan") then
    type:value("Trojan", translate("Trojan"))
end
]]--
if api.is_finded("trojan-plus") then
    type:value("Trojan-Plus", translate("Trojan-Plus"))
end
if api.is_finded("trojan-go") then
    type:value("Trojan-Go", translate("Trojan-Go"))
end
if api.is_finded("naive") then
    type:value("Naiveproxy", translate("NaiveProxy"))
end
if api.is_finded("hysteria") then
    type:value("Hysteria", translate("Hysteria"))
end

xray_tips = s:option(DummyValue, "xray_tips", " ")
xray_tips.rawhtml = true
xray_tips.cfgvalue = function(t, n)
    return string.format('<a style="color: red">%s</a>', translate("Xray is currently directly compatible with V2ray and used."))
end
xray_tips:depends("type", "Xray")

protocol = s:option(ListValue, "protocol", translate("Protocol"))
protocol:value("vmess", translate("Vmess"))
protocol:value("vless", translate("VLESS"))
protocol:value("http", translate("HTTP"))
protocol:value("socks", translate("Socks"))
protocol:value("shadowsocks", translate("Shadowsocks"))
protocol:value("trojan", translate("Trojan"))
protocol:value("_balancing", translate("Balancing"))
protocol:value("_shunt", translate("Shunt"))
protocol:depends("type", "V2ray")
protocol:depends("type", "Xray")

local nodes_table = {}
for k, e in ipairs(api.get_valid_nodes()) do
    if e.node_type == "normal" then
        nodes_table[#nodes_table + 1] = {
            id = e[".name"],
            remarks = e["remark"]
        }
    end
end

-- 负载均衡列表
balancing_node = s:option(DynamicList, "balancing_node", translate("Load balancing node list"), translate("Load balancing node list, <a target='_blank' href='https://toutyrater.github.io/routing/balance2.html'>document</a>"))
for k, v in pairs(nodes_table) do balancing_node:value(v.id, v.remarks) end
balancing_node:depends("protocol", "_balancing")

-- 分流
uci:foreach(appname, "shunt_rules", function(e)
    o = s:option(ListValue, e[".name"], string.format('* <a href="%s" target="_blank">%s</a>', api.url("shunt_rules", e[".name"]), translate(e.remarks)))
    o:value("nil", translate("Close"))
    o:value("_default", translate("Default"))
    o:value("_direct", translate("Direct Connection"))
    o:value("_blackhole", translate("Blackhole"))
    o:depends("protocol", "_shunt")

    if #nodes_table > 0 then
        _proxy = s:option(Flag, e[".name"] .. "_proxy", translate(e.remarks) .. translate("Preproxy"), translate("Use the default node for the transit."))
        _proxy.default = 0

        for k, v in pairs(nodes_table) do
            o:value(v.id, v.remarks)
            _proxy:depends(e[".name"], v.id)
        end
    end
end)

shunt_tips = s:option(DummyValue, "shunt_tips", " ")
shunt_tips.rawhtml = true
shunt_tips.cfgvalue = function(t, n)
    return string.format('<a style="color: red" href="../rule">%s</a>', translate("No shunt rules? Click me to go to add."))
end
shunt_tips:depends("protocol", "_shunt")

default_node = s:option(ListValue, "default_node", translate("Default") .. " " .. translate("Node"))
default_node:value("_direct", translate("Direct Connection"))
default_node:value("_blackhole", translate("Blackhole"))
for k, v in pairs(nodes_table) do default_node:value(v.id, v.remarks) end
default_node:depends("protocol", "_shunt")

if #nodes_table > 0 then
    o = s:option(ListValue, "main_node", translate("Default") .. " " .. translate("Node") .. translate("Preproxy"), translate("Use this node proxy to forward the default node."))
    o:value("nil", translate("Close"))
    for k, v in pairs(nodes_table) do
        o:value(v.id, v.remarks)
        o:depends("default_node", v.id)
    end
end

domainStrategy = s:option(ListValue, "domainStrategy", translate("Domain Strategy"))
domainStrategy:value("AsIs")
domainStrategy:value("IPIfNonMatch")
domainStrategy:value("IPOnDemand")
domainStrategy.description = "<br /><ul><li>" .. translate("'AsIs': Only use domain for routing. Default value.")
.. "</li><li>" .. translate("'IPIfNonMatch': When no rule matches current domain, resolves it into IP addresses (A or AAAA records) and try all rules again.")
.. "</li><li>" .. translate("'IPOnDemand': As long as there is a IP-based rule, resolves the domain into IP immediately.")
.. "</li></ul>"
domainStrategy:depends("protocol", "_balancing")
domainStrategy:depends("protocol", "_shunt")

-- Brook协议
brook_protocol = s:option(ListValue, "brook_protocol", translate("Protocol"))
brook_protocol:value("client", translate("Brook"))
brook_protocol:value("wsclient", translate("WebSocket"))
brook_protocol:depends("type", "Brook")
function brook_protocol.cfgvalue(self, section)
	return m:get(section, "protocol")
end
function brook_protocol.write(self, section, value)
	m:set(section, "protocol", value)
end

brook_tls = s:option(Flag, "brook_tls", translate("Use TLS"))
brook_tls:depends("brook_protocol", "wsclient")

-- Naiveproxy协议
naiveproxy_protocol = s:option(ListValue, "naiveproxy_protocol", translate("Protocol"))
naiveproxy_protocol:value("https", translate("HTTPS"))
naiveproxy_protocol:value("quic", translate("QUIC"))
naiveproxy_protocol:depends("type", "Naiveproxy")
function naiveproxy_protocol.cfgvalue(self, section)
	return m:get(section, "protocol")
end
function naiveproxy_protocol.write(self, section, value)
	m:set(section, "protocol", value)
end

address = s:option(Value, "address", translate("Address (Support Domain Name)"))
address.rmempty = false
address:depends("type", "Socks")
address:depends("type", "SS")
address:depends("type", "SS-Rust")
address:depends("type", "SSR")
address:depends("type", "Brook")
address:depends("type", "Trojan")
address:depends("type", "Trojan-Plus")
address:depends("type", "Trojan-Go")
address:depends("type", "Naiveproxy")
address:depends("type", "Hysteria")
address:depends({ type = "V2ray", protocol = "vmess" })
address:depends({ type = "V2ray", protocol = "vless" })
address:depends({ type = "V2ray", protocol = "http" })
address:depends({ type = "V2ray", protocol = "socks" })
address:depends({ type = "V2ray", protocol = "shadowsocks" })
address:depends({ type = "V2ray", protocol = "trojan" })
address:depends({ type = "Xray", protocol = "vmess" })
address:depends({ type = "Xray", protocol = "vless" })
address:depends({ type = "Xray", protocol = "http" })
address:depends({ type = "Xray", protocol = "socks" })
address:depends({ type = "Xray", protocol = "shadowsocks" })
address:depends({ type = "Xray", protocol = "trojan" })

--[[
use_ipv6 = s:option(Flag, "use_ipv6", translate("Use IPv6"))
use_ipv6.default = 0
use_ipv6:depends("type", "Socks")
use_ipv6:depends("type", "SS")
use_ipv6:depends("type", "SS-Rust")
use_ipv6:depends("type", "SSR")
use_ipv6:depends("type", "Brook")
use_ipv6:depends("type", "Trojan")
use_ipv6:depends("type", "Trojan-Plus")
use_ipv6:depends("type", "Trojan-Go")
use_ipv6:depends("type", "Hysteria")
use_ipv6:depends({ type = "V2ray", protocol = "vmess" })
use_ipv6:depends({ type = "V2ray", protocol = "vless" })
use_ipv6:depends({ type = "V2ray", protocol = "http" })
use_ipv6:depends({ type = "V2ray", protocol = "socks" })
use_ipv6:depends({ type = "V2ray", protocol = "shadowsocks" })
use_ipv6:depends({ type = "V2ray", protocol = "trojan" })
use_ipv6:depends({ type = "Xray", protocol = "vmess" })
use_ipv6:depends({ type = "Xray", protocol = "vless" })
use_ipv6:depends({ type = "Xray", protocol = "http" })
use_ipv6:depends({ type = "Xray", protocol = "socks" })
use_ipv6:depends({ type = "Xray", protocol = "shadowsocks" })
use_ipv6:depends({ type = "Xray", protocol = "trojan" })
--]]

port = s:option(Value, "port", translate("Port"))
port.datatype = "port"
port.rmempty = false
port:depends("type", "Socks")
port:depends("type", "SS")
port:depends("type", "SS-Rust")
port:depends("type", "SSR")
port:depends("type", "Brook")
port:depends("type", "Trojan")
port:depends("type", "Trojan-Plus")
port:depends("type", "Trojan-Go")
port:depends("type", "Naiveproxy")
port:depends("type", "Hysteria")
port:depends({ type = "V2ray", protocol = "vmess" })
port:depends({ type = "V2ray", protocol = "vless" })
port:depends({ type = "V2ray", protocol = "http" })
port:depends({ type = "V2ray", protocol = "socks" })
port:depends({ type = "V2ray", protocol = "shadowsocks" })
port:depends({ type = "V2ray", protocol = "trojan" })
port:depends({ type = "Xray", protocol = "vmess" })
port:depends({ type = "Xray", protocol = "vless" })
port:depends({ type = "Xray", protocol = "http" })
port:depends({ type = "Xray", protocol = "socks" })
port:depends({ type = "Xray", protocol = "shadowsocks" })
port:depends({ type = "Xray", protocol = "trojan" })

username = s:option(Value, "username", translate("Username"))
username:depends("type", "Socks")
username:depends("type", "Naiveproxy")
username:depends({ type = "V2ray", protocol = "http" })
username:depends({ type = "V2ray", protocol = "socks" })
username:depends({ type = "Xray", protocol = "http" })
username:depends({ type = "Xray", protocol = "socks" })

password = s:option(Value, "password", translate("Password"))
password.password = true
password:depends("type", "Socks")
password:depends("type", "SS")
password:depends("type", "SS-Rust")
password:depends("type", "SSR")
password:depends("type", "Brook")
password:depends("type", "Trojan")
password:depends("type", "Trojan-Plus")
password:depends("type", "Trojan-Go")
password:depends("type", "Naiveproxy")
password:depends({ type = "V2ray", protocol = "http" })
password:depends({ type = "V2ray", protocol = "socks" })
password:depends({ type = "V2ray", protocol = "shadowsocks" })
password:depends({ type = "V2ray", protocol = "trojan" })
password:depends({ type = "Xray", protocol = "http" })
password:depends({ type = "Xray", protocol = "socks" })
password:depends({ type = "Xray", protocol = "shadowsocks" })
password:depends({ type = "Xray", protocol = "trojan" })

hysteria_obfs = s:option(Value, "hysteria_obfs", translate("Obfs Password"))
hysteria_obfs:depends("type", "Hysteria")

hysteria_auth_type = s:option(ListValue, "hysteria_auth_type", translate("Auth Type"))
hysteria_auth_type:value("disable", translate("Disable"))
hysteria_auth_type:value("string", translate("STRING"))
hysteria_auth_type:value("base64", translate("BASE64"))
hysteria_auth_type:depends("type", "Hysteria")

hysteria_auth_password = s:option(Value, "hysteria_auth_password", translate("Auth Password"))
hysteria_auth_password.password = true
hysteria_auth_password:depends("hysteria_auth_type", "string")
hysteria_auth_password:depends("hysteria_auth_type", "base64")

ss_encrypt_method = s:option(Value, "ss_encrypt_method", translate("Encrypt Method"))
for a, t in ipairs(ss_encrypt_method_list) do ss_encrypt_method:value(t) end
ss_encrypt_method:depends("type", "SS")
function ss_encrypt_method.cfgvalue(self, section)
	return m:get(section, "method")
end
function ss_encrypt_method.write(self, section, value)
	m:set(section, "method", value)
end

ss_rust_encrypt_method = s:option(Value, "ss_rust_encrypt_method", translate("Encrypt Method"))
for a, t in ipairs(ss_rust_encrypt_method_list) do ss_rust_encrypt_method:value(t) end
ss_rust_encrypt_method:depends("type", "SS-Rust")
function ss_rust_encrypt_method.cfgvalue(self, section)
	return m:get(section, "method")
end
function ss_rust_encrypt_method.write(self, section, value)
	m:set(section, "method", value)
end

ssr_encrypt_method = s:option(Value, "ssr_encrypt_method", translate("Encrypt Method"))
for a, t in ipairs(ssr_encrypt_method_list) do ssr_encrypt_method:value(t) end
ssr_encrypt_method:depends("type", "SSR")
function ssr_encrypt_method.cfgvalue(self, section)
	return m:get(section, "method")
end
function ssr_encrypt_method.write(self, section, value)
	m:set(section, "method", value)
end

security = s:option(ListValue, "security", translate("Encrypt Method"))
for a, t in ipairs(security_list) do security:value(t) end
security:depends({ type = "V2ray", protocol = "vmess" })
security:depends({ type = "Xray", protocol = "vmess" })

encryption = s:option(Value, "encryption", translate("Encrypt Method"))
encryption.default = "none"
encryption:depends({ type = "V2ray", protocol = "vless" })
encryption:depends({ type = "Xray", protocol = "vless" })

v_ss_encrypt_method = s:option(Value, "v_ss_encrypt_method", translate("Encrypt Method"))
for a, t in ipairs(v_ss_encrypt_method_list) do v_ss_encrypt_method:value(t) end
v_ss_encrypt_method:depends("protocol", "shadowsocks")
function v_ss_encrypt_method.cfgvalue(self, section)
	return m:get(section, "method")
end
function v_ss_encrypt_method.write(self, section, value)
	m:set(section, "method", value)
end

ssr_protocol = s:option(Value, "ssr_protocol", translate("Protocol"))
for a, t in ipairs(ssr_protocol_list) do ssr_protocol:value(t) end
ssr_protocol:depends("type", "SSR")
function ssr_protocol.cfgvalue(self, section)
	return m:get(section, "protocol")
end
function ssr_protocol.write(self, section, value)
	m:set(section, "protocol", value)
end

protocol_param = s:option(Value, "protocol_param", translate("Protocol_param"))
protocol_param:depends("type", "SSR")

obfs = s:option(Value, "obfs", translate("Obfs"))
for a, t in ipairs(ssr_obfs_list) do obfs:value(t) end
obfs:depends("type", "SSR")

obfs_param = s:option(Value, "obfs_param", translate("Obfs_param"))
obfs_param:depends("type", "SSR")

timeout = s:option(Value, "timeout", translate("Connection Timeout"))
timeout.datatype = "uinteger"
timeout.default = 300
timeout:depends("type", "SS")
timeout:depends("type", "SS-Rust")
timeout:depends("type", "SSR")

tcp_fast_open = s:option(ListValue, "tcp_fast_open", translate("TCP Fast Open"), translate("Need node support required"))
tcp_fast_open:value("false")
tcp_fast_open:value("true")
tcp_fast_open:depends("type", "SS")
tcp_fast_open:depends("type", "SS-Rust")
tcp_fast_open:depends("type", "SSR")
tcp_fast_open:depends("type", "Trojan")
tcp_fast_open:depends("type", "Trojan-Plus")
tcp_fast_open:depends("type", "Trojan-Go")

ss_plugin = s:option(ListValue, "ss_plugin", translate("plugin"))
ss_plugin:value("none", translate("none"))
if api.is_finded("xray-plugin") then ss_plugin:value("xray-plugin") end
if api.is_finded("v2ray-plugin") then ss_plugin:value("v2ray-plugin") end
if api.is_finded("obfs-local") then ss_plugin:value("obfs-local") end
ss_plugin:depends("type", "SS")
ss_plugin:depends("type", "SS-Rust")
function ss_plugin.cfgvalue(self, section)
	return m:get(section, "plugin")
end
function ss_plugin.write(self, section, value)
	m:set(section, "plugin", value)
end

ss_plugin_opts = s:option(Value, "ss_plugin_opts", translate("opts"))
ss_plugin_opts:depends("ss_plugin", "xray-plugin")
ss_plugin_opts:depends("ss_plugin", "v2ray-plugin")
ss_plugin_opts:depends("ss_plugin", "obfs-local")
function ss_plugin_opts.cfgvalue(self, section)
	return m:get(section, "plugin_opts")
end
function ss_plugin_opts.write(self, section, value)
	m:set(section, "plugin_opts", value)
end

use_kcp = s:option(Flag, "use_kcp", translate("Use") .. "Kcptun",
                   "<span style='color:red'>" .. translate("Please confirm whether the Kcptun is installed. If not, please go to Rule Update download installation.") .. "</span>")
use_kcp.default = 0
use_kcp:depends("type", "SS")
use_kcp:depends("type", "SS-Rust")
use_kcp:depends("type", "SSR")

kcp_server = s:option(Value, "kcp_server", translate("Kcptun Server"))
kcp_server.placeholder = translate("Default:Current Server")
kcp_server:depends("use_kcp", "1")

kcp_port = s:option(Value, "kcp_port", translate("Kcptun Port"))
kcp_port.datatype = "port"
kcp_port:depends("use_kcp", "1")

kcp_opts = s:option(TextValue, "kcp_opts", translate("Kcptun Config"), translate("--crypt aes192 --key abc123 --mtu 1350 --sndwnd 128 --rcvwnd 1024 --mode fast"))
kcp_opts.placeholder = "--crypt aes192 --key abc123 --mtu 1350 --sndwnd 128 --rcvwnd 1024 --mode fast"
kcp_opts:depends("use_kcp", "1")

uuid = s:option(Value, "uuid", translate("ID"))
uuid.password = true
uuid:depends({ type = "V2ray", protocol = "vmess" })
uuid:depends({ type = "V2ray", protocol = "vless" })
uuid:depends({ type = "Xray", protocol = "vmess" })
uuid:depends({ type = "Xray", protocol = "vless" })

alter_id = s:option(Value, "alter_id", translate("Alter ID"))
alter_id:depends("protocol", "vmess")

tls = s:option(Flag, "tls", translate("TLS"))
tls.default = 0
tls.validate = function(self, value, t)
    if value then
        local type = type:formvalue(t) or ""
        if value == "0" and (type == "Trojan" or type == "Trojan-Plus") then
            return nil, translate("Original Trojan only supported 'tls', please choose 'tls'.")
        end
        return value
    end
end
tls:depends({ type = "V2ray", protocol = "vmess" })
tls:depends({ type = "V2ray", protocol = "vless" })
tls:depends({ type = "V2ray", protocol = "socks" })
tls:depends({ type = "V2ray", protocol = "trojan" })
tls:depends({ type = "V2ray", protocol = "shadowsocks" })
tls:depends({ type = "Xray", protocol = "vmess" })
tls:depends({ type = "Xray", protocol = "vless" })
tls:depends({ type = "Xray", protocol = "socks" })
tls:depends({ type = "Xray", protocol = "trojan" })
tls:depends({ type = "Xray", protocol = "shadowsocks" })
tls:depends("type", "Trojan")
tls:depends("type", "Trojan-Plus")
tls:depends("type", "Trojan-Go")

xtls = s:option(Flag, "xtls", translate("XTLS"))
xtls.default = 0
xtls:depends({ type = "Xray", protocol = "vless", tls = true })
xtls:depends({ type = "Xray", protocol = "trojan", tls = true })

flow = s:option(Value, "flow", translate("flow"))
flow.default = "xtls-rprx-direct"
flow:value("xtls-rprx-origin")
flow:value("xtls-rprx-origin-udp443")
flow:value("xtls-rprx-direct")
flow:value("xtls-rprx-direct-udp443")
flow:value("xtls-rprx-splice")
flow:value("xtls-rprx-splice-udp443")
flow:depends("xtls", true)

alpn = s:option(ListValue, "alpn", translate("alpn"))
alpn.default = "default"
alpn:value("default", translate("Default"))
alpn:value("h2,http/1.1")
alpn:value("h2")
alpn:value("http/1.1")
alpn:depends({ type = "V2ray", tls = true })
alpn:depends({ type = "Xray", tls = true })

-- [[ TLS部分 ]] --
tls_sessionTicket = s:option(Flag, "tls_sessionTicket", translate("Session Ticket"))
tls_sessionTicket.default = "0"
tls_sessionTicket:depends({ type = "Trojan", tls = true })
tls_sessionTicket:depends({ type = "Trojan-Plus", tls = true })
tls_sessionTicket:depends({ type = "Trojan-Go", tls = true })

trojan_go_fingerprint = s:option(ListValue, "trojan_go_fingerprint", translate("Finger Print"))
trojan_go_fingerprint:value("disable", translate("Disable"))
trojan_go_fingerprint:value("firefox")
trojan_go_fingerprint:value("chrome")
trojan_go_fingerprint:value("ios")
trojan_go_fingerprint.default = "disable"
trojan_go_fingerprint:depends({ type = "Trojan-Go", tls = true })
function trojan_go_fingerprint.cfgvalue(self, section)
	return m:get(section, "fingerprint")
end
function trojan_go_fingerprint.write(self, section, value)
	m:set(section, "fingerprint", value)
end

tls_serverName = s:option(Value, "tls_serverName", translate("Domain"))
tls_serverName:depends("tls", true)
tls_serverName:depends("type", "Hysteria")

tls_allowInsecure = s:option(Flag, "tls_allowInsecure", translate("allowInsecure"), translate("Whether unsafe connections are allowed. When checked, Certificate validation will be skipped."))
tls_allowInsecure.default = "0"
tls_allowInsecure:depends("tls", true)
tls_allowInsecure:depends("type", "Hysteria")

xray_fingerprint = s:option(ListValue, "xray_fingerprint", translate("Finger Print"))
xray_fingerprint:value("disable", translate("Disable"))
xray_fingerprint:value("chrome")
xray_fingerprint:value("firefox")
xray_fingerprint:value("safari")
xray_fingerprint:value("randomized")
xray_fingerprint.default = "disable"
xray_fingerprint:depends({ type = "Xray", tls = true, xtls = false })
function xray_fingerprint.cfgvalue(self, section)
	return m:get(section, "fingerprint")
end
function xray_fingerprint.write(self, section, value)
	m:set(section, "fingerprint", value)
end

trojan_transport = s:option(ListValue, "trojan_transport", translate("Transport"))
trojan_transport:value("original", translate("Original"))
trojan_transport:value("ws", "WebSocket")
trojan_transport.default = "original"
trojan_transport:depends("type", "Trojan-Go")

trojan_plugin = s:option(ListValue, "plugin_type", translate("Transport Plugin"))
trojan_plugin:value("plaintext", "Plain Text")
trojan_plugin:value("shadowsocks", "ShadowSocks")
trojan_plugin:value("other", "Other")
trojan_plugin.default = "plaintext"
trojan_plugin:depends({ tls = false, trojan_transport = "original" })

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
transport:value("grpc", "gRPC")
transport:depends({ type = "V2ray", protocol = "vmess" })
transport:depends({ type = "V2ray", protocol = "vless" })
transport:depends({ type = "V2ray", protocol = "socks" })
transport:depends({ type = "V2ray", protocol = "shadowsocks" })
transport:depends({ type = "V2ray", protocol = "trojan" })
transport:depends({ type = "Xray", protocol = "vmess" })
transport:depends({ type = "Xray", protocol = "vless" })
transport:depends({ type = "Xray", protocol = "socks" })
transport:depends({ type = "Xray", protocol = "shadowsocks" })
transport:depends({ type = "Xray", protocol = "trojan" })

--[[
ss_transport = s:option(ListValue, "ss_transport", translate("Transport"))
ss_transport:value("ws", "WebSocket")
ss_transport:value("h2", "HTTP/2")
ss_transport:value("h2+ws", "HTTP/2 & WebSocket")
ss_transport:depends({ type = "V2ray", protocol = "shadowsocks" })
ss_transport:depends({ type = "Xray", protocol = "shadowsocks" })
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
ws_host:depends("trojan_transport", "ws")

ws_path = s:option(Value, "ws_path", translate("WebSocket Path"))
ws_path:depends("transport", "ws")
ws_path:depends("ss_transport", "ws")
ws_path:depends("trojan_transport", "ws")
ws_path:depends({ type = "Brook", brook_protocol = "wsclient" })

-- [[ HTTP/2部分 ]]--
h2_host = s:option(Value, "h2_host", translate("HTTP/2 Host"))
h2_host:depends("transport", "h2")
h2_host:depends("ss_transport", "h2")

h2_path = s:option(Value, "h2_path", translate("HTTP/2 Path"))
h2_path:depends("transport", "h2")
h2_path:depends("ss_transport", "h2")

-- [[ DomainSocket部分 ]]--
ds_path = s:option(Value, "ds_path", "Path", translate("A legal file path. This file must not exist before running."))
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

-- [[ gRPC部分 ]]--
grpc_serviceName = s:option(Value, "grpc_serviceName", "ServiceName")
grpc_serviceName:depends("transport", "grpc")

-- [[ Trojan-Go Shadowsocks2 ]] --
ss_aead = s:option(Flag, "ss_aead", translate("Shadowsocks secondary encryption"))
ss_aead:depends("type", "Trojan-Go")
ss_aead.default = "0"

ss_aead_method = s:option(ListValue, "ss_aead_method", translate("Encrypt Method"))
for _, v in ipairs(encrypt_methods_ss_aead) do ss_aead_method:value(v, v) end
ss_aead_method.default = "aes-128-gcm"
ss_aead_method:depends("ss_aead", "1")

ss_aead_pwd = s:option(Value, "ss_aead_pwd", translate("Password"))
ss_aead_pwd.password = true
ss_aead_pwd:depends("ss_aead", "1")

-- [[ Trojan-Go Mux ]]--
mux = s:option(Flag, "smux", translate("smux"))
mux:depends("type", "Trojan-Go")

-- [[ Mux ]]--
mux = s:option(Flag, "mux", translate("mux"))
mux:depends({ type = "V2ray", protocol = "vmess" })
mux:depends({ type = "V2ray", protocol = "vless", xtls = false })
mux:depends({ type = "V2ray", protocol = "http" })
mux:depends({ type = "V2ray", protocol = "socks" })
mux:depends({ type = "V2ray", protocol = "shadowsocks" })
mux:depends({ type = "V2ray", protocol = "trojan" })
mux:depends({ type = "Xray", protocol = "vmess" })
mux:depends({ type = "Xray", protocol = "vless", xtls = false })
mux:depends({ type = "Xray", protocol = "http" })
mux:depends({ type = "Xray", protocol = "socks" })
mux:depends({ type = "Xray", protocol = "shadowsocks" })
mux:depends({ type = "Xray", protocol = "trojan" })

mux_concurrency = s:option(Value, "mux_concurrency", translate("mux concurrency"))
mux_concurrency.default = 8
mux_concurrency:depends("mux", true)
mux_concurrency:depends("smux", true)

smux_idle_timeout = s:option(Value, "smux_idle_timeout", translate("mux idle timeout"))
smux_idle_timeout.default = 60
smux_idle_timeout:depends("smux", true)

hysteria_up_mbps = s:option(Value, "hysteria_up_mbps", translate("Max upload Mbps"))
hysteria_up_mbps.default = "10"
hysteria_up_mbps:depends("type", "Hysteria")

hysteria_down_mbps = s:option(Value, "hysteria_down_mbps", translate("Max download Mbps"))
hysteria_down_mbps.default = "50"
hysteria_down_mbps:depends("type", "Hysteria")

protocol.validate = function(self, value)
    if value == "_shunt" or value == "_balancing" then
        address.rmempty = true
        port.rmempty = true
    end
    return value
end

return m
