local m, s = ...

local api = require "luci.passwall.api"

if not api.finded_com("hysteria") then
	return
end

local type_name = "Hysteria2"

local option_prefix = "hysteria2_"

local function option_name(name)
	return option_prefix .. name
end

-- [[ Hysteria2 ]]

s.fields["type"]:value(type_name, "Hysteria2")

o = s:option(ListValue, option_name("protocol"), translate("Protocol"))
o:value("udp", "UDP")

o = s:option(Value, option_name("address"), translate("Address (Support Domain Name)"))

o = s:option(Value, option_name("port"), translate("Port"))
o.datatype = "port"

o = s:option(Value, option_name("hop"), translate("Additional ports for hysteria hop"))
o.rewrite_option = o.option

o = s:option(Value, option_name("obfs"), translate("Obfs Password"))
o.rewrite_option = o.option

o = s:option(Value, option_name("auth_password"), translate("Auth Password"))
o.password = true
o.rewrite_option = o.option

o = s:option(Flag, option_name("fast_open"), translate("Fast Open"))
o.default = "0"

o = s:option(Value, option_name("tls_serverName"), translate("Domain"))

o = s:option(Flag, option_name("tls_allowInsecure"), translate("allowInsecure"), translate("Whether unsafe connections are allowed. When checked, Certificate validation will be skipped."))
o.default = "0"

o = s:option(Value, option_name("up_mbps"), translate("Max upload Mbps"))
o.rewrite_option = o.option

o = s:option(Value, option_name("down_mbps"), translate("Max download Mbps"))
o.rewrite_option = o.option

o = s:option(Value, option_name("hop_interval"), translate("Hop Interval"))
o.rewrite_option = o.option

o = s:option(Value, option_name("recv_window_conn"), translate("QUIC stream receive window"))
o.rewrite_option = o.option

o = s:option(Value, option_name("recv_window"), translate("QUIC connection receive window"))
o.rewrite_option = o.option

o = s:option(Value, option_name("idle_timeout"), translate("Idle Timeout"))
o.rewrite_option = o.option

o = s:option(Flag, option_name("disable_mtu_discovery"), translate("Disable MTU detection"))
o.default = "0"
o.rewrite_option = o.option

o = s:option(Flag, option_name("lazy_start"), translate("Lazy Start"))
o.default = "0"
o.rewrite_option = o.option

api.luci_types(arg[1], m, s, type_name, option_prefix)
