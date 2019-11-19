-- Copyright 2017 Dirk Brenken (dev@brenken.org)
-- This is free software, licensed under the Apache License, Version 2.0

local uci = require("luci.model.uci").cursor()
local http = require("luci.http")
local cfg = http.formvalue("cfg")

if cfg ~= nil then
	uci:delete("wireless", cfg)
	uci:save("wireless")
	uci:commit("wireless")
end
http.redirect(luci.dispatcher.build_url("admin/services/travelmate/stations"))
