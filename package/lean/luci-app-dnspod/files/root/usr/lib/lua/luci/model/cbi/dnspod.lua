--[[
LuCI - Lua Configuration Interface

Copyright 2010 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0
]]--

require("luci.sys")

m = Map("dnspod", translate("动态DNSPOD"), translate("配置动态DNSPOD"))

s = m:section(TypedSection, "base_arg", "")
s.addremove = false
s.anonymous = true

email = s:option(Value, "login_email", translate("登录邮箱"),"登录DNSPOD的用户名")
pass = s:option(Value, "login_password", translate("登录密码"),"登录DNSPOD的密码")
pass.password=true
main = s:option(Value, "main_domain", translate("主域名"),"想要解析的主域名，例如:baidu.com")
sub = s:option(Value, "sub_domain", translate("子域名"),"想要解析的子域名，多个以空格分开，例如:test hehe")

local apply = luci.http.formvalue("cbi.apply")
if apply then
	io.popen("/etc/init.d/dnspod restart &")
end

return m
