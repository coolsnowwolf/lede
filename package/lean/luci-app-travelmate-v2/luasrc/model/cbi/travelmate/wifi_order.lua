-- Copyright 2017 Dirk Brenken (dev@brenken.org)
-- This is free software, licensed under the Apache License, Version 2.0

local http = require("luci.http")
local cfg = http.formvalue("cfg")
local dir = http.formvalue("dir")
local uci = require("luci.model.uci").cursor()
local trmiface = uci:get("travelmate", "global", "trm_iface") or "trm_wwan"

if cfg ~= nil then
	local section = ""
	local idx = ""
	local idx_change = ""
	local changed = ""
	uci:foreach("wireless", "wifi-iface", function(s)
		local iface = s.network or ""
		if iface == trmiface then
			section = s['.name']
			if cfg == section then
				idx = s['.index']
			else
				idx_change = s['.index']
			end
			if (dir == "up" and idx ~= "" and idx_change ~= "" and idx_change < idx) or
				(dir == "down" and idx ~= "" and idx_change ~= "" and idx_change > idx) then
				changed = uci:reorder("wireless", cfg, idx_change)
				idx = ""
			end
		end
	end)
	if changed ~= "" then
		uci:save("wireless")
		uci:commit("wireless")
	end
end
http.redirect(luci.dispatcher.build_url("admin/services/travelmate/stations"))
