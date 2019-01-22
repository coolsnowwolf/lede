--[[
LuCI - Lua Configuration Interface

Copyright 2010 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0
]]--

local sys = require "luci.sys"
local nixio = require "nixio"

m = Map("sysuh3c", translate("SYSU H3C Client"), translate("Configure H3C 802.1x client."))

s = m:section(TypedSection, "login", "")
s.addremove = true
s.anonymous = true

enable = s:option(Flag, "enable", translate("Enable"))
blockstartup = s:option(Flag, "blockstartup", translate("Block Startup Sequence"),
	"If enabled, the client will block startup sequence until authenticated")
name = s:option(Value, "username", translate("Username"))
pass = s:option(Value, "password", translate("Password"))
pass.password = true

method = s:option(ListValue, "method", translate("EAP Method"))
method:value("xor")  
method:value("md5")  

ifname = s:option(ListValue, "ifname", translate("Interface"))
for k, v in ipairs(nixio.getifaddrs()) do
	if v.flags["up"] and not v.flags["noarp"] and not v.flags["loopback"] then
		ifname:value(v.name)
	end
end

--[[
getwanif = s:option(Button, "_getwanif", translate("Get WAN interface"))
getwanif.inputstyle = "apply"
getwanif.write = function(self, section)
	local ifname = sys.exec("uci get network.wan.ifname")
	if ifname ~= nil then
		self.map:set(section, "ifname", ifname)
	end
end
]]--
--[[
local apply = luci.http.formvalue("cbi.apply")
if apply then
	io.popen("/etc/init.d/sysuh3c restart")
end
]]--
return m
