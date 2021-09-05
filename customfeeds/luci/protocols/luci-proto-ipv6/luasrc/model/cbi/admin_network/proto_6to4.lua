-- Copyright 2011 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local map, section, net = ...

local ipaddr, defaultroute, metric, ttl, mtu


ipaddr = section:taboption("general", Value, "ipaddr",
	translate("Local IPv4 address"),
	translate("Leave empty to use the current WAN address"))

ipaddr.datatype = "ip4addr"

defaultroute = section:taboption("advanced", Flag, "defaultroute",
	translate("Use default gateway"),
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
