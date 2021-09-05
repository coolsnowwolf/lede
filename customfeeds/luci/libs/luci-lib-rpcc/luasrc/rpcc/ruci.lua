-- Copyright 2009 Steven Barth <steven@midlink.org>
-- Licensed to the public under the Apache License 2.0.

local util = require "luci.util"
local rawget, setmetatable = rawget, setmetatable
local ipairs = ipairs

module "luci.rpcc.ruci"


local Proxy = util.class()

function factory(rpccl)
	return {
		cursor = function(...)
			return Proxy(rpccl, rpccl:request("ruci.cursor", {...}))
		end,
		cursor_state = function(...)
			return Proxy(rpccl, rpccl:request("ruci.cursor_state", {...}))
		end
	}
end

function Proxy.__init__(self, rpccl, objid)
	self.__rpccl = rpccl
	self.__objid = objid

	setmetatable(self, {
		__index = function(self, key)
			return rawget(self, key) or Proxy[key] or function(self, ...)
				local argv = {self.__objid, ...}
				return self.__rpccl:request("ruci."..key, argv)
			end
		end
	})
end

function Proxy.foreach(self, config, section, callback)
	local sections = self.__rpccl:request("ruci.foreach", {self.__objid, config, section})
	if sections then
		for _, s in ipairs(sections) do
			callback(s)
		end
		return true
	else
		return false
	end
end
