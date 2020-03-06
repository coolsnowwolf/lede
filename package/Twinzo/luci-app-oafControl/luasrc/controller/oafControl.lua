--[[
# Copyright (c) 2014-2016, latyas <latyas@gmail.com>
# Edit by Twinzo <1282055288@qq.com>
]]--

module("luci.controller.oafControl", package.seeall)

function index()
	
	if not nixio.fs.access("/etc/config/oafControl") then
		return
	end

	local page
	
	page = entry({"admin", "appControl", "oafControl"}, cbi("oafControl"), _("应用过滤控制器"), 45)
	page.dependent = true
end