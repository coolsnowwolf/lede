-- Copyright 2008 Freifunk Leipzig / Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

module("luci.statistics.rrdtool.definitions.tcpconns", package.seeall)

function rrdargs( graph, plugin, plugin_instance, dtype )
	return {
		title = "%H: TCP connections to port %pi",
		vlabel = "Connections/s",
		number_format = "%5.0lf",
		data = {
			types = { "tcp_connections" },
			instances = {
				tcp_connections = {
			                "SYN_SENT", "SYN_RECV", "LISTEN", "ESTABLISHED",
					"LAST_ACK", "TIME_WAIT", "CLOSING", "CLOSE_WAIT",
					"CLOSED", "FIN_WAIT1", "FIN_WAIT2"
				},
				options = {
					load__ESTABLISHED = { title = "%di", noarea = true }
				}
			}
		}
	}
end
