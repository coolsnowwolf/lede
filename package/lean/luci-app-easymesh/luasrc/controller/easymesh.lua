#-- Copyright (C) 2021 dz <dingzhong110@gmail.com>

module("luci.controller.easymesh", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/easymesh") then
		return
	end

	local page

	page = entry({"admin", "network", "easymesh"}, cbi("easymesh"), _("EASY MESH"), 60)
	page.dependent = true
	page.acl_depends = { "luci-app-easymesh" }
end
