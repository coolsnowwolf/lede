--[[
nixio - Linux I/O library for lua

Copyright 2009 Steven Barth <steven@midlink.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

$Id$
]]--

local table = require "table"
local nixio = require "nixio"
local getmetatable, assert, pairs, type = getmetatable, assert, pairs, type
local tostring = tostring

module "nixio.util"

local BUFFERSIZE = nixio.const.buffersize
local ZIOBLKSIZE = 65536
local socket = nixio.meta_socket
local tls_socket = nixio.meta_tls_socket
local file = nixio.meta_file
local uname = nixio.uname()
local ZBUG = uname.sysname == "Linux" and uname.release:sub(1, 3) == "2.4"

function consume(iter, append)
	local tbl = append or {}
	if iter then
		for obj in iter do
			tbl[#tbl+1] = obj
		end
	end
	return tbl
end

local meta = {}

function meta.is_socket(self)
	return (getmetatable(self) == socket)
end

function meta.is_tls_socket(self)
	return (getmetatable(self) == tls_socket)
end

function meta.is_file(self)
	return (getmetatable(self) == file)
end

function meta.readall(self, len)
	local block, code, msg = self:read(len or BUFFERSIZE)

	if not block then
		return nil, code, msg, ""
	elseif #block == 0 then
		return "", nil, nil, ""
	end

	local data, total = {block}, #block

	while not len or len > total do
		block, code, msg = self:read(len and (len - total) or BUFFERSIZE)

		if not block then
			return nil, code, msg, table.concat(data)
		elseif #block == 0 then
			break
		end

		data[#data+1], total = block, total + #block
	end

	local data = #data > 1 and table.concat(data) or data[1]
	return data, nil, nil, data
end
meta.recvall = meta.readall

function meta.writeall(self, data)
	data = tostring(data)
	local sent, code, msg = self:write(data)

	if not sent then
		return nil, code, msg, 0
	end

	local total = sent 

	while total < #data do
		sent, code, msg = self:write(data, total)

		if not sent then
			return nil, code, msg, total
		end

		total = total + sent
	end
	
	return total, nil, nil, total
end
meta.sendall = meta.writeall

function meta.linesource(self, limit)
	limit = limit or BUFFERSIZE
	local buffer = ""
	local bpos = 0
	return function(flush)
		local line, endp, _
		
		if flush then
			line = buffer:sub(bpos + 1)
			buffer = type(flush) == "string" and flush or ""
			bpos = 0
			return line
		end

		while not line do
			_, endp, line = buffer:find("(.-)\r?\n", bpos + 1)
			if line then
				bpos = endp
				return line
			elseif #buffer < limit + bpos then
				local newblock, code, msg = self:read(limit + bpos - #buffer)
				if not newblock then
					return nil, code, msg
				elseif #newblock == 0 then
					return nil
				end
				buffer = buffer:sub(bpos + 1) .. newblock
				bpos = 0
			else
				return nil, 0
			end
		end
	end
end

function meta.blocksource(self, bs, limit)
	bs = bs or BUFFERSIZE
	return function()
		local toread = bs
		if limit then
			if limit < 1 then
				return nil
			elseif limit < toread then
				toread = limit
			end
		end

		local block, code, msg = self:read(toread)

		if not block then
			return nil, code, msg
		elseif #block == 0 then
			return nil
		else
			if limit then
				limit = limit - #block
			end

			return block
		end
	end
end

function meta.sink(self, close)
	return function(chunk, src_err)
		if not chunk and not src_err and close then
			if self.shutdown then
				self:shutdown()
			end
			self:close()
		elseif chunk and #chunk > 0 then
			return self:writeall(chunk)
		end
		return true
	end
end

function meta.copy(self, fdout, size)
	local source = self:blocksource(nil, size)
	local sink = fdout:sink()
	local sent, chunk, code, msg = 0
	
	repeat
		chunk, code, msg = source()
		sink(chunk, code, msg)
		sent = chunk and (sent + #chunk) or sent
	until not chunk
	return not code and sent or nil, code, msg, sent
end

function meta.copyz(self, fd, size)
	local sent, lsent, code, msg = 0
	local splicable

	if not ZBUG and self:is_file() then
		local ftype = self:stat("type")
		if nixio.sendfile and fd:is_socket() and ftype == "reg" then
			repeat
				lsent, code, msg = nixio.sendfile(fd, self, size or ZIOBLKSIZE)
				if lsent then
					sent = sent + lsent
					size = size and (size - lsent)
				end
			until (not lsent or lsent == 0 or (size and size == 0))
			if lsent or (not lsent and sent == 0 and
			 code ~= nixio.const.ENOSYS and code ~= nixio.const.EINVAL) then
				return lsent and sent, code, msg, sent
			end
		elseif nixio.splice and not fd:is_tls_socket() and ftype == "fifo" then 
			splicable = true
		end
	end

	if nixio.splice and fd:is_file() and not splicable then
		splicable = not self:is_tls_socket() and fd:stat("type") == "fifo"
	end

	if splicable then
		repeat
			lsent, code, msg = nixio.splice(self, fd, size or ZIOBLKSIZE)
			if lsent then
				sent = sent + lsent
				size = size and (size - lsent)
			end
		until (not lsent or lsent == 0 or (size and size == 0))
		if lsent or (not lsent and sent == 0 and
		 code ~= nixio.const.ENOSYS and code ~= nixio.const.EINVAL) then
			return lsent and sent, code, msg, sent
		end		
	end

	return self:copy(fd, size)
end

if tls_socket then
	function tls_socket.close(self)
		return self.socket:close()
	end

	function tls_socket.getsockname(self)
		return self.socket:getsockname()
	end

	function tls_socket.getpeername(self)
		return self.socket:getpeername()
	end

	function tls_socket.getsockopt(self, ...)
		return self.socket:getsockopt(...)
	end
	tls_socket.getopt = tls_socket.getsockopt

	function tls_socket.setsockopt(self, ...)
		return self.socket:setsockopt(...)
	end
	tls_socket.setopt = tls_socket.setsockopt
end

for k, v in pairs(meta) do
	file[k] = v
	socket[k] = v
	if tls_socket then
		tls_socket[k] = v
	end
end
