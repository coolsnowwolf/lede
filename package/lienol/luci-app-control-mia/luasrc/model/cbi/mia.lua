local o = require "luci.sys"
local a, t, e
a = Map("mia", translate("上网时间管理"),
        translate("上网时间段控制系统。"))
a.template = "mia/index"
t = a:section(TypedSection, "basic", translate("Running Status"))
t.anonymous = true
e = t:option(DummyValue, "mia_status", translate("当前状态"))
e.template = "mia/mia"
e.value = translate("Collecting data...")
t = a:section(TypedSection, "basic", translate("基本设置"))
t.anonymous = true
e = t:option(Flag, "enable", translate("开启"))
e.rmempty = false
t = a:section(TypedSection, "macbind", translate("客户端设置"))
t.template = "cbi/tblsection"
t.anonymous = true
t.addremove = true
e = t:option(Flag, "enable", translate("开启控制"))
e.rmempty = false
e = t:option(Value, "macaddr", translate("黑名单MAC"))
e.rmempty = true
o.net.mac_hints(function(t, a) e:value(t, "%s (%s)" % {t, a}) end)
e = t:option(Value, "timeon", translate("禁止上网开始时间"))
e.default = "00:00"
e.optional = false
e = t:option(Value, "timeoff", translate("取消禁止上网时间"))
e.default = "23:59"
e.optional = false
e = t:option(Flag, "z1", translate("周一"))
e.rmempty = true
e = t:option(Flag, "z2", translate("周二"))
e.rmempty = true
e = t:option(Flag, "z3", translate("周三"))
e.rmempty = true
e = t:option(Flag, "z4", translate("周四"))
e.rmempty = true
e = t:option(Flag, "z5", translate("周五"))
e.rmempty = true
e = t:option(Flag, "z6", translate("周六"))
e.rmempty = true
e = t:option(Flag, "z7", translate("周日"))
e.rmempty = true
return a
