-- Copyright 2008 Yanira <forum-2008@email.de>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.p910nd", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/p910nd") then
		return
	end

	local page

	page = entry({"admin", "services", "p910nd"}, cbi("p910nd"), _("p910nd - Printer server"), 60)
	page.dependent = true
end
