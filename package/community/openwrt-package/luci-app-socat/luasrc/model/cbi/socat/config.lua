local d = require "luci.dispatcher"

m = Map("socat", translate("Socat Config"))
m.redirect = d.build_url("admin", "network", "socat")

s = m:section(NamedSection, arg[1], "config", "")
s.addremove = false
s.dynamic = false

o = s:option(Flag, "enable", translate("Enable"))
o.default = "1"
o.rmempty = false

o = s:option(Value, "remarks", translate("Remarks"))
o.default = translate("Remarks")
o.rmempty = false

o = s:option(ListValue, "protocol", translate("Protocol"))
o:value("port_forwards", translate("Port Forwards"))

o = s:option(ListValue, "family", translate("Restrict to address family"))
o:value("", translate("IPv4 and IPv6"))
o:value("4", translate("IPv4 only"))
o:value("6", translate("IPv6 only"))
o:depends("protocol", "port_forwards")

o = s:option(ListValue, "proto", translate("Protocol"))
o:value("tcp", "TCP")
o:value("udp", "UDP")
o:depends("protocol", "port_forwards")

o = s:option(Value, "listen_port", translate("Listen port"))
o.datatype = "portrange"
o.rmempty = false
o:depends("protocol", "port_forwards")

o = s:option(Flag, "reuseaddr", "reuseaddr", translate("Bind to a port local"))
o.default = "1"
o.rmempty = false

o = s:option(ListValue, "dest_proto", translate("Destination Protocol"))
o:value("tcp4", "IPv4-TCP")
o:value("udp4", "IPv4-UDP")
o:value("tcp6", "IPv6-TCP")
o:value("udp6", "IPv6-UDP")
o:depends("protocol", "port_forwards")

dest_ipv4 = s:option(Value, "dest_ipv4", translate("Destination address"))
luci.sys.net.ipv4_hints(function(ip, name)
	dest_ipv4:value(ip, "%s (%s)" %{ ip, name })
end)
function dest_ipv4.cfgvalue(self, section)
	return m:get(section, "dest_ip")
end
function dest_ipv4.write(self, section, value)
	m:set(section, "dest_ip", value)
end
dest_ipv4:depends("dest_proto", "tcp4")
dest_ipv4:depends("dest_proto", "udp4")

dest_ipv6 = s:option(Value, "dest_ipv6", translate("Destination address"))
luci.sys.net.ipv6_hints(function(ip, name)
	dest_ipv6:value(ip, "%s (%s)" %{ ip, name })
end)
function dest_ipv6.cfgvalue(self, section)
	return m:get(section, "dest_ip")
end
function dest_ipv6.write(self, section, value)
	m:set(section, "dest_ip", value)
end
dest_ipv6:depends("dest_proto", "tcp6")
dest_ipv6:depends("dest_proto", "udp6")

o = s:option(Value, "dest_port", translate("Destination port"))
o.datatype = "portrange"
o.rmempty = false

o = s:option(Flag, "firewall_accept", translate("Open firewall port"))
o.default = "1"
o.rmempty = false

return m
