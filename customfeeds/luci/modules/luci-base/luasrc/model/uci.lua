-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Licensed to the public under the Apache License 2.0.

local os    = require "os"
local uci   = require "uci"
local util  = require "luci.util"
local table = require "table"


local setmetatable, rawget, rawset = setmetatable, rawget, rawset
local require, getmetatable = require, getmetatable
local error, pairs, ipairs = error, pairs, ipairs
local type, tostring, tonumber, unpack = type, tostring, tonumber, unpack

-- The typical workflow for UCI is:  Get a cursor instance from the
-- cursor factory, modify data (via Cursor.add, Cursor.delete, etc.),
-- save the changes to the staging area via Cursor.save and finally
-- Cursor.commit the data to the actual config files.
-- LuCI then needs to Cursor.apply the changes so deamons etc. are
-- reloaded.
module "luci.model.uci"

cursor = uci.cursor

APIVERSION = uci.APIVERSION

function cursor_state()
	return cursor(nil, "/var/state")
end


inst = cursor()
inst_state = cursor_state()

local Cursor = getmetatable(inst)

function Cursor.apply(self, configlist, command)
	configlist = self:_affected(configlist)
	if command then
		return { "/sbin/luci-reload", unpack(configlist) }
	else
		return os.execute("/sbin/luci-reload %s >/dev/null 2>&1"
			% table.concat(configlist, " "))
	end
end


-- returns a boolean whether to delete the current section (optional)
function Cursor.delete_all(self, config, stype, comparator)
	local del = {}

	if type(comparator) == "table" then
		local tbl = comparator
		comparator = function(section)
			for k, v in pairs(tbl) do
				if section[k] ~= v then
					return false
				end
			end
			return true
		end
	end

	local function helper (section)

		if not comparator or comparator(section) then
			del[#del+1] = section[".name"]
		end
	end

	self:foreach(config, stype, helper)

	for i, j in ipairs(del) do
		self:delete(config, j)
	end
end

function Cursor.section(self, config, type, name, values)
	local stat = true
	if name then
		stat = self:set(config, name, type)
	else
		name = self:add(config, type)
		stat = name and true
	end

	if stat and values then
		stat = self:tset(config, name, values)
	end

	return stat and name
end

function Cursor.tset(self, config, section, values)
	local stat = true
	for k, v in pairs(values) do
		if k:sub(1, 1) ~= "." then
			stat = stat and self:set(config, section, k, v)
		end
	end
	return stat
end

function Cursor.get_bool(self, ...)
	local val = self:get(...)
	return ( val == "1" or val == "true" or val == "yes" or val == "on" )
end

function Cursor.get_list(self, config, section, option)
	if config and section and option then
		local val = self:get(config, section, option)
		return ( type(val) == "table" and val or { val } )
	end
	return {}
end

function Cursor.get_first(self, conf, stype, opt, def)
	local rv = def

	self:foreach(conf, stype,
		function(s)
			local val = not opt and s['.name'] or s[opt]

			if type(def) == "number" then
				val = tonumber(val)
			elseif type(def) == "boolean" then
				val = (val == "1" or val == "true" or
				       val == "yes" or val == "on")
			end

			if val ~= nil then
				rv = val
				return false
			end
		end)

	return rv
end

function Cursor.set_list(self, config, section, option, value)
	if config and section and option then
		if not value or #value == 0 then
			return self:delete(config, section, option)
		end
		return self:set(
			config, section, option,
			( type(value) == "table" and value or { value } )
		)
	end
	return false
end

-- Return a list of initscripts affected by configuration changes.
function Cursor._affected(self, configlist)
	configlist = type(configlist) == "table" and configlist or {configlist}

	local c = cursor()
	c:load("ucitrack")

	-- Resolve dependencies
	local reloadlist = {}

	local function _resolve_deps(name)
		local reload = {name}
		local deps = {}

		c:foreach("ucitrack", name,
			function(section)
				if section.affects then
					for i, aff in ipairs(section.affects) do
						deps[#deps+1] = aff
					end
				end
			end)

		for i, dep in ipairs(deps) do
			for j, add in ipairs(_resolve_deps(dep)) do
				reload[#reload+1] = add
			end
		end

		return reload
	end

	-- Collect initscripts
	for j, config in ipairs(configlist) do
		for i, e in ipairs(_resolve_deps(config)) do
			if not util.contains(reloadlist, e) then
				reloadlist[#reloadlist+1] = e
			end
		end
	end

	return reloadlist
end

-- curser, means it the parent unloads or loads configs, the sub state will
-- do so as well.
function Cursor.substate(self)
	Cursor._substates = Cursor._substates or { }
	Cursor._substates[self] = Cursor._substates[self] or cursor_state()
	return Cursor._substates[self]
end

local _load = Cursor.load
function Cursor.load(self, ...)
	if Cursor._substates and Cursor._substates[self] then
		_load(Cursor._substates[self], ...)
	end
	return _load(self, ...)
end

local _unload = Cursor.unload
function Cursor.unload(self, ...)
	if Cursor._substates and Cursor._substates[self] then
		_unload(Cursor._substates[self], ...)
	end
	return _unload(self, ...)
end

















