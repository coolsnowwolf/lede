--[[
LuCI - Lua Configuration Interface

#
#-- Copyright (C) 2019 dz <dingzhong110@gmail.com>
#
# This is free software, licensed under the Apache License, Version 2.0 .
#
]]--

require("luci.sys")

if luci.sys.call("ps -w | grep ddnspod |grep -v grep >/dev/null") == 0 then
	m = Map("dnspod", translate("动态DNSPOD"), "%s - %s" %{translate("DDNSPOD"), translate("<strong><font color=\"green\">Running</font></strong>")})
else
	m = Map("dnspod", translate("动态DNSPOD"), "%s - %s" %{translate("DDNSPOD"), translate("<strong><font color=\"red\">Not Running</font></strong>")})
end

------------------------------------------------------------
s = m:section(TypedSection, "base_arg", translate("基本设置"))
s.addremove = false
s.anonymous = true

enable = s:option(Flag, "enabled", translate("Enable"), translate("Enable or disable server"))
enable.default = 0
enable.rmempty = false

o = s:option(Value,"time",translate("Inspection Time"),translate("域名检查间隔时间，单位秒"))
o.default = 600
o.rmempty=false

o = s:option(Button,"delete",translate("删除全部域名记录"))
o.inputstyle = "reset"
o.write = function()
  luci.sys.call("cat /dev/null > /usr/share/dnspod/last.ip") 
  luci.http.redirect(luci.dispatcher.build_url("admin", "services", "dnspod"))
end

------------------------------------------------------------
s = m:section(TypedSection, "ip_last", translate("ip获取设置"))
s.addremove = false
s.anonymous = true

o = s:option(ListValue, "IPtype", translate("ip获取途径"))
o:value("1", translate("网络获取一"))
o:value("2", translate("网络获取二"))
o:value("3", translate("网络获取三"))
o:value("4", translate("网络获取四"))
o:value("5", translate("本地获取"))
o.default = 1

o = s:option(Value, "iface", translate("选择接口"))
o:value("wan", translate("WAN"))
o:value("lan", translate("LAN"))
o.default = "wan"
o:depends("IPtype", "5")

o = s:option(Value, "ipv6iface", translate("ipv6选择接口"))
o:value("wan", translate("WAN"))
o:value("lan", translate("LAN"))
o.default = "lan"
o:depends("IPtype", "5")

o = s:option(Value, "linenumber", translate("ipv6选择"))
o:value("1p", translate("第一行"))
o:value("2p", translate("第二行"))
o:value("3p", translate("第三行"))
o:value("4p", translate("第四行"))
o:value("5p", translate("第五行"))
o:value("6p", translate("第六行"))
o.default = "2p"
o:depends("IPtype", "5")

------------------------------------------------------------
s = m:section(TypedSection, "login_mode", translate("账号设置"))
s.addremove = false
s.anonymous = true

o = s:option(ListValue, "mode", translate("解析模式"))
o:value("login", translate("账号解析"))
o:value("token", translate("token解析"))

email = s:option(Value, "login_email", translate("登录邮箱"),"登录DNSPOD的用户名")
email:depends("mode", "login")
pass = s:option(Value, "login_password", translate("登录密码"),"登录DNSPOD的密码")
pass.password=true
pass:depends("mode", "login")

token = s:option(Value, "artoken", translate("ID,TOKEN"))
token:depends("mode", "token")

-----------------------------------------------------------

s = m:section(TypedSection, "domain_name", translate("域名"))
s.template = "cbi/tblsection"
s.anonymous = true
s.addremove = true
s.sortable  = true

record_type = s:option(ListValue, "record_type", translate("记录类型"),"A,AAAA")
record_type:value("A", translate("A"))
record_type:value("AAAA", translate("AAAA"))

sub = s:option(Value, "sub_domain", translate("子域名"),"想要解析的子域名，例如:www")

main = s:option(Value, "main_domain", translate("主域名"),"想要解析的主域名，例如:baidu.com")

comment = s:option(Value, "comment", translate("Comment"))

-------------------------------------------------------
s=m:section(TypedSection,"base",translate("Update Log"))
s.anonymous=true
local a="/var/log/ddnspod.log"
tvlog=s:option(TextValue,"sylogtext")
tvlog.rows=16
tvlog.readonly="readonly"
tvlog.wrap="off"

function tvlog.cfgvalue(s,s)
	sylogtext=""
	if a and nixio.fs.access(a) then
		sylogtext=luci.sys.exec("tail -n 100 %s"%a)
	end
	return sylogtext
end

tvlog.write=function(s,s,s)
end

local apply = luci.http.formvalue("cbi.apply")
if apply then
	io.popen("/etc/init.d/dnspod restart &")
end

return m
