-- Copyright 2017-2018 Dirk Brenken (dev@brenken.org)
-- This is free software, licensed under the Apache License, Version 2.0

local fs       = require("nixio.fs")
local uci      = require("luci.model.uci").cursor()
local json     = require("luci.jsonc")
local util     = require("luci.util")
local nw       = require("luci.model.network").init()
local fw       = require("luci.model.firewall").init()
local dump     = util.ubus("network.interface", "dump", {})
local trmiface = uci:get("travelmate", "global", "trm_iface") or "trm_wwan"
local trminput = uci:get("travelmate", "global", "trm_rtfile") or "/tmp/trm_runtime.json"
local uplink   = uci:get("network", trmiface) or ""
local parse    = json.parse(fs.readfile(trminput) or "")

m = Map("travelmate", translate("Travelmate"),
	translate("Configuration of the travelmate package to to enable travel router functionality. ")
	.. translatef("For further information "
	.. "<a href=\"%s\" target=\"_blank\">"
	.. "see online documentation</a>", "https://github.com/openwrt/packages/blob/master/net/travelmate/files/README.md"))
m:chain("network")
m:chain("firewall")
m.apply_on_parse = true

function m.on_apply(self)
	luci.sys.call("env -i /etc/init.d/travelmate restart >/dev/null 2>&1")
	luci.http.redirect(luci.dispatcher.build_url("admin", "services", "travelmate"))
end

-- Interface Wizard

if uplink == "" then
	ds = m:section(NamedSection, "global", "travelmate", translate("Interface Wizard"))
	o = ds:option(Value, "trm_iface", translate("Create Uplink interface"),
		translate("Create a new wireless wan uplink interface, configure it to use dhcp and ")
		.. translate("add it to the wan zone of the firewall.<br />")
		.. translate("This step has only to be done once."))
	o.datatype = "and(uciname,rangelength(3,15))"
	o.default = trmiface
	o.rmempty = false

	function o.validate(self, value)
		if value then
			local nwnet = nw:get_network(value)
			local zone  = fw:get_zone("wan")
			local fwnet = fw:get_zone_by_network(value)
			if not nwnet then
				nwnet = nw:add_network(value, { proto = "dhcp" })
			end
			if zone and not fwnet then
				fwnet = zone:add_network(value)
			end
		end
		return value
	end
	return m
end

-- Main travelmate options

s = m:section(NamedSection, "global", "travelmate")

o1 = s:option(Flag, "trm_enabled", translate("Enable travelmate"))
o1.default = o1.disabled
o1.rmempty = false

o2 = s:option(Flag, "trm_captive", translate("Captive Portal Detection"),
	translate("Check the internet availability, log captive portal redirections and keep the uplink connection 'alive'."))
o2.default = o2.enabled
o2.rmempty = false

o3 = s:option(ListValue, "trm_iface", translate("Uplink / Trigger interface"),
	translate("Name of the used uplink interface."))
if dump then
	local i, v
	for i, v in ipairs(dump.interface) do
		if v.interface ~= "loopback" and v.interface ~= "lan" then
			o3:value(v.interface)
		end
	end
end
o3.default = trmiface
o3.rmempty = false

if fs.access("/usr/bin/qrencode") then
	btn = s:option(Button, "btn", translate("View AP QR-Codes"),
		translate("Connect your Android or iOS devices to your router's WiFi using the shown QR code."))
	btn.inputtitle = translate("QR-Codes")
	btn.inputstyle = "apply"
	btn.disabled = false

	function btn.write()
		luci.http.redirect(luci.dispatcher.build_url("admin", "services", "travelmate", "apqr"))
	end
end

-- Runtime information

ds = m:section(NamedSection, "global", "travelmate", translate("Runtime Information"))

dv1 = ds:option(DummyValue, "status", translate("Travelmate Status (Quality)"))
dv1.template = "travelmate/runtime"
if parse ~= nil then
	dv1.value = parse.data.travelmate_status or translate("n/a")
else
	dv1.value = translate("n/a")
end

dv2 = ds:option(DummyValue, "travelmate_version", translate("Travelmate Version"))
dv2.template = "travelmate/runtime"
if parse ~= nil then
	dv2.value = parse.data.travelmate_version or translate("n/a")
else
	dv2.value = translate("n/a")
end

dv3 = ds:option(DummyValue, "station_id", translate("Station ID (SSID/BSSID)"))
dv3.template = "travelmate/runtime"
if parse ~= nil then
	dv3.value = parse.data.station_id or translate("n/a")
else
	dv3.value = translate("n/a")
end

dv4 = ds:option(DummyValue, "station_interface", translate("Station Interface"))
dv4.template = "travelmate/runtime"
if parse ~= nil then
	dv4.value = parse.data.station_interface or translate("n/a")
else
	dv4.value = translate("n/a")
end

dv5 = ds:option(DummyValue, "station_radio", translate("Station Radio"))
dv5.template = "travelmate/runtime"
if parse ~= nil then
	dv5.value = parse.data.station_radio or translate("n/a")
else
	dv5.value = translate("n/a")
end

dv6 = ds:option(DummyValue, "last_rundate", translate("Last rundate"))
dv6.template = "travelmate/runtime"
if parse ~= nil then
	dv6.value = parse.data.last_rundate or translate("n/a")
else
	dv6.value = translate("n/a")
end

-- Extra options

e = m:section(NamedSection, "global", "travelmate", translate("Extra options"),
translate("Options for further tweaking in case the defaults are not suitable for you."))

e1 = e:option(Flag, "trm_debug", translate("Enable verbose debug logging"))
e1.default = e1.disabled
e1.rmempty = false

e2 = e:option(Value, "trm_radio", translate("Radio selection"),
	translate("Restrict travelmate to a dedicated radio, e.g. 'radio0'."))
e2.datatype = "and(uciname,rangelength(6,6))"
e2.rmempty = true

e3 = e:option(Value, "trm_triggerdelay", translate("Trigger Delay"),
	translate("Additional trigger delay in seconds before travelmate processing begins."))
e3.datatype = "range(1,60)"
e3.default = 2
e3.rmempty = false

e4 = e:option(Value, "trm_maxretry", translate("Connection Limit"),
	translate("Retry limit to connect to an uplink."))
e4.default = 3
e4.datatype = "range(1,10)"
e4.rmempty = false

e5 = e:option(Value, "trm_minquality", translate("Signal Quality Threshold"),
	translate("Minimum signal quality threshold as percent for conditional uplink (dis-) connections."))
e5.default = 35
e5.datatype = "range(20,80)"
e5.rmempty = false

e6 = e:option(Value, "trm_maxwait", translate("Interface Timeout"),
	translate("How long should travelmate wait for a successful wlan uplink connection."))
e6.default = 30
e6.datatype = "range(20,40)"
e6.rmempty = false

e7 = e:option(Value, "trm_timeout", translate("Overall Timeout"),
	translate("Overall retry timeout in seconds."))
e7.default = 60
e7.datatype = "range(30,300)"
e7.rmempty = false

return m
