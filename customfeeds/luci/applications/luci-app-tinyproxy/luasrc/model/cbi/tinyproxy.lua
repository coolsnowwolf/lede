-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008-2010 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

m = Map("tinyproxy", translate("Tinyproxy"),
	translate("Tinyproxy is a small and fast non-caching HTTP(S)-Proxy"))

s = m:section(TypedSection, "tinyproxy", translate("Server Settings"))
s.anonymous = true

s:tab("general", translate("General settings"))
s:tab("privacy", translate("Privacy settings"))
s:tab("filter", translate("Filtering and ACLs"))
s:tab("limits", translate("Server limits"))


o = s:taboption("general", Flag, "enabled", translate("Enable Tinyproxy server"))
o.rmempty = false

function o.write(self, section, value)
	if value == "1" then
		luci.sys.init.enable("tinyproxy")
	else
		luci.sys.init.disable("tinyproxy")
	end

	return Flag.write(self, section, value)
end


o = s:taboption("general", Value, "Port", translate("Listen port"),
	translate("Specifies the HTTP port Tinyproxy is listening on for requests"))

o.optional = true
o.datatype = "port"
o.placeholder = 8888


o = s:taboption("general", Value, "Listen", translate("Listen address"),
	translate("Specifies the addresses Tinyproxy is listening on for requests"))

o.optional = true
o.datatype = "ipaddr"
o.placeholder = "0.0.0.0"


o = s:taboption("general", Value, "Bind", translate("Bind address"),
	translate("Specifies the address Tinyproxy binds to for outbound forwarded requests"))

o.optional = true
o.datatype = "ipaddr"
o.placeholder = "0.0.0.0"


o = s:taboption("general", Value, "DefaultErrorFile", translate("Error page"),
	translate("HTML template file to serve when HTTP errors occur"))

o.optional = true
o.default = "/usr/share/tinyproxy/default.html"


o = s:taboption("general", Value, "StatFile", translate("Statistics page"),
	translate("HTML template file to serve for stat host requests"))

o.optional = true
o.default = "/usr/share/tinyproxy/stats.html"


o = s:taboption("general", Flag, "Syslog", translate("Use syslog"),
	translate("Writes log messages to syslog instead of a log file"))


o = s:taboption("general", Value, "LogFile", translate("Log file"),
	translate("Log file to use for dumping messages"))

o.default = "/var/log/tinyproxy.log"
o:depends("Syslog", "")


o = s:taboption("general", ListValue, "LogLevel", translate("Log level"),
	translate("Logging verbosity of the Tinyproxy process"))

o:value("Critical")
o:value("Error")
o:value("Warning")
o:value("Notice")
o:value("Connect")
o:value("Info")


o = s:taboption("general", Value, "User", translate("User"),
	translate("Specifies the user name the Tinyproxy process is running as"))

o.default = "nobody"


o = s:taboption("general", Value, "Group", translate("Group"),
	translate("Specifies the group name the Tinyproxy process is running as"))

o.default = "nogroup"


--
-- Privacy
--

o = s:taboption("privacy", Flag, "XTinyproxy", translate("X-Tinyproxy header"),
	translate("Adds an \"X-Tinyproxy\" HTTP header with the client IP address to forwarded requests"))


o = s:taboption("privacy", Value, "ViaProxyName", translate("Via hostname"),
	translate("Specifies the Tinyproxy hostname to use in the Via HTTP header"))

o.placeholder = "tinyproxy"
o.datatype = "hostname"


s:taboption("privacy", DynamicList, "Anonymous", translate("Header whitelist"),
	translate("Specifies HTTP header names which are allowed to pass-through, all others are discarded. Leave empty to disable header filtering"))


--
-- Filter
--

o = s:taboption("filter", DynamicList, "Allow", translate("Allowed clients"),
	translate("List of IP addresses or ranges which are allowed to use the proxy server"))

o.placeholder = "0.0.0.0"
o.datatype = "ipaddr"


o = s:taboption("filter", DynamicList, "ConnectPort", translate("Allowed connect ports"),
	translate("List of allowed ports for the CONNECT method. A single value \"0\" allows all ports"))

o.placeholder = 0
o.datatype = "port"


s:taboption("filter", FileUpload, "Filter", translate("Filter file"),
	translate("Plaintext file with URLs or domains to filter. One entry per line"))


s:taboption("filter", Flag, "FilterURLs", translate("Filter by URLs"),
	translate("By default, filtering is done based on domain names. Enable this to match against URLs instead"))


s:taboption("filter", Flag, "FilterExtended", translate("Filter by RegExp"),
	translate("By default, basic POSIX expressions are used for filtering. Enable this to activate extended regular expressions"))


 s:taboption("filter", Flag, "FilterCaseSensitive", translate("Filter case-sensitive"),
	translate("By default, filter strings are treated as case-insensitive. Enable this to make the matching case-sensitive"))


s:taboption("filter", Flag, "FilterDefaultDeny", translate("Default deny"),
	translate("By default, the filter rules act as blacklist. Enable this option to only allow matched URLs or domain names"))


--
-- Limits
--

o = s:taboption("limits", Value, "Timeout", translate("Connection timeout"),
	translate("Maximum number of seconds an inactive connection is held open"))

o.optional = true
o.datatype = "uinteger"
o.default = 600


o = s:taboption("limits", Value, "MaxClients", translate("Max. clients"),
	translate("Maximum allowed number of concurrently connected clients"))

o.datatype = "uinteger"
o.default = 10


o = s:taboption("limits", Value, "MinSpareServers", translate("Min. spare servers"),
	translate("Minimum number of prepared idle processes"))

o.datatype = "uinteger"
o.default = 5


o = s:taboption("limits", Value, "MaxSpareServers", translate("Max. spare servers"),
	translate("Maximum number of prepared idle processes"))

o.datatype = "uinteger"
o.default = 10


o = s:taboption("limits", Value, "StartServers", translate("Start spare servers"),
	translate("Number of idle processes to start when launching Tinyproxy"))

o.datatype = "uinteger"
o.default = 5


o = s:taboption("limits", Value, "MaxRequestsPerChild", translate("Max. requests per server"),
	translate("Maximum allowed number of requests per process. If it is exeeded, the process is restarted. Zero means unlimited."))

o.datatype = "uinteger"
o.default = 0


--
-- Upstream
--

s = m:section(TypedSection, "upstream", translate("Upstream Proxies"),
	translate("Upstream proxy rules define proxy servers to use when accessing certain IP addresses or domains."))

s.anonymous = true
s.addremove = true


t = s:option(ListValue, "type", translate("Policy"),
	translate("<em>Via proxy</em> routes requests to the given target via the specifed upstream proxy, <em>Reject access</em> disables any upstream proxy for the target"))

t:value("proxy", translate("Via proxy"))
t:value("reject", translate("Reject access"))


ta = s:option(Value, "target", translate("Target host"),
	translate("Can be either an IP address or range, a domain name or \".\" for any host without domain"))

ta.rmempty = true
ta.placeholder = "0.0.0.0/0"
ta.datatype = "host(1)"


v = s:option(Value, "via", translate("Via proxy"),
	translate("Specifies the upstream proxy to use for accessing the target host. Format is <code>address:port</code>"))

v:depends({type="proxy"})
v.placeholder = "10.0.0.1:8080"
v.datatype = "ip4addrport"

return m
