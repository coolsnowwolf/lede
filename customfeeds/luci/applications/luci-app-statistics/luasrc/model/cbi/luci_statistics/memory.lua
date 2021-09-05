--[[

Copyright 2011 Manuel Munz <freifunk at somakoma dot de>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0
]]--

m = Map("luci_statistics",
	translate("Memory Plugin Configuration"),
	translate("The memory plugin collects statistics about the memory usage."))

s = m:section( NamedSection, "collectd_memory", "luci_statistics" )

enable = s:option( Flag, "enable", translate("Enable this plugin") )
enable.default = 0

return m
