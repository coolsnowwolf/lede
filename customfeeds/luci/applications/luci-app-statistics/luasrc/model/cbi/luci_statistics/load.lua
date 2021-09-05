-- Copyright 2008 Freifunk Leipzig / Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

m = Map("luci_statistics",
	translate("Load Plugin Configuration"),
	translate(
		"The load plugin collects statistics about the general system load."
	))

-- collectd_wireless config section
s = m:section( NamedSection, "collectd_load", "luci_statistics" )

-- collectd_wireless.enable
enable = s:option( Flag, "enable", translate("Enable this plugin") )
enable.default = 0

return m
