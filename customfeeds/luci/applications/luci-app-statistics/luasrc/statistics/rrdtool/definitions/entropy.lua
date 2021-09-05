-- Copyright 2015 Hannu Nyman <hannu.nyman@iki.fi>
-- Licensed to the public under the Apache License 2.0.

module("luci.statistics.rrdtool.definitions.entropy", package.seeall)

function rrdargs( graph, plugin, plugin_instance, dtype )

	return {
		title = "%H: Available entropy",
		vlabel = "bits",
		number_format = "%4.0lf",
		data = {
			types = { "entropy" },
			options = { entropy = { title = "Entropy %di" } }
		}
	}

end

