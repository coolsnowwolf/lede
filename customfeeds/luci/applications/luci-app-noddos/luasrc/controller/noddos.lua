-- Copyright 2017 Steven Hessing (steven.hessing@gmail.com)
-- This is free software, licensed under the GNU General Public License v3.
-- /usr/lib/lua/luci/controller/noddos.lua

module("luci.controller.noddos", package.seeall)
function index()
    entry({"admin", "status", "noddos"}, template("noddos/clients"), _("Noddos Clients"), 3)
    entry({"admin", "network", "noddos"}, cbi("noddos"), _("Noddos Client Tracking"), 55)
end

