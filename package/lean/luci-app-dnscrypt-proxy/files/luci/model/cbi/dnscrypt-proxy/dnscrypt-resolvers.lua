-- Copyright (C) 2019 github.com/peter-tank
-- Licensed to the public under the GNU General Public License v3.

local m, _, s, o, id, cfg, src, val, k, v
local dc = require "luci.tools.dnscrypt".init()
local resolvers = dc:resolvers_list(true)
local disp = require "luci.dispatcher"

cfg = "dnscrypt-proxy"

local dnslist_table = {}
for _, s in pairs(dc:dns_list()) do
	if s.name ~= nil then
		dnslist_table[#dnslist_table+1] = "%s.%s:%s@%s://%s:%s" %{s.resolver, s.country, s.name,  s.proto, s.addrs, s.ports}
	end
end

-- [[ Servers Setting ]]--
m = Form(cfg, translate("DNSCrypt Servers"))

local type = "dnscrypt-proxy"
s = m:section(NamedSection, 'ns1', type, translate("Choose a resolver to configure"), translate("Input the name to re-configure, if your resolvers not updated correctly and not showns up."))
s.cfgvalue = function(self, section) return resolvers end
s.template = "dnscrypt-proxy/cfgselection"
s.extedit = luci.dispatcher.build_url("admin/services/dnscrypt-proxy/dnscrypt-resolvers", "%s")

--s = m:section(NamedSection, 'ns1', type, translate("DNSCrypt Resolvers update"))

o = s:option(TextValue, "_Dummy", translate("DNSCrypt Resolver Info"), translate("Total %d records.") % #dnslist_table)
o.rows = 7
o.readonly = true
o.wrap = "soft"
o.cfgvalue = function (self, sec)
local ret = translate("Available Resolvers:")
for k, v in pairs(dnslist_table) do ret = "%s\n%03d) %s" % {ret, k, v} end
return ret
end
o.write = function (...) end

o = s:option(Button,"resolvers_up", translate("Update DNSCrypt Resolvers"))
o.rawhtml  = true
o.template = "dnscrypt-proxy/refresh"
o.ret = translate("Update")
o.value = " %d %s" % {#resolvers, translate("Resolvers")}
o.description = translate("Retrieve DNSCrypt resolvers: ") .. "https://download.dnscrypt.info/dnscrypt-proxy/{*}.[md|minisig|json]"

o = s:option(Button, "dnslist_up", translate("Refresh Resolver Info"))
o.rawhtml  = true
o.template = "dnscrypt-proxy/refresh"
o.name = translate("Update")
o.ret = translate("Resolver")
o.value = " %d %s" % {#dnslist_table, translate("Records")}
o.description = translate("Reload DNS resolver info: ") .. "/usr/share/dnscrypt-proxy/{*}.[md|json]"
o.inputstyle = "apply"

o = s:option(Button, "dnscrypt_ck", translate("DNS resolve test"))
o.rawhtml  = true
o.template = "dnscrypt-proxy/resolve"
o.description = translate("Check DNSCrypt resolver: ") .. "/usr/sbin/dnscrypt-proxy -resolve www.google.com"

return m
