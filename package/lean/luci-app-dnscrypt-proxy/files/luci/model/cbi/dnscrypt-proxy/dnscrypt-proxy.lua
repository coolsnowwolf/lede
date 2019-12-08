-- Copyright (C) 2019 github.com/peter-tank
-- Licensed to the public under the GNU General Public License v3.

local m, _, s, o
local cfg="dnscrypt-proxy"
local sys = require "luci.sys"
local http = require "luci.http"
local disp = require "luci.dispatcher"
local fs = require "nixio.fs"

local function has_bin(name)
	return sys.call("command -v %s >/dev/null" %{name}) == 0
end
local has_dnscrypt = has_bin("dnscrypt-proxy")

local bin_version
local bin_file="/usr/sbin/dnscrypt-proxy"
if not fs.access(bin_file)  then
 bin_version=translate("Not exist")
else
 if not fs.access(bin_file, "rwx", "rx", "rx") then
   fs.chmod(bin_file, 755)
 end
 bin_version=luci.util.trim(luci.sys.exec(bin_file .. " -version"))
 if not bin_version or bin_version == "" then
     bin_version = translate("Unknown")
 end
end

m = Map(cfg, "%s - %s" %{translate("DNSCrypt Proxy"),
		translate("Golbal Setting")})

-- [[ Binary ]]--
s = m:section(SimpleSection, translate("Binary Management"), has_dnscrypt and "" or ('<b style="color:red">%s</b>' % translate("DNSCrypt Proxy binary not found.")))
o = s:option(DummyValue,"dnscrypt_bin", translate("Binary update"))
o.rawhtml  = true
o.template = "dnscrypt-proxy/refresh"
o.name = translate("Update")
o.ret = bin_version:match('^1%..*') and  translate("Version2 atleast: ") or translate("Current version: ")
o.value = o.ret .. bin_version
o.description = translate("Update to final release from: ") .. "https://github.com/dnscrypt/dnscrypt-proxy/releases"
o.write = function (self, ...) end

-- [[ Proxy Setting ]]--
local type = "dnscrypt-proxy"
s = m:section(NamedSection, 'ns1', type, translate("DNSCrypt Proxy"))
s.anonymous = false
-- section might not exist
function s.cfgvalue(self, section)
	if not self.map:get(section) then
 	 self.map:set(section, nil, self.sectiontype)
 	 self.map:set(section, "resolvers", {"public-resolvers", "opennic"})
	end
	return self.map:get(section)
end

o = s:option(Value, "listen_addresses", translate("Listening address"), translate("Split MultiValues by a comma"))
o.default = "127.0.0.1:5335"
o.placeholder = o.default
o.optional = false
o.rmempty = false

o = s:option(Value, "netprobe_address", translate("Net probe address"), translate("Resolver on downloading resolver lists file."))
o.default = "114.114.114.114:53"
o.placeholder = o.default
o.optional = false
o.rmempty = false

o = s:option(Value, "fallback_resolver", translate("Fallback resolver"), translate("DNS resolver on query fails or for forced forwarding domain list."))
o.default = "114.114.114.114:53"
o.placeholder = o.default
o.optional = false
o.rmempty = false

o = s:option(DynamicList, "resolvers", translate("Enabled Resolvers"), translate("Available Resolvers: ") .. "https://download.dnscrypt.info/dnscrypt-resolvers/v2/{*}.md")
o:value("public-resolvers", translate("public-resolvers"))
o:value("opennic", translate("opennic"))
o:value("parental-control", translate("parental-control"))
o.optional = false
o.rmempty = false
o.placeholder = "onion-services"

o = s:option(MultiValue, "force", translate("Force Options"), translate("Items forced for checking, will show your the defaults when unchecked all."))
o.optional = false
o.widget = "select"
o.force_defaults = {
["lb_estimator"] = "true",
["ignore_system_dns"] = "false",
["ipv4_servers"] = "true",
["ipv6_servers"] = "false",
["block_ipv6"] = "true",
["dnscrypt_servers"] = "true",
["doh_servers"] = "true",
["require_dnssec"] = "false",
["force_tcp"] = "false",
["require_nolog"] = "true",
["require_nofilter"] = "true",
["cache"] = "true",
["offline_mode"] = "false",
["dnscrypt_ephemeral_keys"] = "false",
["tls_disable_session_tickets"] = "false",
["cert_ignore_timestamp"] = "false",
["use_syslog"] = "false",
}
for k, v in pairs(o.force_defaults) do o:value(k, translate(k)) end
o.cfgvalue = function (self, section)
local ret, k, d
ret = Value.cfgvalue(self, section)
if ret then return ret end
ret = ""
for k, d in pairs(self.force_defaults) do
  if d == "true" then
    ret = ret .. self.delimiter .. k
  end
end
return ret
end

o = s:option(ListValue, "log_level", translate("Log output level"))
o:value(0, translate("Debug"))
o:value(1, translate("Info"))
o:value(2, translate("Notice"))
o:value(3, translate("Warning"))
o:value(4, translate("Error"))
o:value(5, translate("Critical"))
o:value(6, translate("Fatal"))
o.default = 2
o.optional = true
o.rmempty = true

o = s:option(ListValue, "blocked_query_response", translate("Response for blocked queries."))
o:value("refused", translate("refused"))
o:value("hinfo", translate("hinfo"))
o.default = "hinfo"
o.placeholder = "eg: a:<IPv4>,aaaa:<IPv6>"
o.optional = true
o.rmempty = true

o = s:option(ListValue, "lb_strategy", translate("Load-balancing strategy"))
o:value("p2", translate("p2"))
o:value("ph", translate("ph"))
o:value("first", translate("first"))
o:value("random", translate("random"))
o.default = "p2"
o.optional = true
o.rmempty = true

o = s:option(DynamicList, "forwarding_rules", translate("Forwarding2Fallback"), translate("Domains forced to fallback resolver, [.conf] file treat like dnsmasq configure."))
o.default = "/etc/dnsmasq.oversea/oversea_list.conf"
o.placeholder = "/etc/dnsmasq.oversea/oversea_list.conf"
o.optional = true
o.rmempty = true

o = s:option(DynamicList, "blacklist", translate("Domain Black List"), translate("Domains to blacklist, [.conf|.adblock] file treat like dnsmasq configure: ") .. "https://download.dnscrypt.info/blacklists/domains/mybase.txt")
o.default = "/etc/dnsmasq.ssr/ad.conf"
o.placeholder = "/usr/share/adbyby/dnsmasq.adblock"
o.optional = true
o.rmempty = true

o = s:option(DynamicList, "ip_blacklist", translate("IP Address List"), translate("IP Address to blacklist, [.conf] file treat like dnsmasq configure: ") .. "https://download.dnscrypt.info/blacklists/domains/mybase.txt")
o.default = "https://download.dnscrypt.info/blacklists/domains/mybase.txt"
o.placeholder = "/etc/dnsmasq.ssr/ad.conf"
o.optional = true
o.rmempty = true

o = s:option(DynamicList, "static", translate("Static Stamp"), translate("Mostly useful for testing your own servers."))
o.optional = true
o.rmempty = true
o.placeholder = "eg: sdns:AQcAAAAAAAAAAAAQMi5kbnNjcnlwdC1jZXJ0Lg"

o = s:option(Value, "server_names", translate("Resolver White List"), "%s %s" % {translate("Resolver white list by name, Allow *ALL* in default."), translate("Split MultiValues by a comma")})
o.optional = true
o.rmempty = true
o.placeholder = "eg: goodguy1,goodguy2"

o = s:option(Value, "disabled_server_names", translate("Resolver Black List"), "%s %s" % {translate("Resolver black list by name, disable specified resolver."), translate("Split MultiValues by a comma")})
o.optional = true
o.rmempty = true
o.placeholder = "eg: badguy1,badguy2"

o = s:option(Value, "max_clients", translate("Simulaneous"), translate("Maximum number of simultaneous client connections to accept."))
o.optional = true
o.rmempty = true
o.datatype = "uinteger"
o.default = 500
o.placeholder = o.default

o = s:option(Value, "keepalive", translate("Keep Alive"), translate("Keepalive for HTTP (HTTPS, HTTP/2) queries, in seconds."))
o.optional = true
o.rmempty = true
o.datatype = "uinteger"
o.default = 30
o.placeholder = o.default

o = s:option(Value, "cert_refresh_delay", translate("Cert refresh delay"), translate("Delay, in minutes, after which certificates are reloaded."))
o.optional = true
o.rmempty = true
o.datatype = "uinteger"
o.default = 240
o.placeholder = o.default

o = s:option(Value, "netprobe_timeout", translate("Net probe timer"), translate("Maximum time (in seconds) to wait for network connectivity before initializing the proxy."))
o.optional = true
o.rmempty = true
o.datatype = "uinteger"
o.default = 60
o.placeholder = o.default

o = s:option(Value, "reject_ttl", translate("Reject TTL"), translate("TTL for synthetic responses sent when a request has been blocked (due to IPv6 or blacklists)."))
o.optional = true
o.rmempty = true
o.datatype = "uinteger"
o.default = 60
o.placeholder = o.default

o = s:option(Value, "cache_size", translate("Cache size"), translate("Cache size for queries."))
o.optional = true
o.rmempty = true
o.datatype = "uinteger"
o.default = 512
o.placeholder = o.default

o = s:option(Value, "query_meta", translate("Additional TXT records"), translate("Additional data to attach to outgoing queries.") .. [[<br />]] .. translate("Split MultiValues by a comma"))
o.optional = true
o.rmempty = true
o.placeholder = "eg: key1:value1,key2:value2"

o = s:option(Value, "proxy", translate("SOCKS proxy"), translate("Tor doesn't support UDP, so set `force_tcp` to `true` as well."))
o.optional = true
o.rmempty = true
o.placeholder = "eg: socks5://127.0.0.1:9050"

o = s:option(Value, "http_proxy", translate("HTTP/HTTPS proxy"), translate("Only for DoH servers."))
o.optional = true
o.rmempty = true
o.placeholder = "eg: http://127.0.0.1:8888"

o = s:option(DynamicList, "cloaking_rules", translate("Cloaking rules"), translate("Cloaking returns a predefined address for a specific name."))
o.optional = true
o.rmempty = true
o.placeholder = "eg: /usr/share/dnscrypt-proxy/cloaking-rules.txt"

-- [[ ACL Setting ]]--
s = m:section(TypedSection, "server_addr", translate("DNSCrypt Resolver ACL"), translate("Allow all subscribed resolver servers in default.<br />The proxy will automatically pick the fastest, working servers from the list.<br />Resolver with detailed addresses is a must.") .. " https://dnscrypt.info/stamps/")
	s.sectionhead = translate("Alias")
	s.template = "cbi/tblsection"
	s.addremove = true
	s.anonymous = true
	s.sortable = false
	s.template_addremove = "dnscrypt-proxy/cbi_addserver"

function s.create(self, section)
	local a = m:formvalue("_newsrv.alias")
	local c = m:formvalue("_newsrv.country")
	local n = m:formvalue("_newsrv.name")
	local ad = m:formvalue("_newsrv.addrs")
	local po = m:formvalue("_newsrv.ports")
	local pr = m:formvalue("_newsrv.proto")
	local st = m:formvalue("_newsrv.stamp")

	created = nil
	if a ~= "" and n ~= "" and ad ~= "" and st ~= "" then
		created = TypedSection.create(self, section)

		self.map:set(created, "alias", a)
		self.map:set(created, "country", c or "Unknown")
		self.map:set(created, "name", n or "Custom")
		self.map:set(created, "addrs", ad)
		self.map:set(created, "ports", po or "ALL")
		self.map:set(created, "proto", pr or "Unknown")
		self.map:set(created, "stamp", st or "Error")
	end
end

function s.parse(self, ...)
	TypedSection.parse(self, ...)
	if created then
		m.uci:save("dnscrypt-proxy")
		luci.http.redirect(disp.build_url("admin", "services", "dnscrypt-proxy", "dnscrypt-proxy"))
	end
end

	s:option(DummyValue,"alias",translate("Alias"))
	s:option(DummyValue,"country",translate("Country"))
	s:option(DummyValue,"proto",translate("Protocol"))
	s:option(DummyValue,"name",translate("Resolver"))
	s:option(DummyValue,"addrs",translate("Address"))
	s:option(DummyValue,"ports",translate("Server Port"))
	s:option(DummyValue,"stamp",translate("Stamp"))

return m

