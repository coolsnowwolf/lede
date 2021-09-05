-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.mini.network", package.seeall)

function index()
	entry({"mini", "network"}, alias("mini", "network", "index"), _("Network"), 20).index = true
	entry({"mini", "network", "index"}, cbi("mini/network", {autoapply=true}), _("General"), 1)
	entry({"mini", "network", "wifi"}, cbi("mini/wifi", {autoapply=true}), _("Wireless"), 10)
	entry({"mini", "network", "dhcp"}, cbi("mini/dhcp", {autoapply=true}), _("DHCP"), 20)
end
