-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.ntpc", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/ntpclient") then
		return
	end
	
	local page

	page = entry({"admin", "system", "ntpc"}, cbi("ntpc/ntpc"), _("Time Synchronisation"), 50)
	page.dependent = true

	page = entry({"mini", "system", "ntpc"}, cbi("ntpc/ntpcmini", {autoapply=true}), _("Time Synchronisation"), 50)
	page.dependent = true
end
