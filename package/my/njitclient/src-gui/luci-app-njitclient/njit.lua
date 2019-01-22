--[[
LuCI - Lua Configuration Interface

Copyright 2010 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0
]]--

local sys = require "luci.sys"
local fs  = require "nixio.fs"

m = SimpleForm("njit", translate("njit-client"),
	translate("njit 802.11x client"))

m.submit = translate("Connect")
m.reset  = false

r = m:field(Value, "username", translate("Username"))
s = m:field(Value, "password", translate("Password"))
t = m:section(SimpleSection)
interface = t:option(ListValue, "interface", translate("Network interface"))
for _, e in ipairs(sys.net.devices()) do
		if e ~= "lo" then interface:value(e) end
	end


function interface.write(self, s, val)
	local cmd
	local user = luci.http.formvalue("cbid.njit.1.username")
	local pass = luci.http.formvalue("cbid.njit.1.password")
	local iface = luci.http.formvalue("cbid.njit.1.interface")
	cmd = "njit-client %s %s %s & (sleep 10 && killall njit-client)" %{ user, pass, iface }

	local msg = "<p><strong>%s</strong><br /><br /><code>%s<br /><br />" %{
		translate("Starting njit-client:"), cmd
		}

	local p = io.popen(cmd .. " 2>&1")
	if p then
		while true do
			local l = p:read("*l")
			if l then
				if #l > 100 then l = l:sub(1, 100) .. "..." end
				msg = msg .. l .. "<br />"
			else
				break
			end
		end
		p:close()
	end
	msg = msg .. "</code></p>"
	m.message = msg
end

return m


