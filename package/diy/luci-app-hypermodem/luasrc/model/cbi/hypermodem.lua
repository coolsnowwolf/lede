-- Copyright 2016 David Thornley <david.thornley@touchstargroup.com>
-- Licensed to the public under the Apache License 2.0.

mp = Map("hypermodem")
mp.title = translate("Hyper Modem Server")
mp.description = translate("Modem Server For OpenWrt")

s = mp:section(TypedSection, "service", "Base Setting")
s.anonymous = true

enabled = s:option(Flag, "enabled", translate("Enable"))
enabled.default = 0
enabled.rmempty = false

ipv6 = s:option(Flag, "ipv6", translate("Enable IPv6"))
ipv6.default = 1
ipv6.rmempty = false

device = s:option(Value, "device", translate("Modem device"))
device.rmempty = false

local device_suggestions = nixio.fs.glob("/dev/cdc-wdm*")

if device_suggestions then
	local node
	for node in device_suggestions do
		device:value(node)
	end
end

apn = s:option(Value, "apn", translate("APN"))
apn.rmempty = true

username = s:option(Value, "username", translate("PAP/CHAP username"))
username.rmempty = true

password = s:option(Value, "password", translate("PAP/CHAP password"))
password.rmempty = true

auth = s:option(Value, "auth", translate("Authentication Type"))
auth.rmempty = true
auth:value("", translate("-- Please choose --"))
auth:value("both", "PAP/CHAP (both)")
auth:value("pap", "PAP")
auth:value("chap", "CHAP")
auth:value("none", "NONE")

return mp
