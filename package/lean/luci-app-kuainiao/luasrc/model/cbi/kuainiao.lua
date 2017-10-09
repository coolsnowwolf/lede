#fork from https://github.com/zz090923610/thunder-fastNiao,thanks to zz090923610
require("luci.sys")
require("luci.sys.zoneinfo")
require("luci.config")
local fs = require "nixio.fs"
local ut = require "luci.util"
local o=require"luci.model.network".init()
local sys = require "luci.sys"
local m,t,e
m = Map("kuainiao", translate("迅雷快鸟"),translate("迅雷快鸟是迅雷联合宽带运营商推出的一款致力于帮助用户解决宽带低、网速慢、上网体验差的专业级宽带加速软件。"))
m:section(SimpleSection).template  = "kuainiao/kuainiao_status"
s = m:section(NamedSection, "base", "kuainiao", translate("首次使用请填写完帐号密码保存提交一次，之后再启用。"))
s.addremove = false
s:tab("base",translate("Basic Settings"))
s:tab("log",translate("快鸟日志"))
enabled = s:taboption("base",Flag, "enabled", translate("Enable"))
enabled.default=0
enabled.rmempty = false
enable_down = s:taboption("base",Flag, "enable_down", translate("开启下行加速"))
enable_down.default=1
enabled.rmempty = false
enable_down:depends("enabled",1)

enable_up = s:taboption("base",Flag, "enable_up", translate("开启上行加速"))
enable_up.default=0
enabled.rmempty = false
enable_up:depends("enabled",1)
local a
speed_wan=s:taboption("base",ListValue,"speed_wan",translate("指定加速的接口"))
for a,s in ipairs(o:get_networks())do
if s:name()~="loopback" and s:name()~="lan" then speed_wan:value(s:name())end
end
username = s:taboption("base",Value, "kuainiao_name", translate("迅雷快鸟帐号"))
username.datatype = "minlength(1)"
username.rmempty = false
password = s:taboption("base",Value, "kuainiao_passwd", translate("迅雷快鸟密码"))
password.password = true
password.datatype = "minlength(1)"
password.rmempty = false
kuainiao_config_pwd = s:taboption("base",Value, "kuainiao_config_pwd", translate("加密后密码(自动生成,勿修改)"))
kuainiao_config_pwd.password = true
kuainiao_config_pwd.datatype = "minlength(1)"
kuainiao_config_pwd.rmempty = true
--kuainiao_config_pwd.readonly=true
log=s:taboption("log",TextValue,"log")
log.rows=26
log.wrap="off"
log.readonly=true
log.cfgvalue=function(t,t)
return nixio.fs.readfile("/var/log/kuainiao.log")or""
end
log.write=function(log,log,log)
end
m:section(SimpleSection).template  = "kuainiao/kuainiao_rsa"
local apply = luci.http.formvalue("cbi.apply")
if apply then
    io.popen("luci_kuainiao_apply")
end
return m
