-- Copyright (C) 2014-2018 OpenWrt-dist
-- Copyright (C) 2014-2018 Jian Chang <aa65535@live.com>
-- Licensed to the public under the GNU General Public License v3.

module("luci.controller.redsocks2", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/redsocks2") then
		return
	end

	entry({"admin", "services", "redsocks2"}, cbi("redsocks2"), _("RedSocks2"), 72).dependent = true
end
