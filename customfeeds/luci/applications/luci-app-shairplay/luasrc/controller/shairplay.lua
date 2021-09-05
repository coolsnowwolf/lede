-- Copyright 2014 Álvaro Fernández Rojas <noltari@gmail.com>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.shairplay", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/shairplay") then
		return
	end

	local page = entry({"admin", "services", "shairplay"}, cbi("shairplay"), _("Shairplay"))
	page.dependent = true

end
