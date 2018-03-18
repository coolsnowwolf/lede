local m, s, o
local uci = luci.model.uci.cursor()
local servers = {}

local function has_bin(name)
	return luci.sys.call("command -v %s >/dev/null" %{name}) == 0
end

if not has_bin("udp2raw") then
	return Map("udp2raw", "%s - %s" %{translate("udp2raw-tunnel"),
		translate("Settings")}, '<b style="color:red">udp2raw-tunnel binary file not found. install udp2raw-tunnel package, or copy binary to /usr/bin/udp2raw manually. </b>')
end

uci:foreach("udp2raw", "servers", function(s)
	if s.server_addr and s.server_port then
		servers[#servers+1] = {name = s[".name"], alias = s.alias or "%s:%s" %{s.server_addr, s.server_port}}
	end
end)

m = Map("udp2raw", "%s - %s" %{translate("udp2raw-tunnel"), translate("Settings")})
m:append(Template("udp2raw/status"))

s = m:section(NamedSection, "general", "general", translate("General Settings"))
s.anonymous = true
s.addremove = false

o = s:option(DynamicList, "server", translate("Server"))
o.template = "udp2raw/dynamiclist"
o:value("nil", translate("Disable"))
for _, s in ipairs(servers) do o:value(s.name, s.alias) end
o.default = "nil"
o.rmempty = false

o = s:option(ListValue, "daemon_user", translate("Run Daemon as User"))
for u in luci.util.execi("cat /etc/passwd | cut -d ':' -f1") do o:value(u) end
o.default = "root"
o.rmempty = false

return m
