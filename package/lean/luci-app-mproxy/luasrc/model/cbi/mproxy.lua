local m,s,o
local SYS  = require "luci.sys"


if SYS.call("pidof mproxy >/dev/null") == 0 then
	Status = translate("<strong><font color=\"green\">Mproxy is Running</font></strong>")
else
	Status = translate("<strong><font color=\"red\">Mproxy is Not Running</font></strong>")
end

m = Map("mproxy")
m.title	= translate("Mproxy Server")
m.description = translate("A mini HTTP/HTTPS Proxy Server for Something")

s = m:section(TypedSection, "mproxy", "")
s.addremove = false
s.anonymous = true
s.description = translate(string.format("%s<br /><br />", Status))

enable = s:option(Flag, "enabled", translate("Enable"))
enable.default = 0
enable.rmempty = false

port = s:option(Value, "port", translate("Port"))
port.datatype = "range(1,65535)"

redir = s:option(Value, "redir", translate("Redirect to Local Port"))
redir.datatype = "range(1,65535)"
redir.rmempty = false
redir.description = translate("Redirecting traffic incoming to the specified local port")

function m.on_after_commit(self)
  os.execute("uci set firewall.mproxy.dest_port=$(uci get mproxy.config.port) && uci commit firewall &&  /etc/init.d/firewall restart")
  os.execute("/etc/init.d/mproxy restart")
end

return m
