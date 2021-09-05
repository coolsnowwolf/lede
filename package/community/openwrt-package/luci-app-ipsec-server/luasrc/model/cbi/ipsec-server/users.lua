local sys = require "luci.sys"

m = Map("luci-app-ipsec-server")

s = m:section(TypedSection, "ipsec_users", "IPSec Xauth PSK " .. translate("Users Manager"))
s.description = translate("Use a client that supports IPSec Xauth PSK (iOS or Android) to connect to this server.")
s.addremove = true
s.anonymous = true
s.template = "cbi/tblsection"

o = s:option(Flag, "enabled", translate("Enabled"))
o.rmempty = false

o = s:option(Value, "username", translate("User name"))
o.placeholder = translate("User name")
o.rmempty = true

o = s:option(Value, "password", translate("Password"))
o.rmempty = true

if sys.call("command -v xl2tpd > /dev/null") == 0 then
    s = m:section(TypedSection, "l2tp_users", "L2TP/IPSec PSK " .. translate("Users Manager"))
    s.description = translate("Use a client that supports L2TP over IPSec PSK to connect to this server.")
    s.addremove = true
    s.anonymous = true
    s.template = "cbi/tblsection"

    o = s:option(Flag, "enabled", translate("Enabled"))
    o.rmempty = false

    o = s:option(Value, "username", translate("User name"))
    o.placeholder = translate("User name")
    o.rmempty = true

    o = s:option(Value, "password", translate("Password"))
    o.rmempty = true

    o = s:option(Value, "ipaddress", translate("IP address"))
    o.placeholder = translate("Automatically")
    o.datatype = "ip4addr"
    o.rmempty = true
end

return m
