#-- Copyright (C) 2018 dz <dingzhong110@gmail.com>

module("luci.controller.cupsd", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/cupsd") then
		return
	end

	local page


	entry({"admin", "services", "cupsd"},alias("admin", "services", "cupsd","page1"),_("CUPS 打印服务器"),60).dependent = true
	entry({"admin", "services", "cupsd","page1"}, cbi("cupsd/page1"),_("设置"),10).leaf = true
	entry({"admin", "services", "cupsd","page2"}, cbi("cupsd/page2"),_("高级"),20).leaf = true
end

