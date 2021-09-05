-- Copyright 2008 Aleksandar Krsteski <alekrsteski@gmail.com>
-- Licensed to the public under the Apache License 2.0.

m = Map("polipo", translate("Polipo"),
	translate("Polipo is a small and fast caching web proxy."))

-- General section
s = m:section(NamedSection, "general", "polipo", translate("Proxy"))

s:tab("general", translate("General Settings"))
s:tab("dns",     translate("DNS and Query Settings"))
s:tab("proxy",   translate("Parent Proxy"))
s:tab("logging", translate("Logging and RAM"))

-- General settings
s:taboption("general", Flag, "enabled", translate("enable"))

o = s:taboption("general", Value, "proxyAddress", translate("Listen address"),
	translate("The interface on which Polipo will listen. To listen on all " ..
		"interfaces use 0.0.0.0 or :: (IPv6)."))

o.placeholder = "0.0.0.0"
o.datatype = "ipaddr"


o = s:taboption("general", Value, "proxyPort", translate("Listen port"),
	translate("Port on which Polipo will listen"))

o.optional = true
o.placeholder = "8123"
o.datatype = "port"


o = s:taboption("general", DynamicList, "allowedClients",
	translate("Allowed clients"),
	translate("When listen address is set to 0.0.0.0 or :: (IPv6), you must " ..
		"list clients that are allowed to connect. The format is IP address " ..
		"or network address (192.168.1.123, 192.168.1.0/24, " ..
		"2001:660:116::/48 (IPv6))"))

o.datatype = "ipaddr"
o.placeholder = "0.0.0.0/0"


-- DNS settings
dns = s:taboption("dns", Value, "dnsNameServer", translate("DNS server address"),
	translate("Set the DNS server address to use, if you want Polipo to use " ..
		"different DNS server than the host system."))

dns.optional = true
dns.datatype = "ipaddr"

l = s:taboption("dns", ListValue, "dnsQueryIPv6",
	translate("Query DNS for IPv6"))

l.default = "happily"
l:value("true", translate("Query only IPv6"))
l:value("happily", translate("Query IPv4 and IPv6, prefer IPv6"))
l:value("reluctantly", translate("Query IPv4 and IPv6, prefer IPv4"))
l:value("false", translate("Do not query IPv6"))


l = s:taboption("dns", ListValue, "dnsUseGethostbyname",
	translate("Query DNS by hostname"))

l.default = "reluctantly"
l:value("true", translate("Always use system DNS resolver"))
l:value("happily",
	translate("Query DNS directly, for unknown hosts fall back " ..
		"to system resolver"))
l:value("reluctantly",
	translate("Query DNS directly, fallback to system resolver"))
l:value("false", translate("Never use system DNS resolver"))


-- Proxy settings
o = s:taboption("proxy", Value, "parentProxy",
	translate("Parent proxy address"),
	translate("Parent proxy address (in host:port format), to which Polipo " ..
		"will forward the requests."))

o.optional = true
o.datatype = "hostport"


o = s:taboption("proxy", Value, "parentAuthCredentials",
	translate("Parent proxy authentication"),
	translate("Basic HTTP authentication supported. Provide username and " ..
		"password in username:password format."))

o.optional = true
o.placeholder = "username:password"


-- Logging
s:taboption("logging", Flag, "logSyslog", translate("Log to syslog"))

s:taboption("logging", Value, "logFacility",
	translate("Syslog facility")):depends("logSyslog", "1")


v = s:taboption("logging", Value, "logFile",
	translate("Log file location"),
	translate("Use of external storage device is recommended, because the " ..
		"log file is written frequently and can grow considerably."))

v:depends("logSyslog", "")
v.rmempty = true


o = s:taboption("logging", Value, "chunkHighMark",
	translate("In RAM cache size (in bytes)"),
	translate("How much RAM should Polipo use for its cache."))

o.datatype = "uinteger"


-- Disk cache section
s = m:section(NamedSection, "cache", "polipo", translate("On-Disk Cache"))
s:tab("general",  translate("General Settings"))
s:tab("advanced", translate("Advanced Settings"))


-- Disk cache settings
s:taboption("general", Value, "diskCacheRoot", translate("Disk cache location"),
	translate("Location where polipo will cache files permanently. Use of " ..
		"external storage devices is recommended, because the cache can " ..
		"grow considerably. Leave it empty to disable on-disk " ..
		"cache.")).rmempty = true


s:taboption("general", Flag, "cacheIsShared", translate("Shared cache"),
	translate("Enable if cache (proxy) is shared by multiple users."))


o = s:taboption("advanced", Value, "diskCacheTruncateSize",
	translate("Truncate cache files size (in bytes)"),
	translate("Size to which cached files should be truncated"))

o.optional = true
o.placeholder = "1048576"
o.datatype = "uinteger"


o = s:taboption("advanced", Value, "diskCacheTruncateTime",
	translate("Truncate cache files time"),
	translate("Time after which cached files will be truncated"))

o.optional = true
o.placeholder = "4d12h"


o = s:taboption("advanced", Value, "diskCacheUnlinkTime",
	translate("Delete cache files time"),
	translate("Time after which cached files will be deleted"))

o.optional = true
o.placeholder = "32d"


-- Poor man's multiplexing section
s = m:section(NamedSection, "pmm", "polipo",
	translate("Poor Man's Multiplexing"),
	translate("Poor Man's Multiplexing (PMM) is a technique that simulates " ..
		"multiplexing by requesting an instance in multiple segments. It " ..
		"tries to lower the latency caused by the weakness of HTTP " ..
		"protocol. NOTE: some sites may not work with PMM enabled."))

s:option(Value, "pmmSize", translate("PMM segments size (in bytes)"),
	translate("To enable PMM, PMM segment size must be set to some " ..
		"positive value.")).rmempty = true

s:option(Value, "pmmFirstSize", translate("First PMM segment size (in bytes)"),
	translate("Size of the first PMM segment. If not defined, it defaults " ..
		"to twice the PMM segment size.")).rmempty = true

return m
