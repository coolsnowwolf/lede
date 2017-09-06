--[[
LuCI - Lua Configuration Interface - aria2 support

Copyright 2014 nanpuyue <nanpuyue@gmail.com>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0
]]--

module("luci.controller.ngrokc", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/ngrokc") then
		return
	end

	entry({"admin", "services", "ngrokc"}, cbi("ngrokc/overview"), _("Ngrok Settings")).dependent = true
	entry({"admin", "services", "ngrokc", "detail"}, cbi("ngrokc/detail"), nil ).leaf = true
end
