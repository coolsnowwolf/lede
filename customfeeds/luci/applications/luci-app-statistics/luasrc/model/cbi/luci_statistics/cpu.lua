-- Copyright 2008 Freifunk Leipzig / Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

m = Map("luci_statistics",
	translate("CPU Plugin Configuration"),
	translate("The cpu plugin collects basic statistics about the processor usage."))

-- collectd_cpu config section
s = m:section( NamedSection, "collectd_cpu", "luci_statistics" )

-- collectd_cpu.enable
enable = s:option( Flag, "enable", translate("Enable this plugin") )
enable.default = 0

return m
