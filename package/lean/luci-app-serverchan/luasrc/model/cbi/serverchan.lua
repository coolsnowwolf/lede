local nt = require "luci.sys".net
local log=require"nixio.fs"
local e=luci.model.uci.cursor()
local net = require "luci.model.network".init()
local sys = require "luci.sys"
local ifaces = sys.net:devices()

m=Map("serverchan",translate("ServerChan"),translate("「Server酱」，英文名「ServerChan」，是一款从服务器推送报警信息和日志到微信的工具。"))

m:section(SimpleSection).template  = "serverchan/serverchan_status"

s=m:section(NamedSection,"serverchan","serverchan",translate("Server酱设置"))
s:tab("tab_basic", translate("基本设置"))
s:tab("tab_basic2", translate("设备状态"))
s:tab("tab_basic3", translate("定时推送"))
s:tab("tab_basic4", translate("免打扰"))
s:tab("log",translate("客户端日志"))
s.addremove = false
s.anonymous = true

--Base Setting
a=s:taboption("tab_basic", Flag,"serverchan_enable",translate("启用"))
a.default=0
a.rmempty=true

a=s:taboption("tab_basic", Value,"sckey",translate('SCKEY'))
a.rmempty=true

debuglevel=s:taboption("tab_basic", ListValue,"debuglevel",translate("日志调试等级"))
debuglevel:value("",translate("关闭"))
debuglevel:value("1",translate("简单"))
debuglevel:value("2",translate("详细"))
debuglevel.rmempty = true 
debuglevel.optional = true

device_aliases= s:taboption("tab_basic", DynamicList, "device_aliases", translate("设备别名"))
device_aliases.rmempty = true 
device_aliases.optional = true
device_aliases.description = translate("<br/> 请输入设备 MAC 和设备别名，用“-”隔开，如：<br/> XX:XX:XX:XX:XX:XX-我的手机")


e=s:taboption("log",TextValue,"log")
e:depends({debuglevel="1"})
e:depends({debuglevel="2"})
e.rows=26
e.wrap="off"
e.readonly=true
e.cfgvalue=function(s,s)
return log.readfile("/tmp/serverchan/server_chan.log")or""
end
e.write=function(e,e,e)
end

a=s:taboption("tab_basic2", Flag,"serverchan_ip",translate("WAN ip 变化通知"))
a.default=0
a.rmempty=true
a=s:taboption("tab_basic2", Flag,"serverchan_up",translate("设备上线通知"))
a.default=0
a.rmempty=true
a=s:taboption("tab_basic2", Flag,"serverchan_down",translate("设备下线通知"))
a.default=0
a.rmempty=true
a=s:taboption("tab_basic2", Flag,"cpuload_enable",translate("CPU 负载报警"))
a.default=0
a.rmempty=true
a= s:taboption("tab_basic2", Value, "cpuload", "负载报警阈值")
a.rmempty = true 
a.placeholder = "2.0"
a:depends({cpuload_enable="1"})
a=s:taboption("tab_basic2", Flag,"temperature_enable",translate("CPU 温度报警"))
a.default=0
a.rmempty=true
a= s:taboption("tab_basic2", Value, "temperature", "温度报警阈值")
a.rmempty = true 
a.placeholder = "80"
a:depends({temperature_enable="1"})
a.description = translate("<br/>设备报警只会在连续五次超过阈值的时候才会推送<br/>而且一个小时内不会再提醒第二次")

e=s:taboption("tab_basic3", ListValue,"send_mode",translate("定时任务设定"))
e.default="disable"
e:value("",translate("关闭"))
e:value("1",translate("定时发送"))
e:value("2",translate("间隔发送"))

e=s:taboption("tab_basic3", ListValue,"regular_time",translate("发送时间"))
for t=0,23 do
e:value(t,translate("每天"..t.."点"))
end
e.default=12
e.datatype=uinteger
e:depends("send_mode","1")

e=s:taboption("tab_basic3", ListValue,"interval_time",translate("发送间隔"))
for t=1,23 do
e:value(t,translate(t.."小时"))
end
e.default=6
e.datatype=uinteger
e:depends("send_mode","2")

title= s:taboption("tab_basic3", Value, "send_title", translate("微信推送标题"))
title:depends("send_mode","1")
title:depends("send_mode","2")
title.placeholder = "OpenWrt By tty228 路由状态："
title.optional = true
title.description = translate("<br/>使用特殊符号可能会造成发送失败")

router_status=s:taboption("tab_basic3", Flag,"router_status",translate("系统运行情况"))
router_status:depends("send_mode","1")
router_status:depends("send_mode","2")

router_temp=s:taboption("tab_basic3", Flag,"router_temp",translate("设备温度"))
router_temp:depends("send_mode","1")
router_temp:depends("send_mode","2")
 
router_wan=s:taboption("tab_basic3", Flag,"router_wan",translate("WAN信息"))
router_wan:depends("send_mode","1")
router_wan:depends("send_mode","2")

client_list=s:taboption("tab_basic3", Flag,"client_list",translate("客户端列表"))
client_list:depends("send_mode","1")
client_list:depends("send_mode","2") 

e=s:taboption("tab_basic3", Button,"_add",translate("手动发送"))
e.inputtitle=translate("发送")
e:depends("send_mode","1")
e:depends("send_mode","2")
e.inputstyle = "apply"
function e.write(self, section)

luci.sys.call("cbi.apply")
        luci.sys.call("/usr/bin/serverchan/serverchan send &")
end

sheep=s:taboption("tab_basic4", ListValue,"serverchan_sheep",translate("免打扰时段设置"),translate("在指定整点时间段内，暂停推送消息<br/>免打扰时间中，定时推送也会被阻止。"))
sheep:value("0",translate("关闭"))
sheep:value("1",translate("模式一：脚本挂起，不检测设备"))
--[[sheep:value("2",translate("模式二：脚本继续运行但不发送消息"))--]]
sheep.rmempty = true 
sheep.optional = true
sheep=s:taboption("tab_basic4", ListValue,"starttime",translate("免打扰开始时间"))
for t=0,23 do
sheep:value(t,translate("每天"..t.."点"))
end
sheep.default=0
sheep.datatype=uinteger
sheep:depends({serverchan_sheep="1"})
sheep:depends({serverchan_sheep="2"})
sheep=s:taboption("tab_basic4", ListValue,"endtime",translate("免打扰结束时间"))
for t=0,23 do
sheep:value(t,translate("每天"..t.."点"))
end
sheep.default=8
sheep.datatype=uinteger
sheep:depends({serverchan_sheep="1"})
sheep:depends({serverchan_sheep="2"})

mac=s:taboption("tab_basic4", ListValue,"macmechanism",translate("MAC过滤"))
mac:value("",translate("disable"))
mac:value("allow",translate("忽略列表内设备"))
mac:value("block",translate("仅通知列表内设备"))
mac:value("interface",translate("仅通知此接口设备"))

allowedmac = s:taboption("tab_basic4", DynamicList, "serverchan_whitelist", translate("忽略列表"))
nt.mac_hints(function(mac, name) allowedmac :value(mac, "%s (%s)" %{ mac, name }) end)
allowedmac.rmempty = true 
allowedmac.optional = true
allowedmac:depends({macmechanism="allow"})

blockedmac = s:taboption("tab_basic4", DynamicList, "serverchan_blacklist", translate("关注列表"))
nt.mac_hints(function(mac, name) blockedmac:value(mac, "%s (%s)" %{ mac, name }) end)
blockedmac.rmempty = true 
blockedmac.optional = true
blockedmac:depends({macmechanism="block"})

n = s:taboption("tab_basic4", ListValue, "serverchan_interface", translate("接口名称"))
n:depends({macmechanism="interface"})
for _, iface in ipairs(ifaces) do
	if not (iface == "lo" or iface:match("^ifb.*")) then
		local nets = net:get_interface(iface)
		nets = nets and nets:get_networks() or {}
		for k, v in pairs(nets) do
			nets[k] = nets[k].sid
		end
		nets = table.concat(nets, ",")
		n:value(iface, ((#nets > 0) and "%s (%s)" % {iface, nets} or iface))
	end
end


local apply = luci.http.formvalue("cbi.apply")
 if apply then
     io.popen("/etc/init.d/serverchan start")
end
return m