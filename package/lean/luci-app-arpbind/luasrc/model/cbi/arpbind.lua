local sys = require "luci.sys"
local ifaces = sys.net:devices()

m = Map("arpbind")
m.title = translate("IP/MAC Binding")
m.description = translatef("ARP is used to convert a network address (e.g. an IPv4 address) to a physical address such as a MAC address.Here you can add some static ARP binding rules.")

s = m:section(TypedSection, "arpbind", translate("Rules"))
s.template = "cbi/tblsection"
s.anonymous = true
s.addremove = true

a = s:option(Value, "ipaddr", translate("IP Address"))
a.optional = false
a.datatype = "ipaddr"
luci.ip.neighbors({ family = 4 }, function(entry)
       if entry.reachable then
               a:value(entry.dest:string())
       end
end)

a = s:option(Value, "macaddr", translate("MAC Address"))
a.datatype = "macaddr"
a.optional = false
luci.ip.neighbors({family = 4}, function(neighbor)
	if neighbor.reachable then
		a:value(neighbor.mac, "%s (%s)" %{neighbor.mac, neighbor.dest:string()})
	end
end)

a = s:option(ListValue, "ifname", translate("Interface"))
for _, iface in ipairs(ifaces) do
	if iface ~= "lo" then
		a:value(iface)
	end
end
a.default = "br-lan"
a.rmempty = false

return m
