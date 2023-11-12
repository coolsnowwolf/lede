local m, s = ...

local api = require "luci.passwall.api"

if not api.is_finded("tuic-client") then
	return
end

local type_name = "TUIC"

local option_prefix = "tuic_"

local function option_name(name)
	return option_prefix .. name
end

-- [[ TUIC ]]

s.fields["type"]:value(type_name, translate("TUIC"))

o = s:option(Value, option_name("address"), translate("Address (Support Domain Name)"))

o = s:option(Value, option_name("port"), translate("Port"))
o.datatype = "port"

o = s:option(Value, option_name("uuid"), translate("ID"))
o.password = true

-- Tuic Password for remote server connect
o = s:option(Value, option_name("password"), translate("TUIC User Password For Connect Remote Server"))
o.password = true
o.rmempty = true
o.default = ""
o.rewrite_option = o.option

--[[
-- Tuic username for local socks connect
o = s:option(Value, option_name("socks_username"), translate("TUIC UserName For Local Socks"))
o.rmempty = true
o.default = ""
o.rewrite_option = o.option

-- Tuic Password for local socks connect
o = s:option(Value, option_name("socks_password"), translate("TUIC Password For Local Socks"))
o.password = true
o.rmempty = true
o.default = ""
o.rewrite_option = o.option
--]]

o = s:option(Value, option_name("ip"), translate("Set the TUIC proxy server ip address"))
o.datatype = "ipaddr"
o.rmempty = true
o.rewrite_option = o.option

o = s:option(ListValue, option_name("udp_relay_mode"), translate("UDP relay mode"))
o:value("native", translate("native"))
o:value("quic", translate("QUIC"))
o.default = "native"
o.rmempty = true
o.rewrite_option = o.option

o = s:option(ListValue, option_name("congestion_control"), translate("Congestion control algorithm"))
o:value("bbr", translate("BBR"))
o:value("cubic", translate("CUBIC"))
o:value("new_reno", translate("New Reno"))
o.default = "cubic"
o.rmempty = true
o.rewrite_option = o.option

o = s:option(Value, option_name("heartbeat"), translate("Heartbeat interval(second)"))
o.datatype = "uinteger"
o.default = "3"
o.rmempty = true
o.rewrite_option = o.option

o = s:option(Value, option_name("timeout"), translate("Timeout for establishing a connection to server(second)"))
o.datatype = "uinteger"
o.default = "8"
o.rmempty = true
o.rewrite_option = o.option

o = s:option(Value, option_name("gc_interval"), translate("Garbage collection interval(second)"))
o.datatype = "uinteger"
o.default = "3"
o.rmempty = true
o.rewrite_option = o.option

o = s:option(Value, option_name("gc_lifetime"), translate("Garbage collection lifetime(second)"))
o.datatype = "uinteger"
o.default = "15"
o.rmempty = true
o.rewrite_option = o.option

o = s:option(Value, option_name("send_window"), translate("TUIC send window"))
o.datatype = "uinteger"
o.default = 20971520
o.rmempty = true
o.rewrite_option = o.option

o = s:option(Value, option_name("receive_window"), translate("TUIC receive window"))
o.datatype = "uinteger"
o.default = 10485760
o.rmempty = true
o.rewrite_option = o.option

o = s:option(Value, option_name("max_package_size"), translate("TUIC Maximum packet size the socks5 server can receive from external, in bytes"))
o.datatype = "uinteger"
o.default = 1500
o.rmempty = true
o.rewrite_option = o.option

--Tuic settings for the local inbound socks5 server
o = s:option(Flag, option_name("dual_stack"), translate("Set if the listening socket should be dual-stack"))
o.default = 0
o.rmempty = true
o.rewrite_option = o.option

o = s:option(Flag, option_name("disable_sni"), translate("Disable SNI"))
o.default = 0
o.rmempty = true
o.rewrite_option = o.option

o = s:option(Flag, option_name("zero_rtt_handshake"), translate("Enable 0-RTT QUIC handshake"))
o.default = 0
o.rmempty = true
o.rewrite_option = o.option

o = s:option(DynamicList, option_name("tls_alpn"), translate("TLS ALPN"))
o.rmempty = true
o.rewrite_option = o.option

api.luci_types(arg[1], m, s, type_name, option_prefix)
