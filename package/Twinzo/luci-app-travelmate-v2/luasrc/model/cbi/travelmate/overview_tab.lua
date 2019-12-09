-- Copyright 2017-2018 Dirk Brenken (dev@brenken.org)
-- This is free software, licensed under the Apache License, Version 2.0

local fs       = require("nixio.fs")
local uci      = require("luci.model.uci").cursor()
local json     = require("luci.jsonc")
local util     = require("luci.util")
local nw       = require("luci.model.network").init()
local fw       = require("luci.model.firewall").init()
local dump     = util.ubus("network.interface", "dump", {})
local trmiface = uci.get("travelmate", "global", "trm_iface") or "trm_wwan"
local trminput = uci.get("travelmate", "global", "trm_rtfile") or "/tmp/trm_runtime.json"
local uplink   = uci.get("network", trmiface) or ""
local parse    = json.parse(fs.readfile(trminput) or "")

m = Map("travelmate", translate("Travelmate"),
	translate("Configuration of the travelmate package to to enable travel router functionality. ")
	.. translatef("For further information "
	.. "<a href=\"%s\" target=\"_blank\">"
	.. "see online documentation</a>", "https://github.com/openwrt/packages/blob/master/net/travelmate/files/README.md"))

function m.on_after_commit(self)
	luci.sys.call("env -i /etc/init.d/travelmate restart >/dev/null 2>&1")
	luci.http.redirect(luci.dispatcher.build_url("admin", "services", "travelmate"))
end

-- Interface Wizard

if uplink == "" then
	ds = m:section(NamedSection, "global", "travelmate", translate("Interface Wizard"))

	o = ds:option(Value, "", translate("Uplink interface"))
	o.datatype = "and(uciname,rangelength(3,15))"
	o.default = trmiface
	o.rmempty = false

	btn = ds:option(Button, "trm_iface", translate("Create Uplink Interface"),
		translate("Create a new wireless wan uplink interface, configure it to use dhcp and add it to the wan zone of the firewall. This step has only to be done once."))
	btn.inputtitle = translate("Add Interface")
	btn.inputstyle = "apply"
	btn.disabled = false

	function btn.write(self, section)
		local iface = o:formvalue(section)
		if iface then
			uci:set("travelmate", section, "trm_iface", iface)
			uci:save("travelmate")
			uci:commit("travelmate")
			local net = nw:add_network(iface, { proto = "dhcp" })
			if net then
				nw:save("network")
				nw:commit("network")
				local zone = fw:get_zone_by_network("wan")
				if zone then
					zone:add_network(iface)
					fw:save("firewall")
					fw:commit("firewall")
				end
			end
			luci.sys.call("env -i /bin/ubus call network reload >/dev/null 2>&1")
		end
		luci.http.redirect(luci.dispatcher.build_url("admin", "services", "travelmate"))
	end
	return m
end

-- Main travelmate options

s = m:section(NamedSection, "global", "travelmate")

o1 = s:option(Flag, "trm_enabled", translate("Enable travelmate"))
o1.default = o1.disabled
o1.rmempty = false

o2 = s:option(Flag, "trm_automatic", translate("Enable 'automatic' mode"),
	translate("Keep travelmate in an active state. Check every n seconds the connection status, i.e. the uplink availability."))
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

o4 = s:option(Value, "trm_triggerdelay", translate("Trigger delay"),
	translate("Additional trigger delay in seconds before travelmate processing begins."))
o4.default = 2
o4.datatype = "range(1,90)"
o4.rmempty = false

btn = s:option(Button, "", translate("Manual Rescan"),
	translate("Force a manual uplink rescan / reconnect in 'trigger' mode."))
btn:depends("trm_automatic", "")
btn.inputtitle = translate("Rescan")
btn.inputstyle = "find"
btn.disabled = false

function btn.write()
	luci.sys.call("env -i /etc/init.d/travelmate start >/dev/null 2>&1")
	luci.http.redirect(luci.dispatcher.build_url("admin", "services", "travelmate"))
end

-- Runtime information

ds = m:section(NamedSection, "global", "travelmate", translate("Runtime Information"))

dv1 = ds:option(DummyValue, "status", translate("Travelmate Status"))
dv1.template = "travelmate/runtime"
if parse == nil then
	dv1.value = translate("n/a")
elseif parse.data.travelmate_status == "connected" then
	dv1.value = translate("connected")
elseif parse.data.travelmate_status == "not connected" then
	dv1.value = translate("not connected")
elseif parse.data.travelmate_status == "running" then
	dv1.value = translate("running")
elseif parse.data.travelmate_status == "error" then
	dv1.value = translate("error")
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

e3 = e:option(Value, "trm_maxretry", translate("Connection Limit"),
	translate("How many times should travelmate try to connect to an Uplink. To disable this feature set it to '0' which means unlimited retries."))
e3.default = 3
e3.datatype = "range(0,30)"
e3.rmempty = false

e4 = e:option(Value, "trm_maxwait", translate("Interface Timeout"),
	translate("How long should travelmate wait for a successful wlan interface reload."))
e4.default = 30
e4.datatype = "range(5,60)"
e4.rmempty = false

e5 = e:option(Value, "trm_timeout", translate("Overall Timeout"),
	translate("Timeout in seconds between retries in 'automatic' mode."))
e5.default = 60
e5.datatype = "range(60,300)"
e5.rmempty = false

return m
