-- Copyright (C) 2019 github.com/peter-tank
-- Licensed to the public under the GNU General Public License v3.

local m, s, o, _, v, created
local fs = require "nixio.fs"
local sys = require "luci.sys"
local ds = require "luci.tools.dnscrypt".init()
local cfg, sid = arg[1], arg[2]

local protocol = {
	"DNSCrypt",
	"DoH",
}

function typicalmsg(self)
	if not ds:typicalcfg(cfg) then
		local c, init, exec = luci.model.uci.cursor()
		c:load("ucitrack")
		init = c:get_first("ucitrack", self.config, "init", "null")
		exec = c:get_first("ucitrack", self.config, "exec", "null")
		c:unload("ucitrack")
		local error = {}
		table.insert(error,  translate("Not a typical dnscrypt-proxy resolvers configure!") .. " Original for: [%s]" % init)
		table.insert(error,  translate("Additional changes on apply:"))
		table.insert(error,  "init script: %s --> %s" % {init, "null"})
		table.insert(error,  "exec script: %s --> %s %s" % {exec, "/usr/share/dnscrypt-proxy/uci_handle_exec.sh", self.config})
		self.error = self.error and self.error or {}
		self.error["global"] = self.error["global"] and table(self.error["global"], error) or error
	end
end

function list_dns(m, cfg)
-- [[ Source Setting ]]--
s = m:section(NamedSection, "global", "source",  translate("Configuration -: ") .. cfg)
s.description = translate("Update the resolvers here by a new apply. Permently cover your modifications on specific resolve, if the source resolved successfully.")
s.anonymous = false
s.addremove = true
typicalmsg(s)
-- section might not exist
function s.cfgvalue(self, section)
	if not self.map:get(section) then
 	 self.map:set(section, nil, self.sectiontype)
	end
	return self.map:get(section)
end
function s.remove(self, section)
 	self.map:del(section)
 	self.map.proceed = true
 	return self.map.uci:delete_all(self.config, "dnscrypt", function(section) return true end)
end

o = s:option(Value, "urls", translate("Source URL"), translate("Split MultiValues by a comma"))
o.rmempty = true
o.default = ""
o.placeholder = "https://download.dnscrypt.info/dnscrypt-resolvers/v2/public-resolvers.md"

--o = s:option(Value, "cache_file", translate("Cache File"), translate("Change this only on duplicating this resolver to another source when retrieving: ") .. "/usr/share/dnscrypt-proxy/{*}.md")
--o.default = "public-resolver.md"
--o.placeholder = "eg: custom.md"
--o.rmempty = false

o = s:option(Value, "details_json", translate("Details File URL"), translate("Detailed resolvers JSON file `dnscrypt-proxy -json --list-all` or: ") .. "https://download.dnscrypt.info/dnscrypt-resolvers/json/{*}.json")
o.default = "https://download.dnscrypt.info/dnscrypt-resolvers/json/public-resolvers.json"
o.placeholder = "eg: custom.json"
o.rmempty = true

o = s:option(Flag, "cache_mode", translate("No resolvers update."), translate("Force cached resolvers only: ").. "/usr/share/dnscrypt-proxy/{.*}[.md|.minisig|.json]")
o.default = true
o.optional = false
o.rmempty = false

o = s:option(Flag, "selfsign", translate("Selfsign Resolvers"), translate("Mostly useful for build your own resolvers list on net probe fails. minisign binary needed: ") .. "https://github.com/jedisct1/minisign")
o.default = false
o.optional = false
o.rmempty = true
o:depends("cache_mode", true)

o = s:option(Value, "minisign_key", translate("MiniSign Key"), translate("Resolvers singed by: ") .. "https://download.dnscrypt.info/dnscrypt-resolvers/v2/minisign.pub")
o.default = "RWQf6LRCGA9i53mlYecO4IzT51TGPpvWucNSCh1CBM0QTaLn73Y7GFO3"
o.optional = false
o.placeholder = o.default
o.rmempty = false
o:depends("selfsign", false)

o = s:option(Value, "prefix", translate("Prefix"), translate("Change this only on filtering the resolvers: "))
o.optional = true
o.placeholder = "eg: quad9-"
o.rmempty = true

o = s:option(Value, "format", translate("Version"), translate("Configure file version."))
o.optional = true
o.default = "v2"
o.rmempty = true

s = m:section(TypedSection, 'dnscrypt', translate("DNSCrypt Resolver Info"))
s.anonymous = false
s.addremove = true
s.sortable = false
s.template = "cbi/tblsection"
s.extedit = luci.dispatcher.build_url("admin/services/dnscrypt-proxy/dnscrypt-resolvers", cfg, "%s")
function s.create(self, name)
		AbstractSection.create(self, name)
		luci.http.redirect( self.extedit:format(name) )
end

s:option(DummyValue, "country", translate("Country"))
s:option(DummyValue, "proto", translate("Protocol"))
s:option(DummyValue, "name", translate("Resolver"))
s:option(DummyValue, "description", translate("Description"))
s:option(Flag, "ipv6", translate("IPv6"))
s:option(Flag, "dnssec", translate("DNSSec Validation"))
s:option(Flag, "nolog", translate("NoLog"))
s:option(Flag, "nofilter", translate("NoFilter"))
s:option(DummyValue, "addrs", translate("Address"))
s:option(DummyValue, "ports", translate("Port"))
-- s:option(DummyValue, "stamp", translate("Stamp"))
end 


-- [[ Servers Setting ]]--
function edit_list(m)
s = m:section(NamedSection, sid, "dnscrypt", translate("DNSCrypt Resolver Editing - ") .. sid)
s.anonymous = true
s.addremove   = false

o = s:option(Value, "name", translate("Resolver"))
o.default = sid
o.rmempty = false

o = s:option(Value, "country", translate("Country"))
o.default = "China"

o = s:option(Value, "proto", translate("Protocol"))
for _, v in ipairs(protocol) do o:value(v) end
o.default = "DNSCrypt"
o.rmempty = false

o = s:option(Value, "description", translate("Description"))
o.default = "Dns-over-HTTPS protocol . Non-logging, supports DNSSEC. By https://www.google.com"

o = s:option(Flag, "dnssec", translate("DNSSec Validation"))
o.rmempty = false

o = s:option(Flag, "ipv6", translate("IPv6"))
o.rmempty = false

o = s:option(Flag, "nolog", translate("NoLog"))
o.rmempty = false

o = s:option(Flag, "nofilter", translate("NoFilter"))
o.rmempty = false

o = s:option(Value, "addrs", translate("Resolver Address"), translate("Split MultiValues by a comma"))
o.placeholder = "eg: 8.8.8.8,8.8.4.4"
o.rmempty = false

o = s:option(Value, "ports", translate("Port"), translate("Split MultiValues by a comma"))
o.placeholder = "eg: 443,5353"
o.rmempty = false

o = s:option(Value, "location", translate("Geography Coordinates"))
o.placeholder = "eg: 33.032501, 83.895699"

o = s:option(Value, "stamp", translate("Stamp"))
o.placeholder = "eg: sdns://AgcAAAAAAAAADjExNi4yMDMuNzAuMTU2ID4aGg9sU_PpekktVwhLW5gHBZ7gV6sVBYdv2D_aPbg4EWRvaC5kbnN3YXJkZW4uY29tCy91bmNlbnNvcmVk"
o.rmempty = false
end

m = Map(cfg, translate("DNSCrypt Resolvers Manage"))
m.on_before_commit = function (self)
	local exec = "/usr/share/dnscrypt-proxy/uci_handle_exec.sh"
	local section, stype, on_remove = "global", "source", false
	local sec = "DNSCrypt_resolvers_" .. self.config:gsub("[^%w_]", "_")
	local c = luci.model.uci.cursor()
	local changes = c:changes()
	self.redirect = luci.dispatcher.build_url("admin/services/dnscrypt-proxy/dnscrypt-resolvers", self.config)
	for r, tbl in pairs(changes or next(changes)) do
	 	for s, os in pairs(tbl) do
	 		if os['.type'] and os['.type'] == "" and r == self.config and s == section then on_remove = true end
	 	end
	end
	c:load("ucitrack")
	if self:get(section, "urls") and not on_remove then
	 	if not c:get_first("ucitrack", self.config, "exec", nil) then
	 		c:delete_all("ucitrack", self.config, function(section) return true end)
	 		sec = c:section("ucitrack", self.config, sec)
	 		c:set("ucitrack", sec, "init", "null")
	 		c:set("ucitrack", sec, "test", "0")
	 		c:set("ucitrack", sec, "exec", "%s %s" % {exec, self.config})
	 		c:save("ucitrack"); c:commit("ucitrack");
	 	end
	else
	 	if c:get_first("ucitrack", self.config, "exec", nil) == ("%s %s" %{exec, self.config}) then
	  	c:delete_all("ucitrack", self.config, function(section) return true end)
	  	c:save("ucitrack"); c:commit("ucitrack");
	 	end
	 	self.redirect = luci.dispatcher.build_url("admin/services/dnscrypt-proxy/dnscrypt-resolvers")
	end
	c:unload("ucitrack")
end

if cfg then
	if sid == nil then list_dns(m, cfg) else edit_list(m, cfg) end
else
	luci.http.redirect(luci.dispatcher.build_url("admin/services/dnscrypt-proxy/dnscrypt-resolvers"))
	return
end

return m
