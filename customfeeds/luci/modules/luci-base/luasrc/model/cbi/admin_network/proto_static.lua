-- Copyright 2011 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local map, section, net = ...
local ifc = net:get_interface()

local ipaddr, netmask, gateway, broadcast, dns, accept_ra, send_rs, ip6addr, ip6gw
local mtu, metric


ipaddr = section:taboption("general", Value, "ipaddr", translate("IPv4 address"))
ipaddr.datatype = "ip4addr"


netmask = section:taboption("general", Value, "netmask",
	translate("IPv4 netmask"))

netmask.datatype = "ip4addr"
netmask:value("255.255.255.0")
netmask:value("255.255.0.0")
netmask:value("255.0.0.0")


gateway = section:taboption("general", Value, "gateway", translate("IPv4 gateway"))
gateway.datatype = "ip4addr"


broadcast = section:taboption("general", Value, "broadcast", translate("IPv4 broadcast"))
broadcast.datatype = "ip4addr"


dns = section:taboption("general", DynamicList, "dns",
	translate("Use custom DNS servers"))

dns.datatype = "ipaddr"
dns.cast     = "string"


if luci.model.network:has_ipv6() then

	local ip6assign = section:taboption("general", Value, "ip6assign", translate("IPv6 assignment length"),
		translate("Assign a part of given length of every public IPv6-prefix to this interface"))
	ip6assign:value("", translate("disabled"))
	ip6assign:value("64")
	ip6assign.datatype = "max(64)"

	local ip6hint = section:taboption("general", Value, "ip6hint", translate("IPv6 assignment hint"),
		translate("Assign prefix parts using this hexadecimal subprefix ID for this interface."))
	for i=33,64 do ip6hint:depends("ip6assign", i) end

	ip6addr = section:taboption("general", Value, "ip6addr", translate("IPv6 address"))
	ip6addr.datatype = "ip6addr"
	ip6addr:depends("ip6assign", "")


	ip6gw = section:taboption("general", Value, "ip6gw", translate("IPv6 gateway"))
	ip6gw.datatype = "ip6addr"
	ip6gw:depends("ip6assign", "")


	local ip6prefix = s:taboption("general", Value, "ip6prefix", translate("IPv6 routed prefix"),
		translate("Public prefix routed to this device for distribution to clients."))
	ip6prefix.datatype = "ip6addr"
	ip6prefix:depends("ip6assign", "")

	local ip6ifaceid = s:taboption("general", Value, "ip6ifaceid", translate("IPv6 suffix"),
		translate("Optional. Allowed values: 'eui64', 'random', fixed value like '::1' " ..
			"or '::1:2'. When IPv6 prefix (like 'a:b:c:d::') is received from a " ..
			"delegating server, use the suffix (like '::1') to form the IPv6 address " ..
			"('a:b:c:d::1') for the interface."))
	ip6ifaceid.datatype = "ip6hostid"
	ip6ifaceid.placeholder = "::1"
	ip6ifaceid.rmempty = true

end


luci.tools.proto.opt_macaddr(section, ifc, translate("Override MAC address"))


mtu = section:taboption("advanced", Value, "mtu", translate("Override MTU"))
mtu.placeholder = "1500"
mtu.datatype    = "max(9200)"


metric = section:taboption("advanced", Value, "metric",
	translate("Use gateway metric"))

metric.placeholder = "0"
metric.datatype    = "uinteger"
