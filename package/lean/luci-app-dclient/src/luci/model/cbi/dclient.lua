--[[
LuCI - Lua Configuration Interface

Copyright 2010 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0
]]--

require("luci.sys")

m = Map("dclient", translate("中兴802.1X认证"), translate("配置中兴802.1X和电信天翼认证.(关注微信公众号：DotTimes)"))

s = m:section(TypedSection, "login", "")
s.addremove = false
s.anonymous = true

zteenable = s:option(Flag, "enable", translate("Enable"))
zteuser = s:option(Value, "zteuser", translate("ZTE User"))
ztepass = s:option(Value, "ztepass", translate("ZTE Pass"))
ztepass.password = true

ifname = s:option(ListValue, "ifname", translate("Interface"), translate("Choice a Auth Interface"))
for k, v in ipairs(luci.sys.net.devices()) do
	if v ~= "lo" then
		ifname:value(v)
	end
end
timeout = s:option(ListValue, "timeout", translate("Timeout(s)"), translate("Timeout interval"))
timeout:value(0,translate("Disable"))
timeout:value(300)
timeout:value(420)
timeout:value(600)
timeout:value(900)
timeout:value(1800)
timeout:value(3600)

dhcpclient = s:option(ListValue, "dhcpclient", translate("DHCP Client"))
dhcpclient:value('auto',translate('auto'))
dhcpclient:value('udhcpc')

enet = s:option(Flag, "enet", translate("Enet Auth"))
enetuser = s:option(Value, "enetuser", translate("Enet User"))
enetpass = s:option(Value, "enetpass", translate("Enet Pass"))
enetpass.password = true

local apply = luci.http.formvalue("cbi.apply")
if apply then
	io.popen("/etc/init.d/dclient restart")
end

return m
