-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.openvpn", package.seeall)

function index()
	entry( {"admin", "services", "openvpn"}, cbi("openvpn"), _("OpenVPN") )
	entry( {"admin", "services", "openvpn", "basic"},    cbi("openvpn-basic"),    nil ).leaf = true
	entry( {"admin", "services", "openvpn", "advanced"}, cbi("openvpn-advanced"), nil ).leaf = true
end
