--[[
LuCI - Lua Configuration Interface - Aria2 support

Copyright 2016 maz-1 <ohmygod19993@gmail.com>
]]--

local sys  = require "luci.sys"
local util = require "luci.util"
local uci = require "luci.model.uci".cursor()

ptype = {
	"socks5",
	"socks4",
	"http",
	"socks4a",
}

whocan = {
	"anyone",
	"friends",
	"nobody",
}

function titlesplit(Value)
    return "<p style=\"font-size:20px;font-weight:bold;color: DodgerBlue\">" .. translate(Value) .. "</p>"
end

m = Map("amule", translate("aMule"), translate("aMule is a ED2K/KAD client for all platforms.") .. "<br/><a href=\"https://github.com/maz-1\">luci interface by maz-1</a>")

m:section(SimpleSection).template  = "amule/overview_status"

s = m:section(TypedSection, "amule", translate("aMule Settings"))
s.addremove = false
s.anonymous = true

s:tab("general", translate("General"))
s:tab("connection", translate("Connections"))
s:tab("server", translate("Server"))
s:tab("path_and_file", translate("Path And File"))
s:tab("security", translate("Security"))
s:tab("remote", translate("External Control"))
s:tab("template", translate("Edit Template"))
s:tab("logview", translate("Log File Viewer"))
s:tab("amulecmd", translate("aMule command"))

-- GENERAL --

o = s:taboption("general", Flag, "enabled", translate("Enabled"))
o.rmempty = false

user = s:taboption("general", ListValue, "runasuser", translate("Run daemon as user"))
local p_user
for _, p_user in util.vspairs(util.split(sys.exec("cat /etc/passwd | cut -f 1 -d :"))) do
        user:value(p_user)
end

o = s:taboption("general", Value, "config_dir", translate("Configuration directory"))
o.rmempty = false
o.placeholder = "/var/run/amule"

o = s:taboption("general", Value, "mem_percentage", translate("Memory Limit"), translate("Percentage"))
o.rmempty = false
o.placeholder = "50"
o.datatype = "range(1, 99)"

o = s:taboption("general", Value, "nick", translate("Nickname"))
o.placeholder = "http://www.aMule.org"

o = s:taboption("general", Value, "max_upload", translate("Max upload speed"), translate("Unlimited when set to 0"))
o.datatype = "uinteger"
o.rmempty = false
o.placeholder = "0"

o = s:taboption("general", Value, "max_download", translate("Max download speed"), translate("Unlimited when set to 0"))
o.datatype = "uinteger"
o.rmempty = false
o.placeholder = "0"

o = s:taboption("general", Value, "slot_allocation", translate("Slot allocation"))
o.datatype = "uinteger"
o.rmempty = false
o.placeholder = "2"

o = s:taboption("general", Value, "max_connections", translate("Max connections"))
o.datatype = "uinteger"
o.rmempty = false
o.placeholder = "500"

o = s:taboption("general", Value, "max_sources_per_file", translate("Max sources per file"))
o.datatype = "uinteger"
o.rmempty = false
o.placeholder = "300"

-- CONNECTIONS --

o = s:taboption("connection", Value, "port", translate("TCP port"))
o.datatype = "port"
o.rmempty = false
o.placeholder = "4662"

o = s:taboption("connection", Flag, "udp_enable", translate("Enable UDP port"))
o.rmempty = false

o = s:taboption("connection", Value, "udp_port", translate("UDP port"))
o.datatype = "port"
o.rmempty = false
o.placeholder = "4672"

o = s:taboption("connection", Flag, "upnp_enabled", translate("Enable UPnP"))
o.rmempty = false

o = s:taboption("connection", Value, "upnp_tcp_port", translate("UPnP TCP port"))
o.datatype = "port"
o.rmempty = false
o.placeholder = "50000"

o = s:taboption("connection", Value, "address", translate("Bind Address"), translate("Leave blank to bind all"))
o.datatype = "ip4addr"
o.rmempty = true

o = s:taboption("connection", Flag, "auto_connect", translate("Automatically connect"))
o.rmempty = false

o = s:taboption("connection", Flag, "reconnect", translate("Automatically reconnect"))
o.rmempty = false

o = s:taboption("connection", Flag, "connect_to_kad", translate("Connect to Kad network"))
o.rmempty = false

o = s:taboption("connection", Flag, "connect_to_ed2k", translate("Connect to ED2K network"))
o.rmempty = false

s:taboption("connection", DummyValue,"titlesplit1" ,titlesplit(translate("Proxy Configuration")))

o = s:taboption("connection", Flag, "proxy_enable_proxy", translate("Enable proxy"))
o.rmempty = false

o = s:taboption("connection", ListValue, "proxy_type", translate("Proxy type"))
for i,v in ipairs(ptype) do
	o:value(v)
end
o.rmempty = false

o = s:taboption("connection", Value, "proxy_name", translate("Proxy name"))
o.rmempty = true

o = s:taboption("connection", Value, "proxy_port", translate("Proxy port"))
o.datatype = "port"
o.rmempty = true

o = s:taboption("connection", Flag, "proxy_enable_password", translate("Proxy requires authentication"))
o.rmempty = true

o = s:taboption("connection", Value, "proxy_user", translate("Proxy user"))
--o:depends("proxy_enable_password", "1")
o.rmempty = true

o = s:taboption("connection", Value, "proxy_password", translate("Proxy password"))
o.password = true
o.rmempty = true

-- SERVER --

o = s:taboption("server", Value, "kad_nodes_url", translate("Kad Nodes Url"), "<input type=\"button\" size=\"0\" title=\"" 
    .. translate("Download now") .. "\" onclick=\"onclick_down_kad(this.id)\" "
    .. "value=\"&#10597;&#10597;&#10597;\" "
    .. "style=\"font-weight:bold;text-decoration:overline;\""
    .. "/>")
o.rmempty = false
o.placeholder = "http://upd.emule-security.org/nodes.dat"

o = s:taboption("server", Value, "ed2k_servers_url", translate("Ed2k Servers List Url"), "<input type=\"button\" size=\"0\" title=\"" 
    .. translate("Download now") .. "\" onclick=\"onclick_down_ed2k(this.id)\" "
    .. "value=\"&#10597;&#10597;&#10597;\" "
    .. "style=\"font-weight:bold;text-decoration:overline;\""
    .. "/>")
o.rmempty = false
o.placeholder = "http://upd.emule-security.org/server.met"

o = s:taboption("server", Flag, "remove_dead_server", translate("Remove Dead Server"))
o.rmempty = false

o = s:taboption("server", Value, "dead_server_retry", translate("Dead Server Retry"))
--o:depends("remove_dead_server", "1")
o.datatype = "uinteger"
o.rmempty = false
o.placeholder = "3"
o.default = "3"

o = s:taboption("server", Flag, "add_server_list_from_server", translate("Update server list when connecting to a server"))
o.rmempty = false

o = s:taboption("server", Flag, "add_server_list_from_client", translate("Update server list when a client connects"))
o.rmempty = false

o = s:taboption("server", Flag, "scoresystem", translate("Use priority system"))
o.rmempty = false

o = s:taboption("server", Flag, "smart_id_check", translate("Use smart LowID check on connect"))
o.rmempty = false

o = s:taboption("server", Flag, "safe_server_connect", translate("Safe connect"))
o.rmempty = false

o = s:taboption("server", Flag, "auto_connect_static_only", translate("Auto connect to servers in static list only"))
o.rmempty = false

o = s:taboption("server", Flag, "manual_high_prio", translate("Set manually added servers to high priority"))
o.rmempty = false

o = s:taboption("server", Flag, "serverlist", translate("Auto update server list at startup"), translate("addresses.dat file"))
o.rmempty = false

addr = s:taboption("server", Value, "addresses", translate("Server addresses"), 
                        translate("Content of addresses.dat. One address per line"))
addr:depends("serverlist", "1")
addr.template = "cbi/tvalue"
addr.rows = 5
addr.rmempty = true
function addr.cfgvalue(self, section)
	return nixio.fs.readfile("/etc/amule/addresses.dat")
end

function addr.write(self, section, value)
	value = value:gsub("\r\n?", "\n")
	nixio.fs.writefile("//etc/amule/addresses.dat", value)
end

-- PATH AND FILE --

o = s:taboption("path_and_file", Value, "temp_dir", translate("Temporary directory"))
o.rmempty = false
o.placeholder = "/var/run/amule/.aMule/Temp"

o = s:taboption("path_and_file", Value, "incoming_dir", translate("Incoming directory"))
o.rmempty = false
o.placeholder = "/var/run/amule/.aMule/Incoming"


shareddir = s:taboption("path_and_file", Value, "shareddir", translate("Shared directory"), 
                translate("Content of shareddir.dat. One directory per line"))
shareddir.template = "cbi/tvalue"
shareddir.titleref = luci.dispatcher.build_url("admin", "system", "fstab")
shareddir.rows = 5
shareddir.rmempty = true
function shareddir.cfgvalue(self, section)
	return nixio.fs.readfile("/etc/amule/shareddir.dat")
end

function shareddir.write(self, section, value)
	value = value:gsub("\r\n?", "\n")
	nixio.fs.writefile("//etc/amule/shareddir.dat", value)
end

o = s:taboption("path_and_file", Flag, "ich", translate("Intelligent corruption handling (I.C.H.)"))
o.rmempty = false

o = s:taboption("path_and_file", Flag, "a_ich_trust", translate("Advanced I.C.H trusts every hash (not recommended)"))
o.rmempty = false

o = s:taboption("path_and_file", Flag, "add_new_files_paused", translate("Add files to download in pause mode"))
o.rmempty = false

o = s:taboption("path_and_file", Flag, "dap_pref", translate("Add files to download with auto priority"))
o.rmempty = false

o = s:taboption("path_and_file", Flag, "start_next_file", translate("Start next paused file when a file completes"))
o.rmempty = false

o = s:taboption("path_and_file", Flag, "start_next_file_same_cat", translate("Start next paused file from the same category"))
o:depends("start_next_file", "1")
o.rmempty = true

o = s:taboption("path_and_file", Flag, "start_next_file_alpha", translate("Start next paused file in alphabetic order"))
o:depends("start_next_file", "1")
o.rmempty = true

o = s:taboption("path_and_file", Flag, "allocate_full_file", translate("Preallocate disk space for new files"))
o.rmempty = false

o = s:taboption("path_and_file", Value, "min_free_disk_space", translate("Minimum free disk space. in Mbytes"))
o.datatype = "uinteger"
o.placeholder = "1"
o.rmempty = false

o = s:taboption("path_and_file", Flag, "use_src_seed", translate("Save 10 sources on rare files (< 20 sources)"))
o.rmempty = false

o = s:taboption("path_and_file", Flag, "uap_pref", translate("Add new shares with auto priority"))
o.rmempty = false


-- SECURITY --


o = s:taboption("security", Flag, "use_sec_ident", translate("Use secure user identification"))
o.rmempty = false

o = s:taboption("security", Flag, "is_crypt_layer_requested", translate("Use obfuscation for outgoing connections"))
o.rmempty = false

o = s:taboption("security", Flag, "is_client_crypt_layer_required", translate("Accept only obfuscation connections"))
o.rmempty = false

o = s:taboption("security", ListValue, "see_share", translate("Who can see my shared files"))
for i,v in ipairs(whocan) do
	o:value(v)
end
o.rmempty = false

s:taboption("security", DummyValue,"titlesplit2" ,titlesplit(translate("IP Filter Configuration")))

shareddir = s:taboption("security", Value, "ipfilter_static", translate("Static IP list for filtering"), 
                translate("Content of ipfilter_static.dat"))
shareddir.template = "cbi/tvalue"
shareddir.titleref = luci.dispatcher.build_url("admin", "system", "fstab")
shareddir.rows = 5
shareddir.rmempty = true
function shareddir.cfgvalue(self, section)
	return nixio.fs.readfile("/etc/amule/ipfilter_static.dat")
end

function shareddir.write(self, section, value)
	value = value:gsub("\r\n?", "\n")
	nixio.fs.writefile("//etc/amule/ipfilter_static.dat", value)
end

o = s:taboption("security", Flag, "ip_filter_clients", translate("Filter clients by IP"))
o.rmempty = false

o = s:taboption("security", Flag, "ip_filter_servers", translate("Filter servers by IP"))
o.rmempty = false

o = s:taboption("security", Value, "ip_filter_url", translate("IP filter list URL"))
o.rmempty = true

o = s:taboption("security", Flag, "ip_filter_auto_load", translate("Auto-update ipfilter at startup"))
o.rmempty = false

o = s:taboption("security", Value, "filter_level", translate("Filtering Level"))
o.datatype = "range(1, 255)"
o.rmempty = false
o.placeholder = "127"

o = s:taboption("security", Flag, "filter_lan_ips", translate("Always filter LAN IPs"))
o.rmempty = false

o = s:taboption("security", Flag, "paranoid_filtering", translate("Paranoid handling of non-matching IPs"))
o.rmempty = false

o = s:taboption("security", Flag, "ip_filter_system", translate("Use system-wide ipfilter.dat if available"))
o.rmempty = false

-- REMOTE CONTROL --

o = s:taboption("remote", Value, "ec_address", translate("IP of the listening interface for external connection"))
o.datatype = "ip4addr"
o.placeholder = "127.0.0.1"
o.rmempty = true

o = s:taboption("remote", Value, "ec_port", translate("TCP port for EC"))
o.datatype = "port"
o.placeholder = "4712"
o.rmempty = false

o = s:taboption("remote", Flag, "upnp_ec_enabled", translate("Enable upnp port forwarding on the EC port"))
o.rmempty = false

o = s:taboption("remote", Value, "ec_password", translate("EC password"))
o.password  =  true
o.rmempty = false

s:taboption("remote", DummyValue,"titlesplit3", titlesplit(translate("aMule Web Configuration")))

o = s:taboption("remote", Flag, "web_enabled", translate("Enable web server on startup"))
o.rmempty = false

o = s:taboption("remote", Value, "template", translate("Web template"))
o.rmempty = false
local tpth_suggestions = luci.sys.exec("ls /usr/share/amule/webserver/|sed ':a;N;$!ba;s/\\n/:/g'")
if tpth_suggestions then
	for entry in string.gmatch(tpth_suggestions, "[^:]+") do
		o:value(entry)
	end
end

o = s:taboption("remote", Value, "web_password", translate("Web full rights password"))
o.password  =  true
o.rmempty = true

o = s:taboption("remote", Flag, "use_low_rights_user", translate("Use low rights user"))
o.rmempty = false

o = s:taboption("remote", Value, "password_low", translate("Web low rights password"))
o.password  =  true
o.rmempty = true

o = s:taboption("remote", Value, "web_port", translate("Web TCP port"))
o.datatype = "port"
o.placeholder = "4711"
o.rmempty = false

o = s:taboption("remote", Flag, "upnp_web_server_enabled", translate("Enable UPnP port forwarding of the web server port"))
o.rmempty = false

o = s:taboption("remote", Value, "web_upnp_tcp_port", translate("Web UPnP TCP port"))
o.datatype = "port"
o.placeholder = "50001"
o.rmempty = false

o = s:taboption("remote", Value, "page_refresh_time", translate("Page refresh time(in secs)"))
o.datatype = "range(1, 600)"
o.rmempty = false
o.placeholder = "121"

o = s:taboption("remote", Flag, "use_gzip", translate("Enable Gzip compression"))
o.rmempty = false


-- TEMPLATE --


tmpl = s:taboption("template", Value, "_tmpl",
	translate("Edit the template that is used for generating the aMule configuration."), 
	translate("This is the content of the file '/etc/amule/amule.conf.template' from which your amule configuration will be generated. " ..
		"Values enclosed by pipe symbols ('|') should not be changed. They get their values from other tabs."))

tmpl.template = "cbi/tvalue"
tmpl.rows = 20

function tmpl.cfgvalue(self, section)
	return nixio.fs.readfile("/etc/amule/amule.conf.template")
end

function tmpl.write(self, section, value)
	value = value:gsub("\r\n?", "\n")
	nixio.fs.writefile("//etc/amule/amule.conf.template", value)
end

-- LOGVIEW --
        
local lv	= s:taboption("logview", DummyValue, "_logview")
lv.template	= "amule/detail_logview"
lv.inputtitle	= translate("Read / Reread log file")
lv.rows		= 50

function lv.cfgvalue(self, section)
	return translate("Please press [Read] button")
end

-- AMULECMD --
        
local cmd	= s:taboption("amulecmd", DummyValue, "_amulecmd")
cmd.template	= "amule/webshell"

return m
