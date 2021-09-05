-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Copyright 2013 Manuel Munz <freifunk at somakoma dot de>
-- Copyright 2014-2018 Christian Schoenebeck <christian dot schoenebeck at gmail dot com>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.ddns", package.seeall)

local NX   = require "nixio"
local NXFS = require "nixio.fs"
local DISP = require "luci.dispatcher"
local HTTP = require "luci.http"
local I18N = require "luci.i18n" 		-- not globally avalible here
local IPKG = require "luci.model.ipkg"
local SYS  = require "luci.sys"
local UCI  = require "luci.model.uci"
local UTIL = require "luci.util"
local DDNS = require "luci.tools.ddns"		-- ddns multiused functions

luci_helper = "/usr/lib/ddns/dynamic_dns_lucihelper.sh"

local srv_name    = "ddns-scripts"
local srv_ver_min = "2.7.7"			-- minimum version of service required
local app_name    = "luci-app-ddns"
local app_title   = "Dynamic DNS"
local app_version = "2.4.9-1"

local translate = I18N.translate

function index()
	local nxfs	= require "nixio.fs"		-- global definitions not available
	local sys	= require "luci.sys"		-- in function index()
	local muci	= require "luci.model.uci"

	-- no config create an empty one
	if not nxfs.access("/etc/config/ddns") then
		nxfs.writefile("/etc/config/ddns", "")
	end

	-- preset new option "lookup_host" if not already defined
	local uci = muci.cursor()
	local commit = false
	uci:foreach("ddns", "service", function (s)
		if not s["lookup_host"] and s["domain"] then
			uci:set("ddns", s[".name"], "lookup_host", s["domain"])
			commit = true
		end
	end)
	if commit then uci:commit("ddns") end
	uci:unload("ddns")

	entry( {"admin", "services", "ddns"}, cbi("ddns/overview"), _("Dynamic DNS"), 59)
	entry( {"admin", "services", "ddns", "detail"}, cbi("ddns/detail"), nil ).leaf = true
	entry( {"admin", "services", "ddns", "hints"}, cbi("ddns/hints",
		{hideapplybtn=true, hidesavebtn=true, hideresetbtn=true}), nil ).leaf = true
	entry( {"admin", "services", "ddns", "global"}, cbi("ddns/global"), nil ).leaf = true
	entry( {"admin", "services", "ddns", "logview"}, call("logread") ).leaf = true
	entry( {"admin", "services", "ddns", "startstop"}, post("startstop") ).leaf = true
	entry( {"admin", "services", "ddns", "status"}, call("status") ).leaf = true
end

-- Application specific information functions
function app_description()
	local tmp = {}
	tmp[#tmp+1] =	translate("Dynamic DNS allows that your router can be reached with \
								a fixed hostname while having a dynamically changing IP address.")
	tmp[#tmp+1] =	[[<br />]]
	tmp[#tmp+1] =	translate("OpenWrt Wiki") .. ": "
	tmp[#tmp+1] =	[[<a href="https://openwrt.org/docs/guide-user/services/ddns/client" target="_blank">]]
	tmp[#tmp+1] =	translate("DDNS Client Documentation")
	tmp[#tmp+1] =	[[</a>]]
	tmp[#tmp+1] =	" --- "
	tmp[#tmp+1] =	[[<a href="https://openwrt.org/docs/guide-user/base-system/ddns" target="_blank">]]
	tmp[#tmp+1] =	translate("DDNS Client Configuration")
	tmp[#tmp+1] =	[[</a>]]
	
	return table.concat(tmp)
end
function app_title_back()
	local tmp = {}
	tmp[#tmp+1] = 	[[<a href="]]
	tmp[#tmp+1] =	DISP.build_url("admin", "services", "ddns")
	tmp[#tmp+1] =	[[">]]
	tmp[#tmp+1] =	  translate(app_title)
	tmp[#tmp+1] = 	[[</a>]]
	return table.concat(tmp)
end

-- Standardized application/service functions
function app_title_main()
	local tmp = {}
	tmp[#tmp+1] = 	[[<a href="javascript:alert(']]
	tmp[#tmp+1] = 		 translate("Version Information")
	tmp[#tmp+1] = 		 [[\n\n]] .. app_name
	tmp[#tmp+1] = 		 [[\n]] .. translate("Version") .. [[: ]] .. app_version
	tmp[#tmp+1] = 		 [[\n\n]] .. srv_name .. [[ ]] .. translate("required") .. [[:]]
	tmp[#tmp+1] = 		 [[\n]] .. translate("Version") .. [[: ]]
	tmp[#tmp+1] = 			 srv_ver_min .. [[ ]] .. translate("or higher")
	tmp[#tmp+1] = 		 [[\n\n]] .. srv_name .. [[ ]] .. translate("installed") .. [[:]]
	tmp[#tmp+1] = 		 [[\n]] .. translate("Version") .. [[: ]]
	tmp[#tmp+1] = 			 (service_version() or translate("NOT installed"))
	tmp[#tmp+1] = 		 [[\n\n]]
	tmp[#tmp+1] = 	 [[')">]]
	tmp[#tmp+1] = 	 translate(app_title)
	tmp[#tmp+1] = 	 [[</a>]]
		
	return table.concat(tmp)
end

function service_version()
	
	local srv_ver_cmd = luci_helper .. " -V | awk {'print $2'} "
	local ver

	if IPKG then
		ver = IPKG.info(srv_name)[srv_name].Version
	else
		ver = UTIL.exec(srv_ver_cmd)
	end
	
	if ver and #ver > 0 then return ver or nil end
	
end

function service_ok()
	return	IPKG.compare_versions((service_version() or "0"), ">=", srv_ver_min)
end

-- internal function to read all sections status and return data array
local function _get_status()
	local uci	 = UCI.cursor()
	local service	 = SYS.init.enabled("ddns") and 1 or 0
	local url_start	 = DISP.build_url("admin", "system", "startup")
	local data	 = {}	-- Array to transfer data to javascript

	data[#data+1] 	= {
		enabled	   = service,		-- service enabled
		url_up	   = url_start,		-- link to enable DDS (System-Startup)
	}

	uci:foreach("ddns", "service", function (s)

		-- Get section we are looking at
		-- and enabled state
		local section	= s[".name"]
		local enabled	= tonumber(s["enabled"]) or 0
		local datelast	= "_empty_"	-- formatted date of last update
		local datenext	= "_empty_"	-- formatted date of next update
		local datenextstat = nil

		-- get force seconds
		local force_seconds = DDNS.calc_seconds(
				tonumber(s["force_interval"]) or 72 ,
				s["force_unit"] or "hours" )
		-- get/validate pid and last update
		local pid      = DDNS.get_pid(section)
		local uptime   = SYS.uptime()
		local lasttime = DDNS.get_lastupd(section)
		if lasttime > uptime then 	-- /var might not be linked to /tmp
			lasttime = 0 		-- and/or not cleared on reboot
		end

		-- no last update happen
		if lasttime == 0 then
			datelast = "_never_"

		-- we read last update
		else
			-- calc last update
			--             sys.epoch - sys uptime   + lastupdate(uptime)
			local epoch = os.time() - uptime + lasttime
			-- use linux date to convert epoch
			datelast = DDNS.epoch2date(epoch)
			-- calc and fill next update
			datenext = DDNS.epoch2date(epoch + force_seconds)
		end

		-- process running but update needs to happen
		-- problems if force_seconds > uptime
		force_seconds = (force_seconds > uptime) and uptime or force_seconds
		if pid > 0 and ( lasttime + force_seconds - uptime ) <= 0 then
			datenext = "_verify_"
			datenextstat = translate("Verify")

		-- run once
		elseif force_seconds == 0 then
			datenext = "_runonce_"
			datenextstat = translate("Run once")

		-- no process running and NOT enabled
		elseif pid == 0 and enabled == 0 then
			datenext  = "_disabled_"
			datenextstat = translate("Disabled")

		-- no process running and enabled
		elseif pid == 0 and enabled ~= 0 then
			datenext = "_stopped_"
			datenextstat = translate("Stopped")
		end

		-- get/set monitored interface and IP version
		local iface	= s["interface"] or "wan"
		local use_ipv6	= tonumber(s["use_ipv6"]) or 0
		local ipv = (use_ipv6 == 1) and "IPv6" or "IPv4"
		iface = ipv .. " / " .. iface

		-- try to get registered IP
		local lookup_host = s["lookup_host"] or "_nolookup_"

		local chk_sec  = DDNS.calc_seconds(
					tonumber(s["check_interval"]) or 10,
					s["check_unit"] or "minutes" )
		local reg_ip = DDNS.get_regip(section, chk_sec)
		
		if reg_ip == "NOFILE" then
			local dnsserver	= s["dns_server"] or ""
			local force_ipversion = tonumber(s["force_ipversion"] or 0)
			local force_dnstcp = tonumber(s["force_dnstcp"] or 0)
			local is_glue = tonumber(s["is_glue"] or 0)
			local command = luci_helper .. [[ -]]
			if (use_ipv6 == 1) then command = command .. [[6]] end
			if (force_ipversion == 1) then command = command .. [[f]] end
			if (force_dnstcp == 1) then command = command .. [[t]] end
			if (is_glue == 1) then command = command .. [[g]] end
			command = command .. [[l ]] .. lookup_host
			command = command .. [[ -S ]] .. section
			if (#dnsserver > 0) then command = command .. [[ -d ]] .. dnsserver end
			command = command .. [[ -- get_registered_ip]]
			reg_ip = SYS.exec(command)
		end

		-- fill transfer array
		data[#data+1]	= {
			section  = section,
			enabled  = enabled,
			iface    = iface,
			lookup   = lookup_host,
			reg_ip   = reg_ip,
			pid      = pid,
			datelast = datelast,
			datenext = datenext,
			datenextstat = datenextstat
		}
	end)

	uci:unload("ddns")
	return data
end

-- called by XHR.get from detail_logview.htm
function logread(section)
	-- read application settings
	local uci	= UCI.cursor()
	local ldir	= uci:get("ddns", "global", "ddns_logdir") or "/var/log/ddns"
	local lfile	= ldir .. "/" .. section .. ".log"
	local ldata	= NXFS.readfile(lfile)

	if not ldata or #ldata == 0 then
		ldata="_nodata_"
	end
	uci:unload("ddns")
	HTTP.write(ldata)
end

-- called by XHR.get from overview_status.htm
function startstop(section, enabled)
	local uci  = UCI.cursor()
	local pid  = DDNS.get_pid(section)
	local data = {}		-- Array to transfer data to javascript

	-- if process running we want to stop and return
	if pid > 0 then
		local tmp = NX.kill(pid, 15)	-- terminate
		NX.nanosleep(2)	-- 2 second "show time"
		-- status changed so return full status
		data = _get_status()
		HTTP.prepare_content("application/json")
		HTTP.write_json(data)
		return
	end

	-- read uncommitted changes
	-- we don't save and commit data from other section or other options
	-- only enabled will be done
	local exec	  = true
	local changed     = uci:changes("ddns")
	for k_config, v_section in pairs(changed) do
		-- security check because uci.changes only gets our config
		if k_config ~= "ddns" then
			exec = false
			break
		end
		for k_section, v_option in pairs(v_section) do
			-- check if only section of button was changed
			if k_section ~= section then
				exec = false
				break
			end
			for k_option, v_value in pairs(v_option) do
				-- check if only enabled was changed
				if k_option ~= "enabled" then
					exec = false
					break
				end
			end
		end
	end

	-- we can not execute because other
	-- uncommitted changes pending, so exit here
	if not exec then
		HTTP.write("_uncommitted_")
		return
	end

	-- save enable state
	uci:set("ddns", section, "enabled", ( (enabled == "true") and "1" or "0") )
	uci:save("ddns")
	uci:commit("ddns")
	uci:unload("ddns")

	-- start ddns-updater for section
	local command = "%s -S %s -- start" %{ luci_helper, UTIL.shellquote(section) }
	os.execute(command)
	NX.nanosleep(3)	-- 3 seconds "show time"

	-- status changed so return full status
	data = _get_status()
	HTTP.prepare_content("application/json")
	HTTP.write_json(data)
end

-- called by XHR.poll from overview_status.htm
function status()
	local data = _get_status()
	HTTP.prepare_content("application/json")
	HTTP.write_json(data)
end

