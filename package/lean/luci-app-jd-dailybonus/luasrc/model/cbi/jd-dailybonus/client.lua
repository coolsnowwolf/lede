local jd = 'jd-dailybonus'
local uci = luci.model.uci.cursor()
local sys = require 'luci.sys'

m = Map(jd)
-- [[ 基本设置 ]]--

s = m:section(TypedSection, 'global', translate('基本设置'))
s.anonymous = true

o = s:option(DummyValue, '', '')
o.rawhtml = true
o.template = 'jd-dailybonus/cookie_tools'

o = s:option(Value, 'cookie', translate('第一账号Cookie'))
o.rmempty = false

o = s:option(Value, 'cookie2', translate('第二账号Cookie'))
o.rmempty = true

o = s:option(Value, 'stop', translate('延迟签到'))
o.rmempty = false
o.default = 0
o.datatype = integer
o.description = translate('自定义延迟签到,单位毫秒. 默认分批并发无延迟. (延迟作用于每个签到接口, 如填入延迟则切换顺序签到. ) ')

o = s:option(Value, 'out', translate('接口超时'))
o.rmempty = false
o.default = 0
o.datatype = integer
o.description = translate('接口超时退出,单位毫秒 用于可能发生的网络不稳定, 0则关闭.')

-- server chan
o = s:option(ListValue, 'serverurl', translate('Server酱的推送接口地址'))
o:value('scu', translate('SCU'))
o:value('sct', translate('SCT'))
o.default = 'scu'
o.rmempty = false
o.description = translate('选择Server酱的推送接口')

o = s:option(Value, 'serverchan', translate('Server酱 SCKEY'))
o.rmempty = true
o.description = translate('微信推送，基于Server酱服务，请自行登录 http://sc.ftqq.com/ 绑定并获取 SCKEY。')

-- telegram

o = s:option(Value, 'tg_token', translate('Telegram Bot Token'))
o.rmempty = true
o.description = translate('首先在Telegram上搜索BotFather机器人，创建一个属于自己的通知机器人，并获取Token。')

o = s:option(Value, 'tg_userid', translate('Telegram UserID'))
o.rmempty = true
o.description = translate('在Telegram上搜索getuserIDbot机器人，获取UserID。')

--Auto Run Script Service

o = s:option(Flag, 'auto_run', translate('自动签到'))
o.rmempty = false

o = s:option(ListValue, 'auto_run_time_h', translate('每天签到时间(小时)'))
for t = 0, 23 do
    o:value(t, t)
end
o.default = 1
o.rmempty = true
o:depends('auto_run', '1')
o = s:option(ListValue, 'auto_run_time_m', translate('每天签到时间(分钟)'))
for t = 0, 59 do
    o:value(t, t)
end
o.default = 1
o.rmempty = true
o:depends('auto_run', '1')

-- Auto Update Script Service

o = s:option(Flag, 'auto_update', translate('自动更新'))
o.rmempty = false

o = s:option(ListValue, 'auto_update_time', translate('每天更新时间'))
for t = 0, 23 do
    o:value(t, t .. ':01')
end
o.default = 1
o.rmempty = true
o:depends('auto_update', '1')

o = s:option(Value, 'remote_url', translate('更新源地址'))
o:value('https://raw.githubusercontent.com/NobyDa/Script/master/JD-DailyBonus/JD_DailyBonus.js', translate('GitHub'))
o:value('https://raw.sevencdn.com/NobyDa/Script/master/JD-DailyBonus/JD_DailyBonus.js', translate('GitHub CDN 01'))
o:value('https://cdn.jsdelivr.net/gh/NobyDa/Script/JD-DailyBonus/JD_DailyBonus.js', translate('GitHub CDN 02'))
o:value('https://ghproxy.com/https://raw.githubusercontent.com/NobyDa/Script/master/JD-DailyBonus/JD_DailyBonus.js', translate('韩国首尔'))
o:value('https://gitee.com/jerrykuku/staff/raw/master/JD_DailyBonus.js', translate('Gitee'))
o.default = 'nil'
o.rmempty = false
o.description = translate('当GitHub源无法更新时,可以选择使用国内Gitee源,GitHub CDN可能比原地址更晚更新，但速度快')

o = s:option(DummyValue, '', '')
o.rawhtml = true
o.version = sys.exec('uci get jd-dailybonus.@global[0].version')
o.template = 'jd-dailybonus/update_service'

return m
