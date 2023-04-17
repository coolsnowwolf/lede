local m, s, o
local clash = "clash"
local uci = luci.model.uci.cursor()
local fs = require "nixio.fs"
local sys = require "luci.sys"
local sid = arg[1]


m = Map(clash, translate("Edit Rules"))
--m.pageaction = false
m.redirect = luci.dispatcher.build_url("admin/services/clash/config/create")
if m.uci:get(clash, sid) ~= "rules" then
	luci.http.redirect(m.redirect)
	return
end


s = m:section(NamedSection, sid, "rules")
s.anonymous = true
s.addremove   = false
m.pageaction = false

o = s:option(ListValue, "type", translate("Rule Type"))
o.rmempty = false
o.description = translate("Choose Type")
o:value("RULE-SET", translate("RULE-SET"))
o:value("DST-PORT", translate("DST-PORT"))
o:value("SRC-PORT", translate("SRC-PORT"))
o:value("SRC-IP-CIDR", translate("SRC-IP-CIDR"))
o:value("IP-CIDR", translate("IP-CIDR"))
o:value("IP-CIDR6", translate("IP-CIDR6"))
o:value("DOMAIN", translate("DOMAIN"))
o:value("DOMAIN-KEYWORD", translate("DOMAIN-KEYWORD"))
o:value("DOMAIN-SUFFIX", translate("DOMAIN-SUFFIX"))
o:value("GEOIP", translate("GEOIP"))
o:value("MATCH", translate("MATCH"))


o = s:option(ListValue, "rulename", translate("Description"))
uci:foreach("clash", "ruleprovider",
		function(s)
		  if s.name ~= "" and s.name ~= nil then
			   o:value(s.name)
			end
		end)
o.description = translate("Select rule provider name")
o:depends("type", "RULE-SET")


o = s:option(ListValue, "rulegroups", translate("Select Proxy Group"))
uci:foreach("clash", "pgroups",
		function(s)
		  if s.name ~= "" and s.name ~= nil then
			   o:value(s.name)
			end
		end)
o:value("DIRECT")
o:value("REJECT")
o.description = translate("Select a policy group to add rule")



o = s:option(Value, "rulenamee", translate("Description"))
o:depends("type", "DST-PORT")
o:depends("type", "SRC-PORT")
o:depends("type", "SRC-IP-CIDR")
o:depends("type", "IP-CIDR")
o:depends("type", "DOMAIN")
o:depends("type", "DOMAIN-KEYWORD")
o:depends("type", "DOMAIN-SUFFIX")
o:depends("type", "GEOIP")
o:depends("type", "IP-CIDR6")

o = s:option(Flag, "res", translate("No Resolve"))
o.default = 0
o:depends("type", "IP-CIDR")
o:depends("type", "IP-CIDR6")
o:depends("type", "GEOIP")

local t = {
    {Apply, Return}
}

b = m:section(Table, t)

o = b:option(Button,"Apply")
o.inputtitle = translate("Save & Apply")
o.inputstyle = "apply"
o.write = function()
  m.uci:commit("clash")
  sys.call("/usr/share/clash/create/rules.sh start >/dev/null 2>&1 &")
  luci.http.redirect(luci.dispatcher.build_url("admin", "services", "clash", "config", "create"))
end

o = b:option(Button,"Return")
o.inputtitle = translate("Back to Overview")
o.inputstyle = "reset"
o.write = function()
   m.uci:revert(clash)
   luci.http.redirect(m.redirect)
  --luci.http.redirect(luci.dispatcher.build_url("admin", "services", "clash", "config", "providers"))
end


return m
