-- Copyright 2014 Álvaro Fernández Rojas <noltari@gmail.com>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.dump1090", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/dump1090") then
		return
	end

	local page = entry({"admin", "services", "dump1090"}, cbi("dump1090"), _("dump1090"))
	page.dependent = true

end
