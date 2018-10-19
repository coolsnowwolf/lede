local a=require"luci.sys"
local e=luci.model.uci.cursor()
local e=require"nixio.fs"
require("luci.sys")
local t,e,o
local m,s
t=Map("aliddns",translate("阿里DDNS客户端"),translate("基于阿里云解析的私人DDNS解决方案"))

e=t:section(TypedSection,"base")
e.anonymous=true

e:tab("basic",  translate("设置"))

enable=e:taboption("basic",Flag,"enable",translate("开启"),translate("开启或关闭aliddns动态域名"))
enable.rmempty=false
token=e:taboption("basic",Value,"app_key",translate("APP KEY"))
email=e:taboption("basic",Value,"app_secret",translate("APP SECRET"))
iface=e:taboption("basic",ListValue,"interface",translate("选择外网接口"),translate("限定要动态aliddns的外网接口，如pppoe-wan"))
iface:value("",translate("选择要动态更新的外网接口"))
for t,e in ipairs(a.net.devices())do
if e~="lo"then iface:value(e)end
end
iface.rmempty=false
main=e:taboption("basic",Value,"main_domain",translate("主域名"),"想要解析的主域名，例如:baidu.com")
main.rmempty=false
sub=e:taboption("basic",Value,"sub_domain",translate("子域名"),"想要解析的子域名，例如:test hehe")
sub.rmempty=false
time=e:taboption("basic",Value,"time",translate("检查时间"),"域名检查时间，单位分钟，范围1-59")
time.rmempty=false

e:tab("log",  translate("更新记录"))
--e=t:section(TypedSection,"base",translate("更新记录"))
e.anonymous=true
local a="/var/log/aliddns.log"
tvlog=e:taboption("log",TextValue,"sylogtext")
tvlog.rows=14
tvlog.readonly="readonly"
tvlog.wrap="off"
function tvlog.cfgvalue(e,e)
sylogtext=""
if a and nixio.fs.access(a)then
sylogtext=luci.sys.exec("tail -n 100 %s"%a)
end
return sylogtext
end
tvlog.write=function(e,e,e)
end
local e=luci.http.formvalue("cbi.apply")
if e then
io.popen("/etc/init.d/aliddns restart")
end
return t
