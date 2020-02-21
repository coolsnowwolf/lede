local o = require "luci.sys"
local a, t, e
a = Map("weburl", translate("网址过滤"), translate(
            "在这里设置关键词过滤，可以是URL里任意字符，可以过滤如视频网站、QQ、迅雷、淘宝。。。"))
a.template = "weburl/index"
t = a:section(TypedSection, "basic", translate("Running Status"))
t.anonymous = true
e = t:option(DummyValue, "weburl_status", translate("当前状态"))
e.template = "weburl/weburl"
e.value = translate("Collecting data...")
t = a:section(TypedSection, "basic", translate("基本设置"), translate(
                  "一般来说普通过滤效果就很好了，强制过滤会使用更复杂的算法导致更高的CPU占用。"))
t.anonymous = true
e = t:option(Flag, "enable", translate("开启"))
e.rmempty = false
e = t:option(Flag, "algos", translate("强效过滤"))
e.rmempty = false
t = a:section(TypedSection, "macbind", translate("关键词设置"), translate(
                  "黑名单MAC不设置为全客户端过滤，如设置只过滤指定的客户端。过滤时间可不设置。"))
t.template = "cbi/tblsection"
t.anonymous = true
t.addremove = true
e = t:option(Flag, "enable", translate("开启控制"))
e.rmempty = false
e = t:option(Value, "macaddr", translate("黑名单MAC"))
e.rmempty = true
o.net.mac_hints(function(t, a) e:value(t, "%s (%s)" % {t, a}) end)
e = t:option(Value, "timeon", translate("开始过滤时间"))
e.placeholder = "00:00"
e.rmempty = true
e = t:option(Value, "timeoff", translate("取消过滤时间"))
e.placeholder = "23:59"
e.rmempty = true
e = t:option(Value, "keyword", translate("网址关键词"))
e.rmempty = false
return a
