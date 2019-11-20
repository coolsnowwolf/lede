-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Cpyright by Twizo<1282055288@qq.com>
-- Licensed to the public under the Apache License 2.0.

local m, s
local fs = require "nixio.fs"
local sys = require "luci.sys"

m = Map("dogcom_proxy", translate("防代理设置"),
translate("启用防代理设置，会使某些网站上不了，需要配合代理软件才能看图片，如tinyproxy，privoxy等"))

s = m:section(TypedSection, "dogcom_proxy")
-- 这里的dogcom_log对应config里面的option
s.addremove = false
s.anonymous = true

enable = s:option(Flag, "enable", translate("启用防代理"))

release = s:option(DynamicList,"release",translate("设备放行"),
translate("对这些IP不做任何限制，即是否启用防代理对其没有影响，如果开启了ssr/ss，也将不能科学上网"))
release.placeholder = "192.168.1.241"

natport = s:option(Value, "natport", translate("端口"),translate("需要重定向的端口，默认80端口"))
natport.default = "80"
natport.datatype = "port"

port = s:option(Value, "port", translate("重定向端口"),translate("重定向到此端口，在代理软件中填写此端口"))
port.default = "8118"
port.datatype = "port"

local apply = luci.http.formvalue("cbi.apply")
if apply then
    io.popen("/etc/init.d/dogcom_proxy restart")
end


return m
