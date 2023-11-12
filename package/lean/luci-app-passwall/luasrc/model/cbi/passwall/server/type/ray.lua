local m, s = ...

local api = require "luci.passwall.api"

if not api.finded_com("xray") then
	return
end

local type_name = "Xray"

local option_prefix = "xray_"

local function option_name(name)
	return option_prefix .. name
end

local x_ss_method_list = {
	"aes-128-gcm", "aes-256-gcm", "chacha20-poly1305", "xchacha20-poly1305", "2022-blake3-aes-128-gcm", "2022-blake3-aes-256-gcm", "2022-blake3-chacha20-poly1305"
}

local header_type_list = {
	"none", "srtp", "utp", "wechat-video", "dtls", "wireguard"
}

-- [[ Xray ]]

s.fields["type"]:value(type_name, "Xray")

o = s:option(ListValue, option_name("protocol"), translate("Protocol"))
o:value("vmess", "Vmess")
o:value("vless", "VLESS")
o:value("http", "HTTP")
o:value("socks", "Socks")
o:value("shadowsocks", "Shadowsocks")
o:value("trojan", "Trojan")
o:value("dokodemo-door", "dokodemo-door")

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
o:depends({ [option_name("protocol")] = "socks" })
o:depends({ [option_name("protocol")] = "http" })

o = s:option(Value, option_name("username"), translate("Username"))
o:depends({ [option_name("auth")] = true })

o = s:option(Value, option_name("password"), translate("Password"))
o.password = true
o:depends({ [option_name("auth")] = true })
o:depends({ [option_name("protocol")] = "shadowsocks" })

o = s:option(ListValue, option_name("d_protocol"), translate("Destination protocol"))
o:value("tcp", "TCP")
o:value("udp", "UDP")
o:value("tcp,udp", "TCP,UDP")
o:depends({ [option_name("protocol")] = "dokodemo-door" })

o = s:option(Value, option_name("d_address"), translate("Destination address"))
o:depends({ [option_name("protocol")] = "dokodemo-door" })

o = s:option(Value, option_name("d_port"), translate("Destination port"))
o.datatype = "port"
o:depends({ [option_name("protocol")] = "dokodemo-door" })

o = s:option(Value, option_name("decryption"), translate("Encrypt Method"))
o.default = "none"
o:depends({ [option_name("protocol")] = "vless" })

o = s:option(ListValue, option_name("x_ss_method"), translate("Encrypt Method"))
o.rewrite_option = "method"
for a, t in ipairs(x_ss_method_list) do o:value(t) end
o:depends({ [option_name("protocol")] = "shadowsocks" })

o = s:option(Flag, option_name("iv_check"), translate("IV Check"))
o:depends({ [option_name("protocol")] = "shadowsocks" })

o = s:option(ListValue, option_name("ss_network"), translate("Transport"))
o.default = "tcp,udp"
o:value("tcp", "TCP")
o:value("udp", "UDP")
o:value("tcp,udp", "TCP,UDP")
o:depends({ [option_name("protocol")] = "shadowsocks" })

o = s:option(Flag, option_name("udp_forward"), translate("UDP Forward"))
o.default = "1"
o.rmempty = false
o:depends({ [option_name("protocol")] = "socks" })

o = s:option(DynamicList, option_name("uuid"), translate("ID") .. "/" .. translate("Password"))
for i = 1, 3 do
	o:value(api.gen_uuid(1))
end
o:depends({ [option_name("protocol")] = "vmess" })
o:depends({ [option_name("protocol")] = "vless" })
o:depends({ [option_name("protocol")] = "trojan" })

o = s:option(ListValue, option_name("flow"), translate("flow"))
o.default = ""
o:value("", translate("Disable"))
o:value("xtls-rprx-vision")
o:depends({ [option_name("protocol")] = "vless", [option_name("tls")] = true, [option_name("transport")] = "tcp" })

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
o:depends({ [option_name("protocol")] = "vmess" })
o:depends({ [option_name("protocol")] = "vless" })
o:depends({ [option_name("protocol")] = "socks" })
o:depends({ [option_name("protocol")] = "shadowsocks" })
o:depends({ [option_name("protocol")] = "trojan" })

-- [[ REALITY部分 ]] --
o = s:option(Flag, option_name("reality"), translate("REALITY"))
o.default = 0
o:depends({ [option_name("tls")] = true })

o = s:option(Value, option_name("reality_private_key"), translate("Private Key"))
o:depends({ [option_name("reality")] = true })

o = s:option(Value, option_name("reality_shortId"), translate("Short Id"))
o:depends({ [option_name("reality")] = true })

o = s:option(Value, option_name("reality_dest"), translate("Dest"))
o.default = "google.com:443"
o:depends({ [option_name("reality")] = true })

o = s:option(Value, option_name("reality_serverNames"), translate("serverNames"))
o:depends({ [option_name("reality")] = true })

o = s:option(ListValue, option_name("alpn"), translate("alpn"))
o.default = "h2,http/1.1"
o:value("h2,http/1.1")
o:value("h2")
o:value("http/1.1")
o:depends({ [option_name("tls")] = true })

-- o = s:option(Value, option_name("minversion"), translate("minversion"))
-- o.default = "1.3"
-- o:value("1.3")
--o:depends({ [option_name("tls")] = true })

-- [[ TLS部分 ]] --

o = s:option(FileUpload, option_name("tls_certificateFile"), translate("Public key absolute path"), translate("as:") .. "/etc/ssl/fullchain.pem")
o.default = m:get(s.section, "tls_certificateFile") or "/etc/config/ssl/" .. arg[1] .. ".pem"
o:depends({ [option_name("tls")] = true, [option_name("reality")] = false })
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

-- [[ WebSocket部分 ]]--

o = s:option(Value, option_name("ws_host"), translate("WebSocket Host"))
o:depends({ [option_name("transport")] = "ws" })

o = s:option(Value, option_name("ws_path"), translate("WebSocket Path"))
o:depends({ [option_name("transport")] = "ws" })

-- [[ HTTP/2部分 ]]--

o = s:option(Value, option_name("h2_host"), translate("HTTP/2 Host"))
o:depends({ [option_name("transport")] = "h2" })

o = s:option(Value, option_name("h2_path"), translate("HTTP/2 Path"))
o:depends({ [option_name("transport")] = "h2" })

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

o = s:option(Flag, option_name("acceptProxyProtocol"), translate("acceptProxyProtocol"), translate("Whether to receive PROXY protocol, when this node want to be fallback or forwarded by proxy, it must be enable, otherwise it cannot be used."))
o:depends({ [option_name("transport")] = "tcp" })
o:depends({ [option_name("transport")] = "ws" })

-- [[ Fallback部分 ]]--
o = s:option(Flag, option_name("fallback"), translate("Fallback"))
o:depends({ [option_name("protocol")] = "vless", [option_name("transport")] = "tcp" })
o:depends({ [option_name("protocol")] = "trojan", [option_name("transport")] = "tcp" })

--[[
o = s:option(Value, option_name("fallback_alpn"), "Fallback alpn")
o:depends({ [option_name("fallback")] = true })

o = s:option(Value, option_name("fallback_path"), "Fallback path")
o:depends({ [option_name("fallback")] = true })

o = s:option(Value, option_name("fallback_dest"), "Fallback dest")
o:depends({ [option_name("fallback")] = true })

o = s:option(Value, option_name("fallback_xver"), "Fallback xver")
o.default = 0
o:depends({ [option_name("fallback")] = true })
]]--

o = s:option(DynamicList, option_name("fallback_list"), "Fallback", translate("dest,path"))
o:depends({ [option_name("fallback")] = true })

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
o:value("_iface", translate("Custom Interface") .. " (Only Support Xray)")
for k, v in pairs(nodes_table) do o:value(v.id, v.remarks) end
o.default = "nil"

o = s:option(Value, option_name("outbound_node_address"), translate("Address (Support Domain Name)"))
o:depends({ [option_name("outbound_node")] = "_socks"})
o:depends({ [option_name("outbound_node")] = "_http"})

o = s:option(Value, option_name("outbound_node_port"), translate("Port"))
o.datatype = "port"
o:depends({ [option_name("outbound_node")] = "_socks"})
o:depends({ [option_name("outbound_node")] = "_http"})

o = s:option(Value, option_name("outbound_node_username"), translate("Username"))
o:depends({ [option_name("outbound_node")] = "_socks"})
o:depends({ [option_name("outbound_node")] = "_http"})

o = s:option(Value, option_name("outbound_node_password"), translate("Password"))
o.password = true
o:depends({ [option_name("outbound_node")] = "_socks"})
o:depends({ [option_name("outbound_node")] = "_http"})

o = s:option(Value, option_name("outbound_node_iface"), translate("Interface"))
o.default = "eth1"
o:depends({ [option_name("outbound_node")] = "_iface"})

o = s:option(Flag, option_name("log"), translate("Log"))
o.default = "1"
o.rmempty = false

o = s:option(ListValue, option_name("loglevel"), translate("Log Level"))
o.default = "warning"
o:value("debug")
o:value("info")
o:value("warning")
o:value("error")
o:depends({ [option_name("log")] = true })

api.luci_types(arg[1], m, s, type_name, option_prefix)
