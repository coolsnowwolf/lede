local d = require "luci.dispatcher"
local sys = require "luci.sys"

m = Map("luci-app-ipsec-server", "L2TP/IPSec PSK " .. translate("Users Manager"))
m.redirect = d.build_url("admin", "vpn", "ipsec-server", "users")

if sys.call("command -v xl2tpd > /dev/null") == 0 then
    s = m:section(NamedSection, arg[1], "l2tp_users", "")
    s.addremove = false
    s.anonymous = true

    o = s:option(Flag, "enabled", translate("Enabled"))
    o.default = 1
    o.rmempty = false

    o = s:option(Value, "username", translate("Username"))
    o.placeholder = translate("Username")
    o.rmempty = false

    o = s:option(Value, "password", translate("Password"))
    o.placeholder = translate("Password")
    o.rmempty = false

    o = s:option(Value, "ipaddress", translate("IP address"))
    o.placeholder = translate("Automatically")
    o.datatype = "ip4addr"
    o.rmempty = true

    o = s:option(DynamicList, "routes", translate("Static Routes"))
    o.placeholder = "192.168.10.0/24"
    o.datatype = "ipmask4"
    o.rmempty = true
end

return m
