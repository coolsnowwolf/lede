--[[
# Copyright (c) 2014-2016, latyas <latyas@gmail.com>
# Edit by Twinzo <1282055288@qq.com>
]]--

module("luci.controller.idns", package.seeall)

function index()
	
	if not nixio.fs.access("/etc/config/idns") then
		return
	end

	local page
	
	page = entry({"admin", "system", "idns"}, cbi("idns"), _("IDNS"), 45)
	page.dependent = true
end