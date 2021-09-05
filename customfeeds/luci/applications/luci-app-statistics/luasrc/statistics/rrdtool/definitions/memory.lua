--[[

(c) 2011 Manuel Munz <freifunk at somakoma dot de>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0
]]--

module("luci.statistics.rrdtool.definitions.memory",package.seeall)

function rrdargs( graph, plugin, plugin_instance, dtype )

	return {
		title = "%H: Memory usage",
		vlabel = "MB",
		number_format = "%5.1lf%s",
		y_min = "0",
		alt_autoscale_max = true,
		data = {
			instances = { 
				memory = { "free", "buffered", "cached", "used" }
			},

			options = {
				memory_buffered	= { color = "0000ff", title = "Buffered" },
				memory_cached	= { color = "ff00ff", title = "Cached" },
				memory_used	= { color = "ff0000", title = "Used" },
				memory_free	= { color = "00ff00", title = "Free" }
			}
		}
	}
end
