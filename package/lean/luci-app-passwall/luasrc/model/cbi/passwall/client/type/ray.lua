local m, s = ...

local api = require "luci.passwall.api"

if not api.finded_com("xray") then
	return
end

local appname = api.appname
local uci = api.uci

local type_name = "Xray"

local option_prefix = "xray_"

local function option_name(name)
	return option_prefix .. name
end

local x_ss_encrypt_method_list = {
	"aes-128-gcm", "aes-256-gcm", "chacha20-poly1305", "xchacha20-poly1305", "2022-blake3-aes-128-gcm", "2022-blake3-aes-256-gcm", "2022-blake3-chacha20-poly1305"
}

local security_list = { "none", "auto", "aes-128-gcm", "chacha20-poly1305", "zero" }

local header_type_list = {
	"none", "srtp", "utp", "wechat-video", "dtls", "wireguard"
}

-- [[ Xray ]]

s.fields["type"]:value(type_name, "Xray")

o = s:option(ListValue, option_name("protocol"), translate("Protocol"))
o:value("vmess", translate("Vmess"))
o:value("vless", translate("VLESS"))
o:value("http", translate("HTTP"))
o:value("socks", translate("Socks"))
o:value("shadowsocks", translate("Shadowsocks"))
o:value("trojan", translate("Trojan"))
o:value("wireguard", translate("WireGuard"))
o:value("_balancing", translate("Balancing"))
o:value("_shunt", translate("Shunt"))
o:value("_iface", translate("Custom Interface") .. " (Only Support Xray)")

o = s:option(Value, option_name("iface"), translate("Interface"))
o.default = "eth1"
o:depends({ [option_name("protocol")] = "_iface" })

local nodes_table = {}
local balancers_table = {}
local iface_table = {}
for k, e in ipairs(api.get_valid_nodes()) do
	if e.node_type == "normal" then
		nodes_table[#nodes_table + 1] = {
			id = e[".name"],
			remarks = e["remark"]
		}
	end
	if e.protocol == "_balancing" then
		balancers_table[#balancers_table + 1] = {
			id = e[".name"],
			remarks = e["remark"]
		}
	end
	if e.protocol == "_iface" then
		iface_table[#iface_table + 1] = {
			id = e[".name"],
			remarks = e["remark"]
		}
	end
end

-- 负载均衡列表
local o = s:option(DynamicList, option_name("balancing_node"), translate("Load balancing node list"), translate("Load balancing node list, <a target='_blank' href='https://toutyrater.github.io/routing/balance2.html'>document</a>"))
o:depends({ [option_name("protocol")] = "_balancing" })
for k, v in pairs(nodes_table) do o:value(v.id, v.remarks) end

local o = s:option(ListValue, option_name("balancingStrategy"), translate("Balancing Strategy"))
o:depends({ [option_name("protocol")] = "_balancing" })
o:value("random")
o:value("leastPing")
o.default = "random"

-- 探测地址
local o = s:option(Flag, option_name("useCustomProbeUrl"), translate("Use Custome Probe URL"), translate("By default the built-in probe URL will be used, enable this option to use a custom probe URL."))
o:depends({ [option_name("balancingStrategy")] = "leastPing" })

local o = s:option(Value, option_name("probeUrl"), translate("Probe URL"))
o:depends({ [option_name("useCustomProbeUrl")] = true })
o.default = "https://www.google.com/generate_204"
o.description = translate("The URL used to detect the connection status.")

-- 探测间隔
local o = s:option(Value, option_name("probeInterval"), translate("Probe Interval"))
o:depends({ [option_name("balancingStrategy")] = "leastPing" })
o.default = "1m"
o.description = translate("The interval between initiating probes. Every time this time elapses, a server status check is performed on a server. The time format is numbers + units, such as '10s', '2h45m', and the supported time units are <code>ns</code>, <code>us</code>, <code>ms</code>, <code>s</code>, <code>m</code>, <code>h</code>, which correspond to nanoseconds, microseconds, milliseconds, seconds, minutes, and hours, respectively.")

-- [[ 分流模块 ]]
if #nodes_table > 0 then
	o = s:option(Flag, option_name("preproxy_enabled"), translate("Preproxy"))
	o:depends({ [option_name("protocol")] = "_shunt" })

	o = s:option(Value, option_name("main_node"), string.format('<a style="color:red">%s</a>', translate("Preproxy Node")), translate("Set the node to be used as a pre-proxy. Each rule (including <code>Default</code>) has a separate switch that controls whether this rule uses the pre-proxy or not."))
	o:depends({ [option_name("protocol")] = "_shunt", [option_name("preproxy_enabled")] = true })
	for k, v in pairs(balancers_table) do
		o:value(v.id, v.remarks)
	end
	for k, v in pairs(iface_table) do
		o:value(v.id, v.remarks)
	end
	for k, v in pairs(nodes_table) do
		o:value(v.id, v.remarks)
	end
	o.default = "nil"
end
uci:foreach(appname, "shunt_rules", function(e)
	if e[".name"] and e.remarks then
		o = s:option(Value, option_name(e[".name"]), string.format('* <a href="%s" target="_blank">%s</a>', api.url("shunt_rules", e[".name"]), e.remarks))
		o:value("nil", translate("Close"))
		o:value("_default", translate("Default"))
		o:value("_direct", translate("Direct Connection"))
		o:value("_blackhole", translate("Blackhole"))
		o:depends({ [option_name("protocol")] = "_shunt" })

		if #nodes_table > 0 then
			for k, v in pairs(balancers_table) do
				o:value(v.id, v.remarks)
			end
			for k, v in pairs(iface_table) do
				o:value(v.id, v.remarks)
			end
			local pt = s:option(ListValue, option_name(e[".name"] .. "_proxy_tag"), string.format('* <a style="color:red">%s</a>', e.remarks .. " " .. translate("Preproxy")))
			pt:value("nil", translate("Close"))
			pt:value("main", translate("Preproxy Node"))
			pt.default = "nil"
			for k, v in pairs(nodes_table) do
				o:value(v.id, v.remarks)
				pt:depends({ [option_name("protocol")] = "_shunt", [option_name("preproxy_enabled")] = true, [option_name(e[".name"])] = v.id })
			end
		end
	end
end)

o = s:option(DummyValue, option_name("shunt_tips"), " ")
o.not_rewrite = true
o.rawhtml = true
o.cfgvalue = function(t, n)
	return string.format('<a style="color: red" href="../rule">%s</a>', translate("No shunt rules? Click me to go to add."))
end
o:depends({ [option_name("protocol")] = "_shunt" })

local o = s:option(Value, option_name("default_node"), string.format('* <a style="color:red">%s</a>', translate("Default")))
o:depends({ [option_name("protocol")] = "_shunt" })
o:value("_direct", translate("Direct Connection"))
o:value("_blackhole", translate("Blackhole"))

if #nodes_table > 0 then
	for k, v in pairs(balancers_table) do
		o:value(v.id, v.remarks)
	end
	for k, v in pairs(iface_table) do
		o:value(v.id, v.remarks)
	end
	local dpt = s:option(ListValue, option_name("default_proxy_tag"), string.format('* <a style="color:red">%s</a>', translate("Default Preproxy")), translate("When using, localhost will connect this node first and then use this node to connect the default node."))
	dpt:value("nil", translate("Close"))
	dpt:value("main", translate("Preproxy Node"))
	dpt.default = "nil"
	for k, v in pairs(nodes_table) do
		o:value(v.id, v.remarks)
		dpt:depends({ [option_name("protocol")] = "_shunt", [option_name("preproxy_enabled")] = true, [option_name("default_node")] = v.id })
	end
end

o = s:option(ListValue, option_name("domainStrategy"), translate("Domain Strategy"))
o:value("AsIs")
o:value("IPIfNonMatch")
o:value("IPOnDemand")
o.default = "IPOnDemand"
o.description = "<br /><ul><li>" .. translate("'AsIs': Only use domain for routing. Default value.")
	.. "</li><li>" .. translate("'IPIfNonMatch': When no rule matches current domain, resolves it into IP addresses (A or AAAA records) and try all rules again.")
	.. "</li><li>" .. translate("'IPOnDemand': As long as there is a IP-based rule, resolves the domain into IP immediately.")
	.. "</li></ul>"
o:depends({ [option_name("protocol")] = "_shunt" })

o = s:option(ListValue, option_name("domainMatcher"), translate("Domain matcher"))
o:value("hybrid")
o:value("linear")
o:depends({ [option_name("protocol")] = "_shunt" })

-- [[ 分流模块 End ]]

o = s:option(Value, option_name("address"), translate("Address (Support Domain Name)"))

o = s:option(Value, option_name("port"), translate("Port"))
o.datatype = "port"

local protocols = s.fields[option_name("protocol")].keylist
if #protocols > 0 then
	for index, value in ipairs(protocols) do
		if not value:find("_") then
			s.fields[option_name("address")]:depends({ [option_name("protocol")] = value })
			s.fields[option_name("port")]:depends({ [option_name("protocol")] = value })
		end
	end
end

o = s:option(Value, option_name("username"), translate("Username"))
o:depends({ [option_name("protocol")] = "http" })
o:depends({ [option_name("protocol")] = "socks" })

o = s:option(Value, option_name("password"), translate("Password"))
o.password = true
o:depends({ [option_name("protocol")] = "http" })
o:depends({ [option_name("protocol")] = "socks" })
o:depends({ [option_name("protocol")] = "shadowsocks" })
o:depends({ [option_name("protocol")] = "trojan" })

o = s:option(ListValue, option_name("security"), translate("Encrypt Method"))
for a, t in ipairs(security_list) do o:value(t) end
o:depends({ [option_name("protocol")] = "vmess" })

o = s:option(Value, option_name("encryption"), translate("Encrypt Method"))
o.default = "none"
o:value("none")
o:depends({ [option_name("protocol")] = "vless" })

o = s:option(ListValue, option_name("x_ss_encrypt_method"), translate("Encrypt Method"))
o.rewrite_option = "method"
for a, t in ipairs(x_ss_encrypt_method_list) do o:value(t) end
o:depends({ [option_name("protocol")] = "shadowsocks" })

o = s:option(Flag, option_name("iv_check"), translate("IV Check"))
o:depends({ [option_name("protocol")] = "shadowsocks", [option_name("x_ss_encrypt_method")] = "aes-128-gcm" })
o:depends({ [option_name("protocol")] = "shadowsocks", [option_name("x_ss_encrypt_method")] = "aes-256-gcm" })
o:depends({ [option_name("protocol")] = "shadowsocks", [option_name("x_ss_encrypt_method")] = "chacha20-poly1305" })
o:depends({ [option_name("protocol")] = "shadowsocks", [option_name("x_ss_encrypt_method")] = "xchacha20-poly1305" })

o = s:option(Flag, option_name("uot"), translate("UDP over TCP"), translate("Need Xray-core or sing-box as server side."))
o:depends({ [option_name("protocol")] = "shadowsocks", [option_name("x_ss_encrypt_method")] = "2022-blake3-aes-128-gcm" })
o:depends({ [option_name("protocol")] = "shadowsocks", [option_name("x_ss_encrypt_method")] = "2022-blake3-aes-256-gcm" })
o:depends({ [option_name("protocol")] = "shadowsocks", [option_name("x_ss_encrypt_method")] = "2022-blake3-chacha20-poly1305" })

o = s:option(Value, option_name("uuid"), translate("ID"))
o.password = true
o:depends({ [option_name("protocol")] = "vmess" })
o:depends({ [option_name("protocol")] = "vless" })

o = s:option(ListValue, option_name("flow"), translate("flow"))
o.default = ""
o:value("", translate("Disable"))
o:value("xtls-rprx-vision")
o:depends({ [option_name("protocol")] = "vless", [option_name("tls")] = true, [option_name("transport")] = "tcp" })

o = s:option(Flag, option_name("tls"), translate("TLS"))
o.default = 0
o:depends({ [option_name("protocol")] = "vmess" })
o:depends({ [option_name("protocol")] = "vless" })
o:depends({ [option_name("protocol")] = "socks" })
o:depends({ [option_name("protocol")] = "trojan" })
o:depends({ [option_name("protocol")] = "shadowsocks" })

o = s:option(Flag, option_name("reality"), translate("REALITY"), translate("Only recommend to use with VLESS-TCP-XTLS-Vision."))
o.default = 0
o:depends({ [option_name("tls")] = true, [option_name("transport")] = "tcp" })
o:depends({ [option_name("tls")] = true, [option_name("transport")] = "h2" })
o:depends({ [option_name("tls")] = true, [option_name("transport")] = "grpc" })

o = s:option(ListValue, option_name("alpn"), translate("alpn"))
o.default = "default"
o:value("default", translate("Default"))
o:value("h2,http/1.1")
o:value("h2")
o:value("http/1.1")
o:depends({ [option_name("tls")] = true, [option_name("reality")] = false })

-- o = s:option(Value, option_name("minversion"), translate("minversion"))
-- o.default = "1.3"
-- o:value("1.3")
-- o:depends({ [option_name("tls")] = true })

o = s:option(Value, option_name("tls_serverName"), translate("Domain"))
o:depends({ [option_name("tls")] = true })

o = s:option(Flag, option_name("tls_allowInsecure"), translate("allowInsecure"), translate("Whether unsafe connections are allowed. When checked, Certificate validation will be skipped."))
o.default = "0"
o:depends({ [option_name("tls")] = true, [option_name("reality")] = false })

-- [[ REALITY部分 ]] --
o = s:option(Value, option_name("reality_publicKey"), translate("Public Key"))
o:depends({ [option_name("tls")] = true, [option_name("reality")] = true })

o = s:option(Value, option_name("reality_shortId"), translate("Short Id"))
o:depends({ [option_name("tls")] = true, [option_name("reality")] = true })

o = s:option(Value, option_name("reality_spiderX"), translate("Spider X"))
o.placeholder = "/"
o:depends({ [option_name("tls")] = true, [option_name("reality")] = true })

o = s:option(Flag, option_name("utls"), translate("uTLS"))
o.default = "0"
o:depends({ [option_name("tls")] = true, [option_name("reality")] = false })

o = s:option(ListValue, option_name("fingerprint"), translate("Finger Print"))
o:value("chrome")
o:value("firefox")
o:value("edge")
o:value("safari")
o:value("360")
o:value("qq")
o:value("ios")
o:value("android")
o:value("random")
o:value("randomized")
o.default = "chrome"
o:depends({ [option_name("tls")] = true, [option_name("utls")] = true })
o:depends({ [option_name("tls")] = true, [option_name("reality")] = true })

o = s:option(ListValue, option_name("transport"), translate("Transport"))
o:value("tcp", "TCP")
o:value("mkcp", "mKCP")
o:value("ws", "WebSocket")
o:value("h2", "HTTP/2")
o:value("ds", "DomainSocket")
o:value("quic", "QUIC")
o:value("grpc", "gRPC")
o:depends({ [option_name("protocol")] = "vmess" })
o:depends({ [option_name("protocol")] = "vless" })
o:depends({ [option_name("protocol")] = "socks" })
o:depends({ [option_name("protocol")] = "shadowsocks" })
o:depends({ [option_name("protocol")] = "trojan" })

--[[
o = s:option(ListValue, option_name("ss_transport"), translate("Transport"))
o:value("ws", "WebSocket")
o:value("h2", "HTTP/2")
o:value("h2+ws", "HTTP/2 & WebSocket")
o:depends({ [option_name("protocol")] = "shadowsocks" })
]]--

o = s:option(Value, option_name("wireguard_public_key"), translate("Public Key"))
o:depends({ [option_name("protocol")] = "wireguard" })

o = s:option(Value, option_name("wireguard_secret_key"), translate("Private Key"))
o:depends({ [option_name("protocol")] = "wireguard" })

o = s:option(Value, option_name("wireguard_preSharedKey"), translate("Pre shared key"))
o:depends({ [option_name("protocol")] = "wireguard" })

o = s:option(DynamicList, option_name("wireguard_local_address"), translate("Local Address"))
o:depends({ [option_name("protocol")] = "wireguard" })

o = s:option(Value, option_name("wireguard_mtu"), translate("MTU"))
o.default = "1420"
o:depends({ [option_name("protocol")] = "wireguard" })

if api.compare_versions(api.get_app_version("xray"), ">=", "1.8.0") then
	o = s:option(Value, option_name("wireguard_reserved"), translate("Reserved"), translate("Decimal numbers separated by \",\" or Base64-encoded strings."))
	o:depends({ [option_name("protocol")] = "wireguard" })
end

o = s:option(Value, option_name("wireguard_keepAlive"), translate("Keep Alive"))
o.default = "0"
o:depends({ [option_name("protocol")] = "wireguard" })

-- [[ TCP部分 ]]--

-- TCP伪装
o = s:option(ListValue, option_name("tcp_guise"), translate("Camouflage Type"))
o:value("none", "none")
o:value("http", "http")
o:depends({ [option_name("transport")] = "tcp" })

-- HTTP域名
o = s:option(DynamicList, option_name("tcp_guise_http_host"), translate("HTTP Host"))
o:depends({ [option_name("tcp_guise")] = "http" })

-- HTTP路径
o = s:option(DynamicList, option_name("tcp_guise_http_path"), translate("HTTP Path"))
o.placeholder = "/"
o:depends({ [option_name("tcp_guise")] = "http" })

-- [[ mKCP部分 ]]--

o = s:option(ListValue, option_name("mkcp_guise"), translate("Camouflage Type"), translate('<br />none: default, no masquerade, data sent is packets with no characteristics.<br />srtp: disguised as an SRTP packet, it will be recognized as video call data (such as FaceTime).<br />utp: packets disguised as uTP will be recognized as bittorrent downloaded data.<br />wechat-video: packets disguised as WeChat video calls.<br />dtls: disguised as DTLS 1.2 packet.<br />wireguard: disguised as a WireGuard packet. (not really WireGuard protocol)'))
for a, t in ipairs(header_type_list) do o:value(t) end
o:depends({ [option_name("transport")] = "mkcp" })

o = s:option(Value, option_name("mkcp_mtu"), translate("KCP MTU"))
o.default = "1350"
o:depends({ [option_name("transport")] = "mkcp" })

o = s:option(Value, option_name("mkcp_tti"), translate("KCP TTI"))
o.default = "20"
o:depends({ [option_name("transport")] = "mkcp" })

o = s:option(Value, option_name("mkcp_uplinkCapacity"), translate("KCP uplinkCapacity"))
o.default = "5"
o:depends({ [option_name("transport")] = "mkcp" })

o = s:option(Value, option_name("mkcp_downlinkCapacity"), translate("KCP downlinkCapacity"))
o.default = "20"
o:depends({ [option_name("transport")] = "mkcp" })

o = s:option(Flag, option_name("mkcp_congestion"), translate("KCP Congestion"))
o:depends({ [option_name("transport")] = "mkcp" })

o = s:option(Value, option_name("mkcp_readBufferSize"), translate("KCP readBufferSize"))
o.default = "1"
o:depends({ [option_name("transport")] = "mkcp" })

o = s:option(Value, option_name("mkcp_writeBufferSize"), translate("KCP writeBufferSize"))
o.default = "1"
o:depends({ [option_name("transport")] = "mkcp" })

o = s:option(Value, option_name("mkcp_seed"), translate("KCP Seed"))
o:depends({ [option_name("transport")] = "mkcp" })

-- [[ WebSocket部分 ]]--
o = s:option(Value, option_name("ws_host"), translate("WebSocket Host"))
o:depends({ [option_name("transport")] = "ws" })
o:depends({ [option_name("ss_transport")] = "ws" })

o = s:option(Value, option_name("ws_path"), translate("WebSocket Path"))
o.placeholder = "/"
o:depends({ [option_name("transport")] = "ws" })
o:depends({ [option_name("ss_transport")] = "ws" })

-- [[ HTTP/2部分 ]]--
o = s:option(Value, option_name("h2_host"), translate("HTTP/2 Host"))
o:depends({ [option_name("transport")] = "h2" })
o:depends({ [option_name("ss_transport")] = "h2" })

o = s:option(Value, option_name("h2_path"), translate("HTTP/2 Path"))
o.placeholder = "/"
o:depends({ [option_name("transport")] = "h2" })
o:depends({ [option_name("ss_transport")] = "h2" })

o = s:option(Flag, option_name("h2_health_check"), translate("Health check"))
o:depends({ [option_name("transport")] = "h2" })

o = s:option(Value, option_name("h2_read_idle_timeout"), translate("Idle timeout"))
o.default = "10"
o:depends({ [option_name("h2_health_check")] = true })

o = s:option(Value, option_name("h2_health_check_timeout"), translate("Health check timeout"))
o.default = "15"
o:depends({ [option_name("h2_health_check")] = true })

-- [[ DomainSocket部分 ]]--
o = s:option(Value, option_name("ds_path"), "Path", translate("A legal file path. This file must not exist before running."))
o:depends({ [option_name("transport")] = "ds" })

-- [[ QUIC部分 ]]--
o = s:option(ListValue, option_name("quic_security"), translate("Encrypt Method"))
o:value("none")
o:value("aes-128-gcm")
o:value("chacha20-poly1305")
o:depends({ [option_name("transport")] = "quic" })

o = s:option(Value, option_name("quic_key"), translate("Encrypt Method") .. translate("Key"))
o:depends({ [option_name("transport")] = "quic" })

o = s:option(ListValue, option_name("quic_guise"), translate("Camouflage Type"))
for a, t in ipairs(header_type_list) do o:value(t) end
o:depends({ [option_name("transport")] = "quic" })

-- [[ gRPC部分 ]]--
o = s:option(Value, option_name("grpc_serviceName"), "ServiceName")
o:depends({ [option_name("transport")] = "grpc" })

o = s:option(ListValue, option_name("grpc_mode"), "gRPC " .. translate("Transfer mode"))
o:value("gun")
o:value("multi")
o:depends({ [option_name("transport")] = "grpc" })

o = s:option(Flag, option_name("grpc_health_check"), translate("Health check"))
o:depends({ [option_name("transport")] = "grpc" })

o = s:option(Value, option_name("grpc_idle_timeout"), translate("Idle timeout"))
o.default = "10"
o:depends({ [option_name("grpc_health_check")] = true })

o = s:option(Value, option_name("grpc_health_check_timeout"), translate("Health check timeout"))
o.default = "20"
o:depends({ [option_name("grpc_health_check")] = true })

o = s:option(Flag, option_name("grpc_permit_without_stream"), translate("Permit without stream"))
o.default = "0"
o:depends({ [option_name("grpc_health_check")] = true })

o = s:option(Value, option_name("grpc_initial_windows_size"), translate("Initial Windows Size"))
o.default = "0"
o:depends({ [option_name("transport")] = "grpc" })

-- [[ Mux ]]--
o = s:option(Flag, option_name("mux"), translate("Mux"))
o:depends({ [option_name("protocol")] = "vmess" })
o:depends({ [option_name("protocol")] = "vless", [option_name("flow")] = "" })
o:depends({ [option_name("protocol")] = "http" })
o:depends({ [option_name("protocol")] = "socks" })
o:depends({ [option_name("protocol")] = "shadowsocks" })
o:depends({ [option_name("protocol")] = "trojan" })

o = s:option(Value, option_name("mux_concurrency"), translate("Mux concurrency"))
o.default = 8
o:depends({ [option_name("mux")] = true })

-- [[ XUDP Mux ]]--
o = s:option(Flag, option_name("xmux"), translate("xMux"))
o.default = 1
o:depends({ [option_name("protocol")] = "vless", [option_name("flow")] = "xtls-rprx-vision" })

o = s:option(Value, option_name("xudp_concurrency"), translate("XUDP Mux concurrency"))
o.default = 8
o:depends({ [option_name("xmux")] = true })

api.luci_types(arg[1], m, s, type_name, option_prefix)
