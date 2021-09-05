m = Map("pppoe-server", translate("PPPoE Server"))
m.description = translate(
                    "The PPPoE server is a broadband access authentication server that prevents ARP spoofing.")
s = m:section(TypedSection, "user", translate("Users Manager"))
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
o.datatype = "ipaddr"
o.rmempty = true
function o.cfgvalue(e, t)
    value = e.map:get(t, "ipaddress")
    return value == "*" and "" or value
end
function o.remove(e, t) Value.write(e, t, "*") end
return m
