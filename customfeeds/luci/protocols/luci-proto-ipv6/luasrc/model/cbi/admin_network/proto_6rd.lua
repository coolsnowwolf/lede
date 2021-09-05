-- Copyright 2011-2012 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local map, section, net = ...

local ipaddr, peeraddr, ip6addr, tunnelid, username, password
local defaultroute, metric, ttl, mtu


ipaddr = s:taboption("general", Value, "ipaddr",
	translate("Local IPv4 address"),
	translate("Leave empty to use the current WAN address"))

ipaddr.datatype = "ip4addr"


peeraddr = s:taboption("general", Value, "peeraddr",
	translate("Remote IPv4 address"),
	translate("This IPv4 address of the relay"))

peeraddr.rmempty  = false
peeraddr.datatype = "ip4addr"


ip6addr = s:taboption("general", Value, "ip6prefix",
	translate("IPv6 prefix"),
	translate("The IPv6 prefix assigned to the provider, usually ends with <code>::</code>"))

ip6addr.rmempty  = false
ip6addr.datatype = "ip6addr"


ip6prefixlen = s:taboption("general", Value, "ip6prefixlen",
	translate("IPv6 prefix length"),
	translate("The length of the IPv6 prefix in bits"))

ip6prefixlen.placeholder = "16"
ip6prefixlen.datatype    = "range(0,128)"


ip6prefixlen = s:taboption("general", Value, "ip4prefixlen",
	translate("IPv4 prefix length"),
	translate("The length of the IPv4 prefix in bits, the remainder is used in the IPv6 addresses."))

ip6prefixlen.placeholder = "0"
ip6prefixlen.datatype    = "range(0,32)"



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
