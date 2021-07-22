-- Copyright 2014 Álvaro Fernández Rojas <noltari@gmail.com>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.shairport-sync", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/shairport-sync") then
		return
	end

	entry({"admin", "services", "shairport-sync"}, cbi("shairport-sync"), _("AirPlay 2 Receiver")).dependent = true
	entry({"admin", "services", "shairport-sync", "run"}, call("act_status")).leaf = true
end

function act_status()
	local e={}
	e.running=luci.sys.call("pgrep shairport-sync >/dev/null")==0
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end
