-- Copyright 2008 Freifunk Leipzig / Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

module("luci.statistics.datatree", package.seeall)

local util = require("luci.util")
local sys  = require("luci.sys")
local fs   = require("nixio.fs")
local uci  = require("luci.model.uci").cursor()
local sections = uci:get_all("luci_statistics")


Instance = util.class()

function Instance.__init__( self, host )
	self._host    = host or sys.hostname()
	self._libdir  = "/usr/lib/collectd"
	self._rrddir  = "/tmp/rrd"

	if sections and sections.collectd then
		self._host    = host or sections.collectd.Hostname or sys.hostname()
		self._libdir  = sections.collectd.PluginDir        or "/usr/lib/collectd"
	end
	if sections and sections.collectd_rrdtool then
		self._rrddir  = sections.collectd_rrdtool.DataDir  or "/tmp/rrd"
	end

	self._libdir  = self._libdir:gsub("/$","")
	self._rrddir  = self._rrddir:gsub("/$","")
	self._plugins = { }

	self:_scan()
end

function Instance._mkpath( self, plugin, pinstance )
	local dir = self._rrddir .. "/" .. self._host

	if type(plugin) == "string" and plugin:len() > 0 then
		dir = dir .. "/" .. plugin

		if type(pinstance) == "string" and pinstance:len() > 0 then
			dir = dir .. "-" .. pinstance
		end
	end

	return dir
end

function Instance._ls( self, ... )
	local ditr = fs.dir(self:_mkpath(...))
	if ditr then
		local dirs = { }
		while true do
			local d = ditr()
			if not d then break end
			dirs[#dirs+1] = d
		end
		return dirs
	end
end

function Instance._notzero( self, table )
	for k in pairs(table) do
		return true
	end

	return false
end

function Instance._scan( self )
	local dirs = self:_ls()
	if not dirs then
		return
	end

--	for i, plugin in ipairs( dirs ) do
--		if plugin:match("%w+.so") then
--			self._plugins[ plugin:gsub("%.so$", "") ] = { }
--		end
--	end

	for _, dir in ipairs(dirs) do
		if dir ~= "." and dir ~= ".." and
		   fs.stat(self:_mkpath(dir)).type == "dir"
		then
			local plugin = dir:gsub("%-.+$", "")
			if not self._plugins[plugin] then
				self._plugins[plugin] = { }
			end
		end
	end

	for plugin, instances in pairs( self._plugins ) do

		local dirs = self:_ls()

		if type(dirs) == "table" then
			for i, dir in ipairs(dirs) do
				if dir:find( plugin .. "%-" ) or dir == plugin then
					local instance = ""

					if dir ~= plugin then
						instance = dir:gsub( plugin .. "%-", "", 1 )
					end

					instances[instance] = { }
				end
			end
		end

		for instance, data_instances in pairs( instances ) do

			dirs = self:_ls(plugin, instance)

			if type(dirs) == "table" then
				for i, file in ipairs(dirs) do
					if file:find("%.rrd") then
						file = file:gsub("%.rrd","")

						local data_type
						local data_instance

						if file:find("%-") then
							data_type     = file:gsub( "%-.+","" )
							data_instance = file:gsub( "[^%-]-%-", "", 1 )
						else
							data_type     = file
							data_instance = ""
						end

						if not data_instances[data_type] then
							data_instances[data_type] = { data_instance }
						else
							table.insert( data_instances[data_type], data_instance )
						end
					end
				end
			end
		end
	end
end


function Instance.plugins( self )
	local rv = { }

	for plugin, val in pairs( self._plugins ) do
		if self:_notzero( val ) then
			table.insert( rv, plugin )
		end
	end

	return rv
end

function Instance.plugin_instances( self, plugin )
	local rv = { }

	for instance, val in pairs( self._plugins[plugin] ) do
		table.insert( rv, instance )
	end

	return rv
end

function Instance.data_types( self, plugin, instance )
	local rv = { }
	local p  = self._plugins[plugin]

	if type(p) == "table" and type(p[instance]) == "table" then
		for type, val in pairs(p[instance]) do
			table.insert( rv, type )
		end
	end

	return rv
end

function Instance.data_instances( self, plugin, instance, dtype )
	local rv = { }
	local p  = self._plugins[plugin]

	if type(p) == "table" and type(p[instance]) == "table" and type(p[instance][dtype]) == "table" then
		for i, instance in ipairs(p[instance][dtype]) do
			table.insert( rv, instance )
		end
	end

	return rv
end

function Instance.host_instances( self )
	local hosts_path = fs.glob(self._rrddir..'/*')
	local hosts = { }

	if hosts_path then
		local path
		for path in hosts_path do
			hosts[#hosts+1] = fs.basename(path)
		end
	end

	return hosts
end

