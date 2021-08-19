  
local nt = require "luci.sys".net
local fs=require"nixio.fs"
local e=luci.model.uci.cursor()
local net = require "luci.model.network".init()
local sys = require "luci.sys"
local ifaces = sys.net:devices()

m=Map("pushbot",translate("PushBot"),
translate("「全能推送」，英文名「PushBot」，是一款从服务器推送报警信息和日志到各平台的工具。<br>支持钉钉推送，企业微信推送，PushPlus推送。<br>本插件由tty228/luci-app-serverchan创建，然后七年修改为全能推送自用。<br /><br />如果你在使用中遇到问题，请到这里提交：")
.. [[<a href="https://github.com/zzsj0928/luci-app-pushbot" target="_blank">]]
.. translate("github 项目地址")
.. [[</a>]]
)

m:section(SimpleSection).template  = "pushbot/pushbot_status"

s=m:section(NamedSection,"pushbot","pushbot",translate(""))
s:tab("basic", translate("基本设置"))
s:tab("content", translate("推送内容"))
s:tab("crontab", translate("定时推送"))
s:tab("disturb", translate("免打扰"))
s.addremove = false
s.anonymous = true

--基本设置
a=s:taboption("basic", Flag,"pushbot_enable",translate("启用"))
a.default=0
a.rmempty = true

--精简模式
a=s:taboption("basic", Flag,"lite_enable",translate("精简模式"))
a.default=0
a.rmempty = true

a= s:taboption("basic", Flag, "content_current_device", "精简当前设备列表")
a.default = 0
a.rmempty = true
a:depends({lite_enable="1"})

a= s:taboption("basic", Flag, "content_nowtime", "精简当前时间")
a.default = 0
a.rmempty = true
a:depends({lite_enable="1"})

a= s:taboption("basic", Flag, "content_content", "只推送标题")
a.default = 0
a.rmempty = true
a:depends({lite_enable="1"})

a=s:taboption("basic", ListValue,"send_we",translate("推送模式"))
a.default=""
a.rmempty = true
a:value("",translate("钉钉"))
a:value("1",translate("企业微信"))
a:value("3",translate("飞书"))
a:value("4",translate("Bark"))
a:value("2",translate("PushPlus"))

a=s:taboption("basic", Value,"dd_webhook",translate('Webhook'), translate("钉钉机器人 Webhook").."<br>调用代码获取<a href='https://developers.dingtalk.com/document/robots/custom-robot-access' target='_blank'>点击这里</a><br><br>")
a.rmempty = true
a:depends("send_we","")

a=s:taboption("basic", Value, "we_webhook", translate("Webhook"),translate("企业微信机器人 Webhook").."<br>调用代码获取<a href='https://work.weixin.qq.com/api/doc/90000/90136/91770' target='_blank'>点击这里</a><br><br>")
a.rmempty = true
a:depends("send_we","1")

a=s:taboption("basic", Value,"pp_token",translate('PushPlus Token'), translate("PushPlus Token").."<br>调用代码获取<a href='http://pushplus.plus/doc/' target='_blank'>点击这里</a><br><br>")
a.rmempty = true
a:depends("send_we","2")

a=s:taboption("basic", ListValue,"pp_channel",translate('PushPlus Channel'))
a.rmempty = true
a:depends("send_we","2")
a:value("wechat",translate("wechat：PushPlus微信公众号"))
a:value("cp",translate("cp：企业微信应用"))
a:value("webhook",translate("webhook：第三方webhook"))
a:value("sms",translate("sms：短信"))
a:value("mail",translate("mail：邮箱"))
a.description = translate("第三方webhook：企业微信、钉钉、飞书、server酱<br>sms短信/mail邮箱：PushPlus暂未开放<br>具体channel设定参见：<a href='http://pushplus.plus/doc/extend/webhook.html' target='_blank'>点击这里</a>")

a=s:taboption("basic", Value,"pp_webhook",translate('PushPlus Custom Webhook'), translate("PushPlus 自定义Webhook").."<br>第三方webhook或企业微信调用<br>具体自定义Webhook设定参见：<a href='http://pushplus.plus/doc/extend/webhook.html' target='_blank'>点击这里</a><br><br>")
a.rmempty = true
a:depends("pp_channel","cp")
a:depends("pp_channel","webhook")

a=s:taboption("basic", Flag,"pp_topic_enable",translate("PushPlus 一对多推送"))
a.default=0
a.rmempty = true
a:depends("pp_channel","wechat")

a=s:taboption("basic", Value,"pp_topic",translate('PushPlus Topic'), translate("PushPlus 群组编码").."<br>一对多推送时指定的群组编码<br>具体群组编码Topic设定参见：<a href='http://www.pushplus.plus/push2.html' target='_blank'>点击这里</a><br><br>")
a.rmempty = true
a:depends("pp_topic_enable","1")

a=s:taboption("basic", Value,"fs_webhook",translate('WebHook'), translate("飞书 WebHook").."<br>调用代码获取<a href='https://www.feishu.cn/hc/zh-CN/articles/360024984973' target='_blank'>点击这里</a><br><br>")
a.rmempty = true
a:depends("send_we","3")

a=s:taboption("basic", Value,"bark_token",translate('Bark Token'), translate("Bark Token").."<br>调用代码获取<a href='https://github.com/Finb/Bark' target='_blank'>点击这里</a><br><br>")
a.rmempty = true
a:depends("send_we","4")

a=s:taboption("basic", Flag,"bark_srv_enable",translate("自建 Bark 服务器"))
a.default=0
a.rmempty = true
a:depends("send_we","4")

a=s:taboption("basic", Value,"bark_srv",translate('Bark Server'), translate("Bark 自建服务器地址").."<br>如https://your.domain:port<br>具体自建服务器设定参见：<a href='https://github.com/Finb/Bark' target='_blank'>点击这里</a><br><br>")
a.rmempty = true
a:depends("bark_srv_enable","1")

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

a=s:taboption("basic", Flag,"reset_regularly",translate("每天零点重置流量数据"))
a.rmempty = true

a=s:taboption("basic", Flag,"debuglevel",translate("开启日志"))
a.rmempty = true

a= s:taboption("basic", DynamicList, "device_aliases", translate("设备别名"))
a.rmempty = true
a.description = translate("<br/> 请输入设备 MAC 和设备别名，用“-”隔开，如：<br/> XX:XX:XX:XX:XX:XX-我的手机")

--设备状态
a=s:taboption("content", ListValue,"pushbot_ipv4",translate("ipv4 变动通知"))
a.rmempty = true
a.default=""
a:value("",translate("关闭"))
a:value("1",translate("通过接口获取"))
a:value("2",translate("通过URL获取"))

a = s:taboption("content", ListValue, "ipv4_interface", translate("接口名称"))
a.rmempty = true
a:depends({pushbot_ipv4="1"})
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

a= s:taboption("content", DynamicList, "ipv4_URL", "URL 地址")
a.rmempty = true

a:depends({pushbot_ipv4="2"})
a.description = translate("<br/>会因服务器稳定性、连接频繁等原因导致获取失败<br/>从以上列表中随机一个地址，留空使用默认地址")

a=s:taboption("content", ListValue,"pushbot_ipv6",translate("ipv6 变动通知"))
a.rmempty = true
a.default="disable"
a:value("0",translate("关闭"))
a:value("1",translate("通过接口获取"))
a:value("2",translate("通过URL获取"))

a = s:taboption("content", ListValue, "ipv6_interface", translate("接口名称"))
a.rmempty = true
a:depends({pushbot_ipv6="1"})
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

a= s:taboption("content", DynamicList, "ipv6_URL", "URL 地址")
a.rmempty = true

a:depends({pushbot_ipv6="2"})
a.description = translate("<br/>会因服务器稳定性、连接频繁等原因导致获取失败<br/>从以上列表中随机一个地址，留空使用默认地址")

a=s:taboption("content", Flag,"pushbot_up",translate("设备上线通知"))
a.default=1
a.rmempty = true

a=s:taboption("content", Flag,"pushbot_down",translate("设备下线通知"))
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

a= s:taboption("crontab", Value, "send_title", translate("推送标题"))
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
        luci.sys.call("/usr/bin/pushbot/pushbot send &")
end

--免打扰
a=s:taboption("disturb", ListValue,"pushbot_sheep",translate("免打扰时段设置"),translate("在指定整点时间段内，暂停推送消息<br/>免打扰时间中，定时推送也会被阻止。"))
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
a:depends({pushbot_sheep="1"})
a:depends({pushbot_sheep="2"})
a=s:taboption("disturb", ListValue,"endtime",translate("免打扰结束时间"))
a.rmempty = true

for t=0,23 do
a:value(t,translate("每天"..t.."点"))
end
a.default=8
a.datatype=uinteger
a:depends({pushbot_sheep="1"})
a:depends({pushbot_sheep="2"})

a=s:taboption("disturb", ListValue,"macmechanism",translate("MAC过滤"))
a:value("",translate("disable"))
a:value("allow",translate("忽略列表内设备"))
a:value("block",translate("仅通知列表内设备"))
a:value("interface",translate("仅通知此接口设备"))
a.rmempty = true


a = s:taboption("disturb", DynamicList, "pushbot_whitelist", translate("忽略列表"))
nt.mac_hints(function(mac, name) a :value(mac, "%s (%s)" %{ mac, name }) end)
a.rmempty = true
a:depends({macmechanism="allow"})

a = s:taboption("disturb", DynamicList, "pushbot_blacklist", translate("关注列表"))
nt.mac_hints(function(mac, name) a:value(mac, "%s (%s)" %{ mac, name }) end)
a.rmempty = true
a:depends({macmechanism="block"})

a = s:taboption("disturb", ListValue, "pushbot_interface", translate("接口名称"))
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
