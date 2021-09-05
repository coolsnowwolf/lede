-- Copyright 2008 Aleksandar Krsteski <alekrsteski@gmail.com>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.polipo", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/polipo") then
		return
	end

	entry({"admin", "services", "polipo"}, alias("admin", "services", "polipo", "config"), _("Polipo"))
	entry({"admin", "services", "polipo", "status"}, template("polipo_status"), _("Status"))
	entry({"admin", "services", "polipo", "config"}, cbi("polipo"), _("Configuration"))
end

