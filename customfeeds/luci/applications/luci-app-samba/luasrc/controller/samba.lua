-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.samba", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/samba") then
		return
	end

	local page

	page = entry({"admin", "services", "samba"}, cbi("samba"), _("Network Shares"))
	page.dependent = true
end
