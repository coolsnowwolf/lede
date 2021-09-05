-- Copyright 2008-2011 Jo-Philipp Wich <jow@openwrt.org>
-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Licensed to the public under the Apache License 2.0.

local os   = require "os"
local io   = require "io"
local fs   = require "nixio.fs"
local util = require "luci.util"

local type  = type
local pairs = pairs
local error = error
local table = table

local ipkg = "opkg --force-removal-of-dependent-packages --force-overwrite --nocase"
local icfg = "/etc/opkg.conf"

module "luci.model.ipkg"


-- Internal action function
local function _action(cmd, ...)
	local pkg = ""
	for k, v in pairs({...}) do
		pkg = pkg .. " '" .. v:gsub("'", "") .. "'"
	end

	local c = "%s %s %s >/tmp/opkg.stdout 2>/tmp/opkg.stderr" %{ ipkg, cmd, pkg }
	local r = os.execute(c)
	local e = fs.readfile("/tmp/opkg.stderr")
	local o = fs.readfile("/tmp/opkg.stdout")

	fs.unlink("/tmp/opkg.stderr")
	fs.unlink("/tmp/opkg.stdout")

	return r, o or "", e or ""
end

-- Internal parser function
local function _parselist(rawdata)
	if type(rawdata) ~= "function" then
		error("OPKG: Invalid rawdata given")
	end

	local data = {}
	local c = {}
	local l = nil

	for line in rawdata do
		if line:sub(1, 1) ~= " " then
			local key, val = line:match("(.-): ?(.*)%s*")

			if key and val then
				if key == "Package" then
					c = {Package = val}
					data[val] = c
				elseif key == "Status" then
					c.Status = {}
					for j in val:gmatch("([^ ]+)") do
						c.Status[j] = true
					end
				else
					c[key] = val
				end
				l = key
			end
		else
			-- Multi-line field
			c[l] = c[l] .. "\n" .. line
		end
	end

	return data
end

-- Internal lookup function
local function _lookup(act, pkg)
	local cmd = ipkg .. " " .. act
	if pkg then
		cmd = cmd .. " '" .. pkg:gsub("'", "") .. "'"
	end

	-- OPKG sometimes kills the whole machine because it sucks
	-- Therefore we have to use a sucky approach too and use
	-- tmpfiles instead of directly reading the output
	local tmpfile = os.tmpname()
	os.execute(cmd .. (" >%s 2>/dev/null" % tmpfile))

	local data = _parselist(io.lines(tmpfile))
	os.remove(tmpfile)
	return data
end


function info(pkg)
	return _lookup("info", pkg)
end

function status(pkg)
	return _lookup("status", pkg)
end

function install(...)
	return _action("install", ...)
end

function installed(pkg)
	local p = status(pkg)[pkg]
	return (p and p.Status and p.Status.installed)
end

function remove(...)
	return _action("remove", ...)
end

function update()
	return _action("update")
end

function upgrade()
	return _action("upgrade")
end

-- List helper
local function _list(action, pat, cb)
	local fd = io.popen(ipkg .. " " .. action ..
		(pat and (" '%s'" % pat:gsub("'", "")) or ""))

	if fd then
		local name, version, sz, desc
		while true do
			local line = fd:read("*l")
			if not line then break end

			name, version, sz, desc = line:match("^(.-) %- (.-) %- (.-) %- (.+)")

			if not name then
				name, version, sz = line:match("^(.-) %- (.-) %- (.+)")
				desc = ""
			end

			if name and version then
				if #version > 26 then
					version = version:sub(1,21) .. ".." .. version:sub(-3,-1)
				end

				cb(name, version, sz, desc)
			end

			name    = nil
			version = nil
			sz      = nil
			desc    = nil
		end

		fd:close()
	end
end

function list_all(pat, cb)
	_list("list --size", pat, cb)
end

function list_installed(pat, cb)
	_list("list_installed --size", pat, cb)
end

function find(pat, cb)
	_list("find --size", pat, cb)
end


function overlay_root()
	local od = "/"
	local fd = io.open(icfg, "r")

	if fd then
		local ln

		repeat
			ln = fd:read("*l")
			if ln and ln:match("^%s*option%s+overlay_root%s+") then
				od = ln:match("^%s*option%s+overlay_root%s+(%S+)")

				local s = fs.stat(od)
				if not s or s.type ~= "dir" then
					od = "/"
				end

				break
			end
		until not ln

		fd:close()
	end

	return od
end

function compare_versions(ver1, comp, ver2)
	if not ver1 or not ver2
	or not comp or not (#comp > 0) then
		error("Invalid parameters")
		return nil
	end
	-- correct compare string
	if comp == "<>" or comp == "><" or comp == "!=" or comp == "~=" then comp = "~="
	elseif comp == "<=" or comp == "<" or comp == "=<" then comp = "<="
	elseif comp == ">=" or comp == ">" or comp == "=>" then comp = ">="
	elseif comp == "="  or comp == "==" then comp = "=="
	elseif comp == "<<" then comp = "<"
	elseif comp == ">>" then comp = ">"
	else
		error("Invalid compare string")
		return nil
	end

	local av1 = util.split(ver1, "[%.%-]", nil, true)
	local av2 = util.split(ver2, "[%.%-]", nil, true)

	local max = table.getn(av1)
	if (table.getn(av1) < table.getn(av2)) then
		max = table.getn(av2)
	end

	for i = 1, max, 1  do
		local s1 = av1[i] or ""
		local s2 = av2[i] or ""

		-- first "not equal" found return true
		if comp == "~=" and (s1 ~= s2) then return true end
		-- first "lower" found return true
		if (comp == "<" or comp == "<=") and (s1 < s2) then return true end
		-- first "greater" found return true
		if (comp == ">" or comp == ">=") and (s1 > s2) then return true end
		-- not equal then return false
		if (s1 ~= s2) then return false end
	end

	-- all equal and not compare greater or lower then true
	return not (comp == "<" or comp == ">")
end
