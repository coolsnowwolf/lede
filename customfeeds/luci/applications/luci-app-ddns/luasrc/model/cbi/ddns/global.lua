-- Copyright 2014 Christian Schoenebeck <christian dot schoenebeck at gmail dot com>
-- Licensed to the public under the Apache License 2.0.

local NX   = require "nixio"
local NXFS = require "nixio.fs"
local DISP = require "luci.dispatcher"
local SYS  = require "luci.sys"
local CTRL = require "luci.controller.ddns"	-- this application's controller
local DDNS = require "luci.tools.ddns"		-- ddns multiused functions

-- cbi-map definition -- #######################################################
local m = Map("ddns")
m.title		= CTRL.app_title_back()
m.description	= CTRL.app_description()
m.redirect	= DISP.build_url("admin", "services", "ddns")

function m.commit_handler(self)
	if self.changed then	-- changes ?
		local command = CTRL.luci_helper .. " -- reload"
		os.execute(command)	-- reload configuration
	end
end

-- cbi-section definition -- ###################################################
local ns = m:section( NamedSection, "global", "ddns",
	translate("Global Settings"),
	translate("Configure here the details for all Dynamic DNS services including this LuCI application.") 
	.. [[<br /><strong>]]
	.. translate("It is NOT recommended for casual users to change settings on this page.")
	.. [[</strong><br />]]
	.. [[<a href="https://openwrt.org/docs/guide-user/base-system/ddns#section_ddns" target="_blank">]]
	.. translate("For detailed information about parameter settings look here.")
	.. [[</a>]]
	)

-- section might not exist
function ns.cfgvalue(self, section)
	if not self.map:get(section) then
		self.map:set(section, nil, self.sectiontype)
	end
	return self.map:get(section)
end

-- upd_privateip  -- ###########################################################
local ali	= ns:option(Flag, "upd_privateip")
ali.title	= translate("Allow non-public IP's")
ali.description = translate("Non-public and by default blocked IP's") .. ":"
		.. [[<br /><strong>IPv4: </strong>]]
		.. "0/8, 10/8, 100.64/10, 127/8, 169.254/16, 172.16/12, 192.168/16"
		.. [[<br /><strong>IPv6: </strong>]]
		.. "::/32, f000::/4"
ali.default	= "0"

-- ddns_dateformat  -- #########################################################
local df	= ns:option(Value, "ddns_dateformat")
df.title	= translate("Date format")
df.description	= [[<a href="http://www.cplusplus.com/reference/ctime/strftime/" target="_blank">]]
		.. translate("For supported codes look here") 
		.. [[</a>]]
df.template	= "ddns/global_value"
df.default	= "%F %R"
df.date_string	= ""
function df.cfgvalue(self, section)
	local value = AbstractValue.cfgvalue(self, section) or self.default
	local epoch = os.time()
	self.date_string = DDNS.epoch2date(epoch, value)
	return value
end
function df.parse(self, section, novld)
	DDNS.value_parse(self, section, novld)
end

-- ddns_rundir  -- #############################################################
local rd	= ns:option(Value, "ddns_rundir")
rd.title	= translate("Status directory")
rd.description	= translate("Directory contains PID and other status information for each running section")
rd.default	= "/var/run/ddns"
-- no need to validate. if empty default is used everything else created by dns-scripts
function rd.parse(self, section, novld)
	DDNS.value_parse(self, section, novld)
end

-- ddns_logdir  -- #############################################################
local ld	= ns:option(Value, "ddns_logdir")
ld.title	= translate("Log directory")
ld.description	= translate("Directory contains Log files for each running section")
ld.default	= "/var/log/ddns"
-- no need to validate. if empty default is used everything else created by dns-scripts
function ld.parse(self, section, novld)
	DDNS.value_parse(self, section, novld)
end

-- ddns_loglines  -- ###########################################################
local ll	= ns:option(Value, "ddns_loglines")
ll.title	= translate("Log length")
ll.description	= translate("Number of last lines stored in log files")
ll.default	= "250"
function ll.validate(self, value)
	local n = tonumber(value)
	if not n or math.floor(n) ~= n or n < 1 then
		return nil, self.title .. ": " .. translate("minimum value '1'")
	end
	return value
end
function ll.parse(self, section, novld)
	DDNS.value_parse(self, section, novld)
end

-- use_curl  -- ################################################################
if (SYS.call([[ grep -i "\+ssl" /usr/bin/wget >/dev/null 2>&1 ]]) == 0) 
and NXFS.access("/usr/bin/curl") then
	local pc	= ns:option(Flag, "use_curl")
	pc.title	= translate("Use cURL")
	pc.description	= translate("If both cURL and GNU Wget are installed, Wget is used by default.")
		.. [[<br />]]
		.. translate("To use cURL activate this option.")
	pc.orientation	= "horizontal"
	pc.default	= "0"
end

return m
