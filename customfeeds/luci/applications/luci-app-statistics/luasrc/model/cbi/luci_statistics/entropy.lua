-- Copyright 2015 Hannu Nyman <hannu.nyman@iki.fi>
-- Licensed to the public under the Apache License 2.0.

m = Map("luci_statistics",
	translate("Entropy Plugin Configuration"),
	translate("The entropy plugin collects statistics about the available entropy."))

s = m:section( NamedSection, "collectd_entropy", "luci_statistics" )

enable = s:option( Flag, "enable", translate("Enable this plugin") )
enable.default = 0

return m

