-- Copyright 2017 Dirk Brenken (dev@brenken.org)
-- This is free software, licensed under the Apache License, Version 2.0

module("luci.controller.dnscrypt-proxy", package.seeall)

local util  = require("luci.util")
local i18n  = require("luci.i18n")
local templ = require("luci.template")

function index()
	if not nixio.fs.access("/etc/config/dnscrypt-proxy") then
		nixio.fs.writefile("/etc/config/dnscrypt-proxy", "")
	end
	entry({"admin", "services", "dnscrypt-proxy"}, firstchild(), _("DNSCrypt-Proxy"), 60).dependent = false
	entry({"admin", "services", "dnscrypt-proxy", "tab_from_cbi"}, cbi("dnscrypt-proxy/overview_tab", {hideresetbtn=true, hidesavebtn=true}), _("Overview"), 10).leaf = true
	entry({"admin", "services", "dnscrypt-proxy", "logfile"}, call("logread"), _("View Logfile"), 20).leaf = true
	entry({"admin", "services", "dnscrypt-proxy", "advanced"}, firstchild(), _("Advanced"), 100)
	entry({"admin", "services", "dnscrypt-proxy", "advanced", "configuration"}, form("dnscrypt-proxy/configuration_tab"), _("Edit DNSCrypt-Proxy Configuration"), 110).leaf = true
	entry({"admin", "services", "dnscrypt-proxy", "advanced", "cfg_dnsmasq"}, form("dnscrypt-proxy/cfg_dnsmasq_tab"), _("Edit Dnsmasq Configuration"), 120).leaf = true
	entry({"admin", "services", "dnscrypt-proxy", "advanced", "cfg_resolvcrypt"}, form("dnscrypt-proxy/cfg_resolvcrypt_tab"), _("Edit Resolvcrypt Configuration"), 130).leaf = true
	entry({"admin", "services", "dnscrypt-proxy", "advanced", "view_reslist"}, call("view_reslist"), _("View Resolver List"), 140).leaf = true
end

function view_reslist()
	local reslist = util.trim(util.exec("cat /usr/share/dnscrypt-proxy/dnscrypt-resolvers.csv"))
	templ.render("dnscrypt-proxy/view_reslist", {title = i18n.translate("DNSCrypt-Proxy Resolver List"), content = reslist})
end

function logread()
	local logfile

	if nixio.fs.access("/var/log/messages") then
		logfile = util.trim(util.exec("cat /var/log/messages | grep 'dnscrypt-proxy'"))
	else
		logfile = util.trim(util.exec("logread -e 'dnscrypt-proxy'"))
	end
	templ.render("dnscrypt-proxy/logread", {title = i18n.translate("DNSCrypt-Proxy Logfile"), content = logfile})
end
