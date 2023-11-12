local m, s = ...

local api = require "luci.passwall.api"

local singbox_bin = api.finded_com("singbox")

if not singbox_bin then
	return
end

local singbox_tags = luci.sys.exec(singbox_bin .. " version  | grep 'Tags:' | awk '{print $2}'")

local appname = api.appname
local uci = api.uci

local type_name = "sing-box"

local option_prefix = "singbox_"

local function option_name(name)
	return option_prefix .. name
end

local ss_method_new_list = {
	"none", "aes-128-gcm", "aes-192-gcm", "aes-256-gcm", "chacha20-ietf-poly1305", "xchacha20-ietf-poly1305", "2022-blake3-aes-128-gcm", "2022-blake3-aes-256-gcm", "2022-blake3-chacha20-poly1305"
}

local ss_method_old_list = {
	"aes-128-ctr", "aes-192-ctr", "aes-256-ctr", "aes-128-cfb", "aes-192-cfb", "aes-256-cfb", "rc4-md5", "chacha20-ietf", "xchacha20",
}

local security_list = { "none", "auto", "aes-128-gcm", "chacha20-poly1305", "zero" }

-- [[ sing-box ]]

s.fields["type"]:value(type_name, "Sing-Box")

o = s:option(ListValue, option_name("protocol"), translate("Protocol"))
o:value("socks", "Socks")
o:value("http", "HTTP")
o:value("shadowsocks", "Shadowsocks")
if singbox_tags:find("with_shadowsocksr") then
	o:value("shadowsocksr", "ShadowsocksR")
end
o:value("vmess", "Vmess")
o:value("trojan", "Trojan")
if singbox_tags:find("with_wireguard") then
	o:value("wireguard", "WireGuard")
end
if singbox_tags:find("with_quic") then
	o:value("hysteria", "Hysteria")
end
o:value("vless", "VLESS")
if singbox_tags:find("with_quic") then
	o:value("tuic", "TUIC")
end
if singbox_tags:find("with_quic") then
	o:value("hysteria2", "Hysteria2")
end
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
	if e.protocol == "_iface" then
		iface_table[#iface_table + 1] = {
			id = e[".name"],
			remarks = e["remark"]
		}
	end
end

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
o:depends({ [option_name("protocol")] = "shadowsocksr" })
o:depends({ [option_name("protocol")] = "trojan" })
o:depends({ [option_name("protocol")] = "tuic" })

o = s:option(ListValue, option_name("security"), translate("Encrypt Method"))
for a, t in ipairs(security_list) do o:value(t) end
o:depends({ [option_name("protocol")] = "vmess" })

o = s:option(ListValue, option_name("ss_method"), translate("Encrypt Method"))
o.rewrite_option = "method"
for a, t in ipairs(ss_method_new_list) do o:value(t) end
for a, t in ipairs(ss_method_old_list) do o:value(t) end
o:depends({ [option_name("protocol")] = "shadowsocks" })

if singbox_tags:find("with_shadowsocksr") then
	o = s:option(ListValue, option_name("ssr_method"), translate("Encrypt Method"))
	o.rewrite_option = "method"
	for a, t in ipairs(ss_method_old_list) do o:value(t) end
	o:depends({ [option_name("protocol")] = "shadowsocksr" })

	local ssr_protocol_list = {
		"origin", "verify_simple", "verify_deflate", "verify_sha1", "auth_simple",
		"auth_sha1", "auth_sha1_v2", "auth_sha1_v4", "auth_aes128_md5",
		"auth_aes128_sha1", "auth_chain_a", "auth_chain_b", "auth_chain_c",
		"auth_chain_d", "auth_chain_e", "auth_chain_f"
	}

	o = s:option(ListValue, option_name("ssr_protocol"), translate("Protocol"))
	for a, t in ipairs(ssr_protocol_list) do o:value(t) end
	o:depends({ [option_name("protocol")] = "shadowsocksr" })

	o = s:option(Value, option_name("ssr_protocol_param"), translate("Protocol_param"))
	o:depends({ [option_name("protocol")] = "shadowsocksr" })

	local ssr_obfs_list = {
		"plain", "http_simple", "http_post", "random_head", "tls_simple",
		"tls1.0_session_auth", "tls1.2_ticket_auth"
	}

	o = s:option(ListValue, option_name("ssr_obfs"), translate("Obfs"))
	for a, t in ipairs(ssr_obfs_list) do o:value(t) end
	o:depends({ [option_name("protocol")] = "shadowsocksr" })

	o = s:option(Value, option_name("ssr_obfs_param"), translate("Obfs_param"))
	o:depends({ [option_name("protocol")] = "shadowsocksr" })
end

o = s:option(Flag, option_name("uot"), translate("UDP over TCP"), translate("Need Xray-core or sing-box as server side."))
o:depends({ [option_name("protocol")] = "shadowsocks", [option_name("ss_method")] = "2022-blake3-aes-128-gcm" })
o:depends({ [option_name("protocol")] = "shadowsocks", [option_name("ss_method")] = "2022-blake3-aes-256-gcm" })
o:depends({ [option_name("protocol")] = "shadowsocks", [option_name("ss_method")] = "2022-blake3-chacha20-poly1305" })

o = s:option(Value, option_name("uuid"), translate("ID"))
o.password = true
o:depends({ [option_name("protocol")] = "vmess" })
o:depends({ [option_name("protocol")] = "vless" })
o:depends({ [option_name("protocol")] = "tuic" })

o = s:option(Value, option_name("alter_id"), "Alter ID")
o.datatype = "uinteger"
o.default = "0"
o:depends({ [option_name("protocol")] = "vmess" })

o = s:option(Flag, option_name("global_padding"), "global_padding", translate("Protocol parameter. Will waste traffic randomly if enabled."))
o.default = "0"
o:depends({ [option_name("protocol")] = "vmess" })

o = s:option(Flag, option_name("authenticated_length"), "authenticated_length", translate("Protocol parameter. Enable length block encryption."))
o.default = "0"
o:depends({ [option_name("protocol")] = "vmess" })

o = s:option(ListValue, option_name("flow"), translate("flow"))
o.default = ""
o:value("", translate("Disable"))
o:value("xtls-rprx-vision")
o:depends({ [option_name("protocol")] = "vless", [option_name("tls")] = true })

if singbox_tags:find("with_quic") then
	o = s:option(Value, option_name("hysteria_obfs"), translate("Obfs Password"))
	o:depends({ [option_name("protocol")] = "hysteria" })

	o = s:option(ListValue, option_name("hysteria_auth_type"), translate("Auth Type"))
	o:value("disable", translate("Disable"))
	o:value("string", translate("STRING"))
	o:value("base64", translate("BASE64"))
	o:depends({ [option_name("protocol")] = "hysteria" })

	o = s:option(Value, option_name("hysteria_auth_password"), translate("Auth Password"))
	o.password = true
	o:depends({ [option_name("protocol")] = "hysteria", [option_name("hysteria_auth_type")] = "string"})
	o:depends({ [option_name("protocol")] = "hysteria", [option_name("hysteria_auth_type")] = "base64"})

	o = s:option(Value, option_name("hysteria_up_mbps"), translate("Max upload Mbps"))
	o.default = "10"
	o:depends({ [option_name("protocol")] = "hysteria" })

	o = s:option(Value, option_name("hysteria_down_mbps"), translate("Max download Mbps"))
	o.default = "50"
	o:depends({ [option_name("protocol")] = "hysteria" })

	o = s:option(Value, option_name("hysteria_recv_window_conn"), translate("QUIC stream receive window"))
	o:depends({ [option_name("protocol")] = "hysteria" })

	o = s:option(Value, option_name("hysteria_recv_window"), translate("QUIC connection receive window"))
	o:depends({ [option_name("protocol")] = "hysteria" })

	o = s:option(Flag, option_name("hysteria_disable_mtu_discovery"), translate("Disable MTU detection"))
	o:depends({ [option_name("protocol")] = "hysteria" })

	o = s:option(Value, option_name("hysteria_alpn"), translate("QUIC TLS ALPN"))
	o:depends({ [option_name("protocol")] = "hysteria" })
end

if singbox_tags:find("with_quic") then
	o = s:option(ListValue, option_name("tuic_congestion_control"), translate("Congestion control algorithm"))
	o.default = "cubic"
	o:value("bbr", translate("BBR"))
	o:value("cubic", translate("CUBIC"))
	o:value("new_reno", translate("New Reno"))
	o:depends({ [option_name("protocol")] = "tuic" })

	o = s:option(ListValue, option_name("tuic_udp_relay_mode"), translate("UDP relay mode"))
	o.default = "native"
	o:value("native", translate("native"))
	o:value("quic", translate("QUIC"))
	o:depends({ [option_name("protocol")] = "tuic" })

	--[[
	o = s:option(Flag, option_name("tuic_udp_over_stream"), translate("UDP over stream"))
	o:depends({ [option_name("protocol")] = "tuic" })
	]]--

	o = s:option(Flag, option_name("tuic_zero_rtt_handshake"), translate("Enable 0-RTT QUIC handshake"))
	o.default = 0
	o:depends({ [option_name("protocol")] = "tuic" })

	o = s:option(Value, option_name("tuic_heartbeat"), translate("Heartbeat interval(second)"))
	o.datatype = "uinteger"
	o.default = "3"
	o:depends({ [option_name("protocol")] = "tuic" })

	o = s:option(Value, option_name("tuic_alpn"), translate("QUIC TLS ALPN"))
	o:depends({ [option_name("protocol")] = "tuic" })
end

if singbox_tags:find("with_quic") then
	o = s:option(Value, option_name("hysteria2_up_mbps"), translate("Max upload Mbps"))
	o:depends({ [option_name("protocol")] = "hysteria2" })

	o = s:option(Value, option_name("hysteria2_down_mbps"), translate("Max download Mbps"))
	o:depends({ [option_name("protocol")] = "hysteria2" })

	o = s:option(ListValue, option_name("hysteria2_obfs_type"), translate("Obfs Type"))
	o:value("", translate("Disable"))
	o:value("salamander")
	o:depends({ [option_name("protocol")] = "hysteria2" })

	o = s:option(Value, option_name("hysteria2_obfs_password"), translate("Obfs Password"))
	o:depends({ [option_name("protocol")] = "hysteria2" })

	o = s:option(Value, option_name("hysteria2_auth_password"), translate("Auth Password"))
	o.password = true
	o:depends({ [option_name("protocol")] = "hysteria2"})
end

o = s:option(Flag, option_name("tls"), translate("TLS"))
o.default = 0
o:depends({ [option_name("protocol")] = "vmess" })
o:depends({ [option_name("protocol")] = "vless" })
o:depends({ [option_name("protocol")] = "socks" })
o:depends({ [option_name("protocol")] = "trojan" })
o:depends({ [option_name("protocol")] = "shadowsocks" })

o = s:option(ListValue, option_name("alpn"), translate("alpn"))
o.default = "default"
o:value("default", translate("Default"))
o:value("h2,http/1.1")
o:value("h2")
o:value("http/1.1")
o:depends({ [option_name("tls")] = true })

o = s:option(Value, option_name("tls_serverName"), translate("Domain"))
o:depends({ [option_name("tls")] = true })
o:depends({ [option_name("protocol")] = "hysteria"})
o:depends({ [option_name("protocol")] = "tuic" })
o:depends({ [option_name("protocol")] = "hysteria2" })

o = s:option(Flag, option_name("tls_allowInsecure"), translate("allowInsecure"), translate("Whether unsafe connections are allowed. When checked, Certificate validation will be skipped."))
o.default = "0"
o:depends({ [option_name("tls")] = true })
o:depends({ [option_name("protocol")] = "hysteria"})
o:depends({ [option_name("protocol")] = "tuic" })
o:depends({ [option_name("protocol")] = "hysteria2" })

if singbox_tags:find("with_ech") then
	o = s:option(Flag, option_name("ech"), translate("ECH"))
	o.default = "0"
	o:depends({ [option_name("tls")] = true, [option_name("flow")] = "", [option_name("reality")] = false })
	o:depends({ [option_name("protocol")] = "tuic" })
	o:depends({ [option_name("protocol")] = "hysteria" })
	o:depends({ [option_name("protocol")] = "hysteria2" })

	o = s:option(Value, option_name("ech_config"), translate("ECH Config"))
	o.default = ""
	o:depends({ [option_name("ech")] = true })

	o = s:option(Flag, option_name("pq_signature_schemes_enabled"), translate("PQ signature schemes"))
	o.default = "0"
	o:depends({ [option_name("ech")] = true })

	o = s:option(Flag, option_name("dynamic_record_sizing_disabled"), translate("Disable adaptive sizing of TLS records"))
	o.default = "0"
	o:depends({ [option_name("ech")] = true })
end

if singbox_tags:find("with_utls") then
	o = s:option(Flag, option_name("utls"), translate("uTLS"))
	o.default = "0"
	o:depends({ [option_name("tls")] = true })

	o = s:option(ListValue, option_name("fingerprint"), translate("Finger Print"))
	o:value("chrome")
	o:value("firefox")
	o:value("edge")
	o:value("safari")
	-- o:value("360")
	o:value("qq")
	o:value("ios")
	-- o:value("android")
	o:value("random")
	-- o:value("randomized")
	o.default = "chrome"
	o:depends({ [option_name("tls")] = true, [option_name("utls")] = true })

	-- [[ REALITY部分 ]] --
	o = s:option(Flag, option_name("reality"), translate("REALITY"))
	o.default = 0
	o:depends({ [option_name("protocol")] = "vless", [option_name("utls")] = true })
	o:depends({ [option_name("protocol")] = "vmess", [option_name("utls")] = true })
	o:depends({ [option_name("protocol")] = "shadowsocks", [option_name("utls")] = true })
	o:depends({ [option_name("protocol")] = "socks", [option_name("utls")] = true })
	o:depends({ [option_name("protocol")] = "trojan", [option_name("utls")] = true })
	
	o = s:option(Value, option_name("reality_publicKey"), translate("Public Key"))
	o:depends({ [option_name("utls")] = true, [option_name("reality")] = true })
	
	o = s:option(Value, option_name("reality_shortId"), translate("Short Id"))
	o:depends({ [option_name("utls")] = true, [option_name("reality")] = true })
end

o = s:option(ListValue, option_name("transport"), translate("Transport"))
o:value("tcp", "TCP")
o:value("http", "HTTP")
o:value("ws", "WebSocket")
if singbox_tags:find("with_quic") then
	o:value("quic", "QUIC")
end
if singbox_tags:find("with_grpc") then
	o:value("grpc", "gRPC")
else o:value("grpc", "gRPC-lite")
end
o:depends({ [option_name("protocol")] = "vmess" })
o:depends({ [option_name("protocol")] = "vless" })
o:depends({ [option_name("protocol")] = "socks" })
o:depends({ [option_name("protocol")] = "shadowsocks" })
o:depends({ [option_name("protocol")] = "trojan" })

if singbox_tags:find("with_wireguard") then
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

	o = s:option(Value, option_name("wireguard_reserved"), translate("Reserved"), translate("Decimal numbers separated by \",\" or Base64-encoded strings."))
	o:depends({ [option_name("protocol")] = "wireguard" })
end

-- [[ HTTP部分 ]]--
o = s:option(Value, option_name("http_host"), translate("HTTP Host"))
o:depends({ [option_name("transport")] = "http" })

o = s:option(Value, option_name("http_path"), translate("HTTP Path"))
o.placeholder = "/"
o:depends({ [option_name("transport")] = "http" })

o = s:option(Flag, option_name("http_h2_health_check"), translate("Health check"))
o:depends({ [option_name("tls")] = true, [option_name("transport")] = "http" })

o = s:option(Value, option_name("http_h2_read_idle_timeout"), translate("Idle timeout"))
o.default = "10"
o:depends({ [option_name("tls")] = true, [option_name("transport")] = "http", [option_name("http_h2_health_check")] = true })

o = s:option(Value, option_name("http_h2_health_check_timeout"), translate("Health check timeout"))
o.default = "15"
o:depends({ [option_name("tls")] = true, [option_name("transport")] = "http", [option_name("http_h2_health_check")] = true })

-- [[ WebSocket部分 ]]--
o = s:option(Value, option_name("ws_host"), translate("WebSocket Host"))
o:depends({ [option_name("transport")] = "ws" })

o = s:option(Value, option_name("ws_path"), translate("WebSocket Path"))
o.placeholder = "/"
o:depends({ [option_name("transport")] = "ws" })

o = s:option(Flag, option_name("ws_enableEarlyData"), translate("Enable early data"))
o:depends({ [option_name("transport")] = "ws" })

o = s:option(Value, option_name("ws_maxEarlyData"), translate("Early data length"))
o.default = "1024"
o:depends({ [option_name("ws_enableEarlyData")] = true })

o = s:option(Value, option_name("ws_earlyDataHeaderName"), translate("Early data header name"), translate("Recommended value: Sec-WebSocket-Protocol"))
o:depends({ [option_name("ws_enableEarlyData")] = true })

-- [[ gRPC部分 ]]--
o = s:option(Value, option_name("grpc_serviceName"), "ServiceName")
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

-- [[ Mux ]]--
o = s:option(Flag, option_name("mux"), translate("Mux"))
o.rmempty = false
o:depends({ [option_name("protocol")] = "vmess" })
o:depends({ [option_name("protocol")] = "vless", [option_name("flow")] = "" })
o:depends({ [option_name("protocol")] = "http" })
o:depends({ [option_name("protocol")] = "socks" })
o:depends({ [option_name("protocol")] = "shadowsocks", [option_name("uot")] = "" })
o:depends({ [option_name("protocol")] = "trojan" })

o = s:option(ListValue, option_name("mux_type"), translate("Mux"))
o:value("smux")
o:value("yamux")
o:value("h2mux")
o:depends({ [option_name("mux")] = true })

o = s:option(Value, option_name("mux_concurrency"), translate("Mux concurrency"))
o.default = 8
o:depends({ [option_name("mux")] = true })

o = s:option(Flag, option_name("mux_padding"), translate("Padding"))
o.default = 0
o:depends({ [option_name("mux")] = true })

o = s:option(Flag, option_name("shadowtls"), "ShadowTLS")
o.default = 0
o:depends({ [option_name("protocol")] = "vmess", [option_name("tls")] = false })
o:depends({ [option_name("protocol")] = "shadowsocks", [option_name("tls")] = false })

o = s:option(ListValue, option_name("shadowtls_version"), "ShadowTLS " .. translate("Version"))
o.default = "1"
o:value("1", "ShadowTLS v1")
o:value("2", "ShadowTLS v2")
o:value("3", "ShadowTLS v3")
o:depends({ [option_name("shadowtls")] = true })

o = s:option(Value, option_name("shadowtls_password"), "ShadowTLS " .. translate("Password"))
o.password = true
o:depends({ [option_name("shadowtls")] = true, [option_name("shadowtls_version")] = "2" })
o:depends({ [option_name("shadowtls")] = true, [option_name("shadowtls_version")] = "3" })

o = s:option(Value, option_name("shadowtls_serverName"), "ShadowTLS " .. translate("Domain"))
o:depends({ [option_name("shadowtls")] = true })

if singbox_tags:find("with_utls") then
	o = s:option(Flag, option_name("shadowtls_utls"), "ShadowTLS " .. translate("uTLS"))
	o.default = "0"
	o:depends({ [option_name("shadowtls")] = true })

	o = s:option(ListValue, option_name("shadowtls_fingerprint"), "ShadowTLS " .. translate("Finger Print"))
	o:value("chrome")
	o:value("firefox")
	o:value("edge")
	o:value("safari")
	-- o:value("360")
	o:value("qq")
	o:value("ios")
	-- o:value("android")
	o:value("random")
	-- o:value("randomized")
	o.default = "chrome"
	o:depends({ [option_name("shadowtls")] = true, [option_name("shadowtls_utls")] = true })
end

api.luci_types(arg[1], m, s, type_name, option_prefix)
