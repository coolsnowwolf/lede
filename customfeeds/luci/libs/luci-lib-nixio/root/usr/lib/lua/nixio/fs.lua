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
local type, ipairs, setmetatable = type, ipairs, setmetatable
require "nixio.util"


module ("nixio.fs", function(m) setmetatable(m, {__index = nixio.fs}) end)


function readfile(path, limit)
	local fd, code, msg = nixio.open(path, "r")
	local data
	if not fd then
		return nil, code, msg
	end
	
	data, code, msg = fd:readall(limit)
	
	fd:close()
	return data, code, msg
end


function writefile(path, data)
	local fd, code, msg, stat = nixio.open(path, "w")
	if not fd then
		return nil, code, msg
	end
	
	stat, code, msg = fd:writeall(data)
	
	fd:close()
	return stat, code, msg
end

function datacopy(src, dest, size)
	local fdin, code, msg = nixio.open(src, "r")
	if not fdin then
		return nil, code, msg
	end
	
	local fdout, code, msg = nixio.open(dest, "w")
	if not fdout then
		return nil, code, msg
	end	
	
	local stat, code, msg, sent = fdin:copy(fdout, size)
	fdin:close()
	fdout:close()

	return stat, code, msg, sent
end

function copy(src, dest)
	local stat, code, msg, res = nixio.fs.lstat(src)
	if not stat then
		return nil, code, msg
	end
	
	if stat.type == "dir" then
		if nixio.fs.stat(dest, type) ~= "dir" then
			res, code, msg = nixio.fs.mkdir(dest)
		else
			stat = true
		end
	elseif stat.type == "lnk" then
		res, code, msg = nixio.fs.symlink(nixio.fs.readlink(src), dest)
	elseif stat.type == "reg" then
		res, code, msg = datacopy(src, dest)
	end
	
	if not res then
		return nil, code, msg
	end
	
	nixio.fs.utimes(dest, stat.atime, stat.mtime)
	
	if nixio.fs.lchown then
		nixio.fs.lchown(dest, stat.uid, stat.gid)
	end
	
	if stat.type ~= "lnk" then
		nixio.fs.chmod(dest, stat.modedec)
	end
	
	return true
end

function move(src, dest)
	local stat, code, msg = nixio.fs.rename(src, dest)
	if not stat and code == nixio.const.EXDEV then
		stat, code, msg = copy(src, dest)
		if stat then
			stat, code, msg = nixio.fs.unlink(src)
		end
	end
	return stat, code, msg
end

function mkdirr(dest, mode)
	if nixio.fs.stat(dest, "type") == "dir" then
		return true
	else
		local stat, code, msg = nixio.fs.mkdir(dest, mode)
		if not stat and code == nixio.const.ENOENT then
			stat, code, msg = mkdirr(nixio.fs.dirname(dest), mode)
			if stat then
				stat, code, msg = nixio.fs.mkdir(dest, mode)
			end
		end
		return stat, code, msg
	end
end

local function _recurse(cb, src, dest)
	local type = nixio.fs.lstat(src, "type")
	if type ~= "dir" then
		return cb(src, dest)
	else
		local stat, se, code, msg, s, c, m = true, nixio.const.sep
		if dest then
			s, c, m = cb(src, dest)
			stat, code, msg = stat and s, c or code, m or msg
		end

		for e in nixio.fs.dir(src) do
			if dest then
				s, c, m = _recurse(cb, src .. se .. e, dest .. se .. e)
			else
				s, c, m = _recurse(cb, src .. se .. e)
			end
			stat, code, msg = stat and s, c or code, m or msg
		end

		if not dest then							-- Postfix
			s, c, m = cb(src)
			stat, code, msg = stat and s, c or code, m or msg
		end

		return stat, code, msg
	end
end

function copyr(src, dest)
	return _recurse(copy, src, dest)
end

function mover(src, dest)
	local stat, code, msg = nixio.fs.rename(src, dest)
	if not stat and code == nixio.const.EXDEV then
		stat, code, msg = _recurse(copy, src, dest)
		if stat then
			stat, code, msg = _recurse(nixio.fs.remove, src)
		end
	end
	return stat, code, msg
end

function remover(src)
	return _recurse(nixio.fs.remove, src)
end