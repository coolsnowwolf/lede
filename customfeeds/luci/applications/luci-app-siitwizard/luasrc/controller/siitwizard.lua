-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

module "luci.controller.siitwizard"

function index()
	entry({"admin", "network", "siitwizard"}, form("siitwizard"), "SIIT 4over6 assistent", 99)
end
