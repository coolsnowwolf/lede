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

o = s:option(Value, option_name("port"), translate("Listen Port"))
o.datatype = "port"

o = s:option(ListValue, option_name("protocol"), translate("Protocol"))
o:value("server", "Brook")
o:value("wsserver", "WebSocket")

--o = s:option(Flag, option_name("tls"), translate("Use TLS"))
--o:depends({ [option_name("protocol")] = "wsserver" })

o = s:option(Value, option_name("password"), translate("Password"))
o.password = true

o = s:option(Value, option_name("ws_path"), translate("WebSocket Path"))
o:depends({ [option_name("protocol")] = "wsserver" })

o = s:option(Flag, option_name("log"), translate("Log"))
o.default = "1"
o.rmempty = false

api.luci_types(arg[1], m, s, type_name, option_prefix)
