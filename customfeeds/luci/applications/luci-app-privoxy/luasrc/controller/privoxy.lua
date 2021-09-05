-- Copyright 2014-2016 Christian Schoenebeck <christian dot schoenebeck at gmail dot com>
-- Licensed under the Apache License, Version 2.0

module("luci.controller.privoxy", package.seeall)

local NX   = require "nixio"
local NXFS = require "nixio.fs"
local DISP = require "luci.dispatcher"
local HTTP = require "luci.http"
local I18N = require "luci.i18n" 	-- not globally avalible here
local IPKG = require "luci.model.ipkg"
local UCI  = require "luci.model.uci"
local UTIL = require "luci.util"
local SYS  = require "luci.sys"

local srv_name    = "privoxy"
local srv_ver_min = "3.0.23"			-- minimum version of service required
local srv_ver_cmd = [[/usr/sbin/privoxy --version | awk {'print $3'}]]
local app_name    = "luci-app-privoxy"
local app_title   = "Privoxy WEB proxy"
local app_version = "1.0.6-1"

function index()
	entry( {"admin", "services", "privoxy"}, cbi("privoxy"), _("Privoxy WEB proxy"), 59)
	entry( {"admin", "services", "privoxy", "logview"}, call("logread") ).leaf = true
	entry( {"admin", "services", "privoxy", "startstop"}, post("startstop") ).leaf = true
	entry( {"admin", "services", "privoxy", "status"}, call("get_pid") ).leaf = true
end

-- Application specific information functions
function app_description()
	return	I18N.translate("Privoxy is a non-caching web proxy with advanced filtering "
			.. "capabilities for enhancing privacy, modifying web page data and HTTP headers, "
			.. "controlling access, and removing ads and other obnoxious Internet junk.")
		.. [[<br /><strong>]]
		.. I18N.translate("For help use link at the relevant option")
		.. [[</strong>]]
end

-- Standardized application/service functions
function app_title_main()
	return	[[<a href="javascript:alert(']]
			.. I18N.translate("Version Information")
			.. [[\n\n]] .. app_name
			.. [[\n\t]] .. I18N.translate("Version") .. [[:\t]] .. app_version
			.. [[\n\n]] .. srv_name .. [[ ]] .. I18N.translate("required") .. [[:]]
			.. [[\n\t]] .. I18N.translate("Version") .. [[:\t]]
				.. srv_ver_min .. [[ ]] .. I18N.translate("or higher")
			.. [[\n\n]] .. srv_name .. [[ ]] .. I18N.translate("installed") .. [[:]]
			.. [[\n\t]] .. I18N.translate("Version") .. [[:\t]]
				.. (service_version() or I18N.translate("NOT installed"))
			.. [[\n\n]]
	 	.. [[')">]]
		.. I18N.translate(app_title)
		.. [[</a>]]
end
function service_version()
	local ver = nil
	IPKG.list_installed(srv_name, function(n, v, d)
			if v and (#v > 0) then ver = v end
		end
	)
	if not ver or (#ver == 0) then
		ver = UTIL.exec(srv_ver_cmd)
		if #ver == 0 then ver = nil end
	end
	return	ver
end
function service_ok()
	return	IPKG.compare_versions((service_version() or "0"), ">=", srv_ver_min)
end
function service_update()
	local url = DISP.build_url("admin", "system", "packages")
	if not service_version() then
		return	[[<h3><strong><br /><font color="red">&nbsp;&nbsp;&nbsp;&nbsp;]]
			.. I18N.translate("Software package '%s' is not installed." % srv_name)
			.. [[</font><br /><br />&nbsp;&nbsp;&nbsp;&nbsp;]]
			.. I18N.translate("required") .. [[: ]] .. srv_name .. [[ ]] .. srv_ver_min .. " " .. I18N.translate("or higher")
			.. [[<br /><br />&nbsp;&nbsp;&nbsp;&nbsp;]]
			.. [[<a href="]] .. url ..[[">]]
			.. I18N.translate("Please install current version !")
			.. [[</a><br />&nbsp;</strong></h3>]]
	else
		return	[[<h3><strong><br /><br /><font color="red">&nbsp;&nbsp;&nbsp;&nbsp;]]
			.. I18N.translate("Software package '%s' is outdated." % srv_name)
			.. [[</font><br /><br />&nbsp;&nbsp;&nbsp;&nbsp;]]
			.. I18N.translate("installed") .. ": " .. service_version()
			.. [[<br /><br />&nbsp;&nbsp;&nbsp;&nbsp;]]
			.. I18N.translate("required") .. ": " .. srv_ver_min .. " " .. I18N.translate("or higher")
			.. [[<br /><br />&nbsp;&nbsp;&nbsp;&nbsp;]]
			.. [[<a href="]] .. url ..[[">]]
			.. I18N.translate("Please update to the current version!")
			.. [[</a><br /><br />&nbsp;</strong></h3>]]
	end
end

-- called by XHR.get from detail_logview.htm
function logread()
	-- read application settings
	local uci     = UCI.cursor()
	local logdir  = uci:get("privoxy", "privoxy", "logdir") or "/var/log"
	local logfile = uci:get("privoxy", "privoxy", "logfile") or "privoxy.log"
	uci:unload("privoxy")

	local ldata=NXFS.readfile(logdir .. "/" .. logfile)
	if not ldata or #ldata == 0 then
		ldata="_nodata_"
	end
	HTTP.write(ldata)
end

-- called by XHR.get from detail_startstop.htm
function startstop()
	local pid = get_pid(true)
	if pid > 0 then
		SYS.call("/etc/init.d/privoxy stop")
		NX.nanosleep(1)		-- sleep a second
		if NX.kill(pid, 0) then	-- still running
			NX.kill(pid, 9)	-- send SIGKILL
		end
		pid = 0
	else
		SYS.call("/etc/init.d/privoxy start")
		NX.nanosleep(1)		-- sleep a second
		pid = tonumber(NXFS.readfile("/var/run/privoxy.pid") or 0 )
		if pid > 0 and not NX.kill(pid, 0) then
			pid = 0		-- process did not start
		end
	end
	HTTP.write(tostring(pid))	-- HTTP needs string not number
end

-- called by XHR.poll from detail_startstop.htm
-- and from lua (with parameter "true")
function get_pid(from_lua)
	local pid = tonumber(NXFS.readfile("/var/run/privoxy.pid") or 0 )
	if pid > 0 and not NX.kill(pid, 0) then
		pid = 0
	end
	if from_lua then
		return pid
	else
		HTTP.write(tostring(pid))	-- HTTP needs string not number
	end
end

-- replacement of build-in parse of UCI option
-- modified AbstractValue.parse(self, section, novld) from cbi.lua
-- validate is called if rmempty/optional true or false
-- write is called if rmempty/optional true or false
function value_parse(self, section, novld)
	local fvalue = self:formvalue(section)
	local fexist = ( fvalue and (#fvalue > 0) )	-- not "nil" and "not empty"
	local cvalue = self:cfgvalue(section)
	local rm_opt = ( self.rmempty or self.optional )
	local eq_cfg					-- flag: equal cfgvalue

	-- If favlue and cvalue are both tables and have the same content
	-- make them identical
	if type(fvalue) == "table" and type(cvalue) == "table" then
		eq_cfg = (#fvalue == #cvalue)
		if eq_cfg then
			for i=1, #fvalue do
				if cvalue[i] ~= fvalue[i] then
					eq_cfg = false
				end
			end
		end
		if eq_cfg then
			fvalue = cvalue
		end
	end

	-- removed parameter "section" from function call because used/accepted nowhere
	-- also removed call to function "transfer"
	local vvalue, errtxt = self:validate(fvalue)

	-- error handling; validate return "nil"
	if not vvalue then
		if novld then 		-- and "novld" set
			return		-- then exit without raising an error
		end

		if fexist then		-- and there is a formvalue
			self:add_error(section, "invalid", errtxt or self.title .. ": invalid")
			return		-- so data are invalid
		elseif not rm_opt then	-- and empty formvalue but NOT (rmempty or optional) set
			self:add_error(section, "missing", errtxt or self.title .. ": missing")
			return		-- so data is missing
		elseif errtxt then
			self:add_error(section, "invalid", errtxt)
			return
		end
--		error  ("\n option: " .. self.option ..
--			"\n fvalue: " .. tostring(fvalue) ..
--			"\n fexist: " .. tostring(fexist) ..
--			"\n cvalue: " .. tostring(cvalue) ..
--			"\n vvalue: " .. tostring(vvalue) ..
--			"\n vexist: " .. tostring(vexist) ..
--			"\n rm_opt: " .. tostring(rm_opt) ..
--			"\n eq_cfg: " .. tostring(eq_cfg) ..
--			"\n eq_def: " .. tostring(eq_def) ..
--			"\n novld : " .. tostring(novld) ..
--			"\n errtxt: " .. tostring(errtxt) )
	end

	-- lets continue with value returned from validate
	eq_cfg  = ( vvalue == cvalue )					-- update equal_config flag
	local vexist = ( vvalue and (#vvalue > 0) ) and true or false	-- not "nil" and "not empty"
	local eq_def = ( vvalue == self.default )			-- equal_default flag

	-- (rmempty or optional) and (no data or equal_default)
	if rm_opt and (not vexist or eq_def) then
		if self:remove(section) then		-- remove data from UCI
			self.section.changed = true	-- and push events
		end
		return
	end

	-- not forcewrite and no changes, so nothing to write
	if not self.forcewrite and eq_cfg then
		return
	end

	-- we should have a valid value here
	assert (vvalue, "\n option: " .. self.option ..
			"\n fvalue: " .. tostring(fvalue) ..
			"\n fexist: " .. tostring(fexist) ..
			"\n cvalue: " .. tostring(cvalue) ..
			"\n vvalue: " .. tostring(vvalue) ..
			"\n vexist: " .. tostring(vexist) ..
			"\n rm_opt: " .. tostring(rm_opt) ..
			"\n eq_cfg: " .. tostring(eq_cfg) ..
			"\n eq_def: " .. tostring(eq_def) ..
			"\n errtxt: " .. tostring(errtxt) )

	-- write data to UCI; raise event only on changes
	if self:write(section, vvalue) and not eq_cfg then
		self.section.changed = true
	end
end
