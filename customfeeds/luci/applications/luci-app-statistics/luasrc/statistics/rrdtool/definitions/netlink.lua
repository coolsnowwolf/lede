-- Copyright 2008 Freifunk Leipzig / Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

module("luci.statistics.rrdtool.definitions.netlink", package.seeall)

function rrdargs( graph, plugin, plugin_instance )

	--
	-- traffic diagram
	--
	local traffic = {
		title = "%H: Netlink - Transfer on %pi",
		vlabel = "Bytes/s",

		-- diagram data description
		data = {
			-- defined sources for data types, if ommitted assume a single DS named "value" (optional)
			sources = {
				if_octets = { "tx", "rx" }
			},

			-- special options for single data lines
			options = {
				if_octets__tx = {
					title = "Bytes (TX)",
					total = true,		-- report total amount of bytes
					color = "00ff00"	-- tx is green
				},

				if_octets__rx = {
					title = "Bytes (RX)",
					flip  = true,		-- flip rx line
					total = true,		-- report total amount of bytes
					color = "0000ff"	-- rx is blue
				}
			}
		}
	}


	--
	-- packet diagram
	--
	local packets = {
		title = "%H: Netlink - Packets on %pi",
		vlabel = "Packets/s", detail = true,

		-- diagram data description
		data = {
			-- data type order
			types = { "if_packets", "if_dropped", "if_errors" },

			-- defined sources for data types
			sources = {
				if_packets = { "tx", "rx" },
				if_dropped = { "tx", "rx" },
				if_errors  = { "tx", "rx" }
			},

			-- special options for single data lines
			options = {
				-- processed packets (tx DS)
				if_packets__tx = {
					weight  = 2,
					title   = "Total   (TX)",
					overlay = true,		-- don't summarize
					total   = true,		-- report total amount of bytes
					color   = "00ff00"	-- processed tx is green
				},

				-- processed packets (rx DS)
				if_packets__rx = {
					weight  = 3,
					title   = "Total   (RX)",
					overlay = true,		-- don't summarize
					flip    = true,		-- flip rx line
					total   = true,		-- report total amount of bytes
					color   = "0000ff"	-- processed rx is blue
				},

				-- dropped packets (tx DS)
				if_dropped__tx = {
					weight  = 1,
					title   = "Dropped (TX)",
					overlay = true,		-- don't summarize
					total   = true,		-- report total amount of bytes
					color   = "660055"	-- dropped tx is ... dunno ;)
				},

				-- dropped packets (rx DS)
				if_dropped__rx = {
					weight  = 4,
					title   = "Dropped (RX)",
					overlay = true,		-- don't summarize
					flip    = true,		-- flip rx line
					total   = true,		-- report total amount of bytes
					color   = "ff00ff"	-- dropped rx is violett
				},

				-- packet errors (tx DS)
				if_errors__tx = {
					weight  = 0,
					title   = "Errors  (TX)",
					overlay = true,		-- don't summarize
					total   = true,		-- report total amount of packets
					color   = "ff5500"	-- tx errors are orange
				},

				-- packet errors (rx DS)
				if_errors__rx = {
					weight  = 5,
					title   = "Errors  (RX)",
					overlay = true,		-- don't summarize
					flip    = true,		-- flip rx line
					total   = true,		-- report total amount of packets
					color   = "ff0000"	-- rx errors are red
				}
			}
		}
	}


	--
	-- multicast diagram
	--
	local multicast = {
		title = "%H: Netlink - Multicast on %pi",
		vlabel = "Packets/s", detail = true,

		-- diagram data description
		data = {
			-- data type order
			types = { "if_multicast" },

			-- special options for single data lines
			options = {
				-- multicast packets
				if_multicast = {
					title = "Packets",
					total = true,		-- report total amount of packets
					color = "0000ff"	-- multicast is blue
				}
			}
		}
	}


	--
	-- collision diagram
	--
	local collisions = {
		title = "%H: Netlink - Collisions on %pi",
		vlabel = "Collisions/s", detail = true,

		-- diagram data description
		data = {
			-- data type order
			types = { "if_collisions" },

			-- special options for single data lines
			options = {
				-- collision rate
				if_collisions = {
					title = "Collisions",
					total = true,		-- report total amount of packets
					color = "ff0000"	-- collsions are red
				}
			}
		}
	}


	--
	-- error diagram
	--
	local errors = {
		title = "%H: Netlink - Errors on %pi",
		vlabel = "Errors/s", detail = true,

		-- diagram data description
		data = {
			-- data type order
			types = { "if_tx_errors", "if_rx_errors" },

			-- data type instances
			instances = {
				if_tx_errors = { "aborted", "carrier", "fifo", "heartbeat", "window" },
				if_rx_errors = { "length", "missed", "over", "crc", "fifo", "frame" }
			},

			-- special options for single data lines
			options = {
				if_tx_errors_aborted_value   = { total = true, color = "ffff00", title = "Aborted   (TX)" },
				if_tx_errors_carrier_value   = { total = true, color = "ffcc00", title = "Carrier   (TX)" },
				if_tx_errors_fifo_value      = { total = true, color = "ff9900", title = "Fifo      (TX)" },
				if_tx_errors_heartbeat_value = { total = true, color = "ff6600", title = "Heartbeat (TX)" },
				if_tx_errors_window_value    = { total = true, color = "ff3300", title = "Window    (TX)" },

				if_rx_errors_length_value    = { flip = true, total = true, color = "ff0000", title = "Length    (RX)" },
				if_rx_errors_missed_value    = { flip = true, total = true, color = "ff0033", title = "Missed    (RX)" },
				if_rx_errors_over_value      = { flip = true, total = true, color = "ff0066", title = "Over      (RX)" },
				if_rx_errors_crc_value       = { flip = true, total = true, color = "ff0099", title = "CRC       (RX)" },
				if_rx_errors_fifo_value      = { flip = true, total = true, color = "ff00cc", title = "Fifo      (RX)" },
				if_rx_errors_frame_value     = { flip = true, total = true, color = "ff00ff", title = "Frame     (RX)" }
			}
		}
	}


	return { traffic, packets, multicast, collisions, errors }
end
