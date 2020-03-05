

m = Map("vlmcsd")
m.title	= translate("vlmcsd config")
m.description = translate("A KMS Server Emulator to active your Windows or Office")

m:section(SimpleSection).template  = "vlmcsd/vlmcsd_status"

s = m:section(TypedSection, "vlmcsd")
s.addremove = false
s.anonymous = true

s:tab("basic", translate("Basic Setting"))
enable = s:taboption("basic",Flag, "enabled", translate("Enable"))
enable.rmempty = false

autoactivate = s:taboption("basic", Flag, "autoactivate", translate("Auto activate"))
autoactivate.rmempty = false

s:tab("config", translate("Config File"))
config = s:taboption("config", Value, "config", translate("configfile"), translate("This file is /etc/vlmcsd.ini."), "")
config.template = "cbi/tvalue"
config.rows = 13
config.wrap = "off"

function config.cfgvalue(self, section)
	return nixio.fs.readfile("/etc/vlmcsd.ini")
end

function config.write(self, section, value)
	value = value:gsub("\r\n?", "\n")
	nixio.fs.writefile("/etc/vlmcsd.ini", value)
end

return m
