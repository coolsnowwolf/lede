-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Licensed to the public under the Apache License 2.0.

local io = require "io"
local math = require "math"
local table = require "table"
local debug = require "debug"
local ldebug = require "luci.debug"
local string = require "string"
local coroutine = require "coroutine"
local tparser = require "luci.template.parser"
local json = require "luci.jsonc"

local _ubus = require "ubus"
local _ubus_connection = nil

local getmetatable, setmetatable = getmetatable, setmetatable
local rawget, rawset, unpack = rawget, rawset, unpack
local tostring, type, assert, error = tostring, type, assert, error
local ipairs, pairs, next, loadstring = ipairs, pairs, next, loadstring
local require, pcall, xpcall = require, pcall, xpcall
local collectgarbage, get_memory_limit = collectgarbage, get_memory_limit

module "luci.util"

--
-- Pythonic string formatting extension
--
getmetatable("").__mod = function(a, b)
	local ok, res

	if not b then
		return a
	elseif type(b) == "table" then
		local k, _
		for k, _ in pairs(b) do if type(b[k]) == "userdata" then b[k] = tostring(b[k]) end end

		ok, res = pcall(a.format, a, unpack(b))
		if not ok then
			error(res, 2)
		end
		return res
	else
		if type(b) == "userdata" then b = tostring(b) end

		ok, res = pcall(a.format, a, b)
		if not ok then
			error(res, 2)
		end
		return res
	end
end


--
-- Class helper routines
--

-- Instantiates a class
local function _instantiate(class, ...)
	local inst = setmetatable({}, {__index = class})

	if inst.__init__ then
		inst:__init__(...)
	end

	return inst
end

-- The class object can be instantiated by calling itself.
-- Any class functions or shared parameters can be attached to this object.
-- Attaching a table to the class object makes this table shared between
-- all instances of this class. For object parameters use the __init__ function.
-- Classes can inherit member functions and values from a base class.
-- Class can be instantiated by calling them. All parameters will be passed
-- to the __init__ function of this class - if such a function exists.
-- The __init__ function must be used to set any object parameters that are not shared
-- with other objects of this class. Any return values will be ignored.
function class(base)
	return setmetatable({}, {
		__call  = _instantiate,
		__index = base
	})
end

function instanceof(object, class)
	local meta = getmetatable(object)
	while meta and meta.__index do
		if meta.__index == class then
			return true
		end
		meta = getmetatable(meta.__index)
	end
	return false
end


--
-- Scope manipulation routines
--

local tl_meta = {
	__mode = "k",

	__index = function(self, key)
		local t = rawget(self, coxpt[coroutine.running()]
		 or coroutine.running() or 0)
		return t and t[key]
	end,

	__newindex = function(self, key, value)
		local c = coxpt[coroutine.running()] or coroutine.running() or 0
		local r = rawget(self, c)
		if not r then
			rawset(self, c, { [key] = value })
		else
			r[key] = value
		end
	end
}

-- the current active coroutine. A thread local store is private a table object
-- whose values can't be accessed from outside of the running coroutine.
function threadlocal(tbl)
	return setmetatable(tbl or {}, tl_meta)
end


--
-- Debugging routines
--

function perror(obj)
	return io.stderr:write(tostring(obj) .. "\n")
end

function dumptable(t, maxdepth, i, seen)
	i = i or 0
	seen = seen or setmetatable({}, {__mode="k"})

	for k,v in pairs(t) do
		perror(string.rep("\t", i) .. tostring(k) .. "\t" .. tostring(v))
		if type(v) == "table" and (not maxdepth or i < maxdepth) then
			if not seen[v] then
				seen[v] = true
				dumptable(v, maxdepth, i+1, seen)
			else
				perror(string.rep("\t", i) .. "*** RECURSION ***")
			end
		end
	end
end


--
-- String and data manipulation routines
--

function pcdata(value)
	return value and tparser.pcdata(tostring(value))
end

function striptags(value)
	return value and tparser.striptags(tostring(value))
end

function shellquote(value)
	return string.format("'%s'", string.gsub(value or "", "'", "'\\''"))
end

-- for bash, ash and similar shells single-quoted strings are taken
-- literally except for single quotes (which terminate the string)
-- (and the exception noted below for dash (-) at the start of a
-- command line parameter).
function shellsqescape(value)
   local res
   res, _ = string.gsub(value, "'", "'\\''")
   return res
end

-- bash, ash and other similar shells interpret a dash (-) at the start
-- of a command-line parameters as an option indicator regardless of
-- whether it is inside a single-quoted string.  It must be backlash
-- escaped to resolve this.  This requires in some funky special-case
-- handling.  It may actually be a property of the getopt function
-- rather than the shell proper.
function shellstartsqescape(value)
   res, _ = string.gsub(value, "^\-", "\\-")
   res, _ = string.gsub(res, "^-", "\-")
   return shellsqescape(value)
end

-- containing the resulting substrings. The optional max parameter specifies
-- the number of bytes to process, regardless of the actual length of the given
-- string. The optional last parameter, regex, specifies whether the separator
-- sequence is interpreted as regular expression.
--					pattern as regular expression (optional, default is false)
function split(str, pat, max, regex)
	pat = pat or "\n"
	max = max or #str

	local t = {}
	local c = 1

	if #str == 0 then
		return {""}
	end

	if #pat == 0 then
		return nil
	end

	if max == 0 then
		return str
	end

	repeat
		local s, e = str:find(pat, c, not regex)
		max = max - 1
		if s and max < 0 then
			t[#t+1] = str:sub(c)
		else
			t[#t+1] = str:sub(c, s and s - 1)
		end
		c = e and e + 1 or #str + 1
	until not s or max < 0

	return t
end

function trim(str)
	return (str:gsub("^%s*(.-)%s*$", "%1"))
end

function cmatch(str, pat)
	local count = 0
	for _ in str:gmatch(pat) do count = count + 1 end
	return count
end

-- one token per invocation, the tokens are separated by whitespace. If the
-- input value is a table, it is transformed into a string first. A nil value
-- will result in a valid interator which aborts with the first invocation.
function imatch(v)
	if type(v) == "table" then
		local k = nil
		return function()
			k = next(v, k)
			return v[k]
		end

	elseif type(v) == "number" or type(v) == "boolean" then
		local x = true
		return function()
			if x then
				x = false
				return tostring(v)
			end
		end

	elseif type(v) == "userdata" or type(v) == "string" then
		return tostring(v):gmatch("%S+")
	end

	return function() end
end

-- value or 0 if the unit is unknown. Upper- or lower case is irrelevant.
-- Recognized units are:
--	o "y"	- one year   (60*60*24*366)
--  o "m"	- one month  (60*60*24*31)
--  o "w"	- one week   (60*60*24*7)
--  o "d"	- one day    (60*60*24)
--  o "h"	- one hour	 (60*60)
--  o "min"	- one minute (60)
--  o "kb"  - one kilobyte (1024)
--  o "mb"	- one megabyte (1024*1024)
--  o "gb"	- one gigabyte (1024*1024*1024)
--  o "kib" - one si kilobyte (1000)
--  o "mib"	- one si megabyte (1000*1000)
--  o "gib"	- one si gigabyte (1000*1000*1000)
function parse_units(ustr)

	local val = 0

	-- unit map
	local map = {
		-- date stuff
		y   = 60 * 60 * 24 * 366,
		m   = 60 * 60 * 24 * 31,
		w   = 60 * 60 * 24 * 7,
		d   = 60 * 60 * 24,
		h   = 60 * 60,
		min = 60,

		-- storage sizes
		kb  = 1024,
		mb  = 1024 * 1024,
		gb  = 1024 * 1024 * 1024,

		-- storage sizes (si)
		kib = 1000,
		mib = 1000 * 1000,
		gib = 1000 * 1000 * 1000
	}

	-- parse input string
	for spec in ustr:lower():gmatch("[0-9%.]+[a-zA-Z]*") do

		local num = spec:gsub("[^0-9%.]+$","")
		local spn = spec:gsub("^[0-9%.]+", "")

		if map[spn] or map[spn:sub(1,1)] then
			val = val + num * ( map[spn] or map[spn:sub(1,1)] )
		else
			val = val + num
		end
	end


	return val
end

-- also register functions above in the central string class for convenience
string.pcdata      = pcdata
string.striptags   = striptags
string.split       = split
string.trim        = trim
string.cmatch      = cmatch
string.parse_units = parse_units


function append(src, ...)
	for i, a in ipairs({...}) do
		if type(a) == "table" then
			for j, v in ipairs(a) do
				src[#src+1] = v
			end
		else
			src[#src+1] = a
		end
	end
	return src
end

function combine(...)
	return append({}, ...)
end

function contains(table, value)
	for k, v in pairs(table) do
		if value == v then
			return k
		end
	end
	return false
end

-- Both table are - in fact - merged together.
function update(t, updates)
	for k, v in pairs(updates) do
		t[k] = v
	end
end

function keys(t)
	local keys = { }
	if t then
		for k, _ in kspairs(t) do
			keys[#keys+1] = k
		end
	end
	return keys
end

function clone(object, deep)
	local copy = {}

	for k, v in pairs(object) do
		if deep and type(v) == "table" then
			v = clone(v, deep)
		end
		copy[k] = v
	end

	return setmetatable(copy, getmetatable(object))
end


function dtable()
        return setmetatable({}, { __index =
                function(tbl, key)
                        return rawget(tbl, key)
                         or rawget(rawset(tbl, key, dtable()), key)
                end
        })
end


-- Serialize the contents of a table value.
function _serialize_table(t, seen)
	assert(not seen[t], "Recursion detected.")
	seen[t] = true

	local data  = ""
	local idata = ""
	local ilen  = 0

	for k, v in pairs(t) do
		if type(k) ~= "number" or k < 1 or math.floor(k) ~= k or ( k - #t ) > 3 then
			k = serialize_data(k, seen)
			v = serialize_data(v, seen)
			data = data .. ( #data > 0 and ", " or "" ) ..
				'[' .. k .. '] = ' .. v
		elseif k > ilen then
			ilen = k
		end
	end

	for i = 1, ilen do
		local v = serialize_data(t[i], seen)
		idata = idata .. ( #idata > 0 and ", " or "" ) .. v
	end

	return idata .. ( #data > 0 and #idata > 0 and ", " or "" ) .. data
end

-- with loadstring().
function serialize_data(val, seen)
	seen = seen or setmetatable({}, {__mode="k"})

	if val == nil then
		return "nil"
	elseif type(val) == "number" then
		return val
	elseif type(val) == "string" then
		return "%q" % val
	elseif type(val) == "boolean" then
		return val and "true" or "false"
	elseif type(val) == "function" then
		return "loadstring(%q)" % get_bytecode(val)
	elseif type(val) == "table" then
		return "{ " .. _serialize_table(val, seen) .. " }"
	else
		return '"[unhandled data type:' .. type(val) .. ']"'
	end
end

function restore_data(str)
	return loadstring("return " .. str)()
end


--
-- Byte code manipulation routines
--

-- will be stripped before it is returned.
function get_bytecode(val)
	local code

	if type(val) == "function" then
		code = string.dump(val)
	else
		code = string.dump( loadstring( "return " .. serialize_data(val) ) )
	end

	return code -- and strip_bytecode(code)
end

-- numbers and debugging numbers will be discarded. Original version by
-- Peter Cawley (http://lua-users.org/lists/lua-l/2008-02/msg01158.html)
function strip_bytecode(code)
	local version, format, endian, int, size, ins, num, lnum = code:byte(5, 12)
	local subint
	if endian == 1 then
		subint = function(code, i, l)
			local val = 0
			for n = l, 1, -1 do
				val = val * 256 + code:byte(i + n - 1)
			end
			return val, i + l
		end
	else
		subint = function(code, i, l)
			local val = 0
			for n = 1, l, 1 do
				val = val * 256 + code:byte(i + n - 1)
			end
			return val, i + l
		end
	end

	local function strip_function(code)
		local count, offset = subint(code, 1, size)
		local stripped = { string.rep("\0", size) }
		local dirty = offset + count
		offset = offset + count + int * 2 + 4
		offset = offset + int + subint(code, offset, int) * ins
		count, offset = subint(code, offset, int)
		for n = 1, count do
			local t
			t, offset = subint(code, offset, 1)
			if t == 1 then
				offset = offset + 1
			elseif t == 4 then
				offset = offset + size + subint(code, offset, size)
			elseif t == 3 then
				offset = offset + num
			elseif t == 254 or t == 9 then
				offset = offset + lnum
			end
		end
		count, offset = subint(code, offset, int)
		stripped[#stripped+1] = code:sub(dirty, offset - 1)
		for n = 1, count do
			local proto, off = strip_function(code:sub(offset, -1))
			stripped[#stripped+1] = proto
			offset = offset + off - 1
		end
		offset = offset + subint(code, offset, int) * int + int
		count, offset = subint(code, offset, int)
		for n = 1, count do
			offset = offset + subint(code, offset, size) + size + int * 2
		end
		count, offset = subint(code, offset, int)
		for n = 1, count do
			offset = offset + subint(code, offset, size) + size
		end
		stripped[#stripped+1] = string.rep("\0", int * 3)
		return table.concat(stripped), offset
	end

	return code:sub(1,12) .. strip_function(code:sub(13,-1))
end


--
-- Sorting iterator functions
--

function _sortiter( t, f )
	local keys = { }

	local k, v
	for k, v in pairs(t) do
		keys[#keys+1] = k
	end

	local _pos = 0

	table.sort( keys, f )

	return function()
		_pos = _pos + 1
		if _pos <= #keys then
			return keys[_pos], t[keys[_pos]], _pos
		end
	end
end

-- the provided callback function.
function spairs(t,f)
	return _sortiter( t, f )
end

-- The table pairs are sorted by key.
function kspairs(t)
	return _sortiter( t )
end

-- The table pairs are sorted by value.
function vspairs(t)
	return _sortiter( t, function (a,b) return t[a] < t[b] end )
end


--
-- System utility functions
--

function bigendian()
	return string.byte(string.dump(function() end), 7) == 0
end

function exec(command)
	local pp   = io.popen(command)
	local data = pp:read("*a")
	pp:close()

	return data
end

function execi(command)
	local pp = io.popen(command)

	return pp and function()
		local line = pp:read()

		if not line then
			pp:close()
		end

		return line
	end
end

-- Deprecated
function execl(command)
	local pp   = io.popen(command)
	local line = ""
	local data = {}

	while true do
		line = pp:read()
		if (line == nil) then break end
		data[#data+1] = line
	end
	pp:close()

	return data
end

function ubus(object, method, data)
	if not _ubus_connection then
		_ubus_connection = _ubus.connect()
		assert(_ubus_connection, "Unable to establish ubus connection")
	end

	if object and method then
		if type(data) ~= "table" then
			data = { }
		end
		return _ubus_connection:call(object, method, data)
	elseif object then
		return _ubus_connection:signatures(object)
	else
		return _ubus_connection:objects()
	end
end

function serialize_json(x, cb)
	local js = json.stringify(x)
	if type(cb) == "function" then
		cb(js)
	else
		return js
	end
end


function libpath()
	return require "nixio.fs".dirname(ldebug.__file__)
end

function checklib(fullpathexe, wantedlib)
	local fs = require "nixio.fs"
	local haveldd = fs.access('/usr/bin/ldd')
	if not haveldd then
		return false
	end
	local libs = exec("/usr/bin/ldd " .. fullpathexe)
	if not libs then
		return false
	end
	for k, v in ipairs(split(libs)) do
		if v:find(wantedlib) then
			return true
		end
	end
	return false
end

--
-- Coroutine safe xpcall and pcall versions modified for Luci
-- original version:
-- coxpcall 1.13 - Copyright 2005 - Kepler Project (www.keplerproject.org)
--
-- Copyright © 2005 Kepler Project.
-- Permission is hereby granted, free of charge, to any person obtaining a
-- copy of this software and associated documentation files (the "Software"),
-- to deal in the Software without restriction, including without limitation
-- the rights to use, copy, modify, merge, publish, distribute, sublicense,
-- and/or sell copies of the Software, and to permit persons to whom the
-- Software is furnished to do so, subject to the following conditions:
--
-- The above copyright notice and this permission notice shall be
-- included in all copies or substantial portions of the Software.
--
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
-- EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
-- OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
-- IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
-- DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
-- TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
-- OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

local performResume, handleReturnValue
local oldpcall, oldxpcall = pcall, xpcall
coxpt = {}
setmetatable(coxpt, {__mode = "kv"})

-- Identity function for copcall
local function copcall_id(trace, ...)
  return ...
end

--				values of either the function or the error handler
function coxpcall(f, err, ...)
	local res, co = oldpcall(coroutine.create, f)
	if not res then
		local params = {...}
		local newf = function() return f(unpack(params)) end
		co = coroutine.create(newf)
	end
	local c = coroutine.running()
	coxpt[co] = coxpt[c] or c or 0

	return performResume(err, co, ...)
end

--				values of the function or the error object
function copcall(f, ...)
	return coxpcall(f, copcall_id, ...)
end

-- Handle return value of protected call
function handleReturnValue(err, co, status, ...)
	if not status then
		return false, err(debug.traceback(co, (...)), ...)
	end

	if coroutine.status(co) ~= 'suspended' then
		return true, ...
	end

	return performResume(err, co, coroutine.yield(...))
end

-- Resume execution of protected function call
function performResume(err, co, ...)
	return handleReturnValue(err, co, coroutine.resume(co, ...))
end
