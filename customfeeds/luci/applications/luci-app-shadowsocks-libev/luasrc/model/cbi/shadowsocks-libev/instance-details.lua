-- Copyright 2017 Yousong Zhou <yszhou4tech@gmail.com>
-- Licensed to the public under the Apache License 2.0.

local ds = require "luci.dispatcher"
local ss = require "luci.model.shadowsocks-libev"

local sname = arg[1]
local redirect_url = ds.build_url("admin/services/shadowsocks-libev/instances")
local s, o

local m = Map("shadowsocks-libev")
local sdata = m:get(sname)
if not sdata then
	luci.http.redirect(redirect_url)
	return
end
local stype = sdata[".type"]
m.redirect = redirect_url
m.title = "shadowsocks-libev - %s - %s" % {stype, sname}


s = m:section(NamedSection, sname, stype)
s:tab("general", translate("General Settings"))
s:tab("advanced", translate("Advanced Settings"))
s:taboption("general", Flag, "disabled", translate("Disable"))
ss.option_install_package(s, "general")
ss.options_common(s, "advanced")

if stype == "ss_server" then
	ss.options_server(s, "general")
	o = s:taboption("general", Value, "bind_address",
		translate("Bind address"),
		translate("The address ss-server will initiate connection from"))
	o.datatype = "ipaddr"
	o.placeholder = "0.0.0.0"
	ss.values_ipaddr(o)
else
	ss.options_client(s, "general")
	if stype == "ss_tunnel" then
		o = s:taboption("general", Value, "tunnel_address",
			translate("Tunnel address"),
			translate("The address ss-tunnel will forward traffic to"))
		o.datatype = "hostport"
	end
end

return m
