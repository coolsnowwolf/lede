local o = require "luci.sys"
local a, e, t
a = Map("webrestriction", translate("访问限制"), translate(
            "使用黑名单或者白名单模式控制列表中的客户端是否能够连接到互联网。"))
a.template = "webrestriction/index"
e = a:section(TypedSection, "basic", translate("Running Status"))
e.anonymous = true
t = e:option(DummyValue, "webrestriction_status", translate("当前状态"))
t.template = "webrestriction/webrestriction"
t.value = translate("Collecting data...")
e = a:section(TypedSection, "basic", translate("全局设置"))
e.anonymous = true
t = e:option(Flag, "enable", translate("开启"))
t.rmempty = false
t = e:option(ListValue, "limit_type", translate("限制模式"))
t.default = "blacklist"
t:value("whitelist", translate("白名单"))
t:value("blacklist", translate("Blacklist"))
t.rmempty = false
e = a:section(TypedSection, "macbind", translate("名单设置"), translate(
                  "如果是黑名单模式，列表中的客户端将被禁止连接到互联网；白名单模式表示仅有列表中的客户端可以连接到互联网。"))
e.template = "cbi/tblsection"
e.anonymous = true
e.addremove = true
t = e:option(Flag, "enable", translate("开启控制"))
t.rmempty = false
t = e:option(Value, "macaddr", translate("MAC地址"))
t.rmempty = true
o.net.mac_hints(function(e, a) t:value(e, "%s (%s)" % {e, a}) end)
return a
