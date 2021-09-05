-- Copyright 2008 Freifunk Leipzig / Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

require("luci.sys")


m = Map("luci_statistics",
	translate("Interface Plugin Configuration"),
	translate(
		"The interface plugin collects traffic statistics on " ..
		"selected interfaces."
	))

-- collectd_interface config section
s = m:section( NamedSection, "collectd_interface", "luci_statistics" )

-- collectd_interface.enable
enable = s:option( Flag, "enable", translate("Enable this plugin") )
enable.default = 0

-- collectd_interface.interfaces (Interface)
interfaces = s:option( MultiValue, "Interfaces", translate("Monitor interfaces") )
interfaces.widget = "select"
interfaces.size   = 5
interfaces:depends( "enable", 1 )
for k, v in pairs(luci.sys.net.devices()) do
	interfaces:value(v)
end

-- collectd_interface.ignoreselected (IgnoreSelected)
ignoreselected = s:option( Flag, "IgnoreSelected", translate("Monitor all except specified") )
ignoreselected.default = 0
ignoreselected:depends( "enable", 1 )

return m
