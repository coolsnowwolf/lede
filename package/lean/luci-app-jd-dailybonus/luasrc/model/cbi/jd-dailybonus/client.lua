local jd = "jd-dailybonus"
local uci = luci.model.uci.cursor()
local sys = require "luci.sys"

m = Map(jd)
-- [[ 基本设置 ]]--

s = m:section(TypedSection, "global",
              translate("Base Config"))
s.anonymous = true

o = s:option(DummyValue, "", "")
o.rawhtml = true
o.template = "jd-dailybonus/cookie_tools"

o = s:option(Value, "cookie", translate("First Cookie"))
o.rmempty = false

o = s:option(Value, "cookie2", translate("Second Cookie"))
o.rmempty = true
o.description = translate('双账号用户抓取"账号1"Cookie后, 请勿点击退出账号(可能会导致Cookie失效), 需清除浏览器资料或更换浏览器登录"账号2"抓取.')

o = s:option(Flag, "auto_update", translate("Auto Update"))
o.rmempty = false
o.description = translate("Auto Update Script Service")

o =s:option(ListValue, "auto_update_time", translate("Update time (every day)"))
for t = 0, 23 do o:value(t, t .. ":01") end
o.default = 1
o.rmempty = true
o:depends("auto_update", "1")

o = s:option(Flag, "auto_run", translate("Auto Run"))
o.rmempty = false
o.description = translate("Auto Run Script Service")

o =s:option(ListValue, "auto_run_time", translate("Run time (every day)"))
for t = 0, 23 do o:value(t, t .. ":05") end
o.default = 1
o.rmempty = true
o:depends("auto_run", "1")

o = s:option(DummyValue, "", "")
o.rawhtml = true
o.version = sys.exec('uci get jd-dailybonus.@global[0].version')
o.template = "jd-dailybonus/update_service"
return m