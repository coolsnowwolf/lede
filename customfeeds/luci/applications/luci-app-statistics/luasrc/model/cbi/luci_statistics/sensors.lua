-- Copyright 2015 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

require "luci.sys"

local m, s, o
local sensor_types = {
	["12v"] = "voltage",
	["2.0v"] = "voltage",
	["2.5v"] = "voltage",
	["3.3v"] = "voltage",
	["5.0v"] = "voltage",
	["5v"] = "voltage",
	["ain1"] = "voltage",
	["ain2"] = "voltage",
	["cpu_temp"] = "temperature",
	["fan1"] = "fanspeed",
	["fan2"] = "fanspeed",
	["fan3"] = "fanspeed",
	["fan4"] = "fanspeed",
	["fan5"] = "fanspeed",
	["fan6"] = "fanspeed",
	["fan7"] = "fanspeed",
	["in0"] = "voltage",
	["in10"] = "voltage",
	["in2"] = "voltage",
	["in3"] = "voltage",
	["in4"] = "voltage",
	["in5"] = "voltage",
	["in6"] = "voltage",
	["in7"] = "voltage",
	["in8"] = "voltage",
	["in9"] = "voltage",
	["power1"] = "power",
	["remote_temp"] = "temperature",
	["temp1"] = "temperature",
	["temp2"] = "temperature",
	["temp3"] = "temperature",
	["temp4"] = "temperature",
	["temp5"] = "temperature",
	["temp6"] = "temperature",
	["temp7"] = "temperature",
	["temp"] = "temperature",
	["vccp1"] = "voltage",
	["vccp2"] = "voltage",
	["vdd"] = "voltage",
	["vid1"] = "voltage",
	["vid2"] = "voltage",
	["vid3"] = "voltage",
	["vid4"] = "voltage",
	["vid5"] = "voltage",
	["vid"] = "voltage",
	["vin1"] = "voltage",
	["vin2"] = "voltage",
	["vin3"] = "voltage",
	["vin4"] = "voltage",
	["volt12"] = "voltage",
	["volt5"] = "voltage",
	["voltbatt"] = "voltage",
	["vrm"] = "voltage"

}


m = Map("luci_statistics",
	translate("Sensors Plugin Configuration"),
	translate("The sensors plugin uses the Linux Sensors framework to gather environmental statistics."))

s = m:section( NamedSection, "collectd_sensors", "luci_statistics" )


o = s:option( Flag, "enable", translate("Enable this plugin") )
o.default = 0


o = s:option(Flag, "__all", translate("Monitor all sensors"))
o:depends("enable", 1)
o.default = 1
o.write = function() end
o.cfgvalue = function(self, sid)
	local v = self.map:get(sid, "Sensor")
	if v == nil or (type(v) == "table" and #v == 0) or (type(v) == "string" and #v == 0) then
		return "1"
	end
end


o = s:option(MultiValue, "Sensor", translate("Sensor list"), translate("Hold Ctrl to select multiple items or to deselect entries."))
o:depends({enable = 1, __all = "" })
o.widget = "select"
o.rmempty = true
o.size = 0

local sensorcli = io.popen("/usr/sbin/sensors -u -A")
if sensorcli then
	local bus, sensor

	while true do
		local ln = sensorcli:read("*ln")
		if not ln then
			break
		elseif ln:match("^[%w-]+$") then
			bus = ln
		elseif ln:match("^[%w-]+:$") then
			sensor = ln:sub(0, -2):lower()
			if bus and sensor_types[sensor] then
				o:value("%s/%s-%s" %{ bus, sensor_types[sensor], sensor })
				o.size = o.size + 1
			end
		elseif ln == "" then
			bus = nil
			sensor = nil
		end
	end

	sensorcli:close()
end


o = s:option( Flag, "IgnoreSelected", translate("Monitor all except specified") )
o.default = 0
o.rmempty = true
o:depends({ enable = 1, __all = "" })

return m
