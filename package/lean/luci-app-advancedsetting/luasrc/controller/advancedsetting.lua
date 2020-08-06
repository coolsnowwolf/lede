#-- Copyright (C) 2018 dz <dingzhong110@gmail.com>

module("luci.controller.advancedsetting", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/advancedsetting") then
		return
	end
	
	local page
	page = entry({"admin", "system", "advancedsetting"}, cbi("advancedsetting"), _("Advanced Setting"), 60)
	page.dependent = true
end