-- Copyright 2015 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

module("luci.statistics.rrdtool.definitions.sensors", package.seeall)

function rrdargs( graph, plugin, plugin_instance )
	return {
		{
			per_instance = true,
			title = "%H: %pi - %di",
			vlabel = "\176C",
			number_format = "%4.1lf\176C",
			data = {
				types = { "temperature" },
				options = {
					temperature__value = {
						color = "ff0000",
						title = "Temperature"
					}
				}
			}
		}
	}
end
