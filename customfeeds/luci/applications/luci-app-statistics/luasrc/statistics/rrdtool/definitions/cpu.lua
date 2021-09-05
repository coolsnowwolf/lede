-- Copyright 2008 Freifunk Leipzig / Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

module("luci.statistics.rrdtool.definitions.cpu",package.seeall)

function rrdargs( graph, plugin, plugin_instance, dtype )

	return {
		title = "%H: Processor usage on core #%pi",
		y_min = "0",
		alt_autoscale_max = true,
		vlabel = "Percent",
		number_format = "%5.1lf%%",
		data = {
			instances = { 
				cpu = { "user", "nice", "system", "softirq", "interrupt" }
			},

			options = {
				cpu_idle      = { color = "ffffff", title = "Idle" },
				cpu_nice      = { color = "00e000", title = "Nice" },
				cpu_user      = { color = "0000ff", title = "User" },
				cpu_wait      = { color = "ffb000", title = "Wait" },
				cpu_system    = { color = "ff0000", title = "System" },
				cpu_softirq   = { color = "ff00ff", title = "Softirq" },
				cpu_interrupt = { color = "a000a0", title = "Interrupt" },
				cpu_steal     = { color = "000000", title = "Steal" }
			}
		}
	}
end
