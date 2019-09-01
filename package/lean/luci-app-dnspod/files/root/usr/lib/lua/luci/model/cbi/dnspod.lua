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

email = s:option(Value, "login_token", translate("DNSPOD Token"), "格式: ID,Token")
main = s:option(Value, "main_domain", translate("主域名"), "想要解析的主域名，例如:baidu.com")
sub = s:option(Value, "sub_domain", translate("子域名"), "想要解析的子域名，只允许填写一个。如果想解析*子域名，请填写 \\*")
wait = s:option(Value, "wait_second", translate("更新周期(s)"), "请填写数字，默认为300s")

local apply = luci.http.formvalue("cbi.apply")
if apply then
    io.popen("/etc/init.d/dnspod restart &")
end

return m
