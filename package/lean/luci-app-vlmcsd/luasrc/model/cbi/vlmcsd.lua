

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
s.anonymous=true
local a="/etc/vlmcsd.ini"
config=s:taboption("config",TextValue,"Config_File")
config.description=translate("This file is /etc/vlmcsd.ini.")
config.rows=18
config.wrap="off"
function config.cfgvalue(s,s)
sylogtext=""
if a and nixio.fs.access(a)then
Config_File=luci.sys.exec("tail -n 100 %s"%a)
end
return Config_File
end
function config.write(t,t,e)
e=e:gsub("\r\n?","\n")
nixio.fs.writefile("/etc/vlmcsd.ini",e)
end

return m
