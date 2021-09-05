-- Copyright 2014-2018 Christian Schoenebeck <christian dot schoenebeck at gmail dot com>
-- Licensed to the public under the Apache License 2.0.

local DISP = require "luci.dispatcher"
local HTTP = require "luci.http"
local SYS  = require "luci.sys"
local CTRL = require "luci.controller.ddns"	-- this application's controller
local DDNS = require "luci.tools.ddns"		-- ddns multiused functions

local show_hints = not (DDNS.env_info("has_ipv6")		-- IPv6 support
				   and  DDNS.env_info("has_ssl")		-- HTTPS support
				   and  DDNS.env_info("has_proxy")		-- Proxy support
				   and  DDNS.env_info("has_bindhost")	-- DNS TCP support
				   and  DDNS.env_info("has_forceip")	-- Force IP version
				   and  DDNS.env_info("has_dnsserver")	-- DNS server support
				   and  DDNS.env_info("has_bindnet")	-- Bind to network/interface
				   and  DDNS.env_info("has_cacerts")	-- certificates installed at /etc/ssl/certs
		)
local not_enabled = not SYS.init.enabled("ddns")
local need_update = not CTRL.service_ok()

-- html constants
font_red = [[<font color="red">]]
font_off = [[</font>]]
bold_on  = [[<strong>]]
bold_off = [[</strong>]]

-- cbi-map definition -- #######################################################
m = Map("ddns")
m.title		= CTRL.app_title_main()
m.description	= CTRL.app_description()

m.on_after_commit = function(self)
	if self.changed then	-- changes ?
		local command = CTRL.luci_helper
		if SYS.init.enabled("ddns") then	-- ddns service enabled, restart all
			command = command .. " -- restart"
			os.execute(command)
		else	-- ddns service disabled, send SIGHUP to running
			command = command .. " -- reload"
			os.execute(command)
		end
	end
end

-- SimpleSection definition -- ##################################################
-- with all the JavaScripts we need for "a good Show"
a = m:section( SimpleSection )
a.template = "ddns/overview_status"

-- SimpleSection definition -- #################################################
-- show Hints to optimize installation and script usage
if show_hints or need_update or not_enabled then

	s = m:section( SimpleSection, translate("Hints") )

	-- ddns-scripts needs to be updated for full functionality
	if need_update then
		local dv = s:option(DummyValue, "_update_needed")
		dv.titleref = DISP.build_url("admin", "system", "packages")
		dv.rawhtml  = true
		dv.title = font_red .. bold_on ..
			translate("Software update required") .. bold_off .. font_off
		dv.value = translate("The currently installed 'ddns-scripts' package did not support all available settings.") ..
				"<br />" ..
				translate("Please update to the current version!")
	end

	-- DDNS Service disabled
	if not_enabled then
		local dv = s:option(DummyValue, "_not_enabled")
		dv.titleref = DISP.build_url("admin", "system", "startup")
		dv.rawhtml  = true
		dv.title = bold_on ..
			translate("DDNS Autostart disabled") .. bold_off
		dv.value = translate("Currently DDNS updates are not started at boot or on interface events." .. "<br />" ..
				"You can start/stop each configuration here. It will run until next reboot.")
	end

	-- Show more hints on a separate page
	if show_hints then
		local dv = s:option(DummyValue, "_separate")
		dv.titleref = DISP.build_url("admin", "services", "ddns", "hints")
		dv.rawhtml  = true
		dv.title = bold_on ..
			translate("Show more") .. bold_off
		dv.value = translate("Follow this link" .. "<br />" ..
				"You will find more hints to optimize your system to run DDNS scripts with all options")
	end
end

-- TableSection definition -- ##################################################
ts = m:section( TypedSection, "service",
	translate("Overview"),
	translate("Below is a list of configured DDNS configurations and their current state.")
	.. "<br />"
	.. translate("If you want to send updates for IPv4 and IPv6 you need to define two separate Configurations "
		.. "i.e. 'myddns_ipv4' and 'myddns_ipv6'")
	.. "<br />"
	.. [[<a href="]] .. DISP.build_url("admin", "services", "ddns", "global") .. [[">]]
	.. translate("To change global settings click here") .. [[</a>]] )
ts.sectionhead = translate("Configuration")
ts.template = "cbi/tblsection"
ts.addremove = true
ts.extedit = DISP.build_url("admin", "services", "ddns", "detail", "%s")
function ts.create(self, name)
	AbstractSection.create(self, name)
	HTTP.redirect( self.extedit:format(name) )
end

-- Lookup_Host and registered IP -- #################################################
dom = ts:option(DummyValue, "_lookupIP",
	translate("Lookup Hostname") .. "<br />" .. translate("Registered IP") )
dom.template = "ddns/overview_doubleline"
function dom.set_one(self, section)
	local lookup = self.map:get(section, "lookup_host") or ""
	if lookup ~= "" then
		return lookup
	else
		return [[<em>]] .. translate("config error") .. [[</em>]]
	end
end
function dom.set_two(self, section)
	local chk_sec  = DDNS.calc_seconds(
				tonumber(self.map:get(section, "check_interval") or "") or 10,
				self.map:get(section, "check_unit") or "minutes" )
	local ip = DDNS.get_regip(section, chk_sec)
	if ip == "NOFILE" then
		local lookup_host = self.map:get(section, "lookup_host") or ""
		if lookup_host == "" then return "" end
		local dnsserver = self.map:get(section, "dnsserver") or ""
		local use_ipv6 = tonumber(self.map:get(section, "use_ipv6") or 0)
		local force_ipversion = tonumber(self.map:get(section, "force_ipversion") or 0)
		local force_dnstcp = tonumber(self.map:get(section, "force_dnstcp") or 0)
		local is_glue = tonumber(self.map:get(section, "is_glue") or 0)
		local command = CTRL.luci_helper .. [[ -]]
		if (use_ipv6 == 1) then command = command .. [[6]] end
		if (force_ipversion == 1) then command = command .. [[f]] end
		if (force_dnstcp == 1) then command = command .. [[t]] end
		if (is_glue == 1) then command = command .. [[g]] end
		command = command .. [[l ]] .. lookup_host
		command = command .. [[ -S ]] .. section
		if (#dnsserver > 0) then command = command .. [[ -d ]] .. dnsserver end
		command = command .. [[ -- get_registered_ip]]
		ip = SYS.exec(command)
	end
	if ip == "" then ip = translate("no data") end
	return ip
end

-- enabled
ena = ts:option( Flag, "enabled",
	translate("Enabled"))
ena.template = "ddns/overview_enabled"
ena.rmempty = false

-- show PID and next update
upd = ts:option( DummyValue, "_update",
	translate("Last Update") .. "<br />" .. translate("Next Update"))
upd.template = "ddns/overview_doubleline"
function upd.set_one(self, section)	-- fill Last Update
	-- get/validate last update
	local uptime   = SYS.uptime()
	local lasttime = DDNS.get_lastupd(section)
	if lasttime > uptime then 	-- /var might not be linked to /tmp and cleared on reboot
		lasttime = 0
	end

	-- no last update happen
	if lasttime == 0 then
		return translate("never")

	-- we read last update
	else
		-- calc last update
		--            os.epoch  - sys.uptime + lastupdate(uptime)
		local epoch = os.time() - uptime + lasttime
		-- use linux date to convert epoch
		return DDNS.epoch2date(epoch)
	end
end
function upd.set_two(self, section)	-- fill Next Update
	-- get enabled state
	local enabled	= tonumber(self.map:get(section, "enabled") or 0)
	local datenext	= translate("unknown error")	-- formatted date of next update

	-- get force seconds
	local force_interval = tonumber(self.map:get(section, "force_interval") or 72)
	local force_unit = self.map:get(section, "force_unit") or "hours"
	local force_seconds = DDNS.calc_seconds(force_interval, force_unit)

	-- get last update and get/validate PID
	local uptime   = SYS.uptime()
	local lasttime = DDNS.get_lastupd(section)
	if lasttime > uptime then 	-- /var might not be linked to /tmp and cleared on reboot
		lasttime = 0
	end
	local pid      = DDNS.get_pid(section)

	-- calc next update
	if lasttime > 0 then
		local epoch = os.time() - uptime + lasttime + force_seconds
		-- use linux date to convert epoch
		datelast = DDNS.epoch2date(epoch)
	end

	-- process running but update needs to happen
	if pid > 0 and ( lasttime + force_seconds - uptime ) < 0 then
		datenext = translate("Verify")

	-- run once
	elseif force_seconds == 0 then
		datenext = translate("Run once")

	-- no process running and NOT enabled
	elseif pid == 0 and enabled == 0 then
		datenext  = translate("Disabled")

	-- no process running and NOT
	elseif pid == 0 and enabled ~= 0 then
		datenext = translate("Stopped")
	end

	return datenext
end

-- start/stop button
btn = ts:option( Button, "_startstop",
	translate("Process ID") .. "<br />" .. translate("Start / Stop") )
btn.template = "ddns/overview_startstop"
function btn.cfgvalue(self, section)
	local pid = DDNS.get_pid(section)
	if pid > 0 then
		btn.inputtitle	= "PID: " .. pid
		btn.inputstyle	= "reset"
		btn.disabled	= false
	elseif (self.map:get(section, "enabled") or "0") ~= "0" then
		btn.inputtitle	= translate("Start")
		btn.inputstyle	= "apply"
		btn.disabled	= false
	else
		btn.inputtitle	= "----------"
		btn.inputstyle	= "button"
		btn.disabled	= true
	end
	return true
end

return m
