--[[
 Customize /etc/gfwlist.list content
 Copyright (c) 2015 Justin Liu
 Author: Justin Liu <rssnsj@gmail.com>
 https://github.com/rssnsj/network-feeds
]]--

module("luci.controller.gfwlist", package.seeall)

function index()
	local page
	page = entry({"admin", "services", "gfwlist"}, cbi("gfwlist"), _("Domain Lists"))
	page.dependent = true
end
