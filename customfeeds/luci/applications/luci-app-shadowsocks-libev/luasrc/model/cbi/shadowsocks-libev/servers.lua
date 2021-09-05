-- Copyright 2017 Yousong Zhou <yszhou4tech@gmail.com>
-- Licensed to the public under the Apache License 2.0.

local ds = require "luci.dispatcher"
local ss = require("luci.model.shadowsocks-libev")

local m, s

m = Map("shadowsocks-libev",
	translate("Remote Servers"),
	translate("Definition of remote shadowsocks servers.  \
			Disable any of them will also disable instances refering to it."))

local sname = arg[1]
if sname then
	if not m:get(sname) then
		luci.http.redirect(ds.build_url("admin/services/shadowsocks-libev/servers"))
		return
	end
	s = m:section(NamedSection, sname, "server")
	m.title = m.title .. ' - ' .. sname
else
	s = m:section(TypedSection, "server")
	s.template = 'cbi/tblsection'
	s.addremove = true
end

s:option(Flag, "disabled", translate("Disable"))
ss.options_server(s)

return m
