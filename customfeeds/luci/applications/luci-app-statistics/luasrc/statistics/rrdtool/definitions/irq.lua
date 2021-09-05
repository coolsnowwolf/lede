-- Copyright 2008 Freifunk Leipzig / Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

module("luci.statistics.rrdtool.definitions.irq", package.seeall)

function rrdargs( graph, plugin, plugin_instance, dtype )

	return {
		title = "%H: Interrupts", vlabel = "Issues/s",
		number_format = "%5.0lf", data = {
			types = { "irq" },
			options = {
				irq = { title = "IRQ %di", noarea = true }
			}
		}
	}

end
