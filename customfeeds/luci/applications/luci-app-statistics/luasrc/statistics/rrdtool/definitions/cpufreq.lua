-- Licensed to the public under the Apache License 2.0.

module("luci.statistics.rrdtool.definitions.cpufreq",package.seeall)

function rrdargs( graph, plugin, plugin_instance, dtype )

	return {
		title = "%H: Processor frequency",
		alt_autoscale = true,
		vlabel = "Frequency (Hz)",
		number_format = "%3.2lf%s",
		data = {
			sources = {
				cpufreq = { "" }
			},
			options = {
				cpufreq_0 = { color = "ff0000", title = "Core 0", noarea=true, overlay=true },
				cpufreq_1 = { color = "0000ff", title = "Core 1", noarea=true, overlay=true },
				cpufreq_2 = { color = "00ff00", title = "Core 2", noarea=true, overlay=true },
				cpufreq_3 = { color = "00ffff", title = "Core 3", noarea=true, overlay=true }
			}
		}
	}
end

