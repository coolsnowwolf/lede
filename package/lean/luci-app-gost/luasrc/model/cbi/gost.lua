-- Created By [CTCGFW]Project-OpenWrt
-- https://github.com/project-openwrt

mp = Map("gost", translate("Gost"))
mp.description = translate("A simple security tunnel written in Golang.")

mp:section(SimpleSection).template = "gost/gost_status"

s = mp:section(TypedSection, "gost")
s.anonymous=true
s.addremove=false

enable = s:option(Flag, "enable", translate("Enable"))
enable.default = 0
enable.rmempty = false

run_command = s:option(Value, "run_command", translate("Command"))
run_command.rmempty = false

return mp
