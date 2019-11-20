--[[
# Copyright (c) 2014-2016, latyas <latyas@gmail.com>
# Edit by Twinzo <1282055288@qq.com>
]]--

module("luci.controller.dogcom", package.seeall)

function index()
	
	if not nixio.fs.access("/etc/config/dogcom") then
		return
	end

	entry({"admin", "services", "dogcom"},alias("admin", "services", "dogcom", "dogcom"),_("DROCM客户端")).dependent = true
	entry({"admin", "services", "dogcom", "dogcom"}, cbi("dogcom/dogcom"),_("通用设置"),10).leaf = true
	entry({"admin", "services", "dogcom", "dogcom_proxy"}, cbi("dogcom/dogcom_proxy"),_("防代理设置"),30).leaf = true

end
