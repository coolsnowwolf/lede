local d = require "luci.dispatcher"
local appname = "passwall"

m = Map(appname, "V2ray" .. translate("Shunt") .. translate("Rule"))
m.redirect = d.build_url("admin", "services", appname)

s = m:section(NamedSection, arg[1], "shunt_rules", "")
s.addremove = false
s.dynamic = false

remarks = s:option(Value, "remarks", translate("Remarks"))
remarks.default = arg[1]
remarks.rmempty = false

domain_list = s:option(TextValue, "domain_list", translate("Domain"))
domain_list.rows = 15

ip_list = s:option(TextValue, "ip_list", "IP")
ip_list.rows = 15

return m
