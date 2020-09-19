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

o = s:option(ListValue, "remote_url", translate("Source Update Url"))
o:value("https://cdn.jsdelivr.net/gh/NobyDa/Script/JD-DailyBonus/JD_DailyBonus.js", translate("GitHub"))
o:value("https://gitee.com/jerrykuku/staff/raw/master/JD_DailyBonus.js", translate("Gitee"))
o.default = "nil"
o.rmempty = false
o.description = translate('当GitHub源无法更新时,可以选择使用国内Gitee源')

o = s:option(Value, "cookie", translate("First Cookie"))
o.rmempty = false

o = s:option(Value, "cookie2", translate("Second Cookie"))
o.rmempty = true
o.description = translate('双账号用户抓取"账号1"Cookie后, 请勿点击退出账号(可能会导致Cookie失效), 需清除浏览器资料或更换浏览器登录"账号2"抓取.')

o = s:option(Value, "stop", translate("Execute Delay"))
o.rmempty = false
o.default = 0
o.datatype = integer
o.description = translate("自定义延迟签到,单位毫秒. 默认分批并发无延迟. (延迟作用于每个签到接口, 如填入延迟则切换顺序签到. ) ")

o = s:option(ListValue, "serverurl", translate("ServerURL"))
o:value("scu", translate("SCU"))
o:value("sct", translate("SCT"))
o.default = "scu"
o.rmempty = false
o.description = translate('选择Server酱的推送接口')

o = s:option(Value, "serverchan", translate("ServerChan SCKEY"))
o.rmempty = true
o.description = translate("微信推送，基于Server酱服务，请自行登录 http://sc.ftqq.com/ 绑定并获取 SCKEY (仅在自动签到时推送)")

o = s:option(Flag, "failed", translate("Failed Push"))
o.rmempty = false
o.description = translate("仅当cookie失效时推送")

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
