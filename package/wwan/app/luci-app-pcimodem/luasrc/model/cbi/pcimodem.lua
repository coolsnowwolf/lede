-- Copyright 2016 David Thornley <david.thornley@touchstargroup.com>
-- Licensed to the public under the Apache License 2.0.

mp = Map("pcimodem")
mp.title = translate("PCI Modem Server")
mp.description = translate("Modem Server For OpenWrt")

s = mp:section(TypedSection, "service", "Base Setting")
s.anonymous = true

enabled = s:option(Flag, "enabled", translate("Enable"))
enabled.default = 0
enabled.rmempty = false

apn = s:option(Value, "apn", translate("APN"))
apn.rmempty = true

pincode = s:option(Value, "pincode", translate("PIN"))
pincode.rmempty = true

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

tool = s:option(Value, "tool", translate("Tools"))
tool:value("quectel-CM", "quectel-CM")
tool.rmempty = true

return mp
