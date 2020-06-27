#-- Copyright (C) 2018 dz <dingzhong110@gmail.com>

require("nixio.fs")
require("luci.http")

m = Map("serverchan", translate("serverchan"))

if luci.sys.call("pgrep -f serverchan >/dev/null") == 0 then
	m = Map("serverchan", translate("serverchan"), "%s - %s" %{translate("serverchan"), translate("<strong><font color=\"green\">Running</font></strong>")})
else
	m = Map("serverchan", translate("serverchan"), "%s - %s" %{translate("serverchan"), translate("<strong><font color=\"red\">Not Running</font></strong>")})
end

-- Basic
s = m:section(TypedSection, "serverchan", translate("Settings"), translate("General Settings"))
s.anonymous = true

---- Eanble
enable = s:option(Flag, "enabled", translate("Enable"), translate("Enable or disable serverchan"))
enable.default = 0
enable.rmempty = false

o = s:option(Value, "serverchan_sckey", translate("Serverchan Sckey"), translate("Serverchan Sckey").."<br>调用代码获取<a href='http://sc.ftqq.com'>点击这里</a><br><br>")
o.rmempty = false

new_ip = s:option(Flag, "new_ip", translate("IP Change of External Network"))
new_ip.default = 1
new_ip.rmempty = false

new_equipment = s:option(Flag, "new_equipment", translate("New equipment added"))
new_equipment.default = 1
new_equipment.rmempty = false

equipment = s:option(Flag, "equipment", translate("Equipment Up and Down"))
equipment.default = 1
equipment.rmempty = false

proxy = s:option(Flag, "proxy", translate("Proxy State"))
proxy.default = 1
proxy.rmempty = false

o = s:option(ListValue, "starttime", translate("starttime (every day)"))
for t = 0,23 do
o:value(t, t..":00")
end
o.default=0
o.rmempty = false

o = s:option(ListValue, "endtime", translate("endtime (every day)"))
for t = 0,23 do
o:value(t, t..":00")
end
o.default=23
o.rmempty = false

return m
