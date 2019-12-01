-- Copyright 2018 lean <coolsnowwolf@gmail.com>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.webadmin", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/uhttpd") then
		return
	end

	local page

	page = entry({"admin", "system", "webadmin"}, cbi("webadmin"), _("Web Admin"), 1)
	page.leaf = true

end

