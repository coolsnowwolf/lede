-- Copyright 2020 Weizheng Li <lwz322@qq.com>
-- Licensed to the public under the MIT License.

local uci = require "luci.model.uci".cursor()
local util = require "luci.util"
local fs = require "nixio.fs"
local sys = require "luci.sys"

local m, s, o
local server_table = { }

local function frps_version()
	local file = uci:get("frps", "main", "client_file")

	if not file or file == "" or not fs.stat(file) then
		return "<em style=\"color: red;\">%s</em>" % translate("Invalid client file")
	end

	if not fs.access(file, "rwx", "rx", "rx") then
		fs.chmod(file, 755)
	end

	local version = util.trim(sys.exec("%s -v 2>/dev/null" % file))
	if version == "" then
		return "<em style=\"color: red;\">%s</em>" % translate("Can't get client version")
	end
	return translatef("Version: %s", version)
end

m = Map("frps", "%s - %s" % { translate("Frps"), translate("Common Settings") },
"<p>%s</p><p>%s</p>" % {
	translate("Frp is a fast reverse proxy to help you expose a local server behind a NAT or firewall to the internet."),
	translatef("For more information, please visit: %s",
		"<a href=\"https://github.com/fatedier/frp\" target=\"_blank\">https://github.com/fatedier/frp</a>")
})

m:append(Template("frps/status_header"))

s = m:section(NamedSection, "main", "frps")
s.addremove = false
s.anonymous = true

s:tab("general", translate("General Options"))
s:tab("advanced", translate("Advanced Options"))
s:tab("dashboard", translate("Dashboard Options"))

o = s:taboption("general", Flag, "enabled", translate("Enabled"))

o = s:taboption("general", Value, "client_file", translate("Client file"), frps_version())
o.datatype = "file"
o.rmempty = false

o = s:taboption("general", ListValue, "run_user", translate("Run daemon as user"))
o:value("", translate("-- default --"))
local user
for user in util.execi("cat /etc/passwd | cut -d':' -f1") do
	o:value(user)
end

o = s:taboption("general", Flag, "enable_logging", translate("Enable logging"))

o = s:taboption("general", Value, "log_file", translate("Log file"))
o:depends("enable_logging", "1")
o.placeholder = "/var/log/frps.log"

o = s:taboption("general", ListValue, "log_level", translate("Log level"))
o:depends("enable_logging", "1")
o:value("trace", translate("Trace"))
o:value("debug", translate("Debug"))
o:value("info", translate("Info"))
o:value("warn", translate("Warn"))
o:value("error", translate("Error"))
o.default = "warn"

o = s:taboption("general", Value, "log_max_days", translate("Log max days"))
o:depends("enable_logging", "1")
o.datatype = "uinteger"
o.placeholder = '3'

o = s:taboption("general", Value, "disable_log_color", translate("Disable log color"))
o:depends("enable_logging", "1")
o.enabled = "true"
o.disabled = "false"

o = s:taboption("advanced", Value, "max_pool_count", translate("Max pool count"),
	translate("pool_count in each proxy will change to max_pool_count if they exceed the maximum value"))
o.datatype = "uinteger"

o = s:taboption("advanced", Value, "max_ports_per_client", translate("Max ports per-client"),
	translate("max ports can be used for each client, default value is 0 means no limit"))
o.datatype = "uinteger"
o.defalut = '0'
o.placeholder = '0'

o = s:taboption("advanced", Value, "subdomain_host", translate("Subdomain host"), 
	translatef("if subdomain_host is not empty, you can set subdomain when type is http or https in frpc's configure file; when subdomain is test, the host used by routing is test.frps.com"))
o.datatype = "host"

o = s:taboption("dashboard", Value, "dashboard_addr", translate("Dashboard addr"), translatef("dashboard addr's default value is same with bind_addr"))
o.datatype = "host"

o = s:taboption("dashboard", Value, "dashboard_port", translate("Dashboard port"), translatef("dashboard is available only if dashboard_port is set"))
o.datatype = "port"

o = s:taboption("dashboard", Value, "dashboard_user", translate("Dashboard user"), translatef("dashboard user and passwd for basic auth protect, if not set, both default value is admin"))

o = s:taboption("dashboard", Value, "dashboard_pwd", translate("Dashboard password"))
o.password = true

return m
