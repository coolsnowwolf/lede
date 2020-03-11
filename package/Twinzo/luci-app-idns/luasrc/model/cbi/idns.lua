-- Copyright (C) 2019 Twinzo1 <1282055288@qq.com>

local m, s, n, x, z
local fs = require "nixio.fs"
local sys = require "luci.sys"

m = Map("idns", translate("Intranet domain name service"), translate("Customize the routing domain name. You can access your router through the domain name in the intranet."))

s = m:section(TypedSection, "idns")
s.addremove = false
s.anonymous = true

domain = s:option(Value,"domain",translate("DomainName"),
translate("Such as tplink.cn of TP_LINK"))

n = m:section(TypedSection, "main_router", translate("主路由或开启dhcp的路由"))
n.addremove = false
n.anonymous = true

enable = n:option(Flag,"enabled",translate("Enable"), translate("点击启用则开始创建dhcp.leases的链接"))
enable.rmempty = false

leases = n:option(Value,"leases",translate("链接文件"),
translate("链接文件的名字"))

comment = n:option(Value,"comment_leases",translate("备注文件"),
translate("链接设备备注名文件的名字，看看你的概况页有没有设备备注，如果有则可以填写"))

x = m:section(TypedSection, "relay_router", translate("中继或未开启dhcp的路由"), translate("各种信息需要与主路由相同"))
x.addremove = false
x.anonymous = true

enable = x:option(Flag,"enabled",translate("Enable"), translate("点击启用则开始获取主路由的dhcp.leases文件"))
enable.rmempty = false

leases = x:option(Value,"upstreamIP",translate("主路由ip"),
translate("主路由或开启dhcp的路由的IP地址"))

comment = x:option(Value,"leases",translate("dhcp文件"),
translate("主路由或开启dhcp的路由的链接文件名"))

comment = x:option(Value,"comment_leases",translate("备注文件"),
translate("链接设备备注名文件的名字，看看你的概况页有没有设备备注，如果有则可以填写"))

z= m:section(TypedSection, "redirection",translate("重定向ip到你的自建网页，防止别人通过超级密码修改你的光猫"))
z.addremove = true
z.anonymous = true
z.template="cbi/tblsection"

enable = z:option(Flag,"enabled",translate("Enable"))
enable.rmempty = false

ip = z:option(Value,"ipaddr",translate("光猫IP"),
translate("光猫或你不想让人登录的的ip地址"))

web = z:option(Value,"web",translate("web地址"),
translate("填你的自建web地址或ip"))

local apply = luci.http.formvalue("cbi.apply")
if apply then
    io.popen("/etc/init.d/idns restart")
end

return m

