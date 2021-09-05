-- Copyright 2015 Daniel Dickinson <openwrt@daniel.thecshore.com>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.uhttpd.uhttpd", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/uhttpd") then
		return
	end

	local page

	page = entry({"admin", "services", "uhttpd"}, cbi("uhttpd/uhttpd"), _("uHTTPd"))
	page.leaf = true

end

