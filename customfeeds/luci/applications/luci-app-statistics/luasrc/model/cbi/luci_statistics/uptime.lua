--[[

Copyright 2013 Thomas Endt <tmo26@gmx.de>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0
]]--

m = Map("luci_statistics",
        translate("Uptime Plugin Configuration"),
        translate("The uptime plugin collects statistics about the uptime of the system."))

s = m:section( NamedSection, "collectd_uptime", "luci_statistics" )

enable = s:option( Flag, "enable", translate("Enable this plugin") )
enable.default = 0

return m

