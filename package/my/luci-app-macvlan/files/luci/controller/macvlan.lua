-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.macvlan", package.seeall)

function index()
	local page

	page = entry({"admin", "services", "macvlan"}, cbi("macvlan/macvlan"), _("Macvlan"))
	page.leaf = true
end
