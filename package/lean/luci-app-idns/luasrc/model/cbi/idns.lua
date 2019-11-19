-- Copyright (C) 2019 Twinzo1 <1282055288@qq.com>

local m, s
local fs = require "nixio.fs"
local sys = require "luci.sys"

	m = Map("idns", translate("Intranet domain name service"), translate("Customize the routing domain name. You can access your router through the domain name in the intranet."))

s = m:section(TypedSection, "idns")
s.addremove = false
s.anonymous = true

domain = s:option(Value,"domain",translate("DomainName"),
translate("Such as tplink.cn of TP_LINK"))


local apply = luci.http.formvalue("cbi.apply")
if apply then
    io.popen("/etc/init.d/idns restart")
end

return m
