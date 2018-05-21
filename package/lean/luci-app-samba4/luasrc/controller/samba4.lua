-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.samba4", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/samba4") then
		return
	end

	local page

	page = entry({"admin", "nas", "samba4"}, cbi("samba4"), _("Network Shares"))
	page.dependent = true
end
