local nt = require "luci.sys".net
local fs=require"nixio.fs"

m=Map("serverchan",translate("提示："),
translate("如果你不了解这些选项的含义，请不要修改这些选项"))

s = m:section(TypedSection, "serverchan", "高级设置")
s.anonymous = true
s.addremove = false

a=s:option(Value,"up_timeout",translate('设备上线检测超时'))
a.default = "2"
a.optional=false
a.datatype="uinteger"

a=s:option(Value,"down_timeout",translate('设备离线检测超时'))
a.default = "20"
a.optional=false
a.datatype="uinteger"

a=s:option(Value,"timeout_retry_count",translate('离线检测次数'))
a.default = "2"
a.optional=false
a.datatype="uinteger"
a.description = translate("若无二级路由设备，信号强度良好，可以减少以上数值<br/>因夜间 wifi 休眠较为玄学，遇到设备频繁推送断开，烦请自行调整参数<br/>..╮(╯_╰）╭..")

a=s:option(Value,"thread_num",translate('最大并发进程数'))
a.default = "3"
a.datatype="uinteger"

a=s:option(Value, "soc_code", "自定义温度读取命令")
a.rmempty = true 
a:value("",translate("默认"))
a:value("sensors",translate("sensors命令"))
a.description = translate("请尽量避免使用特殊符号，如双引号、$、!等，执行结果需为数字，用于温度对比")

a=s:option(Button,"soc",translate("测试温度命令"))
a.inputtitle = translate("输出信息")
a.write = function()
	luci.sys.call("/usr/bin/serverchan/serverchan soc")
	luci.http.redirect(luci.dispatcher.build_url("admin","services","serverchan","advanced"))
end

if nixio.fs.access("/tmp/serverchan/soc_tmp") then
e=s:option(TextValue,"soc_tmp")
e.rows=2
e.readonly=true
e.cfgvalue = function()
	return luci.sys.exec("cat /tmp/serverchan/soc_tmp && rm -f /tmp/serverchan/soc_tmp")
end
end

a=s:option(Flag,"err_enable",translate("无人值守任务"))
a.default=0
a.rmempty=true
a.description = translate("请确认脚本可以正常运行，否则可能造成频繁重启等错误！")

a=s:option(Flag,"err_sheep_enable",translate("仅在免打扰时段重拨"))
a.default=0
a.rmempty=true
a.description = translate("避免白天重拨 ddns 域名等待解析，此功能不影响断网检测<br/>因夜间跑流量问题，该功能可能不稳定")
a:depends({err_enable="1"})

a= s:option(DynamicList, "err_device_aliases", translate("关注列表"))
a.rmempty = true 
a.description = translate("只会在列表中设备都不在线时才会执行<br/>免打扰时段一小时后，关注设备五分钟低流量（约100kb/m）将视为离线")
nt.mac_hints(function(mac, name) a :value(mac, "%s (%s)" %{ mac, name }) end)
a:depends({err_enable="1"})

a=s:option(ListValue,"network_err_event",translate("网络断开时"))
a.default=""
a:depends({err_enable="1"})
a:value("",translate("无操作"))
a:value("1",translate("重启路由器"))
a:value("2",translate("重新拨号"))
a:value("3",translate("修改相关设置项，尝试自动修复网络"))
a.description = translate("选项 1 选项 2 不会修改设置，并最多尝试 2 次。<br/>选项 3 会将设置项备份于 /usr/bin/serverchan/configbak 目录，并在失败后还原。<br/>【！！无法保证兼容性！！】不熟悉系统设置项，不会救砖请勿使用")

a=s:option(ListValue,"system_time_event",translate("定时重启"))
a.default=""
a:depends({err_enable="1"})
a:value("",translate("无操作"))
a:value("1",translate("重启路由器"))
a:value("2",translate("重新拨号"))

a= s:option(Value, "autoreboot_time", "系统运行时间大于")
a.rmempty = true 
a.default = "24"
a.datatype="uinteger"
a:depends({system_time_event="1"})
a.description = translate("单位为小时")

a=s:option(Value, "network_restart_time", "网络在线时间大于")
a.rmempty = true 
a.default = "24"
a.datatype="uinteger"
a:depends({system_time_event="2"})
a.description = translate("单位为小时")

a=s:option(Flag,"public_ip_event",translate("重拨尝试获取公网 ip"))
a.default=0
a.rmempty=true
a:depends({err_enable="1"})
a.description = translate("重拨时不会推送 ip 变动通知，并会导致你的域名无法及时更新 ip 地址<br/>请确认你可以通过重拨获取公网 ip，否则这不仅徒劳无功还会引起频繁断网<br/>移动等大内网你就别挣扎了！！")

a= s:option(Value, "public_ip_retry_count", "当天最大重试次数")
a.rmempty = true 
a.default = "10"
a.datatype="uinteger"
a:depends({public_ip_event="1"})

return m