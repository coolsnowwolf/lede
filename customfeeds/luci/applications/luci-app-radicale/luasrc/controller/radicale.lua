-- Copyright 2014-2016 Christian Schoenebeck <christian dot schoenebeck at gmail dot com>
-- Licensed under the Apache License, Version 2.0

module("luci.controller.radicale", package.seeall)

local NX   = require("nixio")
local NXFS = require("nixio.fs")
local DISP = require("luci.dispatcher")
local HTTP = require("luci.http")
local I18N = require("luci.i18n")	-- not globally avalible here
local IPKG = require("luci.model.ipkg")
local UTIL = require("luci.util")
local SYS  = require("luci.sys")

local srv_name    = "radicale"
local srv_ver_min = "1.1"		-- minimum version of service required
local srv_ver_cmd = [[/usr/bin/radicale --version]]
local app_name    = "luci-app-radicale"
local app_title   = I18N.translate("Radicale CalDAV/CardDAV Server")
local app_version = "1.1.0-1"

function index()
	entry( {"admin", "services", "radicale"}, alias("admin", "services", "radicale", "edit"), _("CalDAV/CardDAV"), 58)
	entry( {"admin", "services", "radicale", "edit"}, cbi("radicale") ).leaf = true
	entry( {"admin", "services", "radicale", "logview"}, call("_logread") ).leaf = true
	entry( {"admin", "services", "radicale", "startstop"}, post("_startstop") ).leaf = true
	entry( {"admin", "services", "radicale", "status"}, call("_status") ).leaf = true
end

-- Application / Service specific information functions
function app_description()
	return	I18N.translate("The Radicale Project is a complete CalDAV (calendar) and CardDAV (contact) server solution.") .. [[<br />]]
	     .. I18N.translate("Calendars and address books are available for both local and remote access, possibly limited through authentication policies.") .. [[<br />]]
	     .. I18N.translate("They can be viewed and edited by calendar and contact clients on mobile phones or computers.")
end
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
function app_title_back()
	return	[[<a href="]]
			.. DISP.build_url("admin", "services", "radicale")
		.. [[">]]
		.. I18N.translate(app_title)
		.. [[</a>]]
end
function app_err_value()
	if not service_version() then
		return [[<h3><strong><br /><font color="red">&nbsp;&nbsp;&nbsp;&nbsp;]]
			.. I18N.translate("Software package '%s' is not installed." % srv_name)
			.. [[</font><br /><br />&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;]]
			.. I18N.translate("required") .. [[: ]] .. srv_name .. [[ ]] .. srv_ver_min
			.. [[<br /><br />&nbsp;&nbsp;&nbsp;&nbsp;]]
			.. [[<a href="]] .. DISP.build_url("admin", "system", "packages") ..[[">]]
			.. I18N.translate("Please install current version !")
			.. [[</a><br />&nbsp;</strong></h3>]]
	else
		return [[<h3><strong><br /><font color="red">&nbsp;&nbsp;&nbsp;&nbsp;]]
			.. I18N.translate("Software package '%s' is outdated." % srv_name)
			.. [[</font><br /><br />&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;]]
			.. I18N.translate("installed") .. [[: ]] .. srv_name .. [[ ]] .. service_version()
			.. [[<br />&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;]]
			.. I18N.translate("required") .. [[: ]] .. srv_name .. [[ ]] .. srv_ver_min
			.. [[<br /><br />&nbsp;&nbsp;&nbsp;&nbsp;]]
			.. [[<a href="]] .. DISP.build_url("admin", "system", "packages") ..[[">]]
			.. I18N.translate("Please update to current version !")
			.. [[</a><br />&nbsp;</strong></h3>]]
	end
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

-- called by XHR.get from detail_logview.htm
function _logread()
	-- read application settings
	local uci     = UCI.cursor()
	local logfile = uci:get("radicale", "radicale", "logfile") or "/var/log/radicale"
	uci:unload("radicale")

	local ldata=NXFS.readfile(logfile)
	if not ldata or #ldata == 0 then
		ldata="_nodata_"
	end
	HTTP.write(ldata)
end

-- called by XHR.get from detail_startstop.htm
function _startstop()
	local pid = get_pid()
	if pid > 0 then
		SYS.call("/etc/init.d/radicale stop")
		NX.nanosleep(1)		-- sleep a second
		if NX.kill(pid, 0) then	-- still running
			NX.kill(pid, 9)	-- send SIGKILL
		end
		pid = 0
	else
		SYS.call("/etc/init.d/radicale start")
		NX.nanosleep(1)		-- sleep a second
		pid = get_pid()
		if pid > 0 and not NX.kill(pid, 0) then
			pid = 0		-- process did not start
		end
	end
	HTTP.write(tostring(pid))	-- HTTP needs string not number
end

-- called by XHR.poll from detail_startstop.htm
function _status()
	local pid = get_pid()
	HTTP.write(tostring(pid))	-- HTTP needs string not number
end

--return pid of running process
function get_pid()
	return tonumber(SYS.exec([[ps | grep "[p]ython.*[r]adicale" 2>/dev/null | awk '{print $1}']])) or 0
end

-- replacement of build-in parse of "Value"
-- modified AbstractValue.parse(self, section, novld) from cbi.lua
-- validate is called if rmempty/optional true or false
-- before write check if forcewrite, value eq default, and more
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
			self:add_error(section, "invalid", errtxt)
			return		-- so data are invalid
		elseif not rm_opt then	-- and empty formvalue but NOT (rmempty or optional) set
			self:add_error(section, "missing", errtxt)
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
