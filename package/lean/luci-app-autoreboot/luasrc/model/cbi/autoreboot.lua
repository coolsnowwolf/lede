require("luci.sys")

m=Map("autoreboot",translate("Scheduled Reboot"),translate("Scheduled reboot Setting"))

s=m:section(TypedSection,"login","")
s.addremove=false
s.anonymous=true

enable=s:option(Flag,"enable",translate("Enable"))
enable.rmempty = false
enable.default=0

week=s:option(ListValue,"week",translate("Week Day"))
week:value(*,translate("Everyday"))
week:value(1,translate("Monday"))
week:value(2,translate("Tuesday"))
week:value(3,translate("Wednesday"))
week:value(4,translate("Thursday"))
week:value(5,translate("Friday"))
week:value(6,translate("Saturday"))
week:value(0,translate("Sunday"))
week.default=3

hour=s:option(Value,"hour",translate("Hour"))
hour.datatype = "range(0,23)"
hour.rmempty = false

pass=s:option(Value,"minute",translate("Minute"))
pass.datatype = "range(0,59)"
pass.rmempty = false


local e=luci.http.formvalue("cbi.apply")
if e then
  io.popen("/etc/init.d/autoreboot restart")
end

return m
