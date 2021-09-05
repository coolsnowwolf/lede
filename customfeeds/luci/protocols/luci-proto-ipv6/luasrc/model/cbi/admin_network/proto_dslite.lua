-- Copyright 2011 Jo-Philipp Wich <jow@openwrt.org>
-- Copyright 2013 Steven Barth <steven@midlink.org>
-- Licensed to the public under the Apache License 2.0.

local map, section, net = ...

local peeraddr, ip6addr
local tunlink, defaultroute, metric, ttl, mtu




peeraddr = section:taboption("general", Value, "peeraddr",
	translate("DS-Lite AFTR address"))

peeraddr.rmempty  = false
peeraddr.datatype = "or(hostname,ip6addr)"

ip6addr = section:taboption("general", Value, "ip6addr",
	translate("Local IPv6 address"),
	translate("Leave empty to use the current WAN address"))

ip6addr.datatype = "ip6addr"


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
