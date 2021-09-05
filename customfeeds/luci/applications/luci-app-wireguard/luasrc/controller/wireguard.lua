-- Copyright 2016-2017 Dan Luedtke <mail@danrl.com>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.wireguard", package.seeall)

function index()
  entry({"admin", "status", "wireguard"}, template("wireguard"), _("WireGuard Status"), 92)
end
