-- Copyright 2009 Daniel Dickinson
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.luci_diag", package.seeall)

function index()
	local e

	e = entry({"admin", "network", "diag_config"}, template("diag/network_config_index") , _("Configure Diagnostics"), 120)
	e.index = true
	e.dependent = true

	e = entry({"mini", "diag"}, template("diag/index"), _("Diagnostics"), 120)
	e.index = true
	e.dependent = true
end
