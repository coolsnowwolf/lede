--[[
# Copyright (c) 2014-2016, latyas <latyas@gmail.com>
# Edit by Twinzo <1282055288@qq.com>
]]--

module("luci.controller.watchpig", package.seeall)

function index()
	
	if not nixio.fs.access("/etc/config/watchpig") then
		return
	end

	local page
	
	page = entry({"admin", "services", "watchpig"}, cbi("watchpig"), _("Watchpig"), 45)
	page.dependent = true
end
