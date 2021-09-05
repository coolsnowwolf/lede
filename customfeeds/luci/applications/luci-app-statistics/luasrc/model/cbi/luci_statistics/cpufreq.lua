-- Licensed to the public under the Apache License 2.0.

m = Map("luci_statistics",
	translate("CPU Frequency Plugin Configuration"),
	translate("This plugin collects statistics about the processor frequency scaling."))

-- collectd_cpufreq config section
s = m:section( NamedSection, "collectd_cpufreq", "luci_statistics" )

-- collectd_cpufreq.enable
enable = s:option( Flag, "enable", translate("Enable this plugin") )
enable.default = 0

return m
