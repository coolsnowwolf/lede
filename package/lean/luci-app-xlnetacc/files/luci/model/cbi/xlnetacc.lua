local m, s, o
local uci = luci.model.uci.cursor()

m = Map("xlnetacc", "%s - %s" %{translate("XLNetAcc"), translate("Settings")}, translate("XLNetAcc is a Thunder joint broadband operators launched a commitment to help users solve the low broadband, slow Internet access, poor Internet experience of professional-grade broadband upgrade software."))
m:append(Template("xlnetacc/status"))

s = m:section(NamedSection, "general", "general", translate("General Settings"))
s.anonymous = true
s.addremove = false

o = s:option(Flag, "enabled", translate("Enabled"))
o.rmempty = false

o = s:option(Flag, "down_acc", translate("Enable DownLink Upgrade"))

o = s:option(Flag, "up_acc", translate("Enable UpLink Upgrade"))

o = s:option(Flag, "logging", translate("Enable Logging"))
o.default = "1"

o = s:option(Flag, "verbose", translate("Enable verbose logging"))
o:depends("logging", "1")

o = s:option(ListValue, "network", translate("Upgrade interface"))
uci:foreach("network", "interface", function(section)
	if section[".name"] ~= "loopback" and section[".name"] ~= "lan" then
		o:value(section[".name"])
	end
end)

o = s:option(Value, "account", translate("XLNetAcc account"))

o = s:option(Value, "password", translate("XLNetAcc password"), translate("Does not store the plaintext password, automatically emptied after start."))
o.password = true
o.template = "xlnetacc/rsa"

o = s:option(Value, "encrypt", translate("Encrypted password"), translate("Auto-generate in accordance with the plaintext password, do not modify it!"))
--o.readonly = true

return m
