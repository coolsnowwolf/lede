-- Copyright 2016 David Thornley <david.thornley@touchstargroup.com>
-- Licensed to the public under the Apache License 2.0.

local map, section, net = ...

local device, apn, pincode, username, password
local auth, ipv6


device = section:taboption("general", Value, "device", translate("Modem device"))
device.rmempty = false

local device_suggestions = nixio.fs.glob("/dev/cdc-wdm*")

if device_suggestions then
	local node
	for node in device_suggestions do
		device:value(node)
	end
end


apn = section:taboption("general", Value, "apn", translate("APN"))


pincode = section:taboption("general", Value, "pincode", translate("PIN"))


username = section:taboption("general", Value, "username", translate("PAP/CHAP username"))


password = section:taboption("general", Value, "password", translate("PAP/CHAP password"))
password.password = true

auth = section:taboption("general", Value, "auth", translate("Authentication Type"))
auth:value("", translate("-- Please choose --"))
auth:value("both", "PAP/CHAP (both)")
auth:value("pap", "PAP")
auth:value("chap", "CHAP")
auth:value("none", "NONE")

if luci.model.network:has_ipv6() then
    ipv6 = section:taboption("advanced", Flag, "ipv6", translate("Enable IPv6 negotiation"))
    ipv6.default = ipv6.disabled
end
