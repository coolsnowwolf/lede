-- Copyright 2010 John Crispin <blogic@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

m = Map("telephony", translate("VoIP"))
m.on_after_commit = function() luci.sys.call("/etc/init.d/telephony reload") end

s = m:section(TypedSection, "contact", translate("Contact"), translate("Here You can specify the SIP contacts that you want to use."))
s.anonymous = true
s.addremove = true
s.template = "cbi/tsection"

s:option(Value, "desc", translate("Name"))
s:option(Value, "code", translate("Shortdial"))
s:option(Value, "dial", translate("Dial"))

t = s:option(ListValue, "type", translate("Type"))
t:value("sip", "SIP")
t:value("realm", "Landline")

return m
