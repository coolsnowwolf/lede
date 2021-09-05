-- Copyright 2015 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

module("luci.statistics.rrdtool.definitions.apcups",package.seeall)

function rrdargs( graph, plugin, plugin_instance )

	local lu = require("luci.util")
	local rv = { }

	-- Types and instances supported by APC UPS
	-- e.g. ups_types -> { 'timeleft', 'charge', 'percent', 'voltage' }
	-- e.g. ups_inst['voltage'] -> { 'input', 'battery' }

	local ups_types = graph.tree:data_types( plugin, plugin_instance )

	local ups_inst = {}
	for _, t in ipairs(ups_types) do
		ups_inst[t] = graph.tree:data_instances( plugin, plugin_instance, t )
	end


    -- Check if hash table or array is empty or nil-filled

	local function empty( t )
		for _, v in pairs(t) do
			if type(v) then return false end
		end
		return true
	end


	-- Append graph definition but only types/instances which are
	-- supported and available to the plugin and UPS.

	local function add_supported( t, defs )
		local def_inst = defs['data']['instances']

		if type(def_inst) == "table" then
			for k, v in pairs( def_inst ) do
				if lu.contains( ups_types, k) then
					for j = #v, 1, -1 do
						if not lu.contains( ups_inst[k], v[j] ) then
							table.remove( v, j )
						end
					end
					if #v == 0 then
						def_inst[k] = nil  -- can't assign v: immutable
					end
				else
					def_inst[k] = nil  -- can't assign v: immutable
				end
			end
			if empty(def_inst) then return end
		end
		table.insert( t, defs )
	end


    -- Graph definitions for APC UPS measurements MUST use only 'instances':
    -- e.g. instances = { voltage = {  "input", "output" } }

	local voltagesdc = {
		title = "%H: Voltages on APC UPS - Battery",
		vlabel = "Volts DC",
		alt_autoscale = true,
		number_format = "%5.1lfV",
		data = {
			instances = {
				voltage = { "battery" }
			},
			options = {
				voltage = { title = "Battery voltage", noarea=true }
			}
		}
	}
	add_supported( rv, voltagesdc )

	local voltagesac = {
		title = "%H: Voltages on APC UPS - AC",
		vlabel = "Volts AC",
		alt_autoscale = true,
		number_format = "%5.1lfV",
		data = {
			instances = {
				voltage = {  "input", "output" }
			},
			options = {
				voltage_output  = { color = "00e000", title = "Output voltage", noarea=true, overlay=true },
				voltage_input   = { color = "ffb000", title = "Input voltage", noarea=true, overlay=true }
			}
		}
	}
	add_supported( rv, voltagesac )

	local percentload = {
		title = "%H: Load on APC UPS ",
		vlabel = "Percent",
		y_min = "0",
		y_max = "100",
		number_format = "%5.1lf%%",
		data = {
			instances = {
				percent = { "load" }
			},
			options = {
				percent_load = { color = "00ff00", title = "Load level"  }
			}
		}
	}
	add_supported( rv, percentload )

	local charge_percent = {
		title = "%H: Battery charge on APC UPS ",
		vlabel = "Percent",
		y_min = "0",
		y_max = "100",
		number_format = "%5.1lf%%",
		data = {
			instances = {
				charge = { "" }
			},
			options = {
				charge = { color = "00ff0b", title = "Charge level"  }
			}
		}
	}
	add_supported( rv, charge_percent )

	local temperature = {
		title = "%H: Battery temperature on APC UPS ",
		vlabel = "\176C",
		number_format = "%5.1lf\176C",
		data = {
			instances = {
				temperature = { "" }
			},
			options = {
				temperature = { color = "ffb000", title = "Battery temperature" } }
		}
	}
	add_supported( rv, temperature )

	local timeleft = {
		title = "%H: Time left on APC UPS ",
		vlabel = "Minutes",
		number_format = "%.1lfm",
		data = {
			instances = {
				timeleft = { "" }
			},
			options = {
				timeleft = { color = "0000ff", title = "Time left" }
			}
		}
	}
	add_supported( rv, timeleft )

	local frequency = {
		title = "%H: Incoming line frequency on APC UPS ",
		vlabel = "Hz",
		number_format = "%5.0lfhz",
		data = {
			instances = {
				frequency = { "input" }
			},
			options = {
				frequency_input = { color = "000fff", title = "Line frequency" }
			}
		}
	}
	add_supported( rv, frequency )

	return rv
end
