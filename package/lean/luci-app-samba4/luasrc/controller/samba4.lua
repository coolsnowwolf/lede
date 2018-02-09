--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id$
]]--

module("luci.controller.samba4", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/samba4") then
		return
	end

	local page

	entry({"admin", "nas"}, firstchild(), "NAS", 45).dependent = false
	page = entry({"admin", "nas", "samba4"}, cbi("samba4"), _("Network Shares"))
	page.dependent = true
end
