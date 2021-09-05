-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2010-2015 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local ipc = require "luci.ip"

m = Map("dhcp", translate("Hostnames"))

s = m:section(TypedSection, "domain", translate("Host entries"))
s.addremove = true
s.anonymous = true
s.template = "cbi/tblsection"

hn = s:option(Value, "name", translate("Hostname"))
hn.datatype = "hostname"
hn.rmempty  = true

ip = s:option(Value, "ip", translate("IP address"))
ip.datatype = "ipaddr"
ip.rmempty  = true

ipc.neighbors({ }, function(n)
	if n.mac and n.dest and not n.dest:is6linklocal() then
		ip:value(n.dest:string(), "%s (%s)" %{ n.dest:string(), n.mac })
	end
end)

return m
