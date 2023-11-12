local m, s = ...

local api = require "luci.passwall.api"

if not api.is_finded("naive") then
	return
end

local type_name = "Naiveproxy"

local option_prefix = "naive_"

local function option_name(name)
	return option_prefix .. name
end

-- [[ Naive ]]

s.fields["type"]:value(type_name, translate("NaiveProxy"))

o = s:option(ListValue, option_name("protocol"), translate("Protocol"))
o:value("https", translate("HTTPS"))
o:value("quic", translate("QUIC"))

o = s:option(Value, option_name("address"), translate("Address (Support Domain Name)"))

o = s:option(Value, option_name("port"), translate("Port"))
o.datatype = "port"

o = s:option(Value, option_name("username"), translate("Username"))

o = s:option(Value, option_name("password"), translate("Password"))
o.password = true

api.luci_types(arg[1], m, s, type_name, option_prefix)
