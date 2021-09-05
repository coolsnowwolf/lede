-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local io = require "io"
local fs = require "nixio.fs"
local util = require "luci.util"
local nixio = require "nixio"
local debug = require "debug"
local string = require "string"
local package = require "package"

local type, loadfile = type, loadfile


module "luci.ccache"

function cache_ondemand(...)
	if debug.getinfo(1, 'S').source ~= "=?" then
		cache_enable(...)
	end
end

function cache_enable(cachepath, mode)
	cachepath = cachepath or "/tmp/luci-modulecache"
	mode = mode or "r--r--r--"

	local loader = package.loaders[2]
	local uid    = nixio.getuid()

	if not fs.stat(cachepath) then
		fs.mkdir(cachepath)
	end

	local function _encode_filename(name)
		local encoded = ""
		for i=1, #name do
			encoded = encoded .. ("%2X" % string.byte(name, i))
		end
		return encoded
	end

	local function _load_sane(file)
		local stat = fs.stat(file)
		if stat and stat.uid == uid and stat.modestr == mode then
			return loadfile(file)
		end
	end

	local function _write_sane(file, func)
		if nixio.getuid() == uid then
			local fp = io.open(file, "w")
			if fp then
				fp:write(util.get_bytecode(func))
				fp:close()
				fs.chmod(file, mode)
			end
		end
	end

	package.loaders[2] = function(mod)
		local encoded = cachepath .. "/" .. _encode_filename(mod)
		local modcons = _load_sane(encoded)
		
		if modcons then
			return modcons
		end

		-- No cachefile
		modcons = loader(mod)
		if type(modcons) == "function" then
			_write_sane(encoded, modcons)
		end
		return modcons
	end
end
