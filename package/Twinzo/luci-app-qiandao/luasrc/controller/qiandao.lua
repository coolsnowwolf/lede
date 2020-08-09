--[[
# Copyright (c) 2014-2016, latyas <latyas@gmail.com>
# Edit by Twinzo <1282055288@qq.com>
]]--

module("luci.controller.qiandao", package.seeall)

function index()
	
	if not nixio.fs.access("/etc/config/qiandao") then
		return
	end

	local page
	
	page = entry({"admin", "services", "qiandao"}, cbi("qiandao"), _("Sign-in Dog"), 45)
	page.dependent = true
end
