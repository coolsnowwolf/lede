-- Copyright 2009 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local type, pairs, ipairs, table, luci, math
	= type, pairs, ipairs, table, luci, math

local tpl = require "luci.template.parser"
local utl = require "luci.util"
local uci = require "luci.model.uci"

module "luci.model.firewall"


local uci_r, uci_s

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

function new_zone(self)
	local name = "newzone"
	local count = 1

	while self:get_zone(name) do
		count = count + 1
		name = "newzone%d" % count
	end

	return self:add_zone(name)
end

function add_zone(self, n)
	if _valid_id(n) and not self:get_zone(n) then
		local d = defaults()
		local z = uci_r:section("firewall", "zone", nil, {
			name    = n,
			network = " ",
			input   = d:input()   or "DROP",
			forward = d:forward() or "DROP",
			output  = d:output()  or "DROP"
		})

		return z and zone(z)
	end
end

function get_zone(self, n)
	if uci_r:get("firewall", n) == "zone" then
		return zone(n)
	else
		local z
		uci_r:foreach("firewall", "zone",
			function(s)
				if n and s.name == n then
					z = s['.name']
					return false
				end
			end)
		return z and zone(z)
	end
end

function get_zones(self)
	local zones = { }
	local znl = { }

	uci_r:foreach("firewall", "zone",
		function(s)
			if s.name then
				znl[s.name] = zone(s['.name'])
			end
		end)

	local z
	for z in utl.kspairs(znl) do
		zones[#zones+1] = znl[z]
	end

	return zones
end

function get_zone_by_network(self, net)
	local z

	uci_r:foreach("firewall", "zone",
		function(s)
			if s.name and net then
				local n
				for n in utl.imatch(s.network or s.name) do
					if n == net then
						z = s['.name']
						return false
					end
				end
			end
		end)

	return z and zone(z)
end

function del_zone(self, n)
	local r = false

	if uci_r:get("firewall", n) == "zone" then
		local z = uci_r:get("firewall", n, "name")
		r = uci_r:delete("firewall", n)
		n = z
	else
		uci_r:foreach("firewall", "zone",
			function(s)
				if n and s.name == n then
					r = uci_r:delete("firewall", s['.name'])
					return false
				end
			end)
	end

	if r then
		uci_r:foreach("firewall", "rule",
			function(s)
				if s.src == n or s.dest == n then
					uci_r:delete("firewall", s['.name'])
				end
			end)

		uci_r:foreach("firewall", "redirect",
			function(s)
				if s.src == n or s.dest == n then
					uci_r:delete("firewall", s['.name'])
				end
			end)

		uci_r:foreach("firewall", "forwarding",
			function(s)
				if s.src == n or s.dest == n then
					uci_r:delete("firewall", s['.name'])
				end
			end)
	end

	return r
end

function rename_zone(self, old, new)
	local r = false

	if _valid_id(new) and not self:get_zone(new) then
		uci_r:foreach("firewall", "zone",
			function(s)
				if old and s.name == old then
					if not s.network then
						uci_r:set("firewall", s['.name'], "network", old)
					end
					uci_r:set("firewall", s['.name'], "name", new)
					r = true
					return false
				end
			end)

		if r then
			uci_r:foreach("firewall", "rule",
				function(s)
					if s.src == old then
						uci_r:set("firewall", s['.name'], "src", new)
					end
					if s.dest == old then
						uci_r:set("firewall", s['.name'], "dest", new)
					end
				end)

			uci_r:foreach("firewall", "redirect",
				function(s)
					if s.src == old then
						uci_r:set("firewall", s['.name'], "src", new)
					end
					if s.dest == old then
						uci_r:set("firewall", s['.name'], "dest", new)
					end
				end)

			uci_r:foreach("firewall", "forwarding",
				function(s)
					if s.src == old then
						uci_r:set("firewall", s['.name'], "src", new)
					end
					if s.dest == old then
						uci_r:set("firewall", s['.name'], "dest", new)
					end
				end)
		end
	end

	return r
end

function del_network(self, net)
	local z
	if net then
		for _, z in ipairs(self:get_zones()) do
			z:del_network(net)
		end
	end
end


defaults = utl.class()
function defaults.__init__(self)
	uci_r:foreach("firewall", "defaults",
		function(s)
			self.sid  = s['.name']
			return false
		end)

	self.sid = self.sid or uci_r:section("firewall", "defaults", nil, { })
end

function defaults.get(self, opt)
	return _get("firewall", self.sid, opt)
end

function defaults.set(self, opt, val)
	return _set("firewall", self.sid, opt, val)
end

function defaults.syn_flood(self)
	return (self:get("syn_flood") == "1")
end

function defaults.drop_invalid(self)
	return (self:get("drop_invalid") == "1")
end

function defaults.input(self)
	return self:get("input") or "DROP"
end

function defaults.forward(self)
	return self:get("forward") or "DROP"
end

function defaults.output(self)
	return self:get("output") or "DROP"
end


zone = utl.class()
function zone.__init__(self, z)
	if uci_r:get("firewall", z) == "zone" then
		self.sid  = z
		self.data = uci_r:get_all("firewall", z)
	else
		uci_r:foreach("firewall", "zone",
			function(s)
				if s.name == z then
					self.sid  = s['.name']
					self.data = s
					return false
				end
			end)
	end
end

function zone.get(self, opt)
	return _get("firewall", self.sid, opt)
end

function zone.set(self, opt, val)
	return _set("firewall", self.sid, opt, val)
end

function zone.masq(self)
	return (self:get("masq") == "1")
end

function zone.name(self)
	return self:get("name")
end

function zone.network(self)
	return self:get("network")
end

function zone.input(self)
	return self:get("input") or defaults():input() or "DROP"
end

function zone.forward(self)
	return self:get("forward") or defaults():forward() or "DROP"
end

function zone.output(self)
	return self:get("output") or defaults():output() or "DROP"
end

function zone.add_network(self, net)
	if uci_r:get("network", net) == "interface" then
		local nets = { }

		local n
		for n in utl.imatch(self:get("network") or self:get("name")) do
			if n ~= net then
				nets[#nets+1] = n
			end
		end

		nets[#nets+1] = net

		_M:del_network(net)
		self:set("network", table.concat(nets, " "))
	end
end

function zone.del_network(self, net)
	local nets = { }

	local n
	for n in utl.imatch(self:get("network") or self:get("name")) do
		if n ~= net then
			nets[#nets+1] = n
		end
	end

	if #nets > 0 then
		self:set("network", table.concat(nets, " "))
	else
		self:set("network", " ")
	end
end

function zone.get_networks(self)
	local nets = { }

	local n
	for n in utl.imatch(self:get("network") or self:get("name")) do
		nets[#nets+1] = n
	end

	return nets
end

function zone.clear_networks(self)
	self:set("network", " ")
end

function zone.get_forwardings_by(self, what)
	local name = self:name()
	local forwards = { }

	uci_r:foreach("firewall", "forwarding",
		function(s)
			if s.src and s.dest and s[what] == name then
				forwards[#forwards+1] = forwarding(s['.name'])
			end
		end)

	return forwards
end

function zone.add_forwarding_to(self, dest)
	local exist, forward

	for _, forward in ipairs(self:get_forwardings_by('src')) do
		if forward:dest() == dest then
			exist = true
			break
		end
	end

	if not exist and dest ~= self:name() and _valid_id(dest) then
		local s = uci_r:section("firewall", "forwarding", nil, {
			src     = self:name(),
			dest    = dest
		})

		return s and forwarding(s)
	end
end

function zone.add_forwarding_from(self, src)
	local exist, forward

	for _, forward in ipairs(self:get_forwardings_by('dest')) do
		if forward:src() == src then
			exist = true
			break
		end
	end

	if not exist and src ~= self:name() and _valid_id(src) then
		local s = uci_r:section("firewall", "forwarding", nil, {
			src     = src,
			dest    = self:name()
		})

		return s and forwarding(s)
	end
end

function zone.del_forwardings_by(self, what)
	local name = self:name()

	uci_r:delete_all("firewall", "forwarding",
		function(s)
			return (s.src and s.dest and s[what] == name)
		end)
end

function zone.add_redirect(self, options)
	options = options or { }
	options.src = self:name()

	local s = uci_r:section("firewall", "redirect", nil, options)
	return s and redirect(s)
end

function zone.add_rule(self, options)
	options = options or { }
	options.src = self:name()

	local s = uci_r:section("firewall", "rule", nil, options)
	return s and rule(s)
end

function zone.get_color(self)
	if self and self:name() == "lan" then
		return "#90f090"
	elseif self and self:name() == "wan" then
		return "#f09090"
	elseif self then
		math.randomseed(tpl.hash(self:name()))

		local r   = math.random(128)
		local g   = math.random(128)
		local min = 0
		local max = 128

		if ( r + g ) < 128 then
			min = 128 - r - g
		else
			max = 255 - r - g
		end

		local b = min + math.floor( math.random() * ( max - min ) )

		return "#%02x%02x%02x" % { 0xFF - r, 0xFF - g, 0xFF - b }
	else
		return "#eeeeee"
	end
end


forwarding = utl.class()
function forwarding.__init__(self, f)
	self.sid = f
end

function forwarding.src(self)
	return uci_r:get("firewall", self.sid, "src")
end

function forwarding.dest(self)
	return uci_r:get("firewall", self.sid, "dest")
end

function forwarding.src_zone(self)
	return zone(self:src())
end

function forwarding.dest_zone(self)
	return zone(self:dest())
end


rule = utl.class()
function rule.__init__(self, f)
	self.sid = f
end

function rule.get(self, opt)
	return _get("firewall", self.sid, opt)
end

function rule.set(self, opt, val)
	return _set("firewall", self.sid, opt, val)
end

function rule.src(self)
	return uci_r:get("firewall", self.sid, "src")
end

function rule.dest(self)
	return uci_r:get("firewall", self.sid, "dest")
end

function rule.src_zone(self)
	return zone(self:src())
end

function rule.dest_zone(self)
	return zone(self:dest())
end


redirect = utl.class()
function redirect.__init__(self, f)
	self.sid = f
end

function redirect.get(self, opt)
	return _get("firewall", self.sid, opt)
end

function redirect.set(self, opt, val)
	return _set("firewall", self.sid, opt, val)
end

function redirect.src(self)
	return uci_r:get("firewall", self.sid, "src")
end

function redirect.dest(self)
	return uci_r:get("firewall", self.sid, "dest")
end

function redirect.src_zone(self)
	return zone(self:src())
end

function redirect.dest_zone(self)
	return zone(self:dest())
end
