require("luci.util")

mp = Map("uugamebooster", translate("UU Game Booster"))
mp.description = translate("A Paid Game Acceleration service")
mp:section(SimpleSection).template  = "uugamebooster/uugamebooster_status"

s = mp:section(TypedSection, "uugamebooster")
s.anonymous=true
s.addremove=false

o = s:option(Flag, "enabled", translate("Enable"))
o.default = 0
o.optional = false

function o.write(self, section, value)
	if value == "1" then
		luci.sys.call("/etc/init.d/uugamebooster start >/dev/null && /etc/init.d/uugamebooster enable >/dev/null")
	else
		luci.sys.call("/etc/init.d/uugamebooster stop >/dev/null && /etc/init.d/uugamebooster disable >/dev/null")
	end
	return Flag.write(self, section, value)
end

return mp
