-- Copyright (C) 2017 yushi studio <ywb94@qq.com> github.com/ywb94
-- Licensed to the public under the GNU General Public License v3.
require "nixio.fs"
require "luci.sys"
require "luci.http"
local m, s, o, kcp_enable
local sid = arg[1]
local uuid = luci.sys.exec("cat /proc/sys/kernel/random/uuid")

function is_finded(e)
	return luci.sys.exec('type -t -p "%s"' % e) ~= "" and true or false
end

local server_table = {}
local encrypt_methods = {
	-- ssr
	"none",
	"table",
	"rc4",
	"rc4-md5-6",
	"rc4-md5",
	"aes-128-cfb",
	"aes-192-cfb",
	"aes-256-cfb",
	"aes-128-ctr",
	"aes-192-ctr",
	"aes-256-ctr",
	"bf-cfb",
	"camellia-128-cfb",
	"camellia-192-cfb",
	"camellia-256-cfb",
	"cast5-cfb",
	"des-cfb",
	"idea-cfb",
	"rc2-cfb",
	"seed-cfb",
	"salsa20",
	"chacha20",
	"chacha20-ietf"
}

local encrypt_methods_ss = {
	-- aead
	"aes-128-gcm",
	"aes-192-gcm",
	"aes-256-gcm",
	"chacha20-ietf-poly1305",
	"xchacha20-ietf-poly1305"
	--[[ stream
	"table",
	"rc4",
	"rc4-md5",
	"aes-128-cfb",
	"aes-192-cfb",
	"aes-256-cfb",
	"aes-128-ctr",
	"aes-192-ctr",
	"aes-256-ctr",
	"bf-cfb",
	"camellia-128-cfb",
	"camellia-192-cfb",
	"camellia-256-cfb",
	"salsa20",
	"chacha20",
	"chacha20-ietf" ]]
}

local encrypt_methods_v2ray_ss = {
	-- xray_ss
	"none",
	"plain",
	--[[ stream
	"aes-128-cfb",
	"aes-256-cfb",
	"chacha20",
	"chacha20-ietf", ]]
	-- aead
	"aes-128-gcm",
	"aes-256-gcm",
	"chacha20-poly1305",
	"chacha20-ietf-poly1305",
	"aead_aes_128_gcm",
	"aead_aes_256_gcm",
	"aead_chacha20_poly1305"
}

local protocol = {
	-- ssr
	"origin",
	"verify_deflate",
	"auth_sha1_v4",
	"auth_aes128_sha1",
	"auth_aes128_md5",
	"auth_chain_a",
	"auth_chain_b",
	"auth_chain_c",
	"auth_chain_d",
	"auth_chain_e",
	"auth_chain_f"
}

obfs = {
	-- ssr
	"plain",
	"http_simple",
	"http_post",
	"random_head",
	"tls1.2_ticket_auth"
}

local securitys = {
	-- vmess
	"auto",
	"none",
	"aes-128-gcm",
	"chacha20-poly1305"
}

local flows = {
	-- xlts
	"xtls-rprx-origin",
	"xtls-rprx-origin-udp443",
	"xtls-rprx-direct",
	"xtls-rprx-direct-udp443",
	"xtls-rprx-splice",
	"xtls-rprx-splice-udp443"
}

m = Map("shadowsocksr", translate("Edit ShadowSocksR Server"))
m.redirect = luci.dispatcher.build_url("admin/services/shadowsocksr/servers")
if m.uci:get("shadowsocksr", sid) ~= "servers" then
	luci.http.redirect(m.redirect)
	return
end

-- [[ Servers Setting ]]--
s = m:section(NamedSection, sid, "servers")
s.anonymous = true
s.addremove = false

o = s:option(DummyValue, "ssr_url", "SS/SSR/V2RAY/TROJAN URL")
o.rawhtml = true
o.template = "shadowsocksr/ssrurl"
o.value = sid

o = s:option(ListValue, "type", translate("Server Node Type"))
if is_finded("xray") or is_finded("v2ray") then
	o:value("v2ray", translate("V2Ray/XRay"))
end
if is_finded("ssr-redir") then
	o:value("ssr", translate("ShadowsocksR"))
end
if is_finded("sslocal") or is_finded("ss-redir") then
	o:value("ss", translate("Shadowsocks New Version"))
end
if is_finded("trojan") then
	o:value("trojan", translate("Trojan"))
end
if is_finded("naive") then
	o:value("naiveproxy", translate("NaiveProxy"))
end
if is_finded("ipt2socks") then
	o:value("socks5", translate("Socks5"))
end
if is_finded("redsocks2") then
	o:value("tun", translate("Network Tunnel"))
end

o.description = translate("Using incorrect encryption mothod may causes service fail to start")

o = s:option(Value, "alias", translate("Alias(optional)"))

o = s:option(ListValue, "iface", translate("Network interface to use"))
for _, e in ipairs(luci.sys.net.devices()) do
	if e ~= "lo" then
		o:value(e)
	end
end
o:depends("type", "tun")
o.description = translate("Redirect traffic to this network interface")

o = s:option(ListValue, "v2ray_protocol", translate("V2Ray/XRay protocol"))
o:value("vless", translate("VLESS"))
o:value("vmess", translate("VMess"))
o:value("trojan", translate("Trojan"))
o:value("shadowsocks", translate("Shadowsocks"))
o:value("socks", translate("Socks"))
o:value("http", translate("HTTP"))
o:depends("type", "v2ray")

o = s:option(Value, "server", translate("Server Address"))
o.datatype = "host"
o.rmempty = false
o:depends("type", "ssr")
o:depends("type", "ss")
o:depends("type", "v2ray")
o:depends("type", "trojan")
o:depends("type", "naiveproxy")
o:depends("type", "socks5")

o = s:option(Value, "server_port", translate("Server Port"))
o.datatype = "port"
o.rmempty = false
o:depends("type", "ssr")
o:depends("type", "ss")
o:depends("type", "v2ray")
o:depends("type", "trojan")
o:depends("type", "naiveproxy")
o:depends("type", "socks5")

o = s:option(Flag, "auth_enable", translate("Enable Authentication"))
o.rmempty = false
o.default = "0"
o:depends("type", "socks5")
o:depends({type = "v2ray", v2ray_protocol = "http"})
o:depends({type = "v2ray", v2ray_protocol = "socks"})

o = s:option(Value, "username", translate("Username"))
o.rmempty = true
o:depends("type", "naiveproxy")
o:depends({type = "socks5", auth_enable = true})
o:depends({type = "v2ray", v2ray_protocol = "http", auth_enable = true})
o:depends({type = "v2ray", v2ray_protocol = "socks", auth_enable = true})

o = s:option(Value, "password", translate("Password"))
o.password = true
o.rmempty = true
o:depends("type", "ssr")
o:depends("type", "ss")
o:depends("type", "trojan")
o:depends("type", "naiveproxy")
o:depends({type = "socks5", auth_enable = true})
o:depends({type = "v2ray", v2ray_protocol = "http", auth_enable = true})
o:depends({type = "v2ray", v2ray_protocol = "socks", auth_enable = true})
o:depends({type = "v2ray", v2ray_protocol = "shadowsocks"})
o:depends({type = "v2ray", v2ray_protocol = "trojan"})

o = s:option(ListValue, "encrypt_method", translate("Encrypt Method"))
for _, v in ipairs(encrypt_methods) do
	o:value(v)
end
o.rmempty = true
o:depends("type", "ssr")

o = s:option(ListValue, "encrypt_method_ss", translate("Encrypt Method"))
for _, v in ipairs(encrypt_methods_ss) do
	o:value(v)
end
o.rmempty = true
o:depends("type", "ss")

o = s:option(ListValue, "encrypt_method_v2ray_ss", translate("Encrypt Method"))
for _, v in ipairs(encrypt_methods_v2ray_ss) do
	o:value(v)
end
o.rmempty = true
o:depends({type = "v2ray", v2ray_protocol = "shadowsocks"})

-- Shadowsocks Plugin
o = s:option(Value, "plugin", translate("Obfs"))
o:value("none", translate("None"))
if is_finded("obfs-local") then
	o:value("obfs-local", translate("simple-obfs"))
end
if is_finded("v2ray-plugin") then
	o:value("v2ray-plugin", translate("v2ray-plugin"))
end
if is_finded("xray-plugin") then
	o:value("xray-plugin", translate("xray-plugin"))
end
o.rmempty = true
o:depends("type", "ss")

o = s:option(Value, "plugin_opts", translate("Plugin Opts"))
o.rmempty = true
o:depends({type = "ss", plugin = "obfs-local"})
o:depends({type = "ss", plugin = "v2ray-plugin"})
o:depends({type = "ss", plugin = "xray-plugin"})

o = s:option(ListValue, "protocol", translate("Protocol"))
for _, v in ipairs(protocol) do
	o:value(v)
end
o.rmempty = true
o:depends("type", "ssr")

o = s:option(Value, "protocol_param", translate("Protocol param(optional)"))
o:depends("type", "ssr")

o = s:option(ListValue, "obfs", translate("Obfs"))
for _, v in ipairs(obfs) do
	o:value(v)
end
o.rmempty = true
o:depends("type", "ssr")

o = s:option(Value, "obfs_param", translate("Obfs param(optional)"))
o:depends("type", "ssr")

-- AlterId
o = s:option(Value, "alter_id", translate("AlterId"))
o.datatype = "port"
o.default = 16
o.rmempty = true
o:depends({type = "v2ray", v2ray_protocol = "vmess"})

-- VmessId
o = s:option(Value, "vmess_id", translate("Vmess/VLESS ID (UUID)"))
o.rmempty = true
o.default = uuid
o:depends({type = "v2ray", v2ray_protocol = "vmess"})
o:depends({type = "v2ray", v2ray_protocol = "vless"})

-- VLESS Encryption
o = s:option(Value, "vless_encryption", translate("VLESS Encryption"))
o.rmempty = true
o.default = "none"
o:depends({type = "v2ray", v2ray_protocol = "vless"})

-- 加密方式
o = s:option(ListValue, "security", translate("Encrypt Method"))
for _, v in ipairs(securitys) do
	o:value(v, v:upper())
end
o.rmempty = true
o:depends({type = "v2ray", v2ray_protocol = "vmess"})

-- 传输协议
o = s:option(ListValue, "transport", translate("Transport"))
o:value("tcp", "TCP")
o:value("kcp", "mKCP")
o:value("ws", "WebSocket")
o:value("h2", "HTTP/2")
o:value("quic", "QUIC")
o:value("grpc", "gRPC")
o.rmempty = true
o:depends("type", "v2ray")

-- [[ TCP部分 ]]--
-- TCP伪装
o = s:option(ListValue, "tcp_guise", translate("Camouflage Type"))
o:depends("transport", "tcp")
o:value("none", translate("None"))
o:value("http", "HTTP")
o.rmempty = true

-- HTTP域名
o = s:option(Value, "http_host", translate("HTTP Host"))
o:depends("tcp_guise", "http")
o.rmempty = true

-- HTTP路径
o = s:option(Value, "http_path", translate("HTTP Path"))
o:depends("tcp_guise", "http")
o.rmempty = true

-- [[ WS部分 ]]--
-- WS域名
o = s:option(Value, "ws_host", translate("WebSocket Host"))
o:depends({transport = "ws", tls = false})
o:depends("trojan_transport", "h2+ws")
o:depends("trojan_transport", "ws")
o.datatype = "hostname"
o.rmempty = true

-- WS路径
o = s:option(Value, "ws_path", translate("WebSocket Path"))
o:depends("transport", "ws")
o:depends("trojan_transport", "h2+ws")
o:depends("trojan_transport", "ws")
o.rmempty = true

-- [[ H2部分 ]]--

-- H2域名
o = s:option(Value, "h2_host", translate("HTTP/2 Host"))
o:depends("transport", "h2")
o.rmempty = true

-- H2路径
o = s:option(Value, "h2_path", translate("HTTP/2 Path"))
o:depends("transport", "h2")
o.rmempty = true

-- gRPC
o = s:option(Value, "serviceName", translate("serviceName"))
o:depends("transport", "grpc")
o.rmempty = true

-- [[ QUIC部分 ]]--
o = s:option(ListValue, "quic_security", translate("QUIC Security"))
o:depends("transport", "quic")
o:value("none", translate("None"))
o:value("aes-128-gcm", translate("aes-128-gcm"))
o:value("chacha20-poly1305", translate("chacha20-poly1305"))
o.rmempty = true

o = s:option(Value, "quic_key", translate("QUIC Key"))
o:depends("transport", "quic")
o.rmempty = true

o = s:option(ListValue, "quic_guise", translate("Header"))
o:depends("transport", "quic")
o.rmempty = true
o:value("none", translate("None"))
o:value("srtp", translate("VideoCall (SRTP)"))
o:value("utp", translate("BitTorrent (uTP)"))
o:value("wechat-video", translate("WechatVideo"))
o:value("dtls", translate("DTLS 1.2"))
o:value("wireguard", translate("WireGuard"))

-- [[ mKCP部分 ]]--
o = s:option(ListValue, "kcp_guise", translate("Camouflage Type"))
o:depends("transport", "kcp")
o:value("none", translate("None"))
o:value("srtp", translate("VideoCall (SRTP)"))
o:value("utp", translate("BitTorrent (uTP)"))
o:value("wechat-video", translate("WechatVideo"))
o:value("dtls", translate("DTLS 1.2"))
o:value("wireguard", translate("WireGuard"))
o.rmempty = true

o = s:option(Value, "mtu", translate("MTU"))
o.datatype = "uinteger"
o:depends("transport", "kcp")
o.default = 1350
o.rmempty = true

o = s:option(Value, "tti", translate("TTI"))
o.datatype = "uinteger"
o:depends("transport", "kcp")
o.default = 50
o.rmempty = true

o = s:option(Value, "uplink_capacity", translate("Uplink Capacity"))
o.datatype = "uinteger"
o:depends("transport", "kcp")
o.default = 5
o.rmempty = true

o = s:option(Value, "downlink_capacity", translate("Downlink Capacity"))
o.datatype = "uinteger"
o:depends("transport", "kcp")
o.default = 20
o.rmempty = true

o = s:option(Value, "read_buffer_size", translate("Read Buffer Size"))
o.datatype = "uinteger"
o:depends("transport", "kcp")
o.default = 2
o.rmempty = true

o = s:option(Value, "write_buffer_size", translate("Write Buffer Size"))
o.datatype = "uinteger"
o:depends("transport", "kcp")
o.default = 2
o.rmempty = true

o = s:option(Value, "seed", translate("Obfuscate password (optional)"))
o:depends({v2ray_protocol = "vless", transport = "kcp"})
o.rmempty = true

o = s:option(Flag, "congestion", translate("Congestion"))
o:depends("transport", "kcp")
o.rmempty = true

o = s:option(ListValue, "plugin_type", translate("Plugin Type"))
o:value("plaintext", translate("Plain Text"))
o:value("shadowsocks", translate("ShadowSocks"))
o:value("other", translate("Other"))
o.default = "plaintext"
o:depends({tls = false, trojan_transport = "original"})

o = s:option(Value, "plugin_cmd", translate("Plugin Binary"))
o.placeholder = "eg: /usr/bin/v2ray-plugin"
o:depends({plugin_type = "shadowsocks"})
o:depends({plugin_type = "other"})

o = s:option(Value, "plugin_option", translate("Plugin Option"))
o.placeholder = "eg: obfs=http;obfs-host=www.baidu.com"
o:depends({plugin_type = "shadowsocks"})
o:depends({plugin_type = "other"})

o = s:option(DynamicList, "plugin_arg", translate("Plugin Option Args"))
o.placeholder = "eg: [\"-config\", \"test.json\"]"
o:depends({plugin_type = "shadowsocks"})
o:depends({plugin_type = "other"})

-- [[ TLS ]]--
o = s:option(Flag, "tls", translate("TLS"))
o.rmempty = true
o.default = "0"
o:depends({type = "v2ray", xtls = false})
-- o:depends({type = "v2ray", v2ray_protocol = "vless", xtls = false})
o:depends("type", "trojan")

-- XTLS
if is_finded("xray") then
	o = s:option(Flag, "xtls", translate("XTLS"))
	o.rmempty = true
	o.default = "0"
	o:depends({type = "v2ray", v2ray_protocol = "vless", transport = "tcp", tls = false})
	o:depends({type = "v2ray", v2ray_protocol = "vless", transport = "kcp", tls = false})
	o:depends({type = "v2ray", v2ray_protocol = "trojan", transport = "tcp", tls = false})
	o:depends({type = "v2ray", v2ray_protocol = "trojan", transport = "kcp", tls = false})
end

-- Flow
o = s:option(Value, "vless_flow", translate("Flow"))
for _, v in ipairs(flows) do
	o:value(v, translate(v))
end
o.rmempty = true
o.default = "xtls-rprx-splice"
o:depends("xtls", true)

-- [[ TLS部分 ]] --
o = s:option(Flag, "tls_sessionTicket", translate("Session Ticket"))
o:depends({type = "trojan", tls = true})
o.default = "0"

-- [[ uTLS ]]--
o = s:option(ListValue, "fingerprint", translate("Finger Print"))
o:value("disable", translate("disable"))
o:value("firefox", translate("firefox"))
o:value("chrome", translate("chrome"))
o:value("safari", translate("safari"))
o:value("randomized", translate("randomized"))
o:depends({type = "v2ray", tls = true})
o.default = "disable"

o = s:option(Value, "tls_host", translate("TLS Host"))
o.datatype = "hostname"
o:depends("tls", true)
o:depends("xtls", true)
o.rmempty = true

-- [[ allowInsecure ]]--
o = s:option(Flag, "insecure", translate("allowInsecure"))
o.rmempty = false
o:depends("tls", true)
o:depends("xtls", true)
o.description = translate("If true, allowss insecure connection at TLS client, e.g., TLS server uses unverifiable certificates.")

-- [[ Mux ]]--
o = s:option(Flag, "mux", translate("Mux"))
o.rmempty = false
o:depends({type = "v2ray", xtls = false})

o = s:option(Value, "concurrency", translate("Concurrency"))
o.datatype = "uinteger"
o.rmempty = true
o.default = "8"
o:depends("mux", "1")

-- [[ Cert ]]--
o = s:option(Flag, "certificate", translate("Self-signed Certificate"))
o.rmempty = true
o.default = "0"
o:depends({type = "trojan", tls = true, insecure = false})
o:depends({type = "v2ray", v2ray_protocol = "vmess", tls = true, insecure = false})
o:depends({type = "v2ray", v2ray_protocol = "vless", tls = true, insecure = false})
o:depends({type = "v2ray", v2ray_protocol = "vmess", xtls = true, insecure = false})
o:depends({type = "v2ray", v2ray_protocol = "vless", xtls = true, insecure = false})
o.description = translate("If you have a self-signed certificate,please check the box")

o = s:option(DummyValue, "upload", translate("Upload"))
o.template = "shadowsocksr/certupload"
o:depends("certificate", 1)

cert_dir = "/etc/ssl/private/"
local path

luci.http.setfilehandler(function(meta, chunk, eof)
	if not fd then
		if (not meta) or (not meta.name) or (not meta.file) then
			return
		end
		fd = nixio.open(cert_dir .. meta.file, "w")
		if not fd then
			path = translate("Create upload file error.")
			return
		end
	end
	if chunk and fd then
		fd:write(chunk)
	end
	if eof and fd then
		fd:close()
		fd = nil
		path = '/etc/ssl/private/' .. meta.file .. ''
	end
end)
if luci.http.formvalue("upload") then
	local f = luci.http.formvalue("ulfile")
	if #f <= 0 then
		path = translate("No specify upload file.")
	end
end

o = s:option(Value, "certpath", translate("Current Certificate Path"))
o:depends("certificate", 1)
o:value("/etc/ssl/private/ca.pem")
o.description = translate("Please confirm the current certificate path")
o.default = "/etc/ssl/private/ca.pem"

o = s:option(Flag, "fast_open", translate("TCP Fast Open"))
o.rmempty = true
o.default = "0"
o:depends("type", "ssr")
o:depends("type", "ss")
o:depends("type", "trojan")

o = s:option(Flag, "switch_enable", translate("Enable Auto Switch"))
o.rmempty = false
o.default = "1"

o = s:option(Value, "local_port", translate("Local Port"))
o.datatype = "port"
o.default = 1234
o.rmempty = false

if is_finded("kcptun-client") then
	kcp_enable = s:option(Flag, "kcp_enable", translate("KcpTun Enable"))
	kcp_enable.rmempty = true
	kcp_enable.default = "0"
	kcp_enable:depends("type", "ssr")
	kcp_enable:depends("type", "ss")

	o = s:option(Value, "kcp_port", translate("KcpTun Port"))
	o.datatype = "port"
	o.default = 4000
	o:depends("type", "ssr")
	o:depends("type", "ss")

	o = s:option(Value, "kcp_password", translate("KcpTun Password"))
	o.password = true
	o:depends("type", "ssr")
	o:depends("type", "ss")

	o = s:option(Value, "kcp_param", translate("KcpTun Param"))
	o.default = "--nocomp"
	o:depends("type", "ssr")
	o:depends("type", "ss")
end

return m
