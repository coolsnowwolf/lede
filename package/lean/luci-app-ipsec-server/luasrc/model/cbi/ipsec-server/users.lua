local d = require "luci.dispatcher"
local sys = require "luci.sys"

m = Map("luci-app-ipsec-server")

s = m:section(TypedSection, "ipsec_users", "IPSec Xauth PSK " .. translate("Users Manager"))
s.description = translate("Use a client that supports IPSec Xauth PSK (iOS or Android) to connect to this server.")
s.addremove = true
s.anonymous = true
s.template = "cbi/tblsection"

o = s:option(Flag, "enabled", translate("Enabled"))
o.default = 1
o.rmempty = false

o = s:option(Value, "username", translate("Username"))
o.placeholder = translate("Username")
o.rmempty = false

o = s:option(Value, "password", translate("Password"))
o.placeholder = translate("Password")
o.rmempty = false

if sys.call("command -v xl2tpd > /dev/null") == 0 then
    s = m:section(TypedSection, "l2tp_users", "L2TP/IPSec PSK " .. translate("Users Manager"))
    s.description = translate("Use a client that supports L2TP over IPSec PSK to connect to this server.")
    s.addremove = true
    s.anonymous = true
    s.template = "cbi/tblsection"
    s.extedit = d.build_url("admin", "vpn", "ipsec-server", "l2tp_user", "%s")
    function s.create(e, t)
        t = TypedSection.create(e, t)
        luci.http.redirect(e.extedit:format(t))
    end

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
end

return m
