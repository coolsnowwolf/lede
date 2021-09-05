-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Licensed to the public under the Apache License 2.0.

local wa = require "luci.tools.webadmin"
local fs = require "nixio.fs"

m = Map("network",
	translate("Routes"),
	translate("Routes specify over which interface and gateway a certain host or network " ..
		"can be reached."))

s = m:section(TypedSection, "route", translate("Static IPv4 Routes"))
s.addremove = true
s.anonymous = true

s.template  = "cbi/tblsection"

iface = s:option(ListValue, "interface", translate("Interface"))
wa.cbi_add_networks(iface)

t = s:option(Value, "target", translate("Target"), translate("Host-<abbr title=\"Internet Protocol Address\">IP</abbr> or Network"))
t.datatype = "ip4addr"
t.rmempty = false

n = s:option(Value, "netmask", translate("<abbr title=\"Internet Protocol Version 4\">IPv4</abbr>-Netmask"), translate("if target is a network"))
n.placeholder = "255.255.255.255"
n.datatype = "ip4addr"
n.rmempty = true

g = s:option(Value, "gateway", translate("<abbr title=\"Internet Protocol Version 4\">IPv4</abbr>-Gateway"))
g.datatype = "ip4addr"
g.rmempty = true

metric = s:option(Value, "metric", translate("Metric"))
metric.placeholder = 0
metric.datatype = "range(0,255)"
metric.size = 5
metric.rmempty = true

mtu = s:option(Value, "mtu", translate("MTU"))
mtu.placeholder = 1500
mtu.datatype = "range(64,9000)"
mtu.size = 5
mtu.rmempty = true

routetype = s:option(Value, "type", translate("Route type"))
routetype:value("", "unicast")
routetype:value("local", "local")
routetype:value("broadcast", "broadcast")
routetype:value("multicast", "multicast")
routetype:value("unreachable", "unreachable")
routetype:value("prohibit", "prohibit")
routetype:value("blackhole", "blackhole")
routetype:value("anycast", "anycast")
routetype.default = ""
routetype.rmempty = true

if fs.access("/proc/net/ipv6_route") then
	s = m:section(TypedSection, "route6", translate("Static IPv6 Routes"))
	s.addremove = true
	s.anonymous = true

	s.template  = "cbi/tblsection"

	iface = s:option(ListValue, "interface", translate("Interface"))
	wa.cbi_add_networks(iface)

	t = s:option(Value, "target", translate("Target"), translate("<abbr title=\"Internet Protocol Version 6\">IPv6</abbr>-Address or Network (CIDR)"))
	t.datatype = "ip6addr"
	t.rmempty = false

	g = s:option(Value, "gateway", translate("<abbr title=\"Internet Protocol Version 6\">IPv6</abbr>-Gateway"))
	g.datatype = "ip6addr"
	g.rmempty = true

	metric = s:option(Value, "metric", translate("Metric"))
	metric.placeholder = 0
	metric.datatype = "range(0,65535)" -- XXX: not sure
	metric.size = 5
	metric.rmempty = true

	mtu = s:option(Value, "mtu", translate("MTU"))
	mtu.placeholder = 1500
	mtu.datatype = "range(64,9000)"
	mtu.size = 5
	mtu.rmempty = true

	routetype = s:option(Value, "type", translate("Route type"))
	routetype:value("", "unicast")
	routetype:value("local", "local")
	routetype:value("broadcast", "broadcast")
	routetype:value("multicast", "multicast")
	routetype:value("unreachable", "unreachable")
	routetype:value("prohibit", "prohibit")
	routetype:value("blackhole", "blackhole")
	routetype:value("anycast", "anycast")
	routetype.default = ""
	routetype.rmempty = true
end


return m
