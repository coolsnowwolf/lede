#-- Copyright (C) 2018 dz <dingzhong110@gmail.com>

module("luci.controller.serverchan", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/serverchan") then
		return
	end

	local page

	page = entry({"admin", "services", "serverchan"}, cbi("serverchan"), _("serverchan"), 60)
	page.dependent = true
end
