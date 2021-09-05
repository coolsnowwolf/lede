-- Copyright 2013 Steven Barth <steven@midlink.org>
-- Licensed to the public under the Apache License 2.0.

local map, section, net = ...


local o = section:taboption("general", ListValue, "reqaddress",
	translate("Request IPv6-address"))
o:value("try")
o:value("force")
o:value("none", "disabled")
o.default = "try"


o = section:taboption("general", Value, "reqprefix",
	translate("Request IPv6-prefix of length"))
o:value("auto", translate("Automatic"))
o:value("no", translate("disabled"))
o:value("48")
o:value("52")
o:value("56")
o:value("60")
o:value("64")
o.default = "auto"


o = section:taboption("advanced", Flag, "defaultroute",
	translate("Use default gateway"),
	translate("If unchecked, no default route is configured"))
o.default = o.enabled


o = section:taboption("advanced", Flag, "peerdns",
	translate("Use DNS servers advertised by peer"),
	translate("If unchecked, the advertised DNS server addresses are ignored"))
o.default = o.enabled


o = section:taboption("advanced", Value, "ip6prefix",
	translate("Custom delegated IPv6-prefix"))
o.dataype = "ip6addr"


o = section:taboption("advanced", DynamicList, "dns",
	translate("Use custom DNS servers"))
o:depends("peerdns", "")
o.datatype = "list(ip6addr)"
o.cast     = "string"


o = section:taboption("advanced", Value, "clientid",
	translate("Client ID to send when requesting DHCP"))

luci.tools.proto.opt_macaddr(section, ifc, translate("Override MAC address"))

o = section:taboption("advanced", Value, "mtu", translate("Override MTU"))
o.placeholder = "1500"
o.datatype    = "max(9200)"
