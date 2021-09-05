-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2011 Manuel Munz <freifunk at somakoma dot de>
-- Licensed to the public under the Apache License 2.0.

local uci = require "luci.model.uci".cursor()
local ipv =  uci:get_first("olsrd", "olsrd", "IpVersion", "4")

mh = Map("olsrd", translate("OLSR - HNA-Announcements"), translate("Hosts in a OLSR routed network can announce connecitivity " ..
	"to external networks using HNA messages."))

if ipv == "6and4" or ipv == "4" then
	hna4 = mh:section(TypedSection, "Hna4", translate("Hna4"), translate("Both values must use the dotted decimal notation."))
	hna4.addremove = true
	hna4.anonymous = true
	hna4.template  = "cbi/tblsection"

	net4 = hna4:option(Value, "netaddr", translate("Network address"))
	net4.datatype = "ip4addr"
	net4.placeholder = "10.11.12.13"
	net4.default = "10.11.12.13"
	msk4 = hna4:option(Value, "netmask", translate("Netmask"))
	msk4.datatype = "ip4addr"
	msk4.placeholder = "255.255.255.255"
	msk4.default = "255.255.255.255"
end

if ipv == "6and4" or ipv == "6" then
	hna6 = mh:section(TypedSection, "Hna6", translate("Hna6"), translate("IPv6 network must be given in full notation, " ..
		"prefix must be in CIDR notation."))
	hna6.addremove = true
	hna6.anonymous = true
	hna6.template  = "cbi/tblsection"

	net6 = hna6:option(Value, "netaddr", translate("Network address"))
	net6.datatype = "ip6addr"
	net6.placeholder = "fec0:2200:106:0:0:0:0:0"
	net6.default = "fec0:2200:106:0:0:0:0:0"
	msk6 = hna6:option(Value, "prefix", translate("Prefix"))
	msk6.datatype = "range(0,128)"
	msk6.placeholder = "128"
	msk6.default = "128"
end

return mh

