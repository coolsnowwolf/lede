local m,s,o
local SYS  = require "luci.sys"


if SYS.call("iptables --list | grep FLOWOFFLOAD >/dev/null") == 0 then
	Status = translate("<strong><font color=\"green\">Linux Flow offload Forwarding Engine is Running</font></strong>")
else
	Status = translate("<strong><font color=\"red\">Linux Flow offload Forwarding  Engine is Not Running</font></strong>")
end

m = Map("flowoffload")
m.title	= translate("Linux Flow Offload Forwarding Engine Settings")
m.description = translate("Opensource Linux Flow Offload driver (Fast Path or HWNAT)")

s = m:section(TypedSection, "flowoffload", "")
s.addremove = false
s.anonymous = true
s.description = translate(string.format("%s<br /><br />", Status))

enable = s:option(Flag, "enabled", translate("Enable"))
enable.default = 0
enable.rmempty = false


return m
