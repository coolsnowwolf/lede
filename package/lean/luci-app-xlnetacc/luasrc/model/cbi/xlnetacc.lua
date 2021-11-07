local m,s,o
local uci = luci.model.uci.cursor()

m = Map("xlnetacc")
m.title = translate("XLNetAcc - Settings")
m.description = translate("XLNetAcc is a Thunder joint broadband operators launched a commitment to help users solve the low broadband, slow Internet access, poor Internet experience of professional-grade broadband upgrade software.")

m:section(SimpleSection).template = "xlnetacc/xlnetacc_status"

s = m:section(NamedSection, "general", "general")
s.title = translate("General Settings")
s.anonymous = true
s.addremove = false

o = s:option(Flag, "enabled", translate("Enabled"))
o.rmempty = false

o = s:option(Flag, "down_acc", translate("Enable DownLink Upgrade"))

o = s:option(Flag, "up_acc", translate("Enable UpLink Upgrade"))

o = s:option(Flag, "logging", translate("Enable Logging"))
o.default = 1

o = s:option(Flag, "verbose", translate("Enable verbose logging"))
o:depends("logging",1)

o = s:option(ListValue, "network", translate("Upgrade interface"))
uci:foreach("network","interface",function(section)
	if section[".name"]~="loopback" then
		o:value(section[".name"])
	end
end)

o = s:option(Value, "keepalive", translate("Keepalive interval"))
o.description = translate("5 ~ 60 minutes")
for _,v in ipairs({5,10,20,30,60}) do
	o:value(v,v.." "..translate("minutes"))
end
o.datatype = "range(5,60)"
o.default = 10

o = s:option(Value, "relogin", translate("Account relogin"))
o.description = translate("1 ~ 48 hours")
o:value(0, translate("Not enabled"))
for _,v in ipairs({3,12,18,24,30}) do
	o:value(v,v.." "..translate("hours"))
end
o.datatype = "max(48)"
o.default = 0

o = s:option(Value, "account", translate("XLNetAcc account"))

o = s:option(Value, "password", translate("XLNetAcc password"))
o.password = true

return m
