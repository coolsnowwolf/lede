-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2011 Manuel Munz <freifunk at somakoma dot de>
-- Licensed to the public under the Apache License 2.0.

local uci = require "luci.model.uci".cursor()

mh = Map("olsrd6", translate("OLSR - HNA6-Announcements"), translate("Hosts in a OLSR routed network can announce connecitivity " ..
	"to external networks using HNA6 messages."))

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
return mh

