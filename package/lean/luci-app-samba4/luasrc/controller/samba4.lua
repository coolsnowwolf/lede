-- Licensed to the public under the Apache License 2.0.

module("luci.controller.samba4", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/samba4") then
		return
	end

	entry({"admin", "services", "samba4"}, cbi("samba4"), _("Network Shares")).dependent = true
end
