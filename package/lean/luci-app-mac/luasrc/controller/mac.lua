--[[
# Copyright (c) 2014-2016, latyas <latyas@gmail.com>
# Edit by Twinzo <1282055288@qq.com>
]]--

module("luci.controller.mac", package.seeall)

function index()
	
	if not nixio.fs.access("/etc/config/mac") then
		return
	end

	local page
	
	page = entry({"admin", "network", "mac"}, cbi("mac"), _("MAC Cloning"), 45)
	page.dependent = true
end
