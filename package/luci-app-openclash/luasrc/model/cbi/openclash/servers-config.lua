
local m, s, o
local openclash = "openclash"
local uci = luci.model.uci.cursor()
local fs = require "luci.openclash"
local sys = require "luci.sys"
local sid = arg[1]
local uuid = luci.sys.exec("cat /proc/sys/kernel/random/uuid")

font_red = [[<b style=color:red>]]
font_off = [[</b>]]
bold_on  = [[<strong>]]
bold_off = [[</strong>]]

function IsYamlFile(e)
   e=e or""
   local e=string.lower(string.sub(e,-5,-1))
   return e == ".yaml"
end
function IsYmlFile(e)
   e=e or""
   local e=string.lower(string.sub(e,-4,-1))
   return e == ".yml"
end

local encrypt_methods_ss = {

	-- stream
	"rc4-md5",
	"aes-128-cfb",
	"aes-192-cfb",
	"aes-256-cfb",
	"aes-128-ctr",
	"aes-192-ctr",
	"aes-256-ctr",
	"aes-128-gcm",
	"aes-192-gcm",
	"aes-256-gcm",
	"chacha20-ietf",
	"xchacha20",
	"chacha20-ietf-poly1305",
	"xchacha20-ietf-poly1305",
	"2022-blake3-aes-128-gcm",
	"2022-blake3-aes-256-gcm",
	"2022-blake3-chacha20-poly1305"
}

local encrypt_methods_ssr = {

	"rc4-md5",
	"aes-128-cfb",
	"aes-192-cfb",
	"aes-256-cfb",
	"aes-128-ctr",
	"aes-192-ctr",
	"aes-256-ctr",
	"chacha20-ietf",
	"xchacha20"
}

local securitys = {
	"auto",
	"none",
	"zero",
	"aes-128-gcm",
	"chacha20-poly1305"
}

local protocols = {
	"origin",
	"auth_sha1_v4",
	"auth_aes128_md5",
	"auth_aes128_sha1",
	"auth_chain_a",
	"auth_chain_b"
}

local hysteria_protocols = {
	"udp",
	"wechat-video",
	"faketcp"
}

local obfs = {
	"plain",
	"http_simple",
	"http_post",
	"random_head",
	"tls1.2_ticket_auth",
	"tls1.2_ticket_fastauth"
}

m = Map(openclash, translate("Edit Server"))
m.pageaction = false
m.redirect = luci.dispatcher.build_url("admin/services/openclash/servers")

if m.uci:get(openclash, sid) ~= "servers" then
	luci.http.redirect(m.redirect)
	return
end

-- [[ Servers Setting ]] --
s = m:section(NamedSection, sid, "servers")
s.anonymous = true
s.addremove   = false

o = s:option(DummyValue, "server_url", "SS/SSR/VMESS/TROJAN URL")
o.rawhtml = true
o.template = "openclash/server_url"
o.value = sid

o = s:option(ListValue, "config", translate("Config File"))
o:value("all", translate("Use For All Config File"))
local e,a={}
for t,f in ipairs(fs.glob("/etc/openclash/config/*"))do
	a=fs.stat(f)
	if a then
    e[t]={}
    e[t].name=fs.basename(f)
    if IsYamlFile(e[t].name) or IsYmlFile(e[t].name) then
       o:value(e[t].name)
    end
  end
end

o = s:option(Flag, "manual", translate("Custom Tag"))
o.rmempty = false
o.default = "0"
o.description = translate("Mark as Custom Node to Prevent Retention config from being Deleted When Enabled")

o = s:option(ListValue, "type", translate("Server Node Type"))
o:value("ss", translate("Shadowsocks"))
o:value("ssr", translate("ShadowsocksR"))
o:value("vmess", translate("Vmess"))
o:value("trojan", translate("trojan"))
o:value("vless", translate("Vless ")..translate("(Only Meta Core)"))
o:value("hysteria", translate("Hysteria ")..translate("(Only Meta Core)"))
o:value("wireguard", translate("WireGuard")..translate("(TUN&Meta Core)"))
o:value("tuic", translate("Tuic")..translate("(Only Meta Core)"))
o:value("snell", translate("Snell"))
o:value("socks5", translate("Socks5"))
o:value("http", translate("HTTP(S)"))

o.description = translate("Using incorrect encryption mothod may causes service fail to start")

o = s:option(Value, "name", translate("Server Alias"))
o.rmempty = false
o.default = "Server - "..sid

o = s:option(Value, "server", translate("Server Address"))
o.datatype = "host"
o.rmempty = true

o = s:option(Value, "port", translate("Server Port"))
o.datatype = "port"
o.rmempty = false
o.default = "443"

o = s:option(Value, "ports", translate("Port Hopping"))
o.datatype = "portrange"
o.rmempty = true
o.default = "20000-40000"
o.placeholder = translate("20000-40000")
o:depends("type", "hysteria")

o = s:option(Value, "password", translate("Password"))
o.password = true
o.rmempty = false
o:depends("type", "ss")
o:depends("type", "ssr")
o:depends("type", "trojan")

-- [[ Tuic ]]--
o = s:option(Value, "tc_ip", translate("Server IP"))
o.rmempty = true
o.placeholder = translate("127.0.0.1")
o.datatype = "or(ip4addr, ip6addr)"
o:depends("type", "tuic")

o = s:option(Value, "tc_token", translate("Token"))
o.rmempty = true
o:depends("type", "tuic")

o = s:option(ListValue, "udp_relay_mode", translate("UDP Relay Mode"))
o.rmempty = true
o.default = "native"
o:value("native")
o:value("quic")
o:depends("type", "tuic")

o = s:option(ListValue, "congestion_controller", translate("Congestion Controller"))
o.rmempty = true
o.default = "cubic"
o:value("cubic")
o:value("bbr")
o:value("new_reno")
o:depends("type", "tuic")

o = s:option(DynamicList, "tc_alpn", translate("Alpn"))
o.rmempty = true
o:value("h3")
o:value("h2")
o:depends("type", "tuic")

o = s:option(ListValue, "disable_sni", translate("Disable SNI"))
o.rmempty = true
o.default = "true"
o:value("true")
o:value("false")
o:depends("type", "tuic")

o = s:option(ListValue, "reduce_rtt", translate("Reduce RTT"))
o.rmempty = true
o.default = "true"
o:value("true")
o:value("false")
o:depends("type", "tuic")

o = s:option(Value, "heartbeat_interval", translate("Heartbeat Interval"))
o.rmempty = true
o:depends("type", "tuic")
o.default = "8000"

o = s:option(Value, "request_timeout", translate("Request Timeout"))
o.rmempty = true
o.default = "8000"
o:depends("type", "tuic")

o = s:option(Value, "max_udp_relay_packet_size", translate("Max UDP Relay Packet Size"))
o.rmempty = true
o.default = "1500"
o:depends("type", "tuic")

o = s:option(Value, "max_open_streams", translate("Max Open Streams"))
o.rmempty = true
o.default = "100"
o:depends("type", "tuic")

-- [[ Wireguard ]]--
o = s:option(Value, "wg_ip", translate("IP"))
o.rmempty = true
o.placeholder = translate("127.0.0.1")
o.datatype = "ip4addr"
o:depends("type", "wireguard")

o = s:option(Value, "wg_ipv6", translate("IPv6"))
o.rmempty = true
o.placeholder = translate("your_ipv6")
o.datatype = "ip6addr"
o:depends("type", "wireguard")

o = s:option(Value, "private_key", translate("Private Key"))
o.rmempty = true
o.placeholder = translate("eCtXsJZ27+4PbhDkHnB923tkUn2Gj59wZw5wFA75MnU=")
o:depends("type", "wireguard")

o = s:option(Value, "public_key", translate("Public Key"))
o.rmempty = true
o.placeholder = translate("Cr8hWlKvtDt7nrvf+f0brNQQzabAqrjfBvas9pmowjo=")
o:depends("type", "wireguard")

o = s:option(Value, "preshared_key", translate("Preshared Key"))
o.rmempty = true
o.placeholder = translate("base64")
o:depends("type", "wireguard")

o = s:option(DynamicList, "wg_dns", translate("DNS"))
o.rmempty = true
o:value("1.1.1.1")
o:value("8.8.8.8")
o:depends("type", "wireguard")

o = s:option(Value, "wg_mtu", translate("MTU"))
o.rmempty = true
o.default = "1420"
o.placeholder = translate("1420")
o:depends("type", "wireguard")

o = s:option(ListValue, "hysteria_protocol", translate("Protocol"))
for _, v in ipairs(hysteria_protocols) do o:value(v) end
o.rmempty = false
o:depends("type", "hysteria")

o = s:option(Value, "hysteria_up", translate("up"))
o.rmempty = false
o.description = translate("Required")
o:depends("type", "hysteria")

o = s:option(Value, "hysteria_down", translate("down"))
o.rmempty = false
o.description = translate("Required")
o:depends("type", "hysteria")

o = s:option(Value, "psk", translate("Psk"))
o.rmempty = true
o:depends("type", "snell")

o = s:option(ListValue, "snell_version", translate("Version"))
o:value("2")
o:value("3")
o:depends("type", "snell")

o = s:option(ListValue, "cipher", translate("Encrypt Method"))
for _, v in ipairs(encrypt_methods_ss) do o:value(v) end
o.description = translate("Only Meta Core Support SS2022")
o.rmempty = true
o:depends("type", "ss")

o = s:option(ListValue, "cipher_ssr", translate("Encrypt Method"))
for _, v in ipairs(encrypt_methods_ssr) do o:value(v) end
o:value("dummy", "none")
o.rmempty = true
o:depends("type", "ssr")

o = s:option(ListValue, "protocol", translate("Protocol"))
for _, v in ipairs(protocols) do o:value(v) end
o.rmempty = true
o:depends("type", "ssr")

o = s:option(Value, "protocol_param", translate("Protocol param(optional)"))
o:depends("type", "ssr")

o = s:option(ListValue, "securitys", translate("Encrypt Method"))
for _, v in ipairs(securitys) do o:value(v) end
o.rmempty = true
o:depends("type", "vmess")

o = s:option(ListValue, "obfs_ssr", translate("Obfs"))
for _, v in ipairs(obfs) do o:value(v) end
o.rmempty = true
o:depends("type", "ssr")

o = s:option(Value, "obfs_param", translate("Obfs param(optional)"))
o:depends("type", "ssr")

-- AlterId
o = s:option(Value, "alterId", translate("AlterId"))
o.default = "32"
o.rmempty = true
o:depends("type", "vmess")

-- VmessId
o = s:option(Value, "uuid", translate("UUID"))
o.rmempty = true
o.default = uuid
o:depends("type", "vmess")
o:depends("type", "vless")

o = s:option(ListValue, "udp", translate("UDP Enable"))
o.rmempty = true
o.default = "true"
o:value("true")
o:value("false")
o:depends("type", "ss")
o:depends("type", "ssr")
o:depends("type", "vmess")
o:depends("type", "vless")
o:depends("type", "socks5")
o:depends("type", "trojan")
o:depends({type = "snell", snell_version = "3"})
o:depends("type", "wireguard")

o = s:option(ListValue, "udp_over_tcp", translate("udp-over-tcp"))
o.rmempty = true
o.default = "false"
o:value("true")
o:value("false")
o:depends("type", "ss")

o = s:option(ListValue, "xudp", translate("XUDP Enable")..translate("(Only Meta Core)"))
o.rmempty = true
o.default = "true"
o:value("true")
o:value("false")
o:depends({type = "vmess", udp = "true"})
o:depends({type = "vless", udp = "true"})

o = s:option(ListValue, "obfs", translate("obfs-mode"))
o.rmempty = true
o.default = "none"
o:value("none")
o:value("tls")
o:value("http")
o:value("websocket", translate("websocket (ws)"))
o:value("shadow-tls", translate("shadow-tls")..translate("(Only Meta Core)"))
o:value("restls", translate("restls")..translate("(Only Meta Core)"))
o:depends("type", "ss")

o = s:option(ListValue, "obfs_snell", translate("obfs-mode"))
o.rmempty = true
o.default = "none"
o:value("none")
o:value("tls")
o:value("http")
o:depends("type", "snell")

o = s:option(ListValue, "obfs_vless", translate("obfs-mode"))
o.rmempty = true
o.default = "tcp"
o:value("tcp", translate("tcp"))
o:value("ws", translate("websocket (ws)"))
o:value("grpc", translate("grpc"))
o:depends("type", "vless")

o = s:option(ListValue, "obfs_vmess", translate("obfs-mode"))
o.rmempty = true
o.default = "none"
o:value("none")
o:value("websocket", translate("websocket (ws)"))
o:value("http", translate("http"))
o:value("h2", translate("h2"))
o:value("grpc", translate("grpc"))
o:depends("type", "vmess")

o = s:option(ListValue, "obfs_trojan", translate("obfs-mode"))
o.rmempty = true
o.default = "none"
o:value("none")
o:value("ws", translate("websocket (ws)"))
o:value("grpc", translate("grpc"))
o:depends("type", "trojan")

o = s:option(Value, "host", translate("obfs-hosts"))
o.datatype = "host"
o.placeholder = translate("example.com")
o.rmempty = true
o:depends("obfs", "tls")
o:depends("obfs", "http")
o:depends("obfs", "websocket")
o:depends("obfs", "shadow-tls")
o:depends("obfs", "restls")
o:depends("obfs_snell", "tls")
o:depends("obfs_snell", "http")

o = s:option(Value, "obfs_password", translate("obfs-password"))
o.rmempty = true
o:depends("obfs", "shadow-tls")
o:depends("obfs", "restls")

o = s:option(ListValue, "obfs_version_hint", translate("version-hint"))
o.rmempty = true
o:value("tls13")
o:value("tls12")
o:depends("obfs", "restls")

o = s:option(Value, "obfs_restls_script", translate("restls-script"))
o.rmempty = true
o:depends("obfs", "restls")
o.placeholder = translate("1000?100<1,500~100,350~100,600~100,400~200")

-- vmess路径
o = s:option(Value, "path", translate("path"))
o.rmempty = true
o.placeholder = translate("/")
o:depends("obfs", "websocket")

o = s:option(DynamicList, "h2_host", translate("host"))
o.rmempty = true
o.placeholder = translate("http.example.com")
o.datatype = "host"
o:depends("obfs_vmess", "h2")

o = s:option(Value, "h2_path", translate("path"))
o.rmempty = true
o.default = "/"
o:depends("obfs_vmess", "h2")

o = s:option(DynamicList, "http_path", translate("path"))
o.rmempty = true
o:value("/")
o:value("/video")
o:depends("obfs_vmess", "http")

o = s:option(Value, "custom", translate("headers"))
o.rmempty = true
o.placeholder = translate("v2ray.com")
o:depends("obfs", "websocket")

o = s:option(Value, "ws_opts_path", translate("ws-opts-path"))
o.rmempty = true
o.placeholder = translate("/path")
o:depends("obfs_vmess", "websocket")
o:depends("obfs_vless", "ws")

o = s:option(DynamicList, "ws_opts_headers", translate("ws-opts-headers"))
o.rmempty = true
o.placeholder = translate("Host: v2ray.com")
o:depends("obfs_vmess", "websocket")
o:depends("obfs_vless", "ws")

o = s:option(Value, "vless_flow", translate("flow"))
o.rmempty = true
o.default = "xtls-rprx-direct"
o:value("xtls-rprx-direct")
o:value("xtls-rprx-origin")
o:value("xtls-rprx-vision")
o:depends("obfs_vless", "tcp")

-- [[ grpc ]]--
o = s:option(Value, "grpc_service_name", translate("grpc-service-name"))
o.rmempty = true
o.datatype = "host"
o.placeholder = translate("example")
o:depends("obfs_trojan", "grpc")
o:depends("obfs_vmess", "grpc")
o:depends("obfs_vless", "grpc")

-- [[ reality-public-key ]]--
o = s:option(Value, "reality_public_key", translate("public-key(reality)"))
o.rmempty = true
o.placeholder = translate("CrrQSjAG_YkHLwvM2M-7XkKJilgL5upBKCp0od0tLhE")
o:depends("obfs_vless", "grpc")
o:depends("obfs_vless", "tcp")

-- [[ reality-short-id ]]--
o = s:option(Value, "reality_short_id", translate("short-id(reality)"))
o.rmempty = true
o.placeholder = translate("10f897e26c4b9478")
o:depends("obfs_vless", "grpc")
o:depends("obfs_vless", "tcp")

o = s:option(Value, "max_early_data", translate("max-early-data"))
o.rmempty = true
o.placeholder = translate("2048")
o:depends("obfs_vmess", "websocket")

o = s:option(Value, "early_data_header_name", translate("early-data-header-name"))
o.rmempty = true
o.placeholder = translate("Sec-WebSocket-Protocol")
o:depends("obfs_vmess", "websocket")

-- [[ skip-cert-verify ]]--
o = s:option(ListValue, "skip_cert_verify", translate("skip-cert-verify"))
o.rmempty = true
o.default = "true"
o:value("true")
o:value("false")
o:depends("obfs", "websocket")
o:depends("obfs_vmess", "none")
o:depends("obfs_vmess", "websocket")
o:depends("obfs_vmess", "grpc")
o:depends("type", "socks5")
o:depends("type", "http")
o:depends("type", "trojan")
o:depends("type", "vless")
o:depends("type", "hysteria")
o:depends("type", "tuic")

-- [[ TLS ]]--
o = s:option(ListValue, "tls", translate("TLS"))
o.rmempty = true
o.default = "false"
o:value("true")
o:value("false")
o:depends("obfs", "websocket")
o:depends("type", "vmess")
o:depends("type", "vless")
o:depends("type", "socks5")
o:depends("type", "http")

o = s:option(Value, "servername", translate("servername"))
o.rmempty = true
o.datatype = "host"
o.placeholder = translate("example.com")
o:depends({obfs_vmess = "websocket", tls = "true"})
o:depends({obfs_vmess = "grpc", tls = "true"})
o:depends({obfs_vmess = "none", tls = "true"})
o:depends("type", "vless")

o = s:option(Value, "keep_alive", translate("keep-alive"))
o.rmempty = true
o.default = "true"
o:value("true")
o:value("false")
o:depends("obfs_vmess", "http")

-- [[ MUX ]]--
o = s:option(ListValue, "mux", translate("mux"))
o.rmempty = true
o.default = "false"
o:value("true")
o:value("false")
o:depends("obfs", "websocket")

-- [[ sni ]]--
o = s:option(Value, "sni", translate("sni"))
o.datatype = "host"
o.placeholder = translate("example.com")
o.rmempty = true
o:depends("type", "trojan")
o:depends("type", "http")
o:depends("type", "hysteria")

-- [[ headers ]]--
o = s:option(DynamicList, "http_headers", translate("headers"))
o.rmempty = true
o.placeholder = translate("User-Agent: okhttp/3.11.0 Dalvik/2.1.0 ...... ")
o:depends("type", "http")

-- 验证用户名
o = s:option(Value, "auth_name", translate("Auth Username"))
o:depends("type", "socks5")
o:depends("type", "http")
o.rmempty = true

-- 验证密码
o = s:option(Value, "auth_pass", translate("Auth Password"))
o:depends("type", "socks5")
o:depends("type", "http")
o.rmempty = true

-- [[ alpn ]]--
o = s:option(DynamicList, "alpn", translate("alpn"))
o.rmempty = true
o:value("h2")
o:value("http/1.1")
o:depends("type", "trojan")

-- [[ alpn ]]--
o = s:option(DynamicList, "hysteria_alpn", translate("alpn"))
o.rmempty = false
o:value("h3")
o:value("h2")
o:depends("type", "hysteria")

-- [[ trojan-ws-path ]]--
o = s:option(Value, "trojan_ws_path", translate("Path"))
o.rmempty = true
o.placeholder = translate("/path")
o:depends("obfs_trojan", "ws")

-- [[ trojan-ws-headers ]]--
o = s:option(DynamicList, "trojan_ws_headers", translate("Headers"))
o.rmempty = true
o.placeholder = translate("Host: v2ray.com")
o:depends("obfs_trojan", "ws")

-- [[ hysteria_obfs ]]--
o = s:option(Value, "hysteria_obfs", translate("obfs"))
o.rmempty = true
o.placeholder = translate("yourpassword")
o:depends("type", "hysteria")

-- [[ hysteria_auth ]]--
--o = s:option(Value, "hysteria_auth", translate("auth"))
--o.rmempty = true
--o.placeholder = translate("[BASE64]")
--o:depends("type", "hysteria")

-- [[ hysteria_auth_str ]]--
o = s:option(Value, "hysteria_auth_str", translate("auth_str"))
o.rmempty = true
o.placeholder = translate("yubiyubi")
o:depends("type", "hysteria")

-- [[ hysteria_ca ]]--
o = s:option(Value, "hysteria_ca", translate("ca"))
o.rmempty = true
o.placeholder = translate("./my.ca")
o:depends("type", "hysteria")

-- [[ hysteria_ca_str ]]--
o = s:option(Value, "hysteria_ca_str", translate("ca_str"))
o.rmempty = true
o.placeholder = translate("xyz")
o:depends("type", "hysteria")

-- [[ recv_window_conn ]]--
o = s:option(Value, "recv_window_conn", translate("recv_window_conn"))
o.rmempty = true
o.placeholder = translate("QUIC stream receive window")
o.datatype = "uinteger"
o:depends("type", "hysteria")

-- [[ recv_window ]]--
o = s:option(Value, "recv_window", translate("recv_window"))
o.rmempty = true
o.placeholder = translate("QUIC connection receive window")
o.datatype = "uinteger"
o:depends("type", "hysteria")

-- [[ disable_mtu_discovery ]]--
o = s:option(ListValue, "disable_mtu_discovery", translate("disable_mtu_discovery"))
o.rmempty = true
o:value("true")
o:value("false")
o.default = "false"
o:depends("type", "hysteria")

-- [[ hop_interval ]]--
o = s:option(Value, "hop_interval", translate("Hop Interval"))
o.rmempty = true
o.default = "10"
o:depends("type", "hysteria")

o = s:option(ListValue, "packet-addr", translate("Packet-Addr")..translate("(Only Meta Core)"))
o.rmempty = true
o.default = "true"
o:value("true")
o:value("false")
o:depends({type = "vless", xudp = "false"})

o = s:option(Value, "packet_encoding", translate("Packet-Encoding")..translate("(Only Meta Core)"))
o.rmempty = true
o:depends("type", "vmess")
o:depends("type", "vless")

o = s:option(ListValue, "global_padding", translate("Global-Padding")..translate("(Only Meta Core)"))
o.rmempty = true
o.default = "false"
o:value("true")
o:value("false")
o:depends("type", "vmess")

o = s:option(ListValue, "authenticated_length", translate("Authenticated-Length")..translate("(Only Meta Core)"))
o.rmempty = true
o.default = "false"
o:value("true")
o:value("false")
o:depends("type", "vmess")

-- [[ TFO ]]--
o = s:option(ListValue, "fast_open", translate("Fast Open"))
o.rmempty = true
o.default = "true"
o:value("true")
o:value("false")
o:depends("type", "hysteria")
o:depends("type", "tuic")

-- [[ TFO ]]--
o = s:option(ListValue, "tfo", translate("TFO")..translate("(Only Meta Core)"))
o.rmempty = true
o.default = "true"
o:value("true")
o:value("false")
o:depends("type", "http")
o:depends("type", "socks5")
o:depends("type", "trojan")
o:depends("type", "vless")
o:depends("type", "vmess")
o:depends("type", "ss")
o:depends("type", "ssr")
o:depends("type", "snell")

-- [[ fingerprint ]]--
o = s:option(Value, "fingerprint", translate("Fingerprint")..translate("(Only Meta Core)"))
o.rmempty = true
o:depends("type", "hysteria")
o:depends("type", "socks5")
o:depends("type", "http")
o:depends("type", "trojan")
o:depends("type", "vless")
o:depends({type = "ss", obfs = "websocket"})
o:depends({type = "ss", obfs = "shadow-tls"})
o:depends({type = "vmess", obfs_vmess = "websocket"})
o:depends({type = "vmess", obfs_vmess = "h2"})
o:depends({type = "vmess", obfs_vmess = "grpc"})

-- [[ client-fingerprint ]]--
o = s:option(ListValue, "client_fingerprint", translate("Client Fingerprint")..translate("(Only Meta Core)"))
o.rmempty = true
o:value("none")
o:value("chrome")
o:value("firefox")
o:value("safari")
o:value("ios")
o.default = "none"
o:depends("type", "vless")
o:depends({type = "ss", obfs = "restls"})
o:depends({type = "ss", obfs = "shadow-tls"})
o:depends({type = "trojan", obfs_vmess = "grpc"})
o:depends({type = "vmess", obfs_vmess = "websocket"})
o:depends({type = "vmess", obfs_vmess = "http"})
o:depends({type = "vmess", obfs_vmess = "h2"})
o:depends({type = "vmess", obfs_vmess = "grpc"})

-- [[ ip version ]]--
o = s:option(ListValue, "ip_version", translate("IP Version")..translate("(Only Meta Core)"))
o.rmempty = true
o:value("dual")
o:value("ipv4")
o:value("ipv4-prefer")
o:value("ipv6")
o:value("ipv6-prefer")
o.default = "dual"

-- [[ smux ]]--
o = s:option(ListValue, "multiplex", translate("Multiplex")..translate("(Only Meta Core)"))
o.rmempty = false
o:value("true")
o:value("false")
o.default = "false"

o = s:option(ListValue, "multiplex_protocol", translate("Protocol"))
o.rmempty = true
o:value("smux")
o:value("yamux")
o:value("h2mux")
o.default = "smux"
o:depends("multiplex", "true")

o = s:option(Value, "multiplex_max_connections", translate("Max-connections"))
o.rmempty = true
o.placeholder = "4"
o.default = "4"
o.datatype = "uinteger"
o:depends("multiplex", "true")

o = s:option(Value, "multiplex_min_streams", translate("Min-streams"))
o.rmempty = true
o.placeholder = "4"
o.default = "4"
o.datatype = "uinteger"
o:depends("multiplex", "true")

o = s:option(Value, "multiplex_max_streams", translate("Max-streams"))
o.rmempty = true
o.placeholder = "0"
o.default = "0"
o.datatype = "uinteger"
o:depends("multiplex", "true")

o = s:option(ListValue, "multiplex_padding", translate("Padding"))
o.rmempty = false
o:value("true")
o:value("false")
o.default = "false"
o:depends("multiplex", "true")

o = s:option(ListValue, "multiplex_statistic", translate("Statistic"))
o.rmempty = false
o:value("true")
o:value("false")
o.default = "false"
o:depends("multiplex", "true")

o = s:option(ListValue, "multiplex_only_tcp", translate("Only-tcp"))
o.rmempty = false
o:value("true")
o:value("false")
o.default = "false"
o:depends("multiplex", "true")

-- [[ interface-name ]]--
o = s:option(Value, "interface_name", translate("interface-name"))
o.rmempty = true
o.placeholder = translate("eth0")

-- [[ routing-mark ]]--
o = s:option(Value, "routing_mark", translate("routing-mark"))
o.rmempty = true
o.placeholder = translate("2333")

-- [[ other-setting ]]--
o = s:option(Value, "other_parameters", translate("Other Parameters"))
o.template = "cbi/tvalue"
o.rows = 20
o.wrap = "off"
o.description = font_red..bold_on..translate("Edit Your Other Parameters Here")..bold_off..font_off
o.rmempty = true
function o.cfgvalue(self, section)
	if self.map:get(section, "other_parameters") == nil then
		return "# Example:\n"..
		"# Only support YAML, four spaces need to be reserved at the beginning of each line to maintain formatting alignment\n"..
		"# 示例：\n"..
		"# 仅支持 YAML, 每行行首需要多保留四个空格以使脚本处理后能够与上方配置保持格式对齐\n"..
		"#    type: ss\n"..
		"#    server: \"127.0.0.1\"\n"..
		"#    port: 443\n"..
		"#    cipher: rc4-md5\n"..
		"#    password: \"123456\"\n"..
		"#    udp: true\n"..
		"#    udp-over-tcp: false\n"..
		"#    ip-version: \"dual\"\n"..
		"#    tfo: true\n"..
		"#    smux:\n"..
		"#      enabled: false\n"..
		"#    plugin-opts:\n"..
		"#      mode: tls\n"..
		"#      host: world.taobao.com"
	else
		return Value.cfgvalue(self, section)
	end
end
function o.validate(self, value)
	if value then
		value = value:gsub("\r\n?", "\n")
		value = value:gsub("%c*$", "")
	end
	return value
end

o = s:option(DynamicList, "groups", translate("Proxy Group (Support Regex)"))
o.description = font_red..bold_on..translate("No Need Set when Config Create, The added Proxy Groups Must Exist")..bold_off..font_off
o.rmempty = true
o:value("all", translate("All Groups"))
m.uci:foreach("openclash", "groups",
		function(s)
			if s.name ~= "" and s.name ~= nil then
			   o:value(s.name)
			end
		end)
		
local t = {
    {Commit, Back}
}
a = m:section(Table, t)

o = a:option(Button,"Commit", " ")
o.inputtitle = translate("Commit Settings")
o.inputstyle = "apply"
o.write = function()
   m.uci:commit(openclash)
   luci.http.redirect(m.redirect)
end

o = a:option(Button,"Back", " ")
o.inputtitle = translate("Back Settings")
o.inputstyle = "reset"
o.write = function()
   m.uci:revert(openclash, sid)
   luci.http.redirect(m.redirect)
end

m:append(Template("openclash/toolbar_show"))
return m
