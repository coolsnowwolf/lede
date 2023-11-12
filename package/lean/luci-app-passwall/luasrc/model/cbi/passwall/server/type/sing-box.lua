local m, s = ...

local api = require "luci.passwall.api"

local singbox_bin = api.finded_com("singbox")

if not singbox_bin then
	return
end

local singbox_tags = luci.sys.exec(singbox_bin .. " version  | grep 'Tags:' | awk '{print $2}'")

local type_name = "sing-box"

local option_prefix = "singbox_"

local function option_name(name)
	return option_prefix .. name
end

local ss_method_list = {
	"none", "aes-128-gcm", "aes-192-gcm", "aes-256-gcm", "chacha20-ietf-poly1305", "xchacha20-ietf-poly1305",
	"2022-blake3-aes-128-gcm", "2022-blake3-aes-256-gcm", "2022-blake3-chacha20-poly1305"
}

-- [[ Sing-Box ]]

s.fields["type"]:value(type_name, "Sing-Box")

o = s:option(ListValue, option_name("protocol"), translate("Protocol"))
o:value("mixed", "Mixed")
o:value("socks", "Socks")
o:value("http", "HTTP")
o:value("shadowsocks", "Shadowsocks")
o:value("vmess", "Vmess")
o:value("vless", "VLESS")
o:value("trojan", "Trojan")
o:value("naive", "Naive")
if singbox_tags:find("with_quic") then
	o:value("hysteria", "Hysteria")
end
if singbox_tags:find("with_quic") then
	o:value("tuic", "TUIC")
end
if singbox_tags:find("with_quic") then
	o:value("hysteria2", "Hysteria2")
end
o:value("direct", "Direct")

o = s:option(Value, option_name("port"), translate("Listen Port"))
o.datatype = "port"

o = s:option(Flag, option_name("auth"), translate("Auth"))
o.validate = function(self, value, t)
	if value and value == "1" then
		local user_v = s.fields[option_name("username")] and s.fields[option_name("username")]:formvalue(t) or ""
		local pass_v = s.fields[option_name("password")] and s.fields[option_name("password")]:formvalue(t) or ""
		if user_v == "" or pass_v == "" then
			return nil, translate("Username and Password must be used together!")
		end
	end
	return value
end
o:depends({ [option_name("protocol")] = "mixed" })
o:depends({ [option_name("protocol")] = "socks" })
o:depends({ [option_name("protocol")] = "http" })

o = s:option(Value, option_name("username"), translate("Username"))
o:depends({ [option_name("auth")] = true })
o:depends({ [option_name("protocol")] = "naive" })

o = s:option(Value, option_name("password"), translate("Password"))
o.password = true
o:depends({ [option_name("auth")] = true })
o:depends({ [option_name("protocol")] = "shadowsocks" })
o:depends({ [option_name("protocol")] = "naive" })
o:depends({ [option_name("protocol")] = "tuic" })

if singbox_tags:find("with_quic") then
	o = s:option(Value, option_name("hysteria_up_mbps"), translate("Max upload Mbps"))
	o.default = "100"
	o:depends({ [option_name("protocol")] = "hysteria" })

	o = s:option(Value, option_name("hysteria_down_mbps"), translate("Max download Mbps"))
	o.default = "100"
	o:depends({ [option_name("protocol")] = "hysteria" })

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

	o = s:option(Value, option_name("hysteria_recv_window_conn"), translate("QUIC stream receive window"))
	o:depends({ [option_name("protocol")] = "hysteria" })

	o = s:option(Value, option_name("hysteria_recv_window_client"), translate("QUIC connection receive window"))
	o:depends({ [option_name("protocol")] = "hysteria" })

	o = s:option(Value, option_name("hysteria_max_conn_client"), translate("QUIC concurrent bidirectional streams"))
	o.default = "1024"
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
	o = s:option(Flag, option_name("hysteria2_ignore_client_bandwidth"), translate("Commands the client to use the BBR flow control algorithm"))
	o.default = 0
	o:depends({ [option_name("protocol")] = "hysteria2" })

	o = s:option(Value, option_name("hysteria2_up_mbps"), translate("Max upload Mbps"))
	o:depends({ [option_name("protocol")] = "hysteria2", [option_name("hysteria2_ignore_client_bandwidth")] = false })

	o = s:option(Value, option_name("hysteria2_down_mbps"), translate("Max download Mbps"))
	o:depends({ [option_name("protocol")] = "hysteria2", [option_name("hysteria2_ignore_client_bandwidth")] = false })

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

o = s:option(ListValue, option_name("d_protocol"), translate("Destination protocol"))
o:value("tcp", "TCP")
o:value("udp", "UDP")
o:value("tcp,udp", "TCP,UDP")
o:depends({ [option_name("protocol")] = "direct" })

o = s:option(Value, option_name("d_address"), translate("Destination address"))
o:depends({ [option_name("protocol")] = "direct" })

o = s:option(Value, option_name("d_port"), translate("Destination port"))
o.datatype = "port"
o:depends({ [option_name("protocol")] = "direct" })

o = s:option(Value, option_name("decryption"), translate("Encrypt Method"))
o.default = "none"
o:depends({ [option_name("protocol")] = "vless" })

o = s:option(ListValue, option_name("ss_method"), translate("Encrypt Method"))
o.rewrite_option = "method"
for a, t in ipairs(ss_method_list) do o:value(t) end
o:depends({ [option_name("protocol")] = "shadowsocks" })

o = s:option(DynamicList, option_name("uuid"), translate("ID") .. "/" .. translate("Password"))
for i = 1, 3 do
	o:value(api.gen_uuid(1))
end
o:depends({ [option_name("protocol")] = "vmess" })
o:depends({ [option_name("protocol")] = "vless" })
o:depends({ [option_name("protocol")] = "trojan" })
o:depends({ [option_name("protocol")] = "tuic" })

o = s:option(ListValue, option_name("flow"), translate("flow"))
o.default = ""
o:value("", translate("Disable"))
o:value("xtls-rprx-vision")
o:depends({ [option_name("protocol")] = "vless" })

o = s:option(Flag, option_name("tls"), translate("TLS"))
o.default = 0
o.validate = function(self, value, t)
	if value then
		local reality = s.fields[option_name("reality")] and s.fields[option_name("reality")]:formvalue(t) or nil
		if reality and reality == "1" then return value end
		if value == "1" then
			local ca = s.fields[option_name("tls_certificateFile")] and s.fields[option_name("tls_certificateFile")]:formvalue(t) or ""
			local key = s.fields[option_name("tls_keyFile")] and s.fields[option_name("tls_keyFile")]:formvalue(t) or ""
			if ca == "" or key == "" then
				return nil, translate("Public key and Private key path can not be empty!")
			end
		end
		return value
	end
end
o:depends({ [option_name("protocol")] = "http" })
o:depends({ [option_name("protocol")] = "shadowsocks" })
o:depends({ [option_name("protocol")] = "vmess" })
o:depends({ [option_name("protocol")] = "vless" })
o:depends({ [option_name("protocol")] = "trojan" })

if singbox_tags:find("with_reality_server") then
	-- [[ REALITY部分 ]] --
	o = s:option(Flag, option_name("reality"), translate("REALITY"))
	o.default = 0
	o:depends({ [option_name("protocol")] = "vless", [option_name("tls")] = true })
	o:depends({ [option_name("protocol")] = "vmess", [option_name("tls")] = true })
	o:depends({ [option_name("protocol")] = "shadowsocks", [option_name("tls")] = true })
	o:depends({ [option_name("protocol")] = "http", [option_name("tls")] = true })
	o:depends({ [option_name("protocol")] = "trojan", [option_name("tls")] = true })

	o = s:option(Value, option_name("reality_private_key"), translate("Private Key"))
	o:depends({ [option_name("reality")] = true })

	o = s:option(Value, option_name("reality_shortId"), translate("Short Id"))
	o:depends({ [option_name("reality")] = true })

	o = s:option(Value, option_name("reality_handshake_server"), translate("Handshake Server"))
	o.default = "google.com"
	o:depends({ [option_name("reality")] = true })

	o = s:option(Value, option_name("reality_handshake_server_port"), translate("Handshake Server Port"))
	o.datatype = "port"
	o.default = "443"
	o:depends({ [option_name("reality")] = true })
end

-- [[ TLS部分 ]] --

o = s:option(FileUpload, option_name("tls_certificateFile"), translate("Public key absolute path"), translate("as:") .. "/etc/ssl/fullchain.pem")
o.default = m:get(s.section, "tls_certificateFile") or "/etc/config/ssl/" .. arg[1] .. ".pem"
o:depends({ [option_name("tls")] = true, [option_name("reality")] = false })
o:depends({ [option_name("protocol")] = "naive" })
o:depends({ [option_name("protocol")] = "hysteria" })
o:depends({ [option_name("protocol")] = "tuic" })
o:depends({ [option_name("protocol")] = "hysteria2" })
o.validate = function(self, value, t)
	if value and value ~= "" then
		if not nixio.fs.access(value) then
			return nil, translate("Can't find this file!")
		else
			return value
		end
	end
	return nil
end

o = s:option(FileUpload, option_name("tls_keyFile"), translate("Private key absolute path"), translate("as:") .. "/etc/ssl/private.key")
o.default = m:get(s.section, "tls_keyFile") or "/etc/config/ssl/" .. arg[1] .. ".key"
o:depends({ [option_name("tls")] = true, [option_name("reality")] = false })
o:depends({ [option_name("protocol")] = "naive" })
o:depends({ [option_name("protocol")] = "hysteria" })
o:depends({ [option_name("protocol")] = "tuic" })
o:depends({ [option_name("protocol")] = "hysteria2" })
o.validate = function(self, value, t)
	if value and value ~= "" then
		if not nixio.fs.access(value) then
			return nil, translate("Can't find this file!")
		else
			return value
		end
	end
	return nil
end

if singbox_tags:find("with_ech") then
	o = s:option(Flag, option_name("ech"), translate("ECH"))
	o.default = "0"
	o:depends({ [option_name("tls")] = true, [option_name("flow")] = "", [option_name("reality")] = false })
	o:depends({ [option_name("protocol")] = "naive" })
	o:depends({ [option_name("protocol")] = "hysteria" })
	o:depends({ [option_name("protocol")] = "tuic" })
	o:depends({ [option_name("protocol")] = "hysteria2" })

	o = s:option(Value, option_name("ech_key"), translate("ECH Key"))
	o.default = ""
	o:depends({ [option_name("ech")] = true })

	o = s:option(Flag, option_name("pq_signature_schemes_enabled"), translate("PQ signature schemes"))
	o.default = "0"
	o:depends({ [option_name("ech")] = true })

	o = s:option(Flag, option_name("dynamic_record_sizing_disabled"), translate("Disable adaptive sizing of TLS records"))
	o.default = "0"
	o:depends({ [option_name("ech")] = true })
end

o = s:option(ListValue, option_name("transport"), translate("Transport"))
o:value("tcp", "TCP")
o:value("http", "HTTP")
o:value("ws", "WebSocket")
o:value("quic", "QUIC")
o:value("grpc", "gRPC")
o:depends({ [option_name("protocol")] = "shadowsocks" })
o:depends({ [option_name("protocol")] = "vmess" })
o:depends({ [option_name("protocol")] = "vless" })
o:depends({ [option_name("protocol")] = "trojan" })

-- [[ HTTP部分 ]]--

o = s:option(Value, option_name("http_host"), translate("HTTP Host"))
o:depends({ [option_name("transport")] = "http" })

o = s:option(Value, option_name("http_path"), translate("HTTP Path"))
o:depends({ [option_name("transport")] = "http" })

-- [[ WebSocket部分 ]]--

o = s:option(Value, option_name("ws_host"), translate("WebSocket Host"))
o:depends({ [option_name("transport")] = "ws" })

o = s:option(Value, option_name("ws_path"), translate("WebSocket Path"))
o:depends({ [option_name("transport")] = "ws" })

-- [[ gRPC部分 ]]--
o = s:option(Value, option_name("grpc_serviceName"), "ServiceName")
o:depends({ [option_name("transport")] = "grpc" })

o = s:option(Flag, option_name("bind_local"), translate("Bind Local"), translate("When selected, it can only be accessed locally, It is recommended to turn on when using reverse proxies or be fallback."))
o.default = "0"

o = s:option(Flag, option_name("accept_lan"), translate("Accept LAN Access"), translate("When selected, it can accessed lan , this will not be safe!"))
o.default = "0"

local nodes_table = {}
for k, e in ipairs(api.get_valid_nodes()) do
	if e.node_type == "normal" and e.type == type_name then
		nodes_table[#nodes_table + 1] = {
			id = e[".name"],
			remarks = e["remark"]
		}
	end
end

o = s:option(ListValue, option_name("outbound_node"), translate("outbound node"))
o:value("nil", translate("Close"))
o:value("_socks", translate("Custom Socks"))
o:value("_http", translate("Custom HTTP"))
o:value("_iface", translate("Custom Interface"))
for k, v in pairs(nodes_table) do o:value(v.id, v.remarks) end
o.default = "nil"

o = s:option(Value, option_name("outbound_node_address"), translate("Address (Support Domain Name)"))
o:depends({ [option_name("outbound_node")] = "_socks" })
o:depends({ [option_name("outbound_node")] = "_http" })

o = s:option(Value, option_name("outbound_node_port"), translate("Port"))
o.datatype = "port"
o:depends({ [option_name("outbound_node")] = "_socks" })
o:depends({ [option_name("outbound_node")] = "_http" })

o = s:option(Value, option_name("outbound_node_username"), translate("Username"))
o:depends({ [option_name("outbound_node")] = "_socks" })
o:depends({ [option_name("outbound_node")] = "_http" })

o = s:option(Value, option_name("outbound_node_password"), translate("Password"))
o.password = true
o:depends({ [option_name("outbound_node")] = "_socks" })
o:depends({ [option_name("outbound_node")] = "_http" })

o = s:option(Value, option_name("outbound_node_iface"), translate("Interface"))
o.default = "eth1"
o:depends({ [option_name("outbound_node")] = "_iface" })

o = s:option(Flag, option_name("log"), translate("Log"))
o.default = "1"
o.rmempty = false

o = s:option(ListValue, option_name("loglevel"), translate("Log Level"))
o.default = "info"
o:value("debug")
o:value("info")
o:value("warn")
o:value("error")
o:depends({ [option_name("log")] = true })

api.luci_types(arg[1], m, s, type_name, option_prefix)
