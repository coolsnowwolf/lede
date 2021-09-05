--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id$
]]--

module("luci.controller.sqm", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/sqm") then
		return
	end

	local page

	page = entry({"admin", "network", "sqm"}, cbi("sqm"), _("SQM QoS"))
	page.dependent = true
	page.acl_depends = { "luci-app-sqm" }
end
