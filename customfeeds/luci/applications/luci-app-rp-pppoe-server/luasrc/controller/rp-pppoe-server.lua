-- Copyright 2015 Daniel Dickinson <openwrt@daniel.thecshore.com>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.rp-pppoe-server", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/pppoe") then
		return
	end

	entry({"admin", "services", "rp-pppoe-server"}, cbi("rp-pppoe-server"), _("RP PPPoE Server"))
end

