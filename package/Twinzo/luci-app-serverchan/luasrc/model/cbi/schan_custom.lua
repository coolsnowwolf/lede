local nt = require "luci.sys".net
local log=require"nixio.fs"
local e=luci.model.uci.cursor()
local net = require "luci.model.network".init()
local sys = require "luci.sys"
local ifaces = sys.net:devices()

m=Map("serverchan",translate("Custom messege"),translate("在这里可以自定义Server酱推送消息，日志请查看另一页面"))

m:section(SimpleSection).template  = "serverchan/serverchan_status"

s=m:section(NamedSection,"serverchan","custom_messeges",translate("Server酱设置"))

s.addremove = true
s.anonymous = true

a=s:taboption("tab_basic", Flag,"serverchan_enable",translate("启用"))
a.default=0
a.rmempty=true

f=s:taboption(TextValue,"custom_messages",translate("自定义消息"))
f.description = translate("<br/><b>使用须知：</b><ol><li>格式为：<b>发送标题:-:发送模式:-:发送时间:-:自定义消息"
.."<br/>例如：1:-:00:00:-:睡觉</b></li>".."<li>发送模式：<ul><li>0为间隔模式,可选时间为1h-24h及1m-59m，可结合使用，如1h-30m</li>"
.."<li>1为定时模式，时间格式为：01:30</li></ul></li>".."<li>开头使用#可以注释掉，即忽略该条信息，如：#1:-:00:00:-:睡觉</li>"
.."<li>使用特殊符号可能会造成发送失败</li></ol>")
f.placeholder=("0:-:00:00:-:睡觉")

e=s:option(ListValue,"send_mode",translate("定时任务设定"))
e.default="disable"
e:value("",translate("关闭"))
e:value("1",translate("定时发送"))
e:value("2",translate("间隔发送"))

title= s:option(Value, "send_title", translate("微信推送标题"))
title:depends("send_mode","1")
title:depends("send_mode","2")
title.placeholder = "OpenWrt By tty228 路由状态："
title.optional = true
title.description = translate("<br/>使用特殊符号可能会造成发送失败")

e=s:option(ListValue,"regular_time",translate("发送时间"))
for t=0,23 do
e:value(t,translate("每天"..t.."点"))
end
e.default=12
e.datatype=uinteger
e:depends("send_mode","1")

e=s:option(ListValue,"interval_time",translate("发送间隔"))
for t=1,23 do
e:value(t,translate(t.."小时"))
end
e.default=6
e.datatype=uinteger
e:depends("send_mode","2")

a = s:option(Value, "message_location", translate("消息文件保存位置，会自动在文件名后加上时间"))
a.rmempty = false
a.default = "/etc/serverchan/messages.txt"

a = s:option(TextValue, "messages", translate("自定义消息"))
a.rows = 5
a.size = 60
a.wrap = "off"
local locate_scm = uci:get("serverchan","serverchan","message_location")
function a.cfgvalue(self, section)
	return nixio.fs.readfile(locate_scm)
end

function a.write(self, section, value)
	value = value:gsub("\r\n?", "\n")
	nixio.fs.writefile(locate_scm, value)
end

e=s:taboption("log",TextValue,"log")

local apply = luci.http.formvalue("cbi.apply")
 if apply then
     io.popen("/etc/init.d/serverchan start")
end
return m