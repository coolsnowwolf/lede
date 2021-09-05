-- Copyright 2008 Freifunk Leipzig / Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

module("luci.statistics.rrdtool.definitions.df", package.seeall)

function rrdargs( graph, plugin, plugin_instance, dtype )

	return {
		title = "%H: Disk space usage on %pi",
		vlabel = "Bytes",
		number_format = "%5.1lf%sB",

		data = {
			instances = {
				df_complex = { "free", "used", "reserved" }
			},

			options = {
				df_complex_free = {
					color = "00ff00",
					overlay = false,
					title = "free"
				},

				df_complex_used = {
					color = "ff0000",
					overlay = false,
					title = "used"
				},

				df_complex_reserved = {
					color = "0000ff",
					overlay = false,
					title = "reserved"
				}
			}
		}
	}
end
