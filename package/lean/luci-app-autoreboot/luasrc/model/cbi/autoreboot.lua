require("luci.sys")

m=Map("autoreboot",translate("Scheduled Reboot"),translate("Scheduled reboot Setting"))

s=m:section(TypedSection,"login","")
s.addremove=false
s.anonymous=true

enable=s:option(Flag,"enable",translate("Enable"))

week=s:option(ListValue,"week",translate("Week Day"))
week:value(0,translate("Everyday"))
for e=1,7 do
week:value(e,translate("Day"..e))
end
week.default=0

pass=s:option(Value,"minute",translate("Minute"))
pass.datatype = "range(0,59)"
pass.rmempty = false

hour=s:option(Value,"hour",translate("Hour"))
hour.datatype = "range(0,23)"
hour.rmempty = false
local e=luci.http.formvalue("cbi.apply")
if e then
  io.popen("/etc/init.d/autoreboot restart")
end

return m
