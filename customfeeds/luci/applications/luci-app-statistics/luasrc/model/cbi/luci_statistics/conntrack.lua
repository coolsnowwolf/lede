--[[

Copyright 2011 Jo-Philipp Wich <jow@openwrt.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0
]]--

m = Map("luci_statistics",
	translate("Conntrack Plugin Configuration"),
	translate("The conntrack plugin collects statistics about the number of tracked connections."))

s = m:section( NamedSection, "collectd_conntrack", "luci_statistics" )

enable = s:option( Flag, "enable", translate("Enable this plugin") )
enable.default = 0

return m
