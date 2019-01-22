-- Copyright (C) 2016-2018 OpenWrt-dist
-- Copyright (C) 2016-2018 Jian Chang <aa65535@live.com>
-- Licensed to the public under the GNU General Public License v3.

module("luci.controller.dns-forwarder", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/dns-forwarder") then
		return
	end

	entry({"admin", "services", "dns-forwarder"}, cbi("dns-forwarder"), _("DNS-Forwarder"), 60).dependent = true
end
