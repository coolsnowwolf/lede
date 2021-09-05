-- Copyright 2013 Steven Barth <steven@midlink.org>
-- Licensed to the public under the Apache License 2.0.

local map, section, net = ...

local mode = section:taboption("general", ListValue, "mode", translate("Category"))
mode:value("auto", translate("Automatic"))
mode:value("external", translate("External"))
mode:value("internal", translate("Internal"))
mode:value("leaf", translate("Leaf"))
mode:value("guest", translate("Guest"))
mode:value("adhoc", translate("Ad-Hoc"))
mode:value("hybrid", translate("Hybrid"))
mode.default = "auto"



local plen = section:taboption("advanced", Value, "ip6assign", translate("IPv6 assignment length"),
	translate("Assign a part of given length of every public IPv6-prefix to this interface"))
plen.datatype = "max(128)"
plen.default = "64"

section:taboption("advanced", Value, "link_id", translate("IPv6 assignment hint"),
	translate("Assign prefix parts using this hexadecimal subprefix ID for this interface."))

plen = section:taboption("advanced", Value, "ip4assign", translate("IPv4 assignment length"))
plen.datatype = "max(32)"
plen.default = "24"

local o = section:taboption("advanced", Value, "dnsname", translate("DNS-Label / FQDN"))
o.default = map.name

luci.tools.proto.opt_macaddr(section, ifc, translate("Override MAC address"))

o = section:taboption("advanced", Value, "mtu", translate("Override MTU"))
o.placeholder = "1500"
o.datatype    = "max(9200)"
