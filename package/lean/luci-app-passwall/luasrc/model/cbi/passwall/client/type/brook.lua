local m, s = ...

local api = require "luci.passwall.api"

if not api.finded_com("brook") then
	return
end

local type_name = "Brook"

local option_prefix = "brook_"

local function option_name(name)
	return option_prefix .. name
end

-- [[ Brook ]]

s.fields["type"]:value(type_name, translate("Brook"))

o = s:option(ListValue, option_name("protocol"), translate("Protocol"))
o:value("client", translate("Brook"))
o:value("wsclient", translate("WebSocket"))

o = s:option(Value, option_name("address"), translate("Address (Support Domain Name)"))

o = s:option(Value, option_name("port"), translate("Port"))
o.datatype = "port"

o = s:option(Value, option_name("ws_path"), translate("WebSocket Path"))
o.placeholder = "/"
o:depends({ [option_name("protocol")] = "wsclient" })

o = s:option(Flag, option_name("tls"), translate("Use TLS"))
o:depends({ [option_name("protocol")] = "wsclient" })

o = s:option(Value, option_name("password"), translate("Password"))
o.password = true

api.luci_types(arg[1], m, s, type_name, option_prefix)
