#-- Copyright (C) 2018 dz <dingzhong110@gmail.com>

module("luci.controller.oscam", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/oscam") then
		return
	end

	local page

	page = entry({"admin", "services", "oscam"}, cbi("oscam"), _("OSCAM"), 60)
	page.dependent = true
end
