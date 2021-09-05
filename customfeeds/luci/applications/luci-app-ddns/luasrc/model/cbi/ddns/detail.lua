-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Copyright 2013 Manuel Munz <freifunk at somakoma dot de>
-- Copyright 2014-2016 Christian Schoenebeck <christian dot schoenebeck at gmail dot com>
-- Licensed to the public under the Apache License 2.0.

local NX   = require "nixio"
local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"
local UTIL = require "luci.util"
local HTTP = require "luci.http"
local DISP = require "luci.dispatcher"
local WADM = require "luci.tools.webadmin"
local DTYP = require "luci.cbi.datatypes"
local CTRL = require "luci.controller.ddns"	-- this application's controller
local DDNS = require "luci.tools.ddns"		-- ddns multiused functions

-- takeover arguments -- #######################################################
local section = arg[1]

-- html constants -- ###########################################################
local font_red	= "<font color='red'>"
local font_off	= "</font>"
local bold_on	= "<strong>"
local bold_off	= "</strong>"

-- error text constants -- #####################################################
local err_ipv6_plain = translate("IPv6 not supported") .. " - " ..
		translate("please select 'IPv4' address version")
local err_ipv6_basic = bold_on ..
			font_red ..
				translate("IPv6 not supported") ..
			font_off ..
			"<br />" .. translate("please select 'IPv4' address version") ..
		 bold_off
local err_ipv6_other = bold_on ..
			font_red ..
				translate("IPv6 not supported") ..
			font_off ..
			"<br />" .. translate("please select 'IPv4' address version in") .. " " ..
			[[<a href="]] ..
					DISP.build_url("admin", "services", "ddns", "detail", section) ..
					"?tab.dns." .. section .. "=basic" ..
				[[">]] ..
				translate("Basic Settings") ..
			[[</a>]] ..
		 bold_off

function err_tab_basic(self)
	return translate("Basic Settings") .. " - " .. self.title .. ": "
end
function err_tab_adv(self)
	return translate("Advanced Settings") .. " - " .. self.title .. ": "
end
function err_tab_timer(self)
	return translate("Timer Settings") .. " - " .. self.title .. ": "
end

-- read services/services_ipv6 files -- ########################################
local services4 = { }		-- IPv4 --
local fd4 = io.open("/etc/ddns/services", "r")
if fd4 then
	local ln, s, t
	repeat
		ln = fd4:read("*l")
		s  = ln and ln:match('^%s*".*')	-- only handle lines beginning with "
		s  = s  and  s:gsub('"','')	-- remove "
		t  = s  and UTIL.split(s,"(%s+)",nil,true)	-- split on whitespaces
		if t then services4[t[1]]=t[2] end
	until not ln
	fd4:close()
end

local services6 = { }		-- IPv6 --
local fd6 = io.open("/etc/ddns/services_ipv6", "r")
if fd6 then
	local ln, s, t
	repeat
		ln = fd6:read("*l")
		s  = ln and ln:match('^%s*".*')	-- only handle lines beginning with "
		s  = s  and  s:gsub('"','')	-- remove "
		t  = s  and UTIL.split(s,"(%s+)",nil,true)	-- split on whitespaces
		if t then services6[t[1]]=t[2] end
	until not ln
	fd6:close()
end

-- multi-used functions -- ####################################################
-- function to verify settings around ip_source
-- will use dynamic_dns_lucihelper to check if
-- local IP can be read
local function _verify_ip_source()
	-- section is globally defined here be calling argument (see above)
	local _arg

	local _ipv6   = usev6:formvalue(section)
	local _source = (_ipv6 == "1")
			and src6:formvalue(section)
			or  src4:formvalue(section)

	local command = CTRL.luci_helper .. [[ -]]
	if (_ipv6 == "1")  then command = command .. [[6]] end

	if _source == "network" then
		_arg = (_ipv6 == "1")
			and ipn6:formvalue(section)
			or  ipn4:formvalue(section)
		command = command .. [[n ]] .. _arg
	elseif _source == "web" then
		_arg = (_ipv6 == "1")
			and iurl6:formvalue(section)
			or  iurl4:formvalue(section)
		command = command .. [[u ]] .. _arg

		-- proxy only needed for checking url
		_arg = (pxy) and pxy:formvalue(section) or ""
		if (_arg and #_arg > 0) then
			command = command .. [[ -p ]] .. _arg
		end
	elseif _source == "interface" then
		command = command .. [[i ]] .. ipi:formvalue(section)
	elseif _source == "script" then
		command = command .. [[s ]] .. ips:formvalue(section)
	end
	command = command .. [[ -- get_local_ip]]
	return (SYS.call(command) == 0)
end

-- function to check if option is used inside url or script
-- return -1 on error, 0 NOT required, 1 required
local function _option_used(option, urlscript)
	local surl	-- search string for url
	local ssh	-- search string for script
	local required	-- option used inside url or script

	if     option == "domain"    then surl, ssh = '%[DOMAIN%]', '%$domain'
	elseif option == "username"  then surl, ssh = '%[USERNAME%]', '%$username'
	elseif option == "password"  then surl, ssh = '%[PASSWORD%]', '%$password'
	elseif option == "param_enc" then surl, ssh = '%[PARAMENC%]', '%$param_enc'
	elseif option == "param_opt" then surl, ssh = '%[PARAMOPT%]', '%$param_opt'
	else
		error("undefined option")
		return -1	-- return on error
	end

	local required = false
	-- handle url
	if urlscript:find('http') then
		required = ( urlscript:find(surl) )
	-- handle script
	else
		if not urlscript:find("/") then
			-- might be inside ddns-scripts directory
			urlscript = "/usr/lib/ddns/" .. urlscript
		end
		-- problem with script exit here
		if not NXFS.access(urlscript) then return -1 end

		local f = io.input(urlscript)
		-- still problem with script exit here
		if not f then return -1 end
		for l in f:lines() do
			repeat
				if l:find('^#') then break end  -- continue on comment lines
				required = ( l:find(surl) or l:find(ssh) )
			until true
			if required then break end
		end
		f:close()
	end
	return (required and 1 or 0)
end

-- function to verify if option is valid
local function _option_validate(self, value, optional)
	-- section is globally defined here be calling argument (see above)
	local fusev6 = usev6:formvalue(section) or "0"
	local fsvc4  = svc4:formvalue(section) or "-"
	local fsvc6  = svc6:formvalue(section) or "-"
	local urlsh, used

	-- IP-Version dependent custom service selected
	if (fusev6 == "0" and fsvc4 == "-") or
	   (fusev6 == "1" and fsvc6 == "-") then
		-- read custom url
		urlsh = uurl:formvalue(section) or ""
		-- no url then read custom script
		if (#urlsh == 0) then
			urlsh = ush:formvalue(section) or ""
		end
	-- IPv4 read from services4 table
	elseif (fusev6 == "0") then
		urlsh = services4[fsvc4] or ""
	-- IPv6 read from services6 table
	else
		urlsh = services6[fsvc6] or ""
	end
	-- problem with url or script exit here
	-- error handled somewhere else
	if (#urlsh == 0) then return "" end

	used = _option_used(self.option, urlsh)
	-- on error or not used return empty string
	if used < 1 then return "" end
	-- needed but no data then return error
	if not value or (#value == 0) then
		if optional then return nil end						 
		return nil, err_tab_basic(self) .. translate("missing / required")
	end
	return value
end

-- cbi-map definition -- #######################################################
local m 	= Map("ddns")
m.title		= CTRL.app_title_back()
m.description	= CTRL.app_description()
m.redirect	= DISP.build_url("admin", "services", "ddns")

m.on_after_commit = function(self)
	if self.changed then	-- changes ?
		local pid = DDNS.get_pid(section)
		if pid > 0 then	-- running ?
			local tmp = NX.kill(pid, 1)	-- send SIGHUP
		end
	end
end

-- provider switch was requested, save and reload page
if m:formvalue("cbid.ddns.%s._switch" % section) then	-- section == arg[1]
	local fsvc
	local fusev6 = m:formvalue("cbid.ddns.%s.use_ipv6" % section) or "0"
	if fusev6 == "1" then
		fsvc = m:formvalue("cbid.ddns.%s.ipv6_service_name" % section) or ""
	else
		fsvc = m:formvalue("cbid.ddns.%s.ipv4_service_name" % section) or ""
	end

	if fusev6 ~= (m:get(section, "use_ipv6") or "0") then	-- IPv6 was changed
		m:set(section, "use_ipv6", fusev6)		-- save it
	end

	if fsvc ~= "-" then					-- NOT "custom"
		m:set(section, "service_name", fsvc)		-- save it
	else							-- else
		m:del(section, "service_name")			-- delete it
	end
	m.uci:save(m.config)

	-- reload page
	HTTP.redirect( DISP.build_url("admin", "services", "ddns", "detail", section) )
	return
end

-- read application settings -- ################################################
-- log directory
local logdir = m.uci:get(m.config, "global", "ddns_logdir") or "/var/log/ddns"

-- cbi-section definition -- ###################################################
local ns = m:section( NamedSection, section, "service",
	translate("Details for") .. ([[: <strong>%s</strong>]] % section),
	translate("Configure here the details for selected Dynamic DNS service.") )
ns.instance = section	-- arg [1]
ns:tab("basic", translate("Basic Settings"), nil )
ns:tab("advanced", translate("Advanced Settings"), nil )
ns:tab("timer", translate("Timer Settings"), nil )
ns:tab("logview", translate("Log File Viewer"), nil )

-- TAB: Basic  #####################################################################################
-- enabled -- #################################################################
en = ns:taboption("basic", Flag, "enabled",
	translate("Enabled"),
	translate("If this service section is disabled it could not be started." .. "<br />" ..
		"Neither from LuCI interface nor from console") )
en.orientation = "horizontal"

-- IPv4/IPv6 - lookup_host -- #################################################
luh = ns:taboption("basic", Value, "lookup_host",
		translate("Lookup Hostname"),
		translate("Hostname/FQDN to validate, if IP update happen or necessary") )
luh.rmempty	= false
luh.placeholder = "myhost.example.com"
function luh.validate(self, value)
	if not value
	or not (#value > 0)
	or not DTYP.hostname(value) then
		return nil, err_tab_basic(self) .. translate("invalid FQDN / required - Sample") .. ": 'myhost.example.com'"
	else
		return UTIL.trim(value)
	end
end
function luh.parse(self, section, novld)
	DDNS.value_parse(self, section, novld)
end

-- use_ipv6 -- ################################################################

--We call it globally as it's called 11 times even outside specific function, saves 11 os.execute slow command!
local has_ipv6 = DDNS.env_info("has_ipv6") 

usev6 = ns:taboption("basic", ListValue, "use_ipv6",
	translate("IP address version"),
	translate("Defines which IP address 'IPv4/IPv6' is send to the DDNS provider") )
usev6.widget  = "radio"
usev6.default = "0"
usev6:value("0", translate("IPv4-Address") )
function usev6.cfgvalue(self, section)
	local value = AbstractValue.cfgvalue(self, section) or "0"
	if has_ipv6 or (value == "1" and not has_ipv6) then
		self:value("1", translate("IPv6-Address") )
	end
	if value == "1" and not has_ipv6 then
		self.description = err_ipv6_basic
	end
	return value
end
function usev6.validate(self, value)
	if (value == "1" and has_ipv6) or value == "0" then
		return value
	end
	return nil, err_tab_basic(self) .. err_ipv6_plain
end
function usev6.parse(self, section, novld)
	DDNS.value_parse(self, section, novld)
end

-- IPv4 - service_name -- #####################################################
svc4 = ns:taboption("basic", ListValue, "ipv4_service_name",
	translate("DDNS Service provider") .. " [IPv4]" )
svc4.default	= "-"
svc4:depends("use_ipv6", "0")	-- only show on IPv4
function svc4.cfgvalue(self, section)
	local v =  DDNS.read_value(self, section, "service_name")
	if v and (#v > 0) then
		for s, u in UTIL.kspairs(services4) do
			if v == s then return v end
		end
	end
	return "-"
end
function svc4.validate(self, value)
	if usev6:formvalue(section) ~= "1" then	-- do only on IPv4
		return value
	else
		return ""	-- suppress validate error
	end
end
function svc4.write(self, section, value)
	if usev6:formvalue(section) ~= "1" then	-- do only IPv4 here
		self.map:del(section, self.option)	-- to be shure
		if value ~= "-" then			-- and write "service_name
			self.map:del(section, "update_url")	-- delete update_url
			self.map:del(section, "update_script")	-- delete update_script
			return self.map:set(section, "service_name", value)
		else
			return self.map:del(section, "service_name")
		end
	end
end
function svc4.parse(self, section, novld)
	DDNS.value_parse(self, section, novld)
end

-- IPv6 - service_name -- #####################################################
svc6 = ns:taboption("basic", ListValue, "ipv6_service_name",
	translate("DDNS Service provider") .. " [IPv6]" )
svc6.default	= "-"
svc6:depends("use_ipv6", "1")	-- only show on IPv6
if not has_ipv6 then
	svc6.description = err_ipv6_basic
end
function svc6.cfgvalue(self, section)
	local v =  DDNS.read_value(self, section, "service_name")
	if v and (#v > 0) then
		for s, u in UTIL.kspairs(services4) do
			if v == s then return v end
		end
	end
	return "-"
end
function svc6.validate(self, value)
	if usev6:formvalue(section) == "1" then	-- do only on IPv6
		if has_ipv6 then return value end
		return nil, err_tab_basic(self) .. err_ipv6_plain
	else
		return ""	-- suppress validate error
	end
end
function svc6.write(self, section, value)
	if usev6:formvalue(section) == "1" then		-- do only when IPv6
		self.map:del(section, self.option)	-- delete "ipv6_service_name" helper
		if value ~= "-" then			-- and write "service_name
			self.map:del(section, "update_url")	-- delete update_url
			self.map:del(section, "update_script")	-- delete update_script
			return self.map:set(section, "service_name", value)
		else
			return self.map:del(section, "service_name")
		end
	end
end
function svc6.parse(self, section, novld)
	DDNS.value_parse(self, section, novld)
end

-- IPv4/IPv6 - change Provider -- #############################################
svs		= ns:taboption("basic", Button, "_switch")
svs.title      = translate("Really change DDNS provider?")
svs.inputtitle = translate("Change provider")
svs.inputstyle = "apply"

-- IPv4/IPv6 - update_url -- ##################################################
uurl = ns:taboption("basic", Value, "update_url",
	translate("Custom update-URL"),
	translate("Update URL to be used for updating your DDNS Provider." .. "<br />" ..
		"Follow instructions you will find on their WEB page.") )
function uurl.validate(self, value)
	local fush   = ush:formvalue(section)
	local fusev6 = usev6:formvalue(section)

	if (fusev6 ~= "1" and svc4:formvalue(section) ~= "-") or
	   (fusev6 == "1" and svc6:formvalue(section) ~= "-") then
		return ""	-- suppress validate error
	elseif not value or (#value == 0) then
		if not fush or (#fush == 0) then
			return nil, err_tab_basic(self) .. translate("missing / required")
		else
			return ""	-- suppress validate error / update_script is given
		end
	elseif (#fush > 0) then
		return nil, err_tab_basic(self) .. translate("either url or script could be set")
	end

	local url = DDNS.parse_url(value)
	if not url.scheme == "http" then
		return nil, err_tab_basic(self) .. translate("must start with 'http://'")
	elseif not url.query then
		return nil, err_tab_basic(self) .. "<QUERY> " .. translate("missing / required")
	elseif not url.host then
		return nil, err_tab_basic(self) .. "<HOST> " .. translate("missing / required")
	elseif SYS.call([[nslookup ]] .. url.host .. [[ >/dev/null 2>&1]]) ~= 0 then
		return nil, err_tab_basic(self) .. translate("can not resolve host: ") .. url.host
	end

	return value
end
function uurl.parse(self, section, novld)
	DDNS.value_parse(self, section, novld)
end

-- IPv4/IPv6 - update_script -- ###############################################
ush = ns:taboption("basic", Value, "update_script",
	translate("Custom update-script"),
	translate("Custom update script to be used for updating your DDNS Provider.") )
function ush.validate(self, value)
	local fuurl  = uurl:formvalue(section)
	local fusev6 = usev6:formvalue(section)

	if (fusev6 ~= "1" and svc4:formvalue(section) ~= "-") or
	   (fusev6 == "1" and svc6:formvalue(section) ~= "-") then
		return ""	-- suppress validate error
	elseif not value or (#value == 0) then
		if not fuurl or (#fuurl == 0) then
			return nil, err_tab_basic(self) .. translate("missing / required")
		else
			return ""	-- suppress validate error / update_url is given
		end
	elseif (#fuurl > 0) then
		return nil, err_tab_basic(self) .. translate("either url or script could be set")
	elseif not NXFS.access(value) then
		return nil, err_tab_basic(self) .. translate("File not found")
	end
	return value
end
function ush.parse(self, section, novld)
	DDNS.value_parse(self, section, novld)
end

-- IPv4/IPv6 - domain -- ######################################################
dom = ns:taboption("basic", Value, "domain",
		translate("Domain"),
		translate("Replaces [DOMAIN] in Update-URL") )
dom.placeholder = "myhost.example.com"
function dom.validate(self, value)
	return _option_validate(self, value)
end
function dom.parse(self, section, novld)
	DDNS.value_parse(self, section, novld)
end

-- IPv4/IPv6 - username -- ####################################################
user = ns:taboption("basic", Value, "username",
		translate("Username"),
		translate("Replaces [USERNAME] in Update-URL (URL-encoded)") )
function user.validate(self, value)
	return _option_validate(self, value)
end
function user.parse(self, section, novld)
	DDNS.value_parse(self, section, novld)
end

-- IPv4/IPv6 - password -- ####################################################
pw = ns:taboption("basic", Value, "password",
		translate("Password"),
		translate("Replaces [PASSWORD] in Update-URL (URL-encoded)") )
pw.password = true
function pw.validate(self, value)
	return _option_validate(self, value)
end
function pw.parse(self, section, novld)
	DDNS.value_parse(self, section, novld)
end

-- IPv4/IPv6 - param_enc -- ###################################################
pe = ns:taboption("basic", Value, "param_enc",
		translate("Optional Encoded Parameter"),
		translate("Optional: Replaces [PARAMENC] in Update-URL (URL-encoded)") )
function pe.validate(self, value)
	return _option_validate(self, value, true)
end
function pe.parse(self, section, novld)
	DDNS.value_parse(self, section, novld)
end

-- IPv4/IPv6 - param_opt -- ###################################################
po = ns:taboption("basic", Value, "param_opt",
		translate("Optional Parameter"),
		translate("Optional: Replaces [PARAMOPT] in Update-URL (NOT URL-encoded)") )
function po.validate(self, value)
	return _option_validate(self, value, true)
end
function po.parse(self, section, novld)
	DDNS.value_parse(self, section, novld)
end

-- handled service dependent show/display -- ##################################
-- IPv4 --
local cv4 = svc4:cfgvalue(section)
if cv4 ~= "-" then
	svs:depends ("ipv4_service_name", "-" )	-- show only if "-"
	ush:depends ("ipv4_service_name", "?")
	uurl:depends("ipv4_service_name", "?")
else
	uurl:depends("ipv4_service_name", "-")
	ush:depends ("ipv4_service_name", "-")
	dom:depends("ipv4_service_name", "-" )
	user:depends("ipv4_service_name", "-" )
	pw:depends("ipv4_service_name", "-" )
	pe:depends("ipv4_service_name", "-" )
	po:depends("ipv4_service_name", "-" )
end
for s, u in UTIL.kspairs(services4) do
	svc4:value(s)	-- fill DropDown-List
	if cv4 ~= s then
		svs:depends("ipv4_service_name", s )
	else
		dom:depends ("ipv4_service_name", ((_option_used(dom.option, u) == 1) and s or "?") )
		user:depends("ipv4_service_name", ((_option_used(user.option, u) == 1) and s or "?") )
		pw:depends  ("ipv4_service_name", ((_option_used(pw.option, u) == 1) and s or "?") )
		pe:depends  ("ipv4_service_name", ((_option_used(pe.option, u) == 1) and s or "?") )
		po:depends  ("ipv4_service_name", ((_option_used(po.option, u) == 1) and s or "?") )
	end
end
svc4:value("-", translate("-- custom --") )

-- IPv6 --
local cv6 = svc6:cfgvalue(section)
if cv6 ~= "-" then
	svs:depends ("ipv6_service_name", "-" )
	uurl:depends("ipv6_service_name", "?")
	ush:depends ("ipv6_service_name", "?")
else
	uurl:depends("ipv6_service_name", "-")
	ush:depends ("ipv6_service_name", "-")
	dom:depends("ipv6_service_name", "-" )
	user:depends("ipv6_service_name", "-" )
	pw:depends("ipv6_service_name", "-" )
	pe:depends("ipv6_service_name", "-" )
	po:depends("ipv6_service_name", "-" )
end
for s, u in UTIL.kspairs(services6) do
	svc6:value(s)	-- fill DropDown-List
	if cv6 ~= s then
		svs:depends("ipv6_service_name", s )
	else
		dom:depends ("ipv6_service_name", ((_option_used(dom.option, u) == 1) and s or "?") )
		user:depends("ipv6_service_name", ((_option_used(user.option, u) == 1) and s or "?") )
		pw:depends  ("ipv6_service_name", ((_option_used(pw.option, u) == 1) and s or "?") )
		pe:depends  ("ipv6_service_name", ((_option_used(pe.option, u) == 1) and s or "?") )
		po:depends  ("ipv6_service_name", ((_option_used(po.option, u) == 1) and s or "?") )
	end
end
svc6:value("-", translate("-- custom --") )

-- IPv4/IPv6 - use_https -- ###################################################

--We call it globally as it's called 4 times outside specific function.
local has_ssl = DDNS.env_info("has_ssl")

if has_ssl or ( ( m:get(section, "use_https") or "0" ) == "1" ) then
	https = ns:taboption("basic", Flag, "use_https",
		translate("Use HTTP Secure") )
	https.orientation = "horizontal"
	function https.cfgvalue(self, section)
		local value = AbstractValue.cfgvalue(self, section)
		if not has_ssl and value == "1" then
			self.description = bold_on .. font_red ..
				translate("HTTPS not supported") .. font_off .. "<br />" ..
				translate("please disable") .. " !" .. bold_off
		else
			self.description = translate("Enable secure communication with DDNS provider")
		end
		return value
	end
	function https.validate(self, value)
		if (value == "1" and has_ssl ) or value == "0" then return value end
		return nil, err_tab_basic(self) .. translate("HTTPS not supported") .. " !"
	end
	function https.write(self, section, value)
		if value == "1" then
			return self.map:set(section, self.option, value)
		else
			self.map:del(section, "cacert")
			return self.map:del(section, self.option)
		end
	end
end

-- IPv4/IPv6 - cacert -- ######################################################
if has_ssl then
	cert = ns:taboption("basic", Value, "cacert",
		translate("Path to CA-Certificate"),
		translate("directory or path/file") .. "<br />" ..
		translate("or") .. bold_on .. " IGNORE " .. bold_off ..
		translate("to run HTTPS without verification of server certificates (insecure)") )
	cert:depends("use_https", "1")
	cert.placeholder = "/etc/ssl/certs"
	cert.forcewrite = true
	function cert.validate(self, value)
		if https:formvalue(section) ~= "1" then
			return ""	-- suppress validate error if NOT https
		end
		if value then	-- otherwise errors in datatype check
			if DTYP.directory(value)
			or DTYP.file(value)
			or (value == "IGNORE")
			or (#value == 0) then
				return value
			end
		end
		return nil, err_tab_basic(self) ..
			translate("file or directory not found or not 'IGNORE'") .. " !"
	end
	function cert.parse(self, section, novld)
		DDNS.value_parse(self, section, novld)
	end
end

-- TAB: Advanced  #################################################################################
-- IPv4 - ip_source -- ########################################################
src4 = ns:taboption("advanced", ListValue, "ipv4_source",
	translate("IP address source") .. " [IPv4]",
	translate("Defines the source to read systems IPv4-Address from, that will be send to the DDNS provider") )
src4:depends("use_ipv6", "0")	-- IPv4 selected
src4.default = "network"
src4:value("network", translate("Network"))
src4:value("web", translate("URL"))
src4:value("interface", translate("Interface"))
src4:value("script", translate("Script"))
function src4.cfgvalue(self, section)
	return DDNS.read_value(self, section, "ip_source")
end
function src4.validate(self, value)
	if usev6:formvalue(section) == "1" then
		return ""	-- ignore on IPv6 selected
	elseif not _verify_ip_source() then
		return nil, err_tab_adv(self) ..
			translate("can not detect local IP. Please select a different Source combination")
	else
		return value
	end
end
function src4.write(self, section, value)
	if usev6:formvalue(section) == "1" then
		return true	-- ignore on IPv6 selected
	elseif value == "network" then
		self.map:del(section, "ip_url")		-- delete not need parameters
		self.map:del(section, "ip_interface")
		self.map:del(section, "ip_script")
	elseif value == "web" then
		self.map:del(section, "ip_network")	-- delete not need parameters
		self.map:del(section, "ip_interface")
		self.map:del(section, "ip_script")
	elseif value == "interface" then
		self.map:del(section, "ip_network")	-- delete not need parameters
		self.map:del(section, "ip_url")
		self.map:del(section, "ip_script")
	elseif value == "script" then
		self.map:del(section, "ip_network")
		self.map:del(section, "ip_url")		-- delete not need parameters
		self.map:del(section, "ip_interface")
	end
	self.map:del(section, self.option)		 -- delete "ipv4_source" helper
	return self.map:set(section, "ip_source", value) -- and write "ip_source
end
function src4.parse(self, section, novld)
	DDNS.value_parse(self, section, novld)
end

-- IPv6 - ip_source -- ########################################################
src6 = ns:taboption("advanced", ListValue, "ipv6_source",
	translate("IP address source") .. " [IPv6]",
	translate("Defines the source to read systems IPv6-Address from, that will be send to the DDNS provider") )
src6:depends("use_ipv6", 1)	-- IPv6 selected
src6.default = "network"
src6:value("network", translate("Network"))
src6:value("web", translate("URL"))
src6:value("interface", translate("Interface"))
src6:value("script", translate("Script"))
if not has_ipv6 then
	src6.description = err_ipv6_other
end
function src6.cfgvalue(self, section)
	return DDNS.read_value(self, section, "ip_source")
end
function src6.validate(self, value)
	if usev6:formvalue(section) ~= "1" then
		return ""	-- ignore on IPv4 selected
	elseif not has_ipv6 then
		return nil, err_tab_adv(self) .. err_ipv6_plain
	elseif not _verify_ip_source() then
		return nil, err_tab_adv(self) ..
			translate("can not detect local IP. Please select a different Source combination")
	else
		return value
	end
end
function src6.write(self, section, value)
	if usev6:formvalue(section) ~= "1" then
		return true	-- ignore on IPv4 selected
	elseif value == "network" then
		self.map:del(section, "ip_url")		-- delete not need parameters
		self.map:del(section, "ip_interface")
		self.map:del(section, "ip_script")
	elseif value == "web" then
		self.map:del(section, "ip_network")	-- delete not need parameters
		self.map:del(section, "ip_interface")
		self.map:del(section, "ip_script")
	elseif value == "interface" then
		self.map:del(section, "ip_network")	-- delete not need parameters
		self.map:del(section, "ip_url")
		self.map:del(section, "ip_script")
	elseif value == "script" then
		self.map:del(section, "ip_network")
		self.map:del(section, "ip_url")		-- delete not need parameters
		self.map:del(section, "ip_interface")
	end
	self.map:del(section, self.option)		 -- delete "ipv4_source" helper
	return self.map:set(section, "ip_source", value) -- and write "ip_source
end
function src6.parse(self, section, novld)
	DDNS.value_parse(self, section, novld)
end

-- IPv4 - ip_network (default "wan") -- #######################################
ipn4 = ns:taboption("advanced", ListValue, "ipv4_network",
	translate("Network") .. " [IPv4]",
	translate("Defines the network to read systems IPv4-Address from") )
ipn4:depends("ipv4_source", "network")
ipn4.default = "wan"
WADM.cbi_add_networks(ipn4)
function ipn4.cfgvalue(self, section)
	return DDNS.read_value(self, section, "ip_network")
end
function ipn4.validate(self, value)
	if usev6:formvalue(section) == "1"
	 or src4:formvalue(section) ~= "network" then
		-- ignore if IPv6 selected OR
		-- ignore everything except "network"
		return ""
	else
		return value
	end
end
function ipn4.write(self, section, value)
	if usev6:formvalue(section) == "1"
	 or src4:formvalue(section) ~= "network" then
		-- ignore if IPv6 selected OR
		-- ignore everything except "network"
		return true
	else
		-- set also as "interface" for monitoring events changes/hot-plug
		self.map:set(section, "interface", value)
		self.map:del(section, self.option)		  -- delete "ipv4_network" helper
		return self.map:set(section, "ip_network", value) -- and write "ip_network"
	end
end
function ipn4.parse(self, section, novld)
	DDNS.value_parse(self, section, novld)
end

-- IPv6 - ip_network (default "wan6") -- ######################################
ipn6 = ns:taboption("advanced", ListValue, "ipv6_network",
	translate("Network") .. " [IPv6]" )
ipn6:depends("ipv6_source", "network")
ipn6.default = "wan6"
WADM.cbi_add_networks(ipn6)
if has_ipv6 then
	ipn6.description = translate("Defines the network to read systems IPv6-Address from")
else
	ipn6.description = err_ipv6_other
end
function ipn6.cfgvalue(self, section)
	return DDNS.read_value(self, section, "ip_network")
end
function ipn6.validate(self, value)
	if usev6:formvalue(section) ~= "1"
	 or src6:formvalue(section) ~= "network" then
		-- ignore if IPv4 selected OR
		-- ignore everything except "network"
		return ""
	elseif has_ipv6 then
		return value
	else
		return nil, err_tab_adv(self) .. err_ipv6_plain
	end
end
function ipn6.write(self, section, value)
	if usev6:formvalue(section) ~= "1"
	 or src6:formvalue(section) ~= "network" then
		-- ignore if IPv4 selected OR
		-- ignore everything except "network"
		return true
	else
		-- set also as "interface" for monitoring events changes/hotplug
		self.map:set(section, "interface", value)
		self.map:del(section, self.option)		  -- delete "ipv6_network" helper
		return self.map:set(section, "ip_network", value) -- and write "ip_network"
	end
end
function ipn6.parse(self, section, novld)
	DDNS.value_parse(self, section, novld)
end

-- IPv4 - ip_url (default "checkip.dyndns.com") -- ############################
iurl4 = ns:taboption("advanced", Value, "ipv4_url",
	translate("URL to detect") .. " [IPv4]",
	translate("Defines the Web page to read systems IPv4-Address from") )
iurl4:depends("ipv4_source", "web")
iurl4.default = "http://checkip.dyndns.com"
function iurl4.cfgvalue(self, section)
	return DDNS.read_value(self, section, "ip_url")
end
function iurl4.validate(self, value)
	if usev6:formvalue(section) == "1"
	 or src4:formvalue(section) ~= "web" then
		-- ignore if IPv6 selected OR
		-- ignore everything except "web"
		return ""
	elseif not value or #value == 0 then
		return nil, err_tab_adv(self) .. translate("missing / required")
	end

	local url = DDNS.parse_url(value)
	if not (url.scheme == "http" or url.scheme == "https") then
		return nil, err_tab_adv(self) .. translate("must start with 'http://'")
	elseif not url.host then
		return nil, err_tab_adv(self) .. "<HOST> " .. translate("missing / required")
	elseif SYS.call([[nslookup ]] .. url.host .. [[>/dev/null 2>&1]]) ~= 0 then
		return nil, err_tab_adv(self) .. translate("can not resolve host: ") .. url.host
	else
		return value
	end
end
function iurl4.write(self, section, value)
	if usev6:formvalue(section) == "1"
	 or src4:formvalue(section) ~= "web" then
		-- ignore if IPv6 selected OR
		-- ignore everything except "web"
		return true
	else
		self.map:del(section, self.option)		-- delete "ipv4_url" helper
		return self.map:set(section, "ip_url", value)	-- and write "ip_url"
	end
end
function iurl4.parse(self, section, novld)
	DDNS.value_parse(self, section, novld)
end

-- IPv6 - ip_url (default "checkipv6.dyndns.com") -- ##########################
iurl6 = ns:taboption("advanced", Value, "ipv6_url",
	translate("URL to detect") .. " [IPv6]" )
iurl6:depends("ipv6_source", "web")
iurl6.default = "http://checkipv6.dyndns.com"
if has_ipv6 then
	iurl6.description = translate("Defines the Web page to read systems IPv6-Address from")
else
	iurl6.description = err_ipv6_other
end
function iurl6.cfgvalue(self, section)
	return DDNS.read_value(self, section, "ip_url")
end
function iurl6.validate(self, value)
	if usev6:formvalue(section) ~= "1"
	 or src6:formvalue(section) ~= "web" then
		-- ignore if IPv4 selected OR
		-- ignore everything except "web"
		return ""
	elseif not has_ipv6 then
		return nil, err_tab_adv(self) .. err_ipv6_plain
	elseif not value or #value == 0 then
		return nil, err_tab_adv(self) .. translate("missing / required")
	end

	local url = DDNS.parse_url(value)
	if not (url.scheme == "http" or url.scheme == "https") then
		return nil, err_tab_adv(self) .. translate("must start with 'http://'")
	elseif not url.host then
		return nil, err_tab_adv(self) .. "<HOST> " .. translate("missing / required")
	elseif SYS.call([[nslookup ]] .. url.host .. [[>/dev/null 2>&1]]) ~= 0 then
		return nil, err_tab_adv(self) .. translate("can not resolve host: ") .. url.host
	else
		return value
	end
end
function iurl6.write(self, section, value)
	if usev6:formvalue(section) ~= "1"
	 or src6:formvalue(section) ~= "web" then
		-- ignore if IPv4 selected OR
		-- ignore everything except "web"
		return true
	else
		self.map:del(section, self.option)		-- delete "ipv6_url" helper
		return self.map:set(section, "ip_url", value)	-- and write "ip_url"
	end
end
function iurl6.parse(self, section, novld)
	DDNS.value_parse(self, section, novld)
end

-- IPv4 + IPv6 - ip_interface -- ##############################################
ipi = ns:taboption("advanced", ListValue, "ip_interface",
	translate("Interface"),
	translate("Defines the interface to read systems IP-Address from") )
ipi:depends("ipv4_source", "interface")	-- IPv4
ipi:depends("ipv6_source", "interface")	-- or IPv6
for _, v in pairs(SYS.net.devices()) do
	-- show only interface set to a network
	-- and ignore loopback
	net = WADM.iface_get_network(v)
	if net and net ~= "loopback" then
		ipi:value(v)
	end
end
function ipi.validate(self, value)
	local fusev6 = usev6:formvalue(section)
	if (fusev6 ~= "1" and src4:formvalue(section) ~= "interface")
	or (fusev6 == "1" and src6:formvalue(section) ~= "interface") then
		return ""
	else
		return value
	end
end
function ipi.write(self, section, value)
	local fusev6 = usev6:formvalue(section)
	if (fusev6 ~= "1" and src4:formvalue(section) ~= "interface")
	or (fusev6 == "1" and src6:formvalue(section) ~= "interface") then
		return true
	else
		-- get network from device to
		-- set also as "interface" for monitoring events changes/hotplug
		local net = WADM.iface_get_network(value)
		self.map:set(section, "interface", net)
		return self.map:set(section, self.option, value)
	end
end
function ipi.parse(self, section, novld)
	DDNS.value_parse(self, section, novld)
end

-- IPv4 + IPv6 - ip_script -- #################################################
ips = ns:taboption("advanced", Value, "ip_script",
	translate("Script"),
	translate("User defined script to read systems IP-Address") )
ips:depends("ipv4_source", "script")	-- IPv4
ips:depends("ipv6_source", "script")	-- or IPv6
ips.placeholder = "/path/to/script.sh"
function ips.validate(self, value)
	local fusev6 = usev6:formvalue(section)
	local split
	if value then split = UTIL.split(value, " ") end

	if (fusev6 ~= "1" and src4:formvalue(section) ~= "script")
	or (fusev6 == "1" and src6:formvalue(section) ~= "script") then
		return ""
	elseif not value or not (#value > 0) or not NXFS.access(split[1], "x") then
		return nil, err_tab_adv(self) ..
			translate("not found or not executable - Sample: '/path/to/script.sh'")
	else
		return value
	end
end
function ips.write(self, section, value)
	local fusev6 = usev6:formvalue(section)
	if (fusev6 ~= "1" and src4:formvalue(section) ~= "script")
	or (fusev6 == "1" and src6:formvalue(section) ~= "script") then
		return true
	else
		return self.map:set(section, self.option, value)
	end
end
function ips.parse(self, section, novld)
	DDNS.value_parse(self, section, novld)
end

-- IPv4 - interface - default "wan" -- ########################################
-- event network to monitor changes/hotplug/dynamic_dns_updater.sh
-- only needs to be set if "ip_source"="web" or "script"
-- if "ip_source"="network" or "interface" we use their network
eif4 = ns:taboption("advanced", ListValue, "ipv4_interface",
	translate("Event Network") .. " [IPv4]",
	translate("Network on which the ddns-updater scripts will be started") )
eif4:depends("ipv4_source", "web")
eif4:depends("ipv4_source", "script")
eif4.default = "wan"
WADM.cbi_add_networks(eif4)
function eif4.cfgvalue(self, section)
	return DDNS.read_value(self, section, "interface")
end
function eif4.validate(self, value)
	local fsrc4 = src4:formvalue(section) or ""
	if usev6:formvalue(section) == "1"
	 or fsrc4 == "network"
	 or fsrc4 == "interface" then
		return ""	-- ignore IPv6, network, interface
	else
		return value
	end
end
function eif4.write(self, section, value)
	local fsrc4 = src4:formvalue(section) or ""
	if usev6:formvalue(section) == "1"
	 or fsrc4 == "network"
	 or fsrc4 == "interface" then
		return true	-- ignore IPv6, network, interface
	else
		self.map:del(section, self.option)		 -- delete "ipv4_interface" helper
		return self.map:set(section, "interface", value) -- and write "interface"
	end
end
function eif4.parse(self, section, novld)
	DDNS.value_parse(self, section, novld)
end

-- IPv6 - interface - default "wan6" -- #######################################
-- event network to monitor changes/hotplug
-- only needs to be set if "ip_source"="web" or "script"
-- if "ip_source"="network" or "interface" we use their network
eif6 = ns:taboption("advanced", ListValue, "ipv6_interface",
	translate("Event Network") .. " [IPv6]" )
eif6:depends("ipv6_source", "web")
eif6:depends("ipv6_source", "script")
eif6.default = "wan6"
WADM.cbi_add_networks(eif6)
if not has_ipv6 then
	eif6.description = err_ipv6_other
else
	eif6.description = translate("Network on which the ddns-updater scripts will be started")
end
function eif6.cfgvalue(self, section)
	return DDNS.read_value(self, section, "interface")
end
function eif6.validate(self, value)
	local fsrc6 = src6:formvalue(section) or ""
	if usev6:formvalue(section) ~= "1"
	 or fsrc6 == "network"
	 or fsrc6 == "interface" then
		return ""	-- ignore IPv4, network, interface
	elseif not has_ipv6 then
		return nil, err_tab_adv(self) .. err_ipv6_plain
	else
		return value
	end
end
function eif6.write(self, section, value)
	local fsrc6 = src6:formvalue(section) or ""
	if usev6:formvalue(section) ~= "1"
	 or fsrc6 == "network"
	 or fsrc6 == "interface" then
		return true	-- ignore IPv4, network, interface
	else
		self.map:del(section, self.option)		 -- delete "ipv6_interface" helper
		return self.map:set(section, "interface", value) -- and write "interface"
	end
end
function eif6.parse(self, section, novld)
	DDNS.value_parse(self, section, novld)
end 

-- IPv4/IPv6 - bind_network -- ################################################

local has_bindnet = DDNS.env_info("has_bindnet")

if has_bindnet or ( ( m:get(section, "bind_network") or "" ) ~= "" ) then
	bnet = ns:taboption("advanced", ListValue, "bind_network",
		translate("Bind Network") )
	bnet:depends("ipv4_source", "web")
	bnet:depends("ipv6_source", "web")
	bnet.default = ""
	bnet:value("", translate("-- default --"))
	WADM.cbi_add_networks(bnet)
	function bnet.cfgvalue(self, section)
		local value = AbstractValue.cfgvalue(self, section)
		if not has_bindnet and value ~= "" then
			self.description = bold_on .. font_red ..
				translate("Binding to a specific network not supported") .. font_off .. "<br />" ..
				translate("please set to 'default'") .. " !" .. bold_off
		else
			self.description = translate("OPTIONAL: Network to use for communication") ..
				"<br />" .. translate("Casual users should not change this setting")
		end
		return value
	end
	function bnet.validate(self, value)
		if ( (value ~= "") and has_bindnet ) or (value == "") then return value end
		return nil, err_tab_adv(self) .. translate("Binding to a specific network not supported") .. " !"
	end
	function bnet.parse(self, section, novld)
		DDNS.value_parse(self, section, novld)
	end
end

-- IPv4 + IPv6 - force_ipversion -- ###########################################
-- optional to force wget/curl and host to use only selected IP version
-- command parameter "-4" or "-6"

local has_forceip = DDNS.env_info("has_forceip")

if has_forceip or ( ( m:get(section, "force_ipversion") or "0" ) ~= "0" ) then
	fipv = ns:taboption("advanced", Flag, "force_ipversion",
		translate("Force IP Version") )
	fipv.orientation = "horizontal"
	function fipv.cfgvalue(self, section)
		local value = AbstractValue.cfgvalue(self, section)
		if not has_forceip and value ~= "0" then
			self.description = bold_on .. font_red ..
				translate("Force IP Version not supported") .. font_off .. "<br />" ..
				translate("please disable") .. " !" .. bold_off
		else
			self.description = translate("OPTIONAL: Force the usage of pure IPv4/IPv6 only communication.")
		end
		return value
	end
	function fipv.validate(self, value)
		if (value == "1" and has_forceip) or value == "0" then return value end
		return nil, err_tab_adv(self) .. translate("Force IP Version not supported")
	end
end

-- IPv4 + IPv6 - dns_server -- ################################################
-- optional DNS Server to use resolving my IP

local has_dnsserver = DDNS.env_info("has_dnsserver")

if has_dnsserver or ( ( m:get(section, "dns_server") or "" ) ~= "" ) then
	dns = ns:taboption("advanced", Value, "dns_server",
		translate("DNS-Server"),
		translate("OPTIONAL: Use non-default DNS-Server to detect 'Registered IP'.") .. "<br />" ..
		translate("Format: IP or FQDN"))
	dns.placeholder = "mydns.lan"
	function dns.validate(self, value)
		-- if .datatype is set, then it is checked before calling this function
		if not value or (#value == 0) then
			return ""	-- ignore on empty
		elseif not has_dnsserver then
			return nil, err_tab_adv(self) .. translate("Specifying a DNS-Server is not supported")
		elseif not DTYP.host(value) then
			return nil, err_tab_adv(self) .. translate("use hostname, FQDN, IPv4- or IPv6-Address")
		else
			local ipv6  = usev6:formvalue(section) or "0"
			local force = fipv:formvalue(section)  or "0"
			local command = CTRL.luci_helper .. [[ -]]
			if (ipv6 == 1)  then command = command .. [[6]] end
			if (force == 1) then command = command .. [[f]] end
			command = command .. [[d ]] .. value .. [[ -- verify_dns]]

			local ret = SYS.call(command)
			if     ret == 0 then return value	-- everything OK
			elseif ret == 2 then return nil, err_tab_adv(self) .. translate("nslookup can not resolve host")
			elseif ret == 3 then return nil, err_tab_adv(self) .. translate("nc (netcat) can not connect")
			elseif ret == 4 then return nil, err_tab_adv(self) .. translate("Forced IP Version don't matched")
			else                 return nil, err_tab_adv(self) .. translate("unspecific error")
			end
		end
	end
	function dns.parse(self, section, novld)
		DDNS.value_parse(self, section, novld)
	end
end

-- IPv4 + IPv6 - force_dnstcp -- ##############################################

local has_bindhost = DDNS.env_info("has_bindhost")

if has_bindhost or ( ( m:get(section, "force_dnstcp") or "0" ) ~= "0" ) then
	tcp = ns:taboption("advanced", Flag, "force_dnstcp",
		translate("Force TCP on DNS") )
	tcp.orientation = "horizontal"
	function tcp.cfgvalue(self, section)
		local value = AbstractValue.cfgvalue(self, section)
		if not has_bindhost and value ~= "0" then
			self.description = bold_on .. font_red ..
				translate("DNS requests via TCP not supported") .. font_off .. "<br />" ..
				translate("please disable") .. " !" .. bold_off
		else
			self.description = translate("OPTIONAL: Force the use of TCP instead of default UDP on DNS requests.")
		end
		return value
	end
	function tcp.validate(self, value)
		if (value == "1" and has_bindhost ) or value == "0" then
			return value
		end
		return nil, err_tab_adv(self) .. translate("DNS requests via TCP not supported")
	end
end

-- IPv4 + IPv6 - proxy -- #####################################################
-- optional Proxy to use for http/https requests  [user:password@]proxyhost[:port]

local has_proxy = DDNS.env_info("has_proxy")

if has_proxy or ( ( m:get(section, "proxy") or "" ) ~= "" ) then
	pxy = ns:taboption("advanced", Value, "proxy",
		translate("PROXY-Server") )
	pxy.placeholder="user:password@myproxy.lan:8080"
	function pxy.cfgvalue(self, section)
		local value = AbstractValue.cfgvalue(self, section)
		if not has_proxy and value ~= "" then
			self.description = bold_on .. font_red ..
				translate("PROXY-Server not supported") .. font_off .. "<br />" ..
				translate("please remove entry") .. "!" .. bold_off
		else
			self.description = translate("OPTIONAL: Proxy-Server for detection and updates.") .. "<br />" ..
				translate("Format") .. ": " .. bold_on .. "[user:password@]proxyhost:port" .. bold_off .. "<br />" ..
				translate("IPv6 address must be given in square brackets") .. ": " ..
				bold_on .. " [2001:db8::1]:8080" .. bold_off
		end
		return value
	end
	function pxy.validate(self, value)
		-- if .datatype is set, then it is checked before calling this function
		if not value or (#value == 0) then
			return ""	-- ignore on empty
		elseif has_proxy then
			local ipv6  = usev6:formvalue(section) or "0"
			local force = fipv:formvalue(section) or "0"
			local command = CTRL.luci_helper .. [[ -]]
			if (ipv6 == 1)  then command = command .. [[6]] end
			if (force == 1) then command = command .. [[f]] end
			command = command .. [[p ]] .. value .. [[ -- verify_proxy]]
			local ret = SYS.call(command)
			if     ret == 0 then return value
			elseif ret == 2 then return nil, err_tab_adv(self) .. translate("nslookup can not resolve host")
			elseif ret == 3 then return nil, err_tab_adv(self) .. translate("nc (netcat) can not connect")
			elseif ret == 4 then return nil, err_tab_adv(self) .. translate("Forced IP Version don't matched")
			elseif ret == 5 then return nil, err_tab_adv(self) .. translate("proxy port missing")
			else                 return nil, err_tab_adv(self) .. translate("unspecific error")
			end
		else
			return nil, err_tab_adv(self) .. translate("PROXY-Server not supported")
		end
	end
	function pxy.parse(self, section, novld)
		DDNS.value_parse(self, section, novld)
	end
end

-- use_syslog -- ##############################################################
slog = ns:taboption("advanced", ListValue, "use_syslog",
	translate("Log to syslog"),
	translate("Writes log messages to syslog. Critical Errors will always be written to syslog.") )
slog.default = "2"
slog:value("0", translate("No logging"))
slog:value("1", translate("Info"))
slog:value("2", translate("Notice"))
slog:value("3", translate("Warning"))
slog:value("4", translate("Error"))
function slog.parse(self, section, novld)
	DDNS.value_parse(self, section, novld)
end

-- use_logfile -- #############################################################
logf = ns:taboption("advanced", Flag, "use_logfile",
	translate("Log to file"),
	translate("Writes detailed messages to log file. File will be truncated automatically.") .. "<br />" ..
	translate("File") .. [[: "]] .. logdir .. [[/]] .. section .. [[.log"]] )
logf.orientation = "horizontal"
logf.default     = "1"		-- if not defined write to log by default

-- TAB: Timer  ####################################################################################
-- check_interval -- ##########################################################
ci = ns:taboption("timer", Value, "check_interval",
	translate("Check Interval") )
ci.template = "ddns/detail_value"
ci.default  = "10"
function ci.validate(self, value)
	if not DTYP.uinteger(value)
	or tonumber(value) < 1 then
		return nil, err_tab_timer(self) .. translate("minimum value 5 minutes == 300 seconds")
	end

	local secs = DDNS.calc_seconds(value, cu:formvalue(section))
	if secs >= 300 then
		return value
	else
		return nil, err_tab_timer(self) .. translate("minimum value 5 minutes == 300 seconds")
	end
end
function ci.write(self, section, value)
	-- remove when default
	local secs = DDNS.calc_seconds(value, cu:formvalue(section))
	if secs ~= 600 then	--default 10 minutes
		return self.map:set(section, self.option, value)
	else
		self.map:del(section, "check_unit")
		return self.map:del(section, self.option)
	end
end
function ci.parse(self, section, novld)
	DDNS.value_parse(self, section, novld)
end

-- check_unit -- ##############################################################
cu = ns:taboption("timer", ListValue, "check_unit", "not displayed, but needed otherwise error",
	translate("Interval to check for changed IP" .. "<br />" ..
		"Values below 5 minutes == 300 seconds are not supported") )
cu.template = "ddns/detail_lvalue"
cu.default  = "minutes"
cu:value("seconds", translate("seconds"))
cu:value("minutes", translate("minutes"))
cu:value("hours", translate("hours"))
--cu:value("days", translate("days"))
function cu.write(self, section, value)
	-- remove when default
	local secs = DDNS.calc_seconds(ci:formvalue(section), value)
	if secs ~= 600 then	--default 10 minutes
		return self.map:set(section, self.option, value)
	else
		return true
	end
end
function cu.parse(self, section, novld)
	DDNS.value_parse(self, section, novld)
end

-- force_interval (modified) -- ###############################################
fi = ns:taboption("timer", Value, "force_interval",
	translate("Force Interval") )
fi.template = "ddns/detail_value"
fi.default  = "72" 	-- see dynamic_dns_updater.sh script
--fi.rmempty = false	-- validate ourselves for translatable error messages
function fi.validate(self, value)
	if not DTYP.uinteger(value)
	or tonumber(value) < 0 then
		return nil, err_tab_timer(self) .. translate("minimum value '0'")
	end

	local force_s = DDNS.calc_seconds(value, fu:formvalue(section))
	if force_s == 0 then
		return value
	end

	local ci_value = ci:formvalue(section)
	if not DTYP.uinteger(ci_value) then
		return ""	-- ignore because error in check_interval above
	end

	local check_s = DDNS.calc_seconds(ci_value, cu:formvalue(section))
	if force_s >= check_s then
		return value
	end

	return nil, err_tab_timer(self) .. translate("must be greater or equal 'Check Interval'")
end
function fi.write(self, section, value)
	-- simulate rmempty=true remove default
	local secs = DDNS.calc_seconds(value, fu:formvalue(section))
	if secs ~= 259200 then	--default 72 hours == 3 days
		return self.map:set(section, self.option, value)
	else
		self.map:del(section, "force_unit")
		return self.map:del(section, self.option)
	end
end
function fi.parse(self, section, novld)
	DDNS.value_parse(self, section, novld)
end

-- force_unit -- ##############################################################
fu = ns:taboption("timer", ListValue, "force_unit", "not displayed, but needed otherwise error",
	translate("Interval to force updates send to DDNS Provider" .. "<br />" ..
		"Setting this parameter to 0 will force the script to only run once" .. "<br />" ..
		"Values lower 'Check Interval' except '0' are not supported") )
fu.template = "ddns/detail_lvalue"
fu.default  = "hours"
--fu.rmempty  = false	-- want to control write process
--fu:value("seconds", translate("seconds"))
fu:value("minutes", translate("minutes"))
fu:value("hours", translate("hours"))
fu:value("days", translate("days"))
function fu.write(self, section, value)
	-- simulate rmempty=true remove default
	local secs = DDNS.calc_seconds(fi:formvalue(section), value)
	if secs ~= 259200 and secs ~= 0 then	--default 72 hours == 3 days
		return self.map:set(section, self.option, value)
	else
		return true
	end
end
function fu.parse(self, section, novld)
	DDNS.value_parse(self, section, novld)
end

-- retry_count -- #############################################################
rc = ns:taboption("timer", Value, "retry_count")
rc.title	= translate("Error Retry Counter")
rc.description	= translate("On Error the script will stop execution after given number of retrys")
		.. "<br />"
		.. translate("The default setting of '0' will retry infinite.")
rc.default	= "0"
function rc.validate(self, value)
	if not DTYP.uinteger(value) then
		return nil, err_tab_timer(self) .. translate("minimum value '0'")
	else
		return value
	end
end
function rc.parse(self, section, novld)
	DDNS.value_parse(self, section, novld)
end

-- retry_interval -- ##########################################################
ri = ns:taboption("timer", Value, "retry_interval",
	translate("Error Retry Interval") )
ri.template = "ddns/detail_value"
ri.default  = "60"
function ri.validate(self, value)
	if not DTYP.uinteger(value)
	or tonumber(value) < 1 then
		return nil, err_tab_timer(self) .. translate("minimum value '1'")
	else
		return value
	end
end
function ri.write(self, section, value)
	-- simulate rmempty=true remove default
	local secs = DDNS.calc_seconds(value, ru:formvalue(section))
	if secs ~= 60 then	--default 60seconds
		return self.map:set(section, self.option, value)
	else
		self.map:del(section, "retry_unit")
		return self.map:del(section, self.option)
	end
end
function ri.parse(self, section, novld)
	DDNS.value_parse(self, section, novld)
end

-- retry_unit -- ##############################################################
ru = ns:taboption("timer", ListValue, "retry_unit", "not displayed, but needed otherwise error",
	translate("On Error the script will retry the failed action after given time") )
ru.template = "ddns/detail_lvalue"
ru.default  = "seconds"
--ru.rmempty  = false	-- want to control write process
ru:value("seconds", translate("seconds"))
ru:value("minutes", translate("minutes"))
--ru:value("hours", translate("hours"))
--ru:value("days", translate("days"))
function ru.write(self, section, value)
	-- simulate rmempty=true remove default
	local secs = DDNS.calc_seconds(ri:formvalue(section), value)
	if secs ~= 60 then	--default 60seconds
		return self.map:set(section, self.option, value)
	else
		return true -- will be deleted by retry_interval
	end
end
function ru.parse(self, section, novld)
	DDNS.value_parse(self, section, novld)
end

-- TAB: LogView  ##################################################################################
lv = ns:taboption("logview", DummyValue, "_logview")
lv.template = "ddns/detail_logview"
lv.inputtitle = translate("Read / Reread log file")
lv.rows = 50
function lv.cfgvalue(self, section)
	local lfile=logdir .. "/" .. section .. ".log"
	if NXFS.access(lfile) then
		return lfile .. "\n" .. translate("Please press [Read] button")
	end
	return lfile .. "\n" .. translate("File not found or empty")
end

return m
