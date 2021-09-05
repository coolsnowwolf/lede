-- Copyright 2015 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

m = Map("luci_statistics",
	translate("APCUPS Plugin Configuration"),
	translate(
		"The APCUPS plugin collects statistics about the APC UPS."
	))

-- collectd_apcups config section
s = m:section( NamedSection, "collectd_apcups", "luci_statistics" )

-- collectd_apcups.enable
enable = s:option( Flag, "enable", translate("Enable this plugin") )
enable.default = 0

-- collectd_apcups.host (Host)
host = s:option( Value, "Host", translate("Monitor host"), translate ("Add multiple hosts separated by space."))
host.default = "localhost"
host:depends( "enable", 1 )

-- collectd_apcups.port (Port)
port = s:option( Value, "Port", translate("Port for apcupsd communication") )
port.isinteger = true
port.default   = 3551
port:depends( "enable", 1 )

return m
