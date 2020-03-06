-- Copyright (C) 2019 Twinzo1 <1282055288@qq.com>

local m, s, a
local fs = require "nixio.fs"
local sys = require "luci.sys"
local uci = require("luci.model.uci").cursor()

function time_validator(self, value, desc)
    if value ~= nil then
        h_str, m_str = string.match(value, "^(%d%d?):(%d%d?)$")
        h = tonumber(h_str)
        m = tonumber(m_str)
        if ( h ~= nil and
             h >= 0   and
             h <= 23  and
             m ~= nil and
             m >= 0   and
             m <= 59) then
            return value
        end
    end
    return nil, translatef("The value %s is invalid", desc)
end

local time_area = {"00", "01", "02", "03", "04", "05", 
				"06", "07", "08", "09", "10", "11", 
				"12", "13", "14", "15", "16", "17", 
				"18", "19", "20", "21", "22", "23"}
local days = {"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"}  
---------------------------------------------------------------------------------------------------

-- BEGIN Map
m = Map("oafControl", translate("应用过滤控制器"), translate("在这里可以控制应用过滤的时间"))

-- END Map

-- BEGIN Global Section
a = m:section(TypedSection, "global")
a.optional = false
a.rmempty = false
a.anonymous = true
-- END Section

--b = a:option(Button, "jump", translate("点此转到应用过滤页面"))
--b.template="cbi/jump"
--function m.on_commit(self)
 --   sys.exec("/usr/bin/oafControl.sh cron")
--end

b = a:option(Flag, "global_enabled", translate("Enable"),translate("总开关"))
b.optional = false
b.rmempty = true

b = a:option(Flag, "global_update", translate("自动更新"),translate("自动更新特征库"))
b.optional = false
b.rmempty = true

b = a:option(Value, "global_run", translate("远程运行"),translate("单位为分钟，建议填1到30的整数。从github获取控制状态，便于远程控制，“0”代表禁止。可以远程启用或停止，默认一分钟检测一次"))
b.optional = false
b.rmempty = true
b.default = "1"

b = a:option(Value, "global_wget", translate("远程控制"),translate("远程控制的url，如果不知道，请默认或清空"))
b.optional = false
b.rmempty = true
b.default = "https://github.com/Twinzo1/openwrt/oafStatus"

-- BEGIN Global Section
a = m:section(TypedSection, "vacation",translate("假期"))
a.optional = false
a.rmempty = false
a.addremove=true
a.anonymous=true
a.template="cbi/tblsection"
-- END Section

b = a:option(Flag, "enable", translate("Enable"))
b.optional = false
b.rmempty = false

b = a:option(Datepicker, "rest_start", translate("开始时间"))
b.optional = false
b.rmempty = true

b = a:option(Datepicker, "rest_end", translate("结束时间"))
b.optional = false
b.rmempty = true

b = a:option(Value, "comment", translate("Comments"))
b.optional = false
b.rmempty = true

-- BEGIN Global Section
a = m:section(TypedSection, "overtime",translate("加班"), translate("添加加班时间，比如今天乃非工作日，但是要上班"))
a.addremove=true
a.anonymous=true
a.template="cbi/tblsection"
-- END Section

b = a:option(Flag, "enable", translate("Enable"))
b.optional = false
b.rmempty = false

b = a:option(Datepicker, "overtime_start", translate("开始时间"))
b.optional = false
b.rmempty = true

b = a:option(Datepicker, "overtime_end", translate("结束时间"))
b.optional = false
b.rmempty = true

b = a:option(Value, "comment", translate("Comments"))
b.rmempty = true

-- BEGIN Section
x = m:section(TypedSection, "workday", translate("工作日"))
x.addremove = false
x.anonymous = true
-- END Section

-- BEGIN Enable Checkbox
c = x:option(Flag, "enabled", translate("Enable"))
c.optional = false
c.rmempty = false
-- END Enable Checkbox

-- BEGIN Day(s) of Week
dow = x:option(MultiValue, "daysofweek", translate("Day(s) of Week"))
dow.optional = false
dow.rmempty = false
for i,v in ipairs(days) do  dow:value(i-1, translate(v)) end
-- END Day(s) of Week

c =  x:option(Value, "time_control", translate("时间控制"))
c.template="oafControlWorkday"

-- BEGIN Section
y = m:section(TypedSection, "weekday", translate("非工作日"), translate("如果没有必要可以不启用"))
y.addremove = false
y.anonymous = true
-- END Section

-- BEGIN Enable Checkbox
c = y:option(Flag, "enabled", translate("Enable"))
c.optional = false
c.rmempty = false
-- END Enable Checkbox

-- BEGIN Day(s) of Week
dow = y:option(MultiValue, "daysofweek", translate("Day(s) of Week"))
dow.optional = false
dow.rmempty = false
for i,v in ipairs(days) do  dow:value(i-1, translate(v)) end
-- END Day(s) of Week

c =  y:option(Value, "time_control", translate("时间控制"))
c.template="oafControlNotWorkday"

local apply = luci.http.formvalue("cbi.apply")
local w_m_start=luci.http.formvalue("w_m_start")
local w_a_start=luci.http.formvalue("w_a_start")
local w_n_start=luci.http.formvalue("w_n_start")
local w_m_stop=luci.http.formvalue("w_m_stop")
local w_a_stop=luci.http.formvalue("w_a_stop")
local w_n_stop=luci.http.formvalue("w_n_stop")
local nw_m_start=luci.http.formvalue("nw_m_start")
local nw_a_start=luci.http.formvalue("nw_a_start")
local nw_n_start=luci.http.formvalue("nw_n_start")
local nw_m_stop=luci.http.formvalue("nw_m_stop")
local nw_a_stop=luci.http.formvalue("nw_a_stop")
local nw_n_stop=luci.http.formvalue("nw_n_stop")
if apply then
    luci.sys.exec("uci set oafControl.@workday[0].workday_morning_start=%s" % w_m_start)
    luci.sys.exec("uci set oafControl.@workday[0].workday_morning_stop=%s" % w_m_stop)
    luci.sys.exec("uci set oafControl.@workday[0].workday_afternoon_start=%s" % w_a_start)
    luci.sys.exec("uci set oafControl.@workday[0].workday_afternoon_stop=%s" % w_a_stop)
    luci.sys.exec("uci set oafControl.@workday[0].workday_night_start=%s" % w_n_start)
    luci.sys.exec("uci set oafControl.@workday[0].workday_night_stop=%s" % w_n_stop)
	luci.sys.exec("uci set oafControl.@weekday[0].weekday_morning_start=%s" % nw_m_start)
    luci.sys.exec("uci set oafControl.@weekday[0].weekday_morning_stop=%s" % nw_m_stop)
    luci.sys.exec("uci set oafControl.@weekday[0].weekday_afternoon_start=%s" % nw_a_start)
    luci.sys.exec("uci set oafControl.@weekday[0].weekday_afternoon_stop=%s" % nw_a_stop)
    luci.sys.exec("uci set oafControl.@weekday[0].weekday_night_start=%s" % nw_n_start)
    luci.sys.exec("uci set oafControl.@weekday[0].weekday_night_stop=%s" % nw_n_stop)
	luci.sys.exec("uci commit oafControl")
end

return m