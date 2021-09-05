-- Copyright 2015-2016 Christian Schoenebeck <christian dot schoenebeck at gmail dot com>
-- Licensed under the Apache License, Version 2.0

local NXFS = require("nixio.fs")
local DISP = require("luci.dispatcher")
local DTYP = require("luci.cbi.datatypes")
local HTTP = require("luci.http")
local UTIL = require("luci.util")
local UCI  = require("luci.model.uci")
local SYS  = require("luci.sys")
local WADM = require("luci.tools.webadmin")
local CTRL = require("luci.controller.radicale")	-- this application's controller and multiused functions

-- #################################################################################################
-- Error handling if not installed or wrong version -- #########################
if not CTRL.service_ok() then
	local f		= SimpleForm("__sf")
	f.title		= CTRL.app_title_main()
	f.description	= CTRL.app_description()
	f.embedded	= true
	f.submit	= false
	f.reset		= false

	local s = f:section(SimpleSection)
	s.title = [[<font color="red">]] .. [[<strong>]]
		.. translate("Software update required")
		.. [[</strong>]] .. [[</font>]]

	local v   = s:option(DummyValue, "_dv")
	v.rawhtml = true
	v.value   = CTRL.app_err_value

	return f
end

-- #################################################################################################
-- Error handling if no config, create an empty one -- #########################
if not NXFS.access("/etc/config/radicale") then
	NXFS.writefile("/etc/config/radicale", "")
end

-- #################################################################################################
-- takeover arguments if any -- ################################################
-- then show/edit selected file
if arg[1] then
	local argument	= arg[1]
	local filename	= ""

	-- SimpleForm ------------------------------------------------
	local ft	= SimpleForm("_text")
	ft.title	= CTRL.app_title_back()
	ft.description	= CTRL.app_description()
	ft.redirect	= DISP.build_url("admin", "services", "radicale") .. "#cbi-radicale-" .. argument
	if argument == "logger" then
		ft.reset	= false
		ft.submit	= translate("Reload")
		local uci	= UCI.cursor()
		filename = uci:get("radicale", "logger", "file_path") or "/var/log/radicale"
		uci:unload("radicale")
		filename = filename .. "/radicale"
	elseif argument == "auth" then
		ft.submit	= translate("Save")
		filename = "/etc/radicale/users"
	elseif argument == "rights" then
		ft.submit	= translate("Save")
		filename = "/etc/radicale/rights"
	else
		error("Invalid argument given as section")
	end
	if argument ~= "logger" and not NXFS.access(filename) then
		NXFS.writefile(filename, "")
	end

	-- SimpleSection ---------------------------------------------
	local fs	= ft:section(SimpleSection)
	if argument == "logger" then
		fs.title	= translate("Log-file Viewer")
		fs.description	= translate("Please press [Reload] button below to reread the file.")
	elseif argument == "auth" then
		fs.title	= translate("Authentication")
		fs.description	=  translate("Place here the 'user:password' pairs for your users which should have access to Radicale.")
				.. [[<br /><strong>]]
				.. translate("Keep in mind to use the correct hashing algorithm !")
				.. [[</strong>]]
	else	-- rights
		fs.title	= translate("Rights")
		fs.description	=  translate("Authentication login is matched against the 'user' key, "
					.. "and collection's path is matched against the 'collection' key.") .. " "
				.. translate("You can use Python's ConfigParser interpolation values %(login)s and %(path)s.") .. " "
				.. translate("You can also get groups from the user regex in the collection with {0}, {1}, etc.")
				.. [[<br />]]
				.. translate("For example, for the 'user' key, '.+' means 'authenticated user'" .. " "
					.. "and '.*' means 'anybody' (including anonymous users).")
				.. [[<br />]]
				.. translate("Section names are only used for naming the rule.")
				.. [[<br />]]
				.. translate("Leading or ending slashes are trimmed from collection's path.")
	end

	-- TextValue -------------------------------------------------
	local tt	= fs:option(TextValue, "_textvalue")
	tt.rmempty	= true
	if argument == "logger" then
		tt.readonly	= true
		tt.rows		= 30
		function tt.write()
			HTTP.redirect(DISP.build_url("admin", "services", "radicale", "edit", argument))
		end
	else
		tt.rows		= 15
		function tt.write(self, section, value)
			if not value then value = "" end
			NXFS.writefile(filename, value:gsub("\r\n", "\n"))
			return true --HTTP.redirect(DISP.build_url("admin", "services", "radicale", "edit") .. "#cbi-radicale-" .. argument)
		end
	end

	function tt.cfgvalue()
		return NXFS.readfile(filename) or
			string.format(translate("File '%s' not found !"), filename)
	end

	return ft

end

-- cbi-map -- ##################################################################
local m		= Map("radicale")
m.title		= CTRL.app_title_main()
m.description	= CTRL.app_description()
m.template	= "radicale/tabmap_nsections"
m.tabbed	= true
function m.commit_handler(self)
	if self.changed then	-- changes ?
		os.execute("/etc/init.d/radicale reload &")	-- reload configuration
	end
end

-- cbi-section "System" -- #####################################################
local sys	= m:section( NamedSection, "system", "system" )
sys.title	= translate("System")
sys.description	= nil
function sys.cfgvalue(self, section)
	if not self.map:get(section) then	-- section might not exist
		self.map:set(section, nil, self.sectiontype)
	end
	return self.map:get(section)
end

-- start/stop button -----------------------------------------------------------
local btn	= sys:option(DummyValue, "_startstop")
btn.template	= "radicale/btn_startstop"
btn.inputstyle	= nil
btn.rmempty	= true
btn.title	= translate("Start / Stop")
btn.description	= translate("Start/Stop Radicale server")
function btn.cfgvalue(self, section)
	local pid = CTRL.get_pid(true)
	if pid > 0 then
		btn.inputtitle	= "PID: " .. pid
		btn.inputstyle	= "reset"
		btn.disabled	= false
	else
		btn.inputtitle	= translate("Start")
		btn.inputstyle	= "apply"
		btn.disabled	= false
	end
	return true
end

-- enabled ---------------------------------------------------------------------
local ena	= sys:option(Flag, "_enabled")
ena.title       = translate("Auto-start")
ena.description = translate("Enable/Disable auto-start of Radicale on system start-up and interface events")
ena.orientation = "horizontal"	-- put description under the checkbox
ena.rmempty	= false		-- force write() function
function ena.cfgvalue(self, section)
	return (SYS.init.enabled("radicale")) and self.enabled or self.disabled
end
function ena.write(self, section, value)
	if value == self.enabled then
		return SYS.init.enable("radicale")
	else
		return SYS.init.disable("radicale")
	end
end

-- boot_delay ------------------------------------------------------------------
local bd	= sys:option(Value, "boot_delay")
bd.title	= translate("Boot delay")
bd.description	= translate("Delay (in seconds) during system boot before Radicale start")
		.. [[<br />]]
		.. translate("During delay ifup-events are not monitored !")
bd.default	= "10"
function bd.parse(self, section, novld)
	CTRL.value_parse(self, section, novld)
end
function bd.validate(self, value)
	local val = tonumber(value)
	if not val then
		return nil, self.title .. ": " .. translate("Value is not a number")
	elseif val < 0 or val > 300 then
		return nil, self.title .. ": " .. translate("Value not between 0 and 300")
	end
	return value
end


-- cbi-section "Server" -- #####################################################
local srv	= m:section( NamedSection, "server", "setting" )
srv.title	= translate("Server")
srv.description	= nil
function srv.cfgvalue(self, section)
	if not self.map:get(section) then	-- section might not exist
		self.map:set(section, nil, self.sectiontype)
	end
	return self.map:get(section)
end

-- hosts -----------------------------------------------------------------------
local sh	= srv:option( DynamicList, "hosts" )
sh.title	= translate("Address:Port")
sh.description	= translate("'Hostname:Port' or 'IPv4:Port' or '[IPv6]:Port' Radicale should listen on")
		.. [[<br /><strong>]]
		.. translate("Port numbers below 1024 (Privileged ports) are not supported")
		.. [[</strong>]]
sh.placeholder	= "0.0.0.0:5232"
sh.rmempty	= true
function sh.parse(self, section, novld)
	CTRL.value_parse(self, section, novld)
end

-- realm -----------------------------------------------------------------------
local alm	= srv:option( Value, "realm" )
alm.title	= translate("Logon message")
alm.description	= translate("Message displayed in the client when a password is needed.")
alm.default	= "Radicale - Password Required"
function alm.parse(self, section, novld)
	CTRL.value_parse(self, section, novld)
end
function alm.validate(self, value)
	if value then
		return value
	else
		return self.default
	end
end

-- ssl -------------------------------------------------------------------------
local ssl	= srv:option( Flag, "ssl" )
ssl.title	= translate("Enable HTTPS")
ssl.description	= nil
function ssl.write(self, section, value)
	if value == "0" then					-- delete all if not https enabled
		self.map:del(section, "protocol")		-- protocol
		self.map:del(section, "certificate")		-- certificate
		self.map:del(section, "key")			-- private key
		self.map:del(section, "ciphers")		-- ciphers
		return self.map:del(section, self.option)
	else
		return self.map:set(section, self.option, value)
	end
end

-- protocol --------------------------------------------------------------------
local prt	= srv:option( ListValue, "protocol" )
prt.title	= translate("SSL Protocol")
prt.description	= translate("'AUTO' selects the highest protocol version that client and server support.")
prt.widget	= "select"
prt.default	= "PROTOCOL_SSLv23"
prt:depends	("ssl", "1")
prt:value	("PROTOCOL_SSLv23", translate("AUTO"))
prt:value	("PROTOCOL_SSLv2", "SSL v2")
prt:value	("PROTOCOL_SSLv3", "SSL v3")
prt:value	("PROTOCOL_TLSv1", "TLS v1")
prt:value	("PROTOCOL_TLSv1_1", "TLS v1.1")
prt:value	("PROTOCOL_TLSv1_2", "TLS v1.2")
function prt.parse(self, section, novld)
	CTRL.value_parse(self, section, novld)
end

-- certificate -----------------------------------------------------------------
local crt	= srv:option( Value, "certificate" )
crt.title	= translate("Certificate file")
crt.description	= translate("Full path and file name of certificate")
crt.placeholder	= "/etc/radicale/ssl/server.crt"
crt:depends	("ssl", "1")
function crt.parse(self, section, novld)
	CTRL.value_parse(self, section, novld)
end
function crt.validate(self, value)
	local _ssl = ssl:formvalue(srv.section) or "0"
	if _ssl == "0" then
		return ""	-- ignore if not https enabled
	end
	if value then		-- otherwise errors in datatype check
		if DTYP.file(value) then
			return value
		else
			return nil, self.title .. ": " .. translate("File not found !")
		end
	else
		return nil, self.title .. ": " .. translate("Path/File required !")
	end
end

-- key -------------------------------------------------------------------------
local key	= srv:option( Value, "key" )
key.title	= translate("Private key file")
key.description	= translate("Full path and file name of private key")
key.placeholder	= "/etc/radicale/ssl/server.key"
key:depends	("ssl", "1")
function key.parse(self, section, novld)
	CTRL.value_parse(self, section, novld)
end
function key.validate(self, value)
	local _ssl = ssl:formvalue(srv.section) or "0"
	if _ssl == "0" then
		return ""	-- ignore if not https enabled
	end
	if value then		-- otherwise errors in datatype check
		if DTYP.file(value) then
			return value
		else
			return nil, self.title .. ": " .. translate("File not found !")
		end
	else
		return nil, self.title .. ": " .. translate("Path/File required !")
	end
end

-- ciphers ---------------------------------------------------------------------
--local cip	= srv:option( Value, "ciphers" )
--cip.title	= translate("Ciphers")
--cip.description	= translate("OPTIONAL: See python's ssl module for available ciphers")
--cip.rmempty	= true
--cip:depends	("ssl", "1")

-- cbi-section "Authentication" -- #############################################
local aut	= m:section( NamedSection, "auth", "setting" )
aut.title	= translate("Authentication")
aut.description	= translate("Authentication method to allow access to Radicale server.")
function aut.cfgvalue(self, section)
	if not self.map:get(section) then	-- section might not exist
		self.map:set(section, nil, self.sectiontype)
	end
	return self.map:get(section)
end

-- type -----------------------------------------------------------------------
local aty	= aut:option( ListValue, "type" )
aty.title	= translate("Authentication method")
aty.description	= nil
aty.widget	= "select"
aty.default	= "None"
aty:value	("None", translate("None"))
aty:value	("htpasswd", translate("htpasswd file"))
--aty:value	("IMAP", "IMAP")			-- The IMAP authentication module relies on the imaplib module.
--aty:value	("LDAP", "LDAP")			-- The LDAP authentication module relies on the python-ldap module.
--aty:value	("PAM", "PAM")				-- The PAM authentication module relies on the python-pam module.
--aty:value	("courier", "courier")
--aty:value	("HTTP", "HTTP")			-- The HTTP authentication module relies on the requests module
--aty:value	("remote_user", "remote_user")
--aty:value	("custom", translate("custom"))
function aty.parse(self, section, novld)
	CTRL.value_parse(self, section, novld)
end
function aty.write(self, section, value)
	if value ~= "htpasswd" then
		self.map:del(section, "htpasswd_encryption")
	elseif value ~= "IMAP" then
		self.map:del(section, "imap_hostname")
		self.map:del(section, "imap_port")
		self.map:del(section, "imap_ssl")
	end
	if value ~= self.default then
		return self.map:set(section, self.option, value)
	else
		return self.map:del(section, self.option)
	end
end

-- htpasswd_encryption ---------------------------------------------------------
local hte	= aut:option( ListValue, "htpasswd_encryption" )
hte.title	= translate("Encryption method")
hte.description	= nil
hte.widget	= "select"
hte.default	= "crypt"
hte:depends	("type", "htpasswd")
hte:value	("crypt", translate("crypt"))
hte:value	("plain", translate("plain"))
hte:value	("sha1",  translate("SHA-1"))
hte:value	("ssha",  translate("salted SHA-1"))
function hte.parse(self, section, novld)
	CTRL.value_parse(self, section, novld)
end

-- htpasswd_file (dummy) -------------------------------------------------------
local htf	= aut:option( Value, "_htf" )
htf.title	= translate("htpasswd file")
htf.description	= [[<strong>]]
		.. translate("Read only!")
		.. [[</strong> ]]
		.. translate("Radicale uses '/etc/radicale/users' as htpasswd file.")
		.. [[<br /><a href="]]
		.. DISP.build_url("admin", "services", "radicale", "edit") .. [[/auth]]
		.. [[">]]
		.. translate("To edit the file follow this link!")
		.. [[</a>]]
htf.readonly	= true
htf:depends	("type", "htpasswd")
function htf.cfgvalue()
	return "/etc/radicale/users"
end

-- cbi-section "Rights" -- #####################################################
local rig	= m:section( NamedSection, "rights", "setting" )
rig.title	= translate("Rights")
rig.description	= translate("Control the access to data collections.")
function rig.cfgvalue(self, section)
	if not self.map:get(section) then	-- section might not exist
		self.map:set(section, nil, self.sectiontype)
	end
	return self.map:get(section)
end

-- type -----------------------------------------------------------------------
local rty	= rig:option( ListValue, "type" )
rty.title	= translate("Rights backend")
rty.description	= nil
rty.widget	= "select"
rty.default	= "None"
rty:value	("None", translate("Full access for everybody (including anonymous)"))
rty:value	("authenticated", translate("Full access for authenticated Users") )
rty:value	("owner_only", translate("Full access for Owner only") )
rty:value	("owner_write", translate("Owner allow write, authenticated users allow read") )
rty:value	("from_file", translate("Rights are based on a regexp-based file") )
--rty:value	("custom", "Custom handler")
function rty.parse(self, section, novld)
	CTRL.value_parse(self, section, novld)
end
function rty.write(self, section, value)
	if value ~= "custom" then
		self.map:del(section, "custom_handler")
	end
	if value ~= self.default then
		return self.map:set(section, self.option, value)
	else
		return self.map:del(section, self.option)
	end
end

-- from_file (dummy) -----------------------------------------------------------
local rtf	= rig:option( Value, "_rtf" )
rtf.title	= translate("RegExp file")
rtf.description	= [[<strong>]]
		.. translate("Read only!")
		.. [[</strong> ]]
		.. translate("Radicale uses '/etc/radicale/rights' as regexp-based file.")
		.. [[<br /><a href="]]
		.. DISP.build_url("admin", "services", "radicale", "edit") .. [[/rights]]
		.. [[">]]
		.. translate("To edit the file follow this link!")
		.. [[</a>]]
rtf.readonly	= true
rtf:depends	("type", "from_file")
function rtf.cfgvalue()
	return "/etc/radicale/rights"
end

-- cbi-section "Storage" -- ####################################################
local sto	= m:section( NamedSection, "storage", "setting" )
sto.title	= translate("Storage")
sto.description	= nil
function sto.cfgvalue(self, section)
	if not self.map:get(section) then	-- section might not exist
		self.map:set(section, nil, self.sectiontype)
	end
	return self.map:get(section)
end

-- type -----------------------------------------------------------------------
local sty	= sto:option( ListValue, "type" )
sty.title	= translate("Storage backend")
sty.description	= translate("WARNING: Only 'File-system' is documented and tested by Radicale development")
sty.widget	= "select"
sty.default	= "filesystem"
sty:value	("filesystem", translate("File-system"))
--sty:value	("multifilesystem", translate("") )
--sty:value	("database", translate("Database") )
--sty:value	("custom", translate("Custom") )
function sty.parse(self, section, novld)
	CTRL.value_parse(self, section, novld)
end
function sty.write(self, section, value)
	if value ~= "filesystem" then
		self.map:del(section, "filesystem_folder")
	end
	if value ~= self.default then
		return self.map:set(section, self.option, value)
	else
		return self.map:del(section, self.option)
	end
end

--filesystem_folder ------------------------------------------------------------
local sfi	= sto:option( Value, "filesystem_folder" )
sfi.title	= translate("Directory")
sfi.description	= nil
sfi.placeholder	= "/srv/radicale"
sfi:depends	("type", "filesystem")
function sfi.parse(self, section, novld)
	CTRL.value_parse(self, section, novld)
end
function sfi.validate(self, value)
	local _typ = sty:formvalue(sto.section) or ""
	if _typ ~= "filesystem" then
		return ""	-- ignore if not htpasswd
	end
	if value then		-- otherwise errors in datatype check
		if DTYP.directory(value) then
			return value
		else
			return nil, self.title .. ": " .. translate("Directory not exists/found !")
		end
	else
		return nil, self.title .. ": " .. translate("Directory required !")
	end
end

-- cbi-section "Logging" -- ####################################################
local log	= m:section( NamedSection, "logger", "logging" )
log.title	= translate("Logging")
log.description	= nil
function log.cfgvalue(self, section)
	if not self.map:get(section) then	-- section might not exist
		self.map:set(section, nil, self.sectiontype)
	end
	return self.map:get(section)
end

-- console_level ---------------------------------------------------------------
local lco	= log:option( ListValue, "console_level" )
lco.title	= translate("Console Log level")
lco.description	= nil
lco.widget	= "select"
lco.default	= "ERROR"
lco:value	("DEBUG", translate("Debug"))
lco:value	("INFO", translate("Info") )
lco:value	("WARNING", translate("Warning") )
lco:value	("ERROR", translate("Error") )
lco:value	("CRITICAL", translate("Critical") )
function lco.parse(self, section, novld)
	CTRL.value_parse(self, section, novld)
end
function lco.write(self, section, value)
	if value ~= self.default then
		return self.map:set(section, self.option, value)
	else
		return self.map:del(section, self.option)
	end
end

-- syslog_level ----------------------------------------------------------------
local lsl	= log:option( ListValue, "syslog_level" )
lsl.title	= translate("Syslog Log level")
lsl.description	= nil
lsl.widget	= "select"
lsl.default	= "WARNING"
lsl:value	("DEBUG", translate("Debug"))
lsl:value	("INFO", translate("Info") )
lsl:value	("WARNING", translate("Warning") )
lsl:value	("ERROR", translate("Error") )
lsl:value	("CRITICAL", translate("Critical") )
function lsl.parse(self, section, novld)
	CTRL.value_parse(self, section, novld)
end
function lsl.write(self, section, value)
	if value ~= self.default then
		return self.map:set(section, self.option, value)
	else
		return self.map:del(section, self.option)
	end
end

-- file_level ------------------------------------------------------------------
local lfi	= log:option( ListValue, "file_level" )
lfi.title	= translate("File Log level")
lfi.description	= nil
lfi.widget	= "select"
lfi.default	= "INFO"
lfi:value	("DEBUG", translate("Debug"))
lfi:value	("INFO", translate("Info") )
lfi:value	("WARNING", translate("Warning") )
lfi:value	("ERROR", translate("Error") )
lfi:value	("CRITICAL", translate("Critical") )
function lfi.parse(self, section, novld)
	CTRL.value_parse(self, section, novld)
end
function lfi.write(self, section, value)
	if value ~= self.default then
		return self.map:set(section, self.option, value)
	else
		return self.map:del(section, self.option)
	end
end

-- file_path -------------------------------------------------------------------
local lfp	= log:option( Value, "file_path" )
lfp.title	= translate("Log-file directory")
lfp.description	= translate("Directory where the rotating log-files are stored")
		.. [[<br /><a href="]]
		.. DISP.build_url("admin", "services", "radicale", "edit") .. [[/logger]]
		.. [[">]]
		.. translate("To view latest log file follow this link!")
		.. [[</a>]]
lfp.default	= "/var/log/radicale"
function lfp.parse(self, section, novld)
	CTRL.value_parse(self, section, novld)
end
function lfp.validate(self, value)
	if not value or (#value < 1) or (value:find("/") ~= 1) then
		return nil, self.title .. ": " .. translate("no valid path given!")
	end
	return value
end

-- file_maxbytes ---------------------------------------------------------------
local lmb	= log:option( Value, "file_maxbytes" )
lmb.title	= translate("Log-file size")
lmb.description	= translate("Maximum size of each rotation log-file.")
		.. [[<br /><strong>]]
		.. translate("Setting this parameter to '0' will disable rotation of log-file.")
		.. [[</strong>]]
lmb.default	= "8196"
function lmb.parse(self, section, novld)
	CTRL.value_parse(self, section, novld)
end
function lmb.validate(self, value)
	if value then		-- otherwise errors in datatype check
		if DTYP.uinteger(value) then
			return value
		else
			return nil, self.title .. ": " .. translate("Value is not an Integer >= 0 !")
		end
	else
		return nil, self.title .. ": " .. translate("Value required ! Integer >= 0 !")
	end
end

-- file_backupcount ------------------------------------------------------------
local lbc	= log:option( Value, "file_backupcount" )
lbc.title	= translate("Log-backup Count")
lbc.description	= translate("Number of backup files of log to create.")
		.. [[<br /><strong>]]
		.. translate("Setting this parameter to '0' will disable rotation of log-file.")
		.. [[</strong>]]
lbc.default	= "1"
function lbc.parse(self, section, novld)
	CTRL.value_parse(self, section, novld)
end
function lbc.validate(self, value)
	if value then		-- otherwise errors in datatype check
		if DTYP.uinteger(value) then
			return value
		else
			return nil, self.title .. ": " .. translate("Value is not an Integer >= 0 !")
		end
	else
		return nil, self.title .. ": " .. translate("Value required ! Integer >= 0 !")
	end
end

-- cbi-section "Encoding" -- ###################################################
local enc	= m:section( NamedSection, "encoding", "setting" )
enc.title	= translate("Encoding")
enc.description	= translate("Change here the encoding Radicale will use instead of 'UTF-8' "
		.. "for responses to the client and/or to store data inside collections.")
function enc.cfgvalue(self, section)
	if not self.map:get(section) then	-- section might not exist
		self.map:set(section, nil, self.sectiontype)
	end
	return self.map:get(section)
end

-- request ---------------------------------------------------------------------
local enr	= enc:option( Value, "request" )
enr.title	= translate("Response Encoding")
enr.description	= translate("Encoding for responding requests.")
enr.default	= "utf-8"
function enr.parse(self, section, novld)
	CTRL.value_parse(self, section, novld)
end

-- stock -----------------------------------------------------------------------
local ens	= enc:option( Value, "stock" )
ens.title	= translate("Storage Encoding")
ens.description	= translate("Encoding for storing local collections.")
ens.default	= "utf-8"
function ens.parse(self, section, novld)
	CTRL.value_parse(self, section, novld)
end

-- cbi-section "Headers" -- ####################################################
local hea	= m:section( NamedSection, "headers", "setting" )
hea.title	= translate("Additional HTTP headers")
hea.description = translate("Cross-origin resource sharing (CORS) is a mechanism that allows restricted resources (e.g. fonts, JavaScript, etc.) "
		.. "on a web page to be requested from another domain outside the domain from which the resource originated.")
function hea.cfgvalue(self, section)
	if not self.map:get(section) then	-- section might not exist
		self.map:set(section, nil, self.sectiontype)
	end
	return self.map:get(section)
end

-- Access_Control_Allow_Origin -------------------------------------------------
local heo	= hea:option( DynamicList, "Access_Control_Allow_Origin" )
heo.title	= translate("Access-Control-Allow-Origin")
heo.description	= nil
function heo.parse(self, section, novld)
	CTRL.value_parse(self, section, novld)
end

-- Access_Control_Allow_Methods ------------------------------------------------
local hem	= hea:option( DynamicList, "Access_Control_Allow_Methods" )
hem.title	= translate("Access-Control-Allow-Methods")
hem.description	= nil
function hem.parse(self, section, novld)
	CTRL.value_parse(self, section, novld)
end

-- Access_Control_Allow_Headers ------------------------------------------------
local heh	= hea:option( DynamicList, "Access_Control_Allow_Headers" )
heh.title	= translate("Access-Control-Allow-Headers")
heh.description	= nil
function heh.parse(self, section, novld)
	CTRL.value_parse(self, section, novld)
end

-- Access_Control_Expose_Headers -----------------------------------------------
local hee	= hea:option( DynamicList, "Access_Control_Expose_Headers" )
hee.title	= translate("Access-Control-Expose-Headers")
hee.description	= nil
function hee.parse(self, section, novld)
	CTRL.value_parse(self, section, novld)
end

return m
