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

o = s:option(Flag, "auto_update", translate("Auto Update"))
o.rmempty = false
o.description = translate("Auto Update Sign Service")

o =s:option(ListValue, "auto_update_time", translate("Update time (every day)"))
for t = 0, 23 do o:value(t, t .. ":02") end
o.default = 2
o.rmempty = false

o = s:option(DummyValue, "", "")
o.rawhtml = true
o.template = "jd-dailybonus/update_service"
return m