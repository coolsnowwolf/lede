--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id$
]]--

require("luci.sys")

module("luci.controller.usb_printer", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/usb_printer") then
		return
	end
	
  entry({"admin", "nas"}, firstchild(), "NAS", 44).dependent = false
	
	local page

	page = entry({"admin", "nas", "usb_printer"}, cbi("usb_printer"), _("USB Printer Server"), 50)
end
