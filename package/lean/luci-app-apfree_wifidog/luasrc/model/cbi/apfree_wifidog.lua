-- Copyright (C) 2016 zhangzf@kunteng.org
-- Licensed to the public under the GNU General Public License v3.

local sys = require "luci.sys"
local opkg = require "luci.model.ipkg"

local packageName = "wifidog"
local m, s, o

local function get_status()
	if luci.sys.call("pidof %s >/dev/null" %{packageName}) == 0 then
		local UpTime = luci.util.exec("wdctl status | grep Uptime | cut -d ' ' -f 2-6")
		local StatusUrl = "http://" .. luci.http.getenv('SERVER_NAME') .. ":2060/wifidog/status"	
		
		return translate("<span id=\"wifidog-status\" style=\"color:green;margin-left:50px\">已运行 " .. UpTime .."</span>") -- .."<a target=\"_blank\" href=" .. StatusUrl .. "> 查看更多</a>")
	else
		return translate("<span id=\"wifidog-status\" style=\"color:red;margin-left:50px\">WifiDog 程序未启动</span>")
	end
end

if opkg.status(packageName)[packageName] then
	return Map(packageName, translate("ApFree_WifiDog"), translate('<b style="color:red">Wifidog is not installed..</b>'))
end

m = Map("wifidog", translate("ApFree_WifiDog"), translate("<a target=\"_blank\" href=\"https://github.com/liudf0716/apfree_wifidog\">ApFree WiFiDog</a>" .. 
															"在完全兼容原版WiFiDog的基础上，在功能、性能和稳定性方面做了大量工作、改进及优化，" ..
															"目前在坤腾固件中广泛使用，使用ApFree WiFidog的在线路由器数量达到1万多台且还在继续增长..." ))

s = m:section(TypedSection, "wifidog", translate("运行状态"), get_status())
s.anonymous = true

s = m:section(TypedSection, "wifidog", translate("程序配置"))
s.anonymous = true
s.addremove = false

s:tab("general", translate("基本设置"))
s:tab("policy", translate("访问控制"))
s:tab("advanced", translate("高级设置"))

-- 基本设置
Enable = s:taboption("general", Flag, "enable", translate("启用"),translate("打开或关闭认证"))
Enable.rmempty = false
Enable.default = "1"

GatewayID = s:taboption("general",Value, "gateway_id", translate("设备 ID"), translate("默认为设备MAC地址"))
GatewayID.placeholder = luci.util.exec("ifconfig br-lan| grep HWaddr | awk -F \" \" '{print $5}' | awk '$1~//{print;exit}' | sed 's/://g'")

s:taboption("general", Value, "auth_server_hostname", translate("认证服务器"), translate("认证服务器地址, 可填写域名或IP"))

ServerPort = s:taboption("general", Value, "auth_server_port", translate("服务器端口"), translate("认证服务器HTTP服务端口"))
ServerPort.datatype = "port"

s:taboption("general", Value, "auth_server_path", translate("服务器路径"), translate("服务端WEB目录，必须以'/'结尾，列'/wifidog/'"))

-- 高级设置
WiredPass = s:taboption("advanced", Flag, "wired_passed", translate("有线免认证"), translate("仅 KunTeng 固件支持"))
WiredPass.rmempty = false

PoolMode = s:taboption("advanced",Flag, "pool_mode", translate("线程池模式"),translate("是否开启线程池"))
PoolMode.rmempty = false

ThreadNumber = s:taboption("advanced", Value, "thread_number", translate("线程数"), translate("设置线程池中的最大线程数"))
ThreadNumber.datatype = "uinteger"
ThreadNumber:depends("pool_mode", "1")
ThreadNumber.default = "5"

QueueSize = s:taboption("advanced", Value, "queue_size", translate("队列大小"), translate("线程池的任务队列长度"))
QueueSizedatatype = "uinteger"
QueueSize:depends("pool_mode", "1")
QueueSize.default = "20"

GatewayInterface = s:taboption("advanced", Value, "gateway_interface", translate("内网接口"), translate("指定开启portal认证的网络接口，默认'br-lan'"))
GatewayInterface.default = "br-lan"
for _, e in ipairs(sys.net.devices()) do
	if e ~= "lo" then GatewayInterface:value(e) end
end

CheckInterval = s:taboption("advanced", Value, "check_interval", translate("检查间隔"), translate("接入客户端在线检测间隔，默认60秒"))
CheckInterval.datatype = "uinteger"
CheckInterval.default = "60"

ClientTimeout = s:taboption("advanced", Value, "client_timeout", translate("超时时间"), translate("客户离线后在此时间段内重新连接不需要再次认证，默认5分"))
ClientTimeout.datatype = "uinteger"
ClientTimeout.default = "5"

MaxConnections = s:taboption("advanced", Value, "httpd_max_conn", translate("最大接入用户数"), translate("最大可同时在线的用户数"))
MaxConnections.datatype = "uinteger"
MaxConnections.default = "200"

-- 访问控制
s:taboption("policy", Value, "trusted_domains", translate("域名白名单"), translate("不用通过认证既可访问的网址，不含\"http://\", 多个URL请用 ,号隔开"))

s:taboption("policy", Value, "trusted_iplist", translate("IP白名单"), translate("不用通过认证既可连接的ip, 多个地址请用 ,号隔开"))

s:taboption("policy", Value, "trusted_maclist", translate("MAC白名单"), translate("非强制认证的用户MAC，多个MAC请用 ,号隔开"))

s:taboption("policy", Value, "untrusted_maclist", translate("MAC黑名单"), translate("禁止访问网络的MAC，多个MAC请用 ,号隔开"))

m:section(SimpleSection).template = "apfree_wifidog/client_list"

return m