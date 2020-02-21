local i = require "luci.sys"
local t, e, o
t = Map("timewol", translate("定时网络唤醒"),
        translate("定时唤醒你的局域网设备"))
t.template = "timewol/index"
e = t:section(TypedSection, "basic", translate("Running Status"))
e.anonymous = true
o = e:option(DummyValue, "timewol_status", translate("当前状态"))
o.template = "timewol/timewol"
o.value = translate("Collecting data...")
e = t:section(TypedSection, "basic", translate("基本设置"))
e.anonymous = true
o = e:option(Flag, "enable", translate("开启"))
o.rmempty = false
e = t:section(TypedSection, "macclient", translate("客户端设置"))
e.template = "cbi/tblsection"
e.anonymous = true
e.addremove = true
nolimit_mac = e:option(Value, "macaddr", translate("客户端MAC"))
nolimit_mac.rmempty = false
i.net.mac_hints(function(e, t) nolimit_mac:value(e, "%s (%s)" % {e, t}) end)
nolimit_eth = e:option(Value, "maceth", translate("网络接口"))
nolimit_eth.rmempty = false
for t, e in ipairs(i.net.devices()) do if e ~= "lo" then nolimit_eth:value(e) end end
a = e:option(Value, "minute", translate("分钟"))
a.optional = false
a = e:option(Value, "hour", translate("小时"))
a.optional = false
a = e:option(Value, "day", translate("日"))
a.optional = false
a = e:option(Value, "month", translate("月"))
a.optional = false
a = e:option(Value, "weeks", translate("星期"))
a.optional = false
local e = luci.http.formvalue("cbi.apply")
if e then io.popen("/etc/init.d/timewol restart") end
return t
