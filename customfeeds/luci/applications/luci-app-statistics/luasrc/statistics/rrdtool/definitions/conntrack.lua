-- Copyright 2011 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

module("luci.statistics.rrdtool.definitions.conntrack",package.seeall)

function rrdargs( graph, plugin, plugin_instance, dtype )
	return {
		title = "%H: Conntrack entries",
		vlabel = "Count",
		number_format = "%5.0lf",
		data = {
			-- collectd 5.5+: specify "" to exclude "max" instance
			instances = {
				conntrack = { "" }
			},
			sources = {
				conntrack = { "value" }
			},
			options = {
				conntrack = { 
					color = "0000ff",
					title = "Tracked connections"
				}
			}
		}
	}
end
