-- Copyright 2014 Álvaro Fernández Rojas <noltari@gmail.com>
-- Licensed to the public under the Apache License 2.0.

m = Map("shairplay", "Shairplay", translate("Shairplay is a simple AirPlay server implementation, here you can configure the settings."))

s = m:section(TypedSection, "shairplay", "")
s.addremove = true
s.anonymous = false

enable=s:option(Flag, "disabled", translate("Enabled"))
enable.enabled="0"
enable.disabled="1"
enable.default = "1"
enable.rmempty = false
respawn=s:option(Flag, "respawn", translate("Respawn"))
respawn.default = false

apname = s:option(Value, "apname", translate("Airport Name"))
apname.rmempty = true

port=s:option(Value, "port", translate("Port"))
port.rmempty = true
port.datatype = "port"

pw = s:option(Value, "password", translate("Password"))
pw.rmempty = true
pw.password = true

hwaddr=s:option(Value, "hwaddr", translate("HW Address"))
hwaddr.rmempty = true
hwaddr.datatype = "macaddr"

ao_driver=s:option(ListValue, "ao_driver", translate("AO Driver"))
ao_driver:value("", translate("Default"))
ao_driver:value("alsa")
--ao_driver:value("alsa05")
--ao_driver:value("arts")
--ao_driver:value("esd")
--ao_driver:value("irix")
--ao_driver:value("nas")
ao_driver:value("oss")
--ao_driver:value("sun")

ao_devicename=s:option(Value, "ao_devicename", translate("AO Device Name"))
ao_devicename.rmempty = true

ao_deviceid = s:option(ListValue, "ao_deviceid", translate("AO Device ID"))
ao_deviceid.rmempty = true
ao_deviceid:value("", translate("Default"))
local pats = io.popen("find /proc/asound/ -type d -name 'card*' | sort")
if pats then
	local l
	while true do
		l = pats:read("*l")
		if not l then break end

		l = string.gsub(l, "/proc/asound/card", "")
		if l then
			ao_deviceid:value(l)
		end
	end
	pats:close()
end

return m
