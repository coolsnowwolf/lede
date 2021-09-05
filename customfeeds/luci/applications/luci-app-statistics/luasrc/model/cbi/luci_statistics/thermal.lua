-- Licensed to the public under the Apache License 2.0.

m = Map("luci_statistics",
	translate("Thermal Plugin Configuration"),
	translate("The thermal plugin will monitor temperature of the system. " ..
		"Data is typically read from /sys/class/thermal/*/temp " ..
		"( '*' denotes the thermal device to be read, e.g. thermal_zone1 )")
	)

-- collectd_thermal config section
s = m:section( NamedSection, "collectd_thermal", "luci_statistics" )

-- collectd_thermal.enable
enable = s:option( Flag, "enable", translate("Enable this plugin") )
enable.default = 0

-- collectd_thermal.tz (Device)
tz = s:option( Value, "Device", translate("Monitor device(s) / thermal zone(s)"),
		translate("Empty value = monitor all") )
tz.optional = true
tz:depends( "enable", 1 )

-- collectd_thermal.ignoreselected (IgnoreSelected)
ignoreselected = s:option( Flag, "IgnoreSelected", translate("Monitor all except specified") )
ignoreselected.default  = 0
ignoreselected.optional = true
ignoreselected:depends( "enable", 1 )

return m
