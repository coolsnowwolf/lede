-- Copyright 2008 Freifunk Leipzig / Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

module("luci.statistics.rrdtool.definitions.iptables", package.seeall)

function rrdargs( graph, plugin, plugin_instance, dtype )

	return {
		{
			title = "%H: Firewall: Processed bytes in %pi",
			vlabel = "Bytes/s",
			number_format = "%5.0lf%sB/s",
			totals_format = "%5.0lf%sB",
			data = { 
				types = { "ipt_bytes" },
				options = {
					ipt_bytes = {
						total = true,
						title = "%di"
					}
				}
			}
		},

		{
			title = "%H: Firewall: Processed packets in %pi",
			vlabel = "Packets/s",
			number_format = "%5.1lf P/s",
			totals_format = "%5.0lf%s",
			data = {
				types = { "ipt_packets" },
				options = {
					ipt_packets = {
						total = true,
						title = "%di"
					}
				}
			}
		}
	}
end
