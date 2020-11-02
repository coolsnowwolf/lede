  
local nt = require "luci.sys".net
local fs=require"nixio.fs"
local e=luci.model.uci.cursor()
local net = require "luci.model.network".init()
local sys = require "luci.sys"
local ifaces = sys.net:devices()

m=Map("serverchan",translate("ServerChan"),
translate("「Server酱」，英文名「ServerChan」，是一款从服务器推送报警信息和日志到微信的工具。<br /><br />如果你在使用中遇到问题，请到这里提交：")
.. [[<a href="https://github.com/tty228/luci-app-serverchan" target="_blank">]]
.. translate("github 项目地址")
.. [[</a>]]
)

m:section(SimpleSection).template  = "serverchan/status"

s=m:section(NamedSection,"serverchan","serverchan",translate(""))
s:tab("basic", translate("基本设置"))
s:tab("content", translate("推送内容"))
s:tab("crontab", translate("定时推送"))
s:tab("disturb", translate("免打扰"))
s.addremove = false
s.anonymous = true

--基本设置
a=s:taboption("basic", Flag,"serverchan_enable",translate("启用"))
a.default=0
a.rmempty = true

a=s:taboption("basic", ListValue,"send_tg",translate("推送模式"))
a.default=""
a.rmempty = true
a:value("",translate("微信"))
a:value("2",translate("微信测试号版"))
a:value("1",translate("Telegram"))

a=s:taboption("basic", Value,"sckey",translate('SCKEY'), translate("Serverchan Sckey").."<br>调用代码获取<a href='http://sc.ftqq.com' target='_blank'>点击这里</a><br><br>")
a.rmempty = true
a:depends("send_tg","")

a=s:taboption("basic", Value,"sctkey",translate('SCKEY'), translate("Serverchan Sckey").."<br>调用代码获取<a href='https://sct.ftqq.com/' target='_blank'>点击这里</a><br><br>")
a.rmempty = true
a:depends("send_tg","2")

a=s:taboption("basic", Value, "tgtoken", translate("tg推送链接"),translate("").."<br>目前公众号已停用，需要自建服务器<br>获取机器人<a href='https://github.com/Fndroid/tg_push_bot' target='_blank'>点击这里</a><br>此处填入推送链接，如https://tgbot.lbyczf.com/sendMessage/:Token<br>")
a.rmempty = true
a:depends("send_tg","1")

a=s:taboption("basic", Value,"device_name",translate('本设备名称'))
a.rmempty = true
a.description = translate("在推送信息标题中会标识本设备名称，用于区分推送信息的来源设备")

a=s:taboption("basic", Value,"sleeptime",translate('检测时间间隔'))
a.rmempty = true
a.optional = false
a.default = "60"
a.description = translate("越短的时间时间响应越及时，但会占用更多的系统资源")

a=s:taboption("basic", ListValue,"oui_data",translate("MAC设备信息数据库"))
a.rmempty = true
a.default=""
a:value("",translate("关闭"))
a:value("1",translate("简化版"))
a:value("2",translate("完整版"))
a:value("3",translate("网络查询"))
a.description = translate("需下载 4.36m 原始数据，处理后完整版约 1.2M，简化版约 250kb <br/>若无梯子，请勿使用网络查询")

a=s:taboption("basic", Flag,"oui_dir",translate("下载到内存"))
a.rmempty = true
a:depends("oui_data","1")
a:depends("oui_data","2")
a.description = translate("懒得做自动更新了，下载到内存中，重启会重新下载 <br/>若无梯子，还是下到机身吧")

a=s:taboption("basic", Flag,"debuglevel",translate("开启日志"))
a.rmempty = true

a= s:taboption("basic", DynamicList, "device_aliases", translate("设备别名"))
a.rmempty = true
a.description = translate("<br/> 请输入设备 MAC 和设备别名，用“-”隔开，如：<br/> XX:XX:XX:XX:XX:XX-我的手机")

--设备状态
a=s:taboption("content", ListValue,"serverchan_ipv4",translate("ipv4 变动通知"))
a.rmempty = true
a.default=""
a:value("",translate("关闭"))
a:value("1",translate("通过接口获取"))
a:value("2",translate("通过URL获取"))

a = s:taboption("content", ListValue, "ipv4_interface", translate("接口名称"))
a.rmempty = true
a:depends({serverchan_ipv4="1"})
for _, iface in ipairs(ifaces) do
	if not (iface == "lo" or iface:match("^ifb.*")) then
		local nets = net:get_interface(iface)
		nets = nets and nets:get_networks() or {}
		for k, v in pairs(nets) do
			nets[k] = nets[k].sid
		end
		nets = table.concat(nets, ",")
		a:value(iface, ((#nets > 0) and "%s (%s)" % {iface, nets} or iface))
	end
end
a.description = translate("<br/>一般选择 wan 接口，多拨环境请自行选择")

a= s:taboption("content", Value, "ipv4_URL", "URL 地址")
a.rmempty = true
a.default = "members.3322.org/dyndns/getip"
a:depends({serverchan_ipv4="2"})
a.description = translate("<br/>会因服务器稳定性、连接频繁等原因导致获取失败<br/>如接口可以正常获取 IP，不推荐使用")

a=s:taboption("content", ListValue,"serverchan_ipv6",translate("ipv6 变动通知"))
a.rmempty = true
a.default="disable"
a:value("0",translate("关闭"))
a:value("1",translate("通过接口获取"))
a:value("2",translate("通过URL获取"))

a = s:taboption("content", ListValue, "ipv6_interface", translate("接口名称"))
a.rmempty = true
a:depends({serverchan_ipv6="1"})
for _, iface in ipairs(ifaces) do
	if not (iface == "lo" or iface:match("^ifb.*")) then
		local nets = net:get_interface(iface)
		nets = nets and nets:get_networks() or {}
		for k, v in pairs(nets) do
			nets[k] = nets[k].sid
		end
		nets = table.concat(nets, ",")
		a:value(iface, ((#nets > 0) and "%s (%s)" % {iface, nets} or iface))
	end
end
a.description = translate("<br/>一般选择 wan 接口，多拨环境请自行选择")

a= s:taboption("content", Value, "ipv6_URL", "URL 地址")
a.rmempty = true
a.default = "v6.ip.zxinc.org/getip"
a:depends({serverchan_ipv6="2"})
a.description = translate("<br/>会因服务器稳定性、连接频繁等原因导致获取失败<br/>如接口可以正常获取 IP，不推荐使用")

a=s:taboption("content", Flag,"serverchan_up",translate("设备上线通知"))
a.default=1
a.rmempty = true

a=s:taboption("content", Flag,"serverchan_down",translate("设备下线通知"))
a.default=1
a.rmempty = true

a=s:taboption("content", Flag,"cpuload_enable",translate("CPU 负载报警"))
a.default=1
a.rmempty = true

a= s:taboption("content", Value, "cpuload", "负载报警阈值")
a.default = 2
a.rmempty = true
a:depends({cpuload_enable="1"})

a=s:taboption("content", Flag,"temperature_enable",translate("CPU 温度报警"))
a.default=1
a.rmempty = true
a.description = translate("请确认设备可以获取温度，如需修改命令，请移步高级设置")

a= s:taboption("content", Value, "temperature", "温度报警阈值")
a.rmempty = true
a.default = "80"
a.datatype="uinteger"
a:depends({temperature_enable="1"})
a.description = translate("<br/>设备报警只会在连续五分钟超过设定值时才会推送<br/>而且一个小时内不会再提醒第二次")

a=s:taboption("content", Flag,"client_usage",translate("设备异常流量"))
a.default=0
a.rmempty = true

a= s:taboption("content", Value, "client_usage_max", "每分钟流量限制")
a.default = "10M"
a.rmempty = true
a:depends({client_usage="1"})
a.description = translate("设备异常流量警报（byte），你可以追加 K 或者 M")

a=s:taboption("content", Flag,"client_usage_disturb",translate("异常流量免打扰"))
a.default=1
a.rmempty = true
a:depends({client_usage="1"})

a = s:taboption("content", DynamicList, "client_usage_whitelist", translate("异常流量关注列表"))
nt.mac_hints(function(mac, name) a:value(mac, "%s (%s)" %{ mac, name }) end)
a.rmempty = true
a:depends({client_usage_disturb="1"})
a.description = translate("请输入设备 MAC")

--定时推送
a=s:taboption("crontab", ListValue,"send_mode",translate("定时任务设定"))
a.rmempty = true
a.default=""
a:value("",translate("关闭"))
a:value("1",translate("定时发送"))
a:value("2",translate("间隔发送"))

a=s:taboption("crontab", ListValue,"regular_time",translate("发送时间"))
a.rmempty = true
for t=0,23 do
a:value(t,translate("每天"..t.."点"))
end	
a.default=8	
a.datatype=uinteger
a:depends("send_mode","1")

a=s:taboption("crontab", ListValue,"regular_time_2",translate("发送时间"))
a.rmempty = true
a:value("",translate("关闭"))
for t=0,23 do
a:value(t,translate("每天"..t.."点"))
end	
a.default="关闭"
a.datatype=uinteger
a:depends("send_mode","1")

a=s:taboption("crontab", ListValue,"regular_time_3",translate("发送时间"))
a.rmempty = true

a:value("",translate("关闭"))
for t=0,23 do
a:value(t,translate("每天"..t.."点"))
end	
a.default="关闭"
a.datatype=uinteger
a:depends("send_mode","1")

a=s:taboption("crontab", ListValue,"interval_time",translate("发送间隔"))
a.rmempty = true
for t=1,23 do
a:value(t,translate(t.."小时"))
end
a.default=6
a.datatype=uinteger
a:depends("send_mode","2")
a.description = translate("<br/>从 00:00 开始，每 * 小时发送一次")

a= s:taboption("crontab", Value, "send_title", translate("微信推送标题"))
a:depends("send_mode","1")
a:depends("send_mode","2")
a.placeholder = "OpenWrt By tty228 路由状态："
a.description = translate("<br/>使用特殊符号可能会造成发送失败")

a=s:taboption("crontab", Flag,"router_status",translate("系统运行情况"))
a.default=1
a:depends("send_mode","1")
a:depends("send_mode","2")

a=s:taboption("crontab", Flag,"router_temp",translate("设备温度"))
a.default=1
a:depends("send_mode","1")
a:depends("send_mode","2")
 
a=s:taboption("crontab", Flag,"router_wan",translate("WAN信息"))
a.default=1
a:depends("send_mode","1")
a:depends("send_mode","2")

a=s:taboption("crontab", Flag,"client_list",translate("客户端列表"))
a.default=1
a:depends("send_mode","1")
a:depends("send_mode","2") 

e=s:taboption("crontab", Button,"_add",translate("手动发送"))
e.inputtitle=translate("发送")
e:depends("send_mode","1")
e:depends("send_mode","2")
e.inputstyle = "apply"
function e.write(self, section)
luci.sys.call("cbi.apply")
        luci.sys.call("/usr/bin/serverchan/serverchan send &")
end

--免打扰
a=s:taboption("disturb", ListValue,"serverchan_sheep",translate("免打扰时段设置"),translate("在指定整点时间段内，暂停推送消息<br/>免打扰时间中，定时推送也会被阻止。"))
a.rmempty = true

a:value("",translate("关闭"))
a:value("1",translate("模式一：脚本挂起"))
a:value("2",translate("模式二：静默模式"))
a.description = translate("模式一停止一切检测，包括无人值守。")
a=s:taboption("disturb", ListValue,"starttime",translate("免打扰开始时间"))
a.rmempty = true

for t=0,23 do
a:value(t,translate("每天"..t.."点"))
end
a.default=0
a.datatype=uinteger
a:depends({serverchan_sheep="1"})
a:depends({serverchan_sheep="2"})
a=s:taboption("disturb", ListValue,"endtime",translate("免打扰结束时间"))
a.rmempty = true

for t=0,23 do
a:value(t,translate("每天"..t.."点"))
end
a.default=8
a.datatype=uinteger
a:depends({serverchan_sheep="1"})
a:depends({serverchan_sheep="2"})

a=s:taboption("disturb", ListValue,"macmechanism",translate("MAC过滤"))
a:value("",translate("disable"))
a:value("allow",translate("忽略列表内设备"))
a:value("block",translate("仅通知列表内设备"))
a:value("interface",translate("仅通知此接口设备"))
a.rmempty = true


a = s:taboption("disturb", DynamicList, "serverchan_whitelist", translate("忽略列表"))
nt.mac_hints(function(mac, name) a :value(mac, "%s (%s)" %{ mac, name }) end)
a.rmempty = true
a:depends({macmechanism="allow"})

a = s:taboption("disturb", DynamicList, "serverchan_blacklist", translate("关注列表"))
nt.mac_hints(function(mac, name) a:value(mac, "%s (%s)" %{ mac, name }) end)
a.rmempty = true
a:depends({macmechanism="block"})

a = s:taboption("disturb", ListValue, "serverchan_interface", translate("接口名称"))
a:depends({macmechanism="interface"})
a.rmempty = true

for _, iface in ipairs(ifaces) do
	if not (iface == "lo" or iface:match("^ifb.*")) then
		local nets = net:get_interface(iface)
		nets = nets and nets:get_networks() or {}
		for k, v in pairs(nets) do
			nets[k] = nets[k].sid
		end
		nets = table.concat(nets, ",")
		a:value(iface, ((#nets > 0) and "%s (%s)" % {iface, nets} or iface))
	end
end

return m