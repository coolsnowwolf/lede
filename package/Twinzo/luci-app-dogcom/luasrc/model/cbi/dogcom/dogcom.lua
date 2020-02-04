-- Copyright (C) 2019 Twinzo1 <1282055288@qq.com>

local m, s
local fs = require "nixio.fs"
local sys = require "luci.sys"
local http  = require "luci.http"

local running = (luci.sys.call("pidof dogcom > /dev/null") == 0)
if running then	

	m = Map("dogcom", translate("DRCOM客户端"),translate("")..
	"<b><font color=\"green\">客户端运行中</font></b>" ..
	"<br />"
	..[[<br /><strong>]]
	..[[<a href="https://github.com/Twinzo1/openwrt/tree/master/dogcom_openwrt" target="_blank">]]
	..translate("本项目在GitHub的项目地址")
	..[[</a>]]
	..[[</strong><br />]])

else
	m = Map("dogcom", translate("DRCOM客户端"), translate("")..
	"<b><font color=\"red\">客户端未运行</font></b>"..
	"<br />"
	..[[<br /><strong>]]
	..translate("使用教程:先使用wireshark抓包，然后去分析制作心跳包，填写配置文件。")
	..[[</a>]]
	..[[</strong><br />]]
	..[[<br /><strong>]]
	..translate("心跳包制作地址:https://drcoms.github.io/drcom-generic/")
	..[[</a>]]
	..[[</strong><br />]])

end

s = m:section(TypedSection, "dogcom")
-- 这里的dogcom对应config里面的option
s.addremove = false
s.anonymous = true

-- Basic Settings --
s:tab("basic", translate("通用设置"))

enable = s:taboption("basic",Flag, "enable", translate("开启客户端"))

version=s:taboption("basic",ListValue,"version",translate("请选择Drcom版本"))
version:value("P",translate("P版"))
version:value("D",translate("D版"))
version.default="P"

patch = s:taboption("basic",DummyValue,"ChinaIa",translate("P版拨号补丁"))
patch.template ="dogcom/patch"

enabledial = s:taboption("basic",Flag, "enabledial", translate("启用PPPoE拨号"))
enabledial:depends({version="P"})

interface = s:taboption("basic",ListValue, "interface", translate("Interface"), translate("请选择你的拨号接口. (通常是WAN/wan.)"))
interface:depends("enabledial", "1")

cur = luci.model.uci.cursor()
net = cur:get_all("network")
for k, v in pairs(net) do
	for k1, v1 in pairs(v) do
		if v1 == "interface" then
			interface:value(k)
			if k == "WAN" or k == "wan" then
				interface.default = k
			end
		end
	end
end

user = s:taboption("basic",Value, "user", translate("Username"))
user:depends("enabledial", "1")

pwd = s:taboption("basic",Value, "pwd", translate("Password"))
pwd:depends("enabledial", "1")
pwd.password = true

macaddr = s:taboption("basic",Value, "macaddr", translate("Mac地址"))
macaddr:depends({version="P"})
macaddr.datatype="macaddr"
luci.ip.neighbors({ family = 4,dest = http.getenv("REMOTE_ADDR") or "?" }, function(n) 
	macaddr.description="当前设备（你的电脑）mac地址："..(n.mac) 
end)

remote_server = s:taboption("basic",Value, "server", translate("认证服务器地址"))
remote_server.datatype = "ip4addr"

pppoe_flag = s:taboption("basic",Value, "pppoe_flag", translate("pppoe_flag"))
pppoe_flag:depends({version="P"})

keep_alive2_flag = s:taboption("basic",Value, "keep_alive2_flag", translate("keep_alive2_flag"))
keep_alive2_flag:depends({version="P"})

username = s:taboption("basic",Value, "username", translate("用户名"))
username:depends({version="D"})
username.default = "123"

password = s:taboption("basic",Value, "password", translate("密码"))
password:depends({version="D"})
password.datatype = "maxlength(16)"
password.password = true
password.default = "123"

host_name = s:taboption("basic",Value, "host_name", translate("主机名称"))
host_name:depends({version="D"})
host_name.datatype = "maxlength(32)"
host_name.default = "HP"

host_os = s:taboption("basic",Value, "host_os", translate("主机操作系统"))
host_os:depends({version="D"})
host_os.datatype = "maxlength(32)"
host_os.default = "DOS"

host_ip = s:taboption("basic",Value, "host_ip", translate("主机IP"))
host_ip:depends({version="D"})
host_ip.datatype = "ip4addr"
host_ip.default = "0.0.0.0"

dhcp_server = s:taboption("basic",Value, "dhcp_server", translate("DHCP服务器"))
dhcp_server:depends({version="D"})
dhcp_server.datatype = "ip4addr"
dhcp_server.default = "0.0.0.0"

mac = s:taboption("basic",Value, "mac", translate("绑定MAC地址"))
mac:depends({version="D"})
mac.default = "0xaabbccddeeff"

PRIMARY_DNS = s:taboption("basic",Value, "PRIMARY_DNS", translate("PRIMARY_DNS"))
PRIMARY_DNS:depends({version="D"})
PRIMARY_DNS.default = "114.114.114.114"

AUTH_VERSION = s:taboption("basic",Value, "AUTH_VERSION", translate("AUTH_VERSION"))
AUTH_VERSION:depends({version="D"})
AUTH_VERSION.default = "\x0a\x00"

KEEP_ALIVE_VERSION = s:taboption("basic",Value, "KEEP_ALIVE_VERSION", translate("KEEP_ALIVE_VERSION"))
KEEP_ALIVE_VERSION:depends({version="D"})
KEEP_ALIVE_VERSION.default = "\xdc\x02"

CONTROLCHECKSTATUS = s:taboption("basic",Value, "CONTROLCHECKSTATUS", translate("CONTROLCHECKSTATUS"))
CONTROLCHECKSTATUS:depends({version="D"})
CONTROLCHECKSTATUS.default = "\x20"

ADAPTERNUM = s:taboption("basic",Value, "ADAPTERNUM", translate("ADAPTERNUM"))
ADAPTERNUM:depends({version="D"})
ADAPTERNUM.default = "\x01"

IPDOG = s:taboption("basic",Value, "IPDOG", translate("IPDOG"))
IPDOG:depends({version="D"})
IPDOG.default = "\x01"

log_clear = s:taboption("basic",Flag, "log_clear", translate("自动清除日志"),translate("每五分钟清除一次日志"))
log_clear.default = "1"

watchdog = s:taboption("basic",Value, "watchdog", translate("网络守护"),translate("每x分钟检测网络连通性，连接失败则自动修改mac地址并重启网络('0'为不启用)"))
watchdog.default = "0"

a = s:taboption("basic",Flag, "en_JumpTwoMac", translate("mac变换"),translate("如启用，则mac地址只会在当前mac地址前后变化（据观察，这样的重连率更高）"))
a:depends({version="P"})
a.default = "1"

-- Generate Configuration --

s:tab("logger", translate("日志"))
s.anonymous=true
local a="/tmp/dogcom.log"
dogcom_log=s:taboption("logger",TextValue,"sylogtext")
dogcom_log.rows=18
dogcom_log.readonly="readonly"
dogcom_log.wrap="off"
function dogcom_log.cfgvalue(s,s)
sylogtext=""
if a and nixio.fs.access(a)then
sylogtext=luci.sys.exec("tail -n 100 %s"%a)
end
return sylogtext
end
dogcom_log.write=function(s,s,s)
end

local apply = luci.http.formvalue("cbi.apply")
if apply then
    io.popen("/etc/init.d/dogcom restart")
end

return m