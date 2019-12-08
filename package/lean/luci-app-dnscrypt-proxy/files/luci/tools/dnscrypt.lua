
local type, pairs, ipairs, table, luci, math
	= type, pairs, ipairs, table, luci, math

local tpl = require "luci.template.parser"
local utl = require "luci.util"
local uci = require "luci.model.uci"

module "luci.tools.dnscrypt"

local uci_r, uci_s
local dsec, dlst = "dnscrypt", "resolvers"
local dglb, dsrc = "source", "urls"

function _valid_id(x)
	return (x and #x > 0 and x:match("^[a-zA-Z0-9_]+$"))
end

function _get(c, s, o)
	return uci_r:get(c, s, o)
end

function _set(c, s, o, v)
	if v ~= nil then
		if type(v) == "boolean" then v = v and "1" or "0" end
		return uci_r:set(c, s, o, v)
	else
		return uci_r:delete(c, s, o)
	end
end

function typicalcfg(self, cfg)
	local dnscrypt_types, all_val, k, t = {"source", "dnscrypt"}, uci_r:get_all(cfg)
	if not all_val then return true end
	for k, t in pairs(all_val) do
		if not luci.util.contains(dnscrypt_types, t[".type"]) then return false end
	end
	return true
end

function resolvers_list(self, with_src)
	local _, cfg, r = nil, nil, { }
	for _, cfg in pairs(uci_r:list_configs()) do
	uci_r:foreach(cfg, dsec,
		function(s)
			if s.name and s.stamp and s.proto and s.country then
				local src = uci_r:get_first(cfg, dglb, dsrc)
				r[#r+1] = with_src and "%s|%s" %{cfg, src or "Unknown"} or cfg
			end
			return false
		end)
	end
	return r
end

function country_list(self)
	local _, cfg, r = nil, nil, { }
	for _, cfg in pairs(self:resolvers_list(false)) do
	uci_r:foreach(cfg, dsec,
		function(s)
			if s.country then
				r[s.country] = s.country
			end
		end)
	end
	return r
end

function proto_list(self)
	local _, cfg, r = nil, nil, { }
	for _, cfg in pairs(self:resolvers_list(false)) do
	uci_r:foreach(cfg, dsec,
		function(s)
			if s.proto then
				r[s.proto] = s.proto
			end
		end)
	end
	return r
end

function dns_list(self, rf, cf, prf)
	local _, cfg, r = nil, nil, { }
	for _, cfg in pairs(self:resolvers_list(false)) do
	uci_r:foreach(cfg, dsec,
		function(s)
		 	if rf and cfg ~= rf then return false end
			if s.name and s.stamp and s.proto then
			 	if cf and cf ~= s.country then return true end
			 	if prf and prf ~= s.proto then return true end
				r[#r+1] = {
					['resolver'] = cfg,
					['proto'] = s.proto,
					['country'] = s.country,
					['name'] = s.name,
					['stamp'] = s.stamp,
					['addrs'] = s.addrs,
					['ports'] = s.ports,
				}
			end
		end)
	end
	return r
end

function del_resolved(self, cfg, sec)
	local r = false

		uci_r:foreach(cfg, dsec,
			function(s)
				if sec and s.name == sec then
					r = uci_r:delete(dsec, s['.name'])
					return false
				end
				r = uci_r:delete(dsec, s['.name'])
			end)

	return r
end

function init(cursor)
	uci_r = cursor or uci_r or uci.cursor()
	uci_s = uci_r:substate()

	return _M
end

function save(self, ...)
	uci_r:save(...)
	uci_r:load(...)
end

function commit(self, ...)
	uci_r:commit(...)
	uci_r:load(...)
end

function get_defaults()
	return defaults()
end
