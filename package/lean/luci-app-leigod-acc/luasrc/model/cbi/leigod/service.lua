local uci = require "luci.model.uci".cursor()

-- config
m = Map("accelerator")
m.title = translate("Leigod Accelerator Config")
m.description = translate("Control Accelerator Config")

s = m:section(TypedSection, "system")
s.addremove = false
s.anonymous = true

enable = s:option(Flag,"enabled" ,translate("Enable"))
enable.rmempty = false
enable.default = 0

tun = s:option(Flag,"tun" ,translate("Tunnel Mode"))
tun.rmempty = false
tun.default = 0
tun.description = translate("Turning Leigod into Tunnel mode can make some proxy plugins available")

schedule_enabled = s:option(Flag, "schedule_enabled", translate("Scheduled Pause"))
schedule_enabled.rmempty = false
schedule_enabled.default = 0
schedule_enabled.description = translate("夜猫子选项")

pause_time = s:option(ListValue, "pause_time", translate("Pause Time"))
pause_time:depends("schedule_enabled", 1)
for i = 0, 23 do
    pause_time:value(string.format("%02d:00", i), string.format("%02d:00", i))
end
pause_time.rmempty = false
pause_time.description = translate("选择好时间雷神加速器会定时暂停，请避开你的游戏时间以免影响游戏体验")


username = s:option(Value, "username", translate("Phone Number"))
username:depends("schedule_enabled", 1)
username.rmempty = false

password = s:option(Value, "password", translate("Leigod Password"))
password:depends("schedule_enabled", 1)
password.password = true
password.rmempty = false

m:section(SimpleSection).template = "leigod/service"

return m