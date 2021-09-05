-- Copyright 2011 Jo-Philipp Wich <jow@openwrt.org>
-- Copyright 2013 Steven Barth <steven@midlink.org>
-- Licensed to the public under the Apache License 2.0.

local map, section, net = ...

local peeraddr, ip6addr
local tunlink, defaultroute, metric, ttl, mtu


maptype = section:taboption("general", ListValue, "type", translate("Type"))
maptype:value("map-e", "MAP-E")
maptype:value("map-t", "MAP-T")
maptype:value("lw4o6", "LW4over6")


peeraddr = section:taboption("general", Value, "peeraddr",
	translate("BR / DMR / AFTR"))

peeraddr.rmempty  = false
peeraddr.datatype = "ip6addr"


ipaddr = section:taboption("general", Value, "ipaddr",
	translate("IPv4 prefix"))
ipaddr.datatype = "ip4addr"


ip4prefixlen = s:taboption("general", Value, "ip4prefixlen",
        translate("IPv4 prefix length"),
        translate("The length of the IPv4 prefix in bits, the remainder is used in the IPv6 addresses."))

ip4prefixlen.placeholder = "32"
ip4prefixlen.datatype    = "range(0,32)"

ip6addr = s:taboption("general", Value, "ip6prefix",
        translate("IPv6 prefix"),
        translate("The IPv6 prefix assigned to the provider, usually ends with <code>::</code>"))

ip6addr.rmempty  = false
ip6addr.datatype = "ip6addr"


ip6prefixlen = s:taboption("general", Value, "ip6prefixlen",
        translate("IPv6 prefix length"),
        translate("The length of the IPv6 prefix in bits"))

ip6prefixlen.placeholder = "16"
ip6prefixlen.datatype    = "range(0,64)"


s:taboption("general", Value, "ealen",
	translate("EA-bits length")).datatype = "range(0,48)"

s:taboption("general", Value, "psidlen",
	translate("PSID-bits length")).datatype = "range(0,16)"

s:taboption("general", Value, "offset",
	translate("PSID offset")).datatype = "range(0,16)"

tunlink = section:taboption("advanced", DynamicList, "tunlink", translate("Tunnel Link"))
tunlink.template = "cbi/network_netlist"
tunlink.nocreate = true


defaultroute = section:taboption("advanced", Flag, "defaultroute",
	translate("Default gateway"),
	translate("If unchecked, no default route is configured"))

defaultroute.default = defaultroute.enabled


metric = section:taboption("advanced", Value, "metric",
	translate("Use gateway metric"))

metric.placeholder = "0"
metric.datatype    = "uinteger"
metric:depends("defaultroute", defaultroute.enabled)


ttl = section:taboption("advanced", Value, "ttl", translate("Use TTL on tunnel interface"))
ttl.placeholder = "64"
ttl.datatype    = "range(1,255)"


mtu = section:taboption("advanced", Value, "mtu", translate("Use MTU on tunnel interface"))
mtu.placeholder = "1280"
mtu.datatype    = "max(9200)"
