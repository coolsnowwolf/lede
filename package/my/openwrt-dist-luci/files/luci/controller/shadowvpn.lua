-- Copyright (C) 2014-2018 OpenWrt-dist
-- Copyright (C) 2014-2018 Jian Chang <aa65535@live.com>
-- Licensed to the public under the GNU General Public License v3.

module("luci.controller.shadowvpn", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/shadowvpn") then
		return
	end

	entry({"admin", "services", "shadowvpn"}, cbi("shadowvpn"), _("ShadowVPN"), 76).dependent = true
end
