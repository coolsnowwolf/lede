-- Copyright 2008 Freifunk Leipzig / Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

module("luci.statistics.rrdtool.definitions.processes", package.seeall)

function rrdargs( graph, plugin, plugin_instance, dtype )

    if  plugin_instance == "" then
	return {
			title = "%H: Processes",
			vlabel = "Processes/s",
			data = {
				instances = {
					ps_state = {
						"sleeping", "running", "paging",
						"blocked", "stopped", "zombies"
					}
				},

				options = {
					ps_state_sleeping = { color = "0000ff", title = "Sleeping" },
					ps_state_running  = { color = "008000", title = "Running" },
					ps_state_paging   = { color = "ffff00", title = "Paging" },
					ps_state_blocked  = { color = "ff5000", title = "Blocked" },
					ps_state_stopped  = { color = "555555", title = "Stopped" },
					ps_state_zombies  = { color = "ff0000", title = "Zombies" }
				}
			}
	}
    else
	return {
		{
			title = "%H: CPU time used by %pi",
			vlabel = "Jiffies",
			data = {
				sources = {
					ps_cputime = { "syst", "user" }
				},

				options = {
					ps_cputime__user = {
						color   = "0000ff",
						title   = "User",
						overlay = true
					},

					ps_cputime__syst = {
						color   = "ff0000",
						title   = "System",
						overlay = true
					}
				}
			}
		},

		{
			title = "%H: Threads and processes belonging to %pi",
			vlabel = "Count",
			detail = true,
			data = {
				sources = {
					ps_count = { "threads", "processes" }
				},

				options = {
					ps_count__threads   = { color = "00ff00", title = "Threads" },
					ps_count__processes = { color = "0000bb", title = "Processes" }
				}
			}
		},

		{
			title = "%H: Page faults in %pi",
			vlabel = "Page faults",
			detail = true,
			data = {
				sources = {
					ps_pagefaults = { "minflt", "majflt" }
				},

				options = {
					ps_pagefaults__minflt = { color = "0000ff", title = "Minor" },
					ps_pagefaults__majflt = { color = "ff0000", title = "Major" }
				}
			}
		},

		{
			title = "%H: Resident segment size (RSS) of %pi",
			vlabel = "Bytes",
			detail = true,
			number_format = "%5.1lf%sB",
			data = {
				types = { "ps_rss" },

				options = {
					ps_rss = { color = "0000ff", title = "Resident segment" }
				}
			}
		},

		{
			title = "%H: Virtual memory size (VSZ) of %pi",
			vlabel = "Bytes",
			detail = true,
			number_format = "%5.1lf%sB",
			data = {
				types = { "ps_vm" },

				options = {
					ps_vm = { color = "0000ff", title = "Virtual memory" }
				}
			}
		}
	}
    end
end
