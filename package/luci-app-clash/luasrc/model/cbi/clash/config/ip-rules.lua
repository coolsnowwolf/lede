local m, s, o
local clash = "clash"
local uci = luci.model.uci.cursor()
local fs = require "nixio.fs"
local sys = require "luci.sys"
local sid = arg[1]


m = Map(clash, translate("Edit Custom Rule & Group"))
--m.pageaction = false
m.redirect = luci.dispatcher.build_url("admin/services/clash/settings/other")
if m.uci:get(clash, sid) ~= "addtype" then
	luci.http.redirect(m.redirect)
	return
end


s = m:section(NamedSection, sid, "addtype")
s.anonymous = true
s.addremove   = false


o = s:option(ListValue, "type", translate("Rule Type"))
o.rmempty = false
o.description = translate("Choose Type")
o:value("DST-PORT", translate("DST-PORT"))
o:value("SRC-PORT", translate("SRC-PORT"))
o:value("SRC-IP-CIDR", translate("SRC-IP-CIDR"))
o:value("IP-CIDR", translate("IP-CIDR"))
o:value("IP-CIDR6", translate("IP-CIDR6"))
o:value("DOMAIN", translate("DOMAIN"))
o:value("DOMAIN-KEYWORD", translate("DOMAIN-KEYWORD"))
o:value("DOMAIN-SUFFIX", translate("DOMAIN-SUFFIX"))
o:value("GEOIP", translate("GEOIP"))


o = s:option(ListValue, "pgroup", translate("Select Proxy Group"))
uci:foreach("clash", "conf_groups",
		function(s)
		  if s.name ~= "" and s.name ~= nil then
			   o:value(s.name)
			end
		end)
o:value("DIRECT")
o:value("REJECT")
o.rmempty = false
o.description = translate("Select a policy group to add rule")


o = s:option(Value, "ipaaddr", translate("IP/Domain/Address/Keyword/Port"))
o.rmempty = false

o = s:option(Flag, "res", translate("No Resolve"))
o.default = 0
o:depends("type", "IP-CIDR")
o:depends("type", "IP-CIDR6")
o:depends("type", "GEOIP")

return m
