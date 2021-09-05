m = Map("simple-adblock", translate("Simple AdBlock Settings"))
s = m:section(NamedSection, "config", "simple-adblock")

-- General options
e = s:option(Flag, "enabled", translate("Start Simple Adblock service"))
e.rmempty  = false
function e.write(self, section, value)
	if value ~= "1" then
		luci.sys.init.stop("simple-adblock")
	end
	return Flag.write(self, section, value)
end

o2 = s:option(ListValue, "verbosity", translate("Output Verbosity Setting"),translate("Controls system log and console output verbosity"))
o2:value("0", translate("Suppress output"))
o2:value("1", translate("Some output"))
o2:value("2", translate("Verbose output"))
o2.rmempty = false
o2.default = 2

o3 = s:option(ListValue, "force_dns", translate("Force Router DNS"), translate("Forces Router DNS use on local devices, also known as DNS Hijacking"))
o3:value("0", translate("Let local devices use their own DNS servers if set"))
o3:value("1", translate("Force Router DNS server to all local devices"))
o3.rmempty = false
o3.default = 1

local sysfs_path = "/sys/class/leds/"
local leds = {}
if nixio.fs.access(sysfs_path) then
	leds = nixio.util.consume((nixio.fs.dir(sysfs_path)))
end
if #leds ~= 0 then
	o3 = s:option(Value, "led", translate("LED to indicate status"), translate("Pick the LED not already used in")
		.. [[ <a href="]] .. luci.dispatcher.build_url("admin/system/leds") .. [[">]]
		.. translate("System LED Configuration") .. [[</a>]])
	o3.rmempty = true
	o3:value("", translate("none"))
	for k, v in ipairs(leds) do
		o3:value(v)
	end
end

s2 = m:section(NamedSection, "config", "simple-adblock")
-- Whitelisted Domains
d1 = s2:option(DynamicList, "whitelist_domain", translate("Whitelisted Domains"), translate("Individual domains to be whitelisted"))
d1.addremove = false
d1.optional = false

-- Blacklisted Domains
d3 = s2:option(DynamicList, "blacklist_domain", translate("Blacklisted Domains"), translate("Individual domains to be blacklisted"))
d3.addremove = false
d3.optional = false

-- Whitelisted Domains URLs
d2 = s2:option(DynamicList, "whitelist_domains_url", translate("Whitelisted Domain URLs"), translate("URLs to lists of domains to be whitelisted"))
d2.addremove = false
d2.optional = false

-- Blacklisted Domains URLs
d4 = s2:option(DynamicList, "blacklist_domains_url", translate("Blacklisted Domain URLs"), translate("URLs to lists of domains to be blacklisted"))
d4.addremove = false
d4.optional = false

-- Blacklisted Hosts URLs
d5 = s2:option(DynamicList, "blacklist_hosts_url", translate("Blacklisted Hosts URLs"), translate("URLs to lists of hosts to be blacklisted"))
d5.addremove = false
d5.optional = false

return m
