-- Copyright 2017-2018 Dirk Brenken (dev@brenken.org)
-- This is free software, licensed under the Apache License, Version 2.0

module("luci.controller.travelmate", package.seeall)

local util  = require("luci.util")
local i18n  = require("luci.i18n")
local templ = require("luci.template")

function index()
	if not nixio.fs.access("/etc/config/travelmate") then
		return
	end
	entry({"admin", "services", "travelmate"}, firstchild(), _("Travelmate"), 40).dependent = false
	entry({"admin", "services", "travelmate", "tab_from_cbi"}, cbi("travelmate/overview_tab", {hideresetbtn=true, hidesavebtn=true}), _("Overview"), 10).leaf = true
	entry({"admin", "services", "travelmate", "stations"}, template("travelmate/stations"), _("Wireless Stations"), 20).leaf = true
	entry({"admin", "services", "travelmate", "logfile"}, call("logread"), _("View Logfile"), 30).leaf = true
	entry({"admin", "services", "travelmate", "advanced"}, firstchild(), _("Advanced"), 100)
	entry({"admin", "services", "travelmate", "advanced", "configuration"}, form("travelmate/configuration_tab"), _("Edit Travelmate Configuration"), 110).leaf = true
	entry({"admin", "services", "travelmate", "advanced", "cfg_wireless"}, form("travelmate/cfg_wireless_tab"), _("Edit Wireless Configuration"), 120).leaf = true
	entry({"admin", "services", "travelmate", "advanced", "cfg_network"}, form("travelmate/cfg_network_tab"), _("Edit Network Configuration"), 130).leaf = true
	entry({"admin", "services", "travelmate", "advanced", "cfg_firewall"}, form("travelmate/cfg_firewall_tab"), _("Edit Firewall Configuration"), 140).leaf = true

	entry({"admin", "services", "travelmate", "apqr"}, template("travelmate/ap_qr")).leaf = true
	entry({"admin", "services", "travelmate", "wifiscan"}, template("travelmate/wifi_scan")).leaf = true
	entry({"admin", "services", "travelmate", "wifiadd"}, form("travelmate/wifi_add", {hideresetbtn=true, hidesavebtn=true})).leaf = true
	entry({"admin", "services", "travelmate", "wifiedit"}, form("travelmate/wifi_edit", {hideresetbtn=true, hidesavebtn=true})).leaf = true
	entry({"admin", "services", "travelmate", "wifidelete"}, form("travelmate/wifi_delete", {hideresetbtn=true, hidesavebtn=true})).leaf = true
	entry({"admin", "services", "travelmate", "wifiorder"}, form("travelmate/wifi_order", {hideresetbtn=true, hidesavebtn=true})).leaf = true
end

function logread()
	local logfile = ""

	if nixio.fs.access("/var/log/messages") then
		logfile = util.trim(util.exec("grep -F 'travelmate-' /var/log/messages"))
	elseif nixio.fs.access("/sbin/logread") then
		logfile = util.trim(util.exec("logread -e 'travelmate-'"))
	end
	templ.render("travelmate/logread", {title = i18n.translate("Travelmate Logfile"), content = logfile})
end
