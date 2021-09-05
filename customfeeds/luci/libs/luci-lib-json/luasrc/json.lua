-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local nixio     = require "nixio"
local util      = require "luci.util"
local table     = require "table"
local string    = require "string"
local coroutine = require "coroutine"

local assert    = assert
local tonumber  = tonumber
local tostring  = tostring
local error     = error
local type	    = type
local pairs	    = pairs
local ipairs    = ipairs
local next      = next
local pcall		= pcall

local band      = nixio.bit.band
local bor       = nixio.bit.bor
local rshift    = nixio.bit.rshift
local char      = string.char

local getmetatable = getmetatable

module "luci.json"


function decode(json, ...)
	local a = ActiveDecoder(function() return nil end, ...)
	a.chunk = json
	local s, obj = pcall(a.get, a)
	return s and obj or nil
end


function encode(obj, ...)
	local out = {}
	local e = Encoder(obj, 1, ...):source()
	local chnk, err
	repeat
		chnk, err = e()
		out[#out+1] = chnk
	until not chnk
	return not err and table.concat(out) or nil
end


function null()
	return null
end

Encoder = util.class()

function Encoder.__init__(self, data, buffersize, fastescape)
	self.data = data
	self.buffersize = buffersize or 512
	self.buffer = ""
	self.fastescape = fastescape

	getmetatable(self).__call = Encoder.source
end

function Encoder.source(self)
	local source = coroutine.create(self.dispatch)
	return function()
		local res, data = coroutine.resume(source, self, self.data, true)
		if res then
			return data
		else
			return nil, data
		end
	end
end

function Encoder.dispatch(self, data, start)
	local parser = self.parsers[type(data)]

	parser(self, data)

	if start then
		if #self.buffer > 0 then
			coroutine.yield(self.buffer)
		end

		coroutine.yield()
	end
end

function Encoder.put(self, chunk)
	if self.buffersize < 2 then
		coroutine.yield(chunk)
	else
		if #self.buffer + #chunk > self.buffersize then
			local written = 0
			local fbuffer = self.buffersize - #self.buffer

			coroutine.yield(self.buffer .. chunk:sub(written + 1, fbuffer))
			written = fbuffer

			while #chunk - written > self.buffersize do
				fbuffer = written + self.buffersize
				coroutine.yield(chunk:sub(written + 1, fbuffer))
				written = fbuffer
			end

			self.buffer = chunk:sub(written + 1)
		else
			self.buffer = self.buffer .. chunk
		end
	end
end

function Encoder.parse_nil(self)
	self:put("null")
end

function Encoder.parse_bool(self, obj)
	self:put(obj and "true" or "false")
end

function Encoder.parse_number(self, obj)
	self:put(tostring(obj))
end

function Encoder.parse_string(self, obj)
	if self.fastescape then
		self:put('"' .. obj:gsub('\\', '\\\\'):gsub('"', '\\"') .. '"')
	else
		self:put('"' ..
			obj:gsub('[%c\\"]',
				function(char)
					return '\\u00%02x' % char:byte()
				end
			)
		.. '"')
	end
end

function Encoder.parse_iter(self, obj)
	if obj == null then
		return self:put("null")
	end

	if type(obj) == "table" and (#obj == 0 and next(obj)) then
		self:put("{")
		local first = true

		for key, entry in pairs(obj) do
			if key ~= null then
				first = first or self:put(",")
				first = first and false
				self:parse_string(tostring(key))
				self:put(":")
				self:dispatch(entry)
			end
		end

		self:put("}")
	else
		self:put("[")
		local first = true

		if type(obj) == "table" then
			for i=1, #obj do
				first = first or self:put(",")
				first = first and nil
				self:dispatch(obj[i])
			end
		else
			for entry in obj do
				first = first or self:put(",")
				first = first and nil
				self:dispatch(entry)
			end
		end

		self:put("]")
	end
end

function Encoder.parse_udata(self, obj)
	return self:parse_string(tostring(obj))
end

Encoder.parsers = {
	['nil']      = Encoder.parse_nil,
	['table']    = Encoder.parse_iter,
	['number']   = Encoder.parse_number,
	['string']   = Encoder.parse_string,
	['boolean']  = Encoder.parse_bool,
	['function'] = Encoder.parse_iter,
	['userdata'] = Encoder.parse_udata,
}


Decoder = util.class()

function Decoder.__init__(self, customnull)
	self.cnull = customnull
	getmetatable(self).__call = Decoder.sink
end

function Decoder.sink(self)
	local sink = coroutine.create(self.dispatch)
	return function(...)
		return coroutine.resume(sink, self, ...)
	end
end


function Decoder.get(self)
	return self.data
end

function Decoder.dispatch(self, chunk, src_err, strict)
	local robject, object
	local oset = false

	while chunk do
		while chunk and #chunk < 1 do
			chunk = self:fetch()
		end

		assert(not strict or chunk, "Unexpected EOS")
		if not chunk then break end

		local char   = chunk:sub(1, 1)
		local parser = self.parsers[char]
		 or (char:match("%s")     and self.parse_space)
		 or (char:match("[0-9-]") and self.parse_number)
		 or error("Unexpected char '%s'" % char)

		chunk, robject = parser(self, chunk)

		if parser ~= self.parse_space then
			assert(not oset, "Scope violation: Too many objects")
			object = robject
			oset = true

			if strict then
				return chunk, object
			end
		end
	end

	assert(not src_err, src_err)
	assert(oset, "Unexpected EOS")

	self.data = object
end


function Decoder.fetch(self)
	local tself, chunk, src_err = coroutine.yield()
	assert(chunk or not src_err, src_err)
	return chunk
end


function Decoder.fetch_atleast(self, chunk, bytes)
	while #chunk < bytes do
		local nchunk = self:fetch()
		assert(nchunk, "Unexpected EOS")
		chunk = chunk .. nchunk
	end

	return chunk
end


function Decoder.fetch_until(self, chunk, pattern)
	local start = chunk:find(pattern)

	while not start do
		local nchunk = self:fetch()
		assert(nchunk, "Unexpected EOS")
		chunk = chunk .. nchunk
		start = chunk:find(pattern)
	end

	return chunk, start
end


function Decoder.parse_space(self, chunk)
	local start = chunk:find("[^%s]")

	while not start do
		chunk = self:fetch()
		if not chunk then
			return nil
		end
		start = chunk:find("[^%s]")
	end

	return chunk:sub(start)
end


function Decoder.parse_literal(self, chunk, literal, value)
	chunk = self:fetch_atleast(chunk, #literal)
	assert(chunk:sub(1, #literal) == literal, "Invalid character sequence")
	return chunk:sub(#literal + 1), value
end


function Decoder.parse_null(self, chunk)
	return self:parse_literal(chunk, "null", self.cnull and null)
end


function Decoder.parse_true(self, chunk)
	return self:parse_literal(chunk, "true", true)
end


function Decoder.parse_false(self, chunk)
	return self:parse_literal(chunk, "false", false)
end


function Decoder.parse_number(self, chunk)
	local chunk, start = self:fetch_until(chunk, "[^0-9eE.+-]")
	local number = tonumber(chunk:sub(1, start - 1))
	assert(number, "Invalid number specification")
	return chunk:sub(start), number
end


function Decoder.parse_string(self, chunk)
	local str = ""
	local object = nil
	assert(chunk:sub(1, 1) == '"', 'Expected "')
	chunk = chunk:sub(2)

	while true do
		local spos = chunk:find('[\\"]')
		if spos then
			str = str .. chunk:sub(1, spos - 1)

			local char = chunk:sub(spos, spos)
			if char == '"' then				-- String end
				chunk = chunk:sub(spos + 1)
				break
			elseif char == "\\" then 		-- Escape sequence
				chunk, object = self:parse_escape(chunk:sub(spos))
				str = str .. object
			end
		else
			str = str .. chunk
			chunk = self:fetch()
			assert(chunk, "Unexpected EOS while parsing a string")
		end
	end

	return chunk, str
end


function Decoder.utf8_encode(self, s1, s2)
	local n = s1 * 256 + s2

	if n >= 0 and n <= 0x7F then
		return char(n)
	elseif n >= 0 and n <= 0x7FF then
		return char(
			bor(band(rshift(n,  6), 0x1F), 0xC0),
			bor(band(n,             0x3F), 0x80)
		)
	elseif n >= 0 and n <= 0xFFFF then
		return char(
			bor(band(rshift(n, 12), 0x0F), 0xE0),
			bor(band(rshift(n,  6), 0x3F), 0x80),
			bor(band(n,             0x3F), 0x80)
		)
	elseif n >= 0 and n <= 0x10FFFF then
		return char(
			bor(band(rshift(n, 18), 0x07), 0xF0),
			bor(band(rshift(n, 12), 0x3F), 0x80),
			bor(band(rshift(n,  6), 0x3F), 0x80),
			bor(band(n,             0x3F), 0x80)
		)
	else
		return "?"
	end
end


function Decoder.parse_escape(self, chunk)
	local str = ""
	chunk = self:fetch_atleast(chunk:sub(2), 1)
	local char = chunk:sub(1, 1)
	chunk = chunk:sub(2)

	if char == '"' then
		return chunk, '"'
	elseif char == "\\" then
		return chunk, "\\"
	elseif char == "u" then
		chunk = self:fetch_atleast(chunk, 4)
		local s1, s2 = chunk:sub(1, 2), chunk:sub(3, 4)
		s1, s2 = tonumber(s1, 16), tonumber(s2, 16)
		assert(s1 and s2, "Invalid Unicode character")

		return chunk:sub(5), self:utf8_encode(s1, s2)
	elseif char == "/" then
		return chunk, "/"
	elseif char == "b" then
		return chunk, "\b"
	elseif char == "f" then
		return chunk, "\f"
	elseif char == "n" then
		return chunk, "\n"
	elseif char == "r" then
		return chunk, "\r"
	elseif char == "t" then
		return chunk, "\t"
	else
		error("Unexpected escaping sequence '\\%s'" % char)
	end
end


function Decoder.parse_array(self, chunk)
	chunk = chunk:sub(2)
	local array = {}
	local nextp = 1

	local chunk, object = self:parse_delimiter(chunk, "%]")

	if object then
		return chunk, array
	end

	repeat
		chunk, object = self:dispatch(chunk, nil, true)
		table.insert(array, nextp, object)
		nextp = nextp + 1

		chunk, object = self:parse_delimiter(chunk, ",%]")
		assert(object, "Delimiter expected")
	until object == "]"

	return chunk, array
end


function Decoder.parse_object(self, chunk)
	chunk = chunk:sub(2)
	local array = {}
	local name

	local chunk, object = self:parse_delimiter(chunk, "}")

	if object then
		return chunk, array
	end

	repeat
		chunk = self:parse_space(chunk)
		assert(chunk, "Unexpected EOS")

		chunk, name   = self:parse_string(chunk)

		chunk, object = self:parse_delimiter(chunk, ":")
		assert(object, "Separator expected")

		chunk, object = self:dispatch(chunk, nil, true)
		array[name] = object

		chunk, object = self:parse_delimiter(chunk, ",}")
		assert(object, "Delimiter expected")
	until object == "}"

	return chunk, array
end


function Decoder.parse_delimiter(self, chunk, delimiter)
	while true do
		chunk = self:fetch_atleast(chunk, 1)
		local char = chunk:sub(1, 1)
		if char:match("%s") then
			chunk = self:parse_space(chunk)
			assert(chunk, "Unexpected EOS")
		elseif char:match("[%s]" % delimiter) then
			return chunk:sub(2), char
		else
			return chunk, nil
		end
	end
end


Decoder.parsers = {
	['"'] = Decoder.parse_string,
	['t'] = Decoder.parse_true,
	['f'] = Decoder.parse_false,
	['n'] = Decoder.parse_null,
	['['] = Decoder.parse_array,
	['{'] = Decoder.parse_object
}


ActiveDecoder = util.class(Decoder)

function ActiveDecoder.__init__(self, source, customnull)
	Decoder.__init__(self, customnull)
	self.source = source
	self.chunk = nil
	getmetatable(self).__call = self.get
end


function ActiveDecoder.get(self)
	local chunk, src_err, object
	if not self.chunk then
		chunk, src_err = self.source()
	else
		chunk = self.chunk
	end

	self.chunk, object = self:dispatch(chunk, src_err, true)
	return object
end


function ActiveDecoder.fetch(self)
	local chunk, src_err = self.source()
	assert(chunk or not src_err, src_err)
	return chunk
end
