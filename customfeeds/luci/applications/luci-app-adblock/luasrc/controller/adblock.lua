-- Copyright 2017-2018 Dirk Brenken (dev@brenken.org)
-- This is free software, licensed under the Apache License, Version 2.0

module("luci.controller.adblock", package.seeall)

local util  = require("luci.util")
local templ = require("luci.template")
local i18n  = require("luci.i18n")

function index()
	if not nixio.fs.access("/etc/config/adblock") then
		return
	end
	entry({"admin", "services", "adblock"}, firstchild(), _("Adblock"), 30).dependent = false
	entry({"admin", "services", "adblock", "tab_from_cbi"}, cbi("adblock/overview_tab", {hideresetbtn=true, hidesavebtn=true}), _("Overview"), 10).leaf = true
	entry({"admin", "services", "adblock", "logfile"}, call("logread"), _("View Logfile"), 20).leaf = true
	entry({"admin", "services", "adblock", "advanced"}, firstchild(), _("Advanced"), 100)
	entry({"admin", "services", "adblock", "advanced", "blacklist"}, form("adblock/blacklist_tab"), _("Edit Blacklist"), 110).leaf = true
	entry({"admin", "services", "adblock", "advanced", "whitelist"}, form("adblock/whitelist_tab"), _("Edit Whitelist"), 120).leaf = true
	entry({"admin", "services", "adblock", "advanced", "configuration"}, form("adblock/configuration_tab"), _("Edit Configuration"), 130).leaf = true
	entry({"admin", "services", "adblock", "advanced", "query"}, template("adblock/query"), _("Query domains"), 140).leaf = true
	entry({"admin", "services", "adblock", "advanced", "result"}, call("queryData"), nil, 150).leaf = true
end

function logread()
	local logfile

	if nixio.fs.access("/var/log/messages") then
		logfile = util.trim(util.exec("grep -F 'adblock-' /var/log/messages"))
	else
		logfile = util.trim(util.exec("logread -e 'adblock-'"))
	end
	templ.render("adblock/logread", {title = i18n.translate("Adblock Logfile"), content = logfile})
end

function queryData(domain)
	if domain then
		luci.http.prepare_content("text/plain")
		local cmd = "/etc/init.d/adblock query %s 2>&1"
		local util = io.popen(cmd % util.shellquote(domain))
		if util then
			while true do
				local line = util:read("*l")
				if not line then
					break
				end
				luci.http.write(line)
				luci.http.write("\n")
			end
			util:close()
		end
	end
end
