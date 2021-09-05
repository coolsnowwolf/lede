-- Copyright 2009 Steven Barth <steven@midlink.org>
-- Licensed to the public under the Apache License 2.0.

local util = require "luci.util"
local json = require "luci.json"
local ltn12 = require "luci.ltn12"
local nixio = require "nixio", require "nixio.util"

local tostring, assert, setmetatable = tostring, assert, setmetatable
local error = error

module "luci.rpcc"

RQLIMIT = 32 * nixio.const.buffersize

Client = util.class()

function Client.__init__(self, fd, v1)
	self.fd = fd
	self.uniqueid = tostring(self):match("0x([a-f0-9]+)")
	self.msgid = 1
	self.v1 = v1
end

function Client.request(self, method, params, notification)
	local oldchunk = self.decoder and self.decoder.chunk
	self.decoder = json.ActiveDecoder(self.fd:blocksource(nil, RQLIMIT))
	self.decoder.chunk = oldchunk
	
	local reqid = self.msgid .. self.uniqueid
	local reqdata = json.Encoder({
		id = (not notification) and (self.msgid .. self.uniqueid) or nil,
		jsonrpc = (not self.v1) and "2.0" or nil,
		method = method,
		params = params
	})
	ltn12.pump.all(reqdata:source(), self.fd:sink())
	if not notification then
		self.msgid = self.msgid + 1
		local response = self.decoder:get()
		assert(response.id == reqid, "Invalid response id")
		if response.error then
			error(response.error.message or response.error)
		end
		return response.result
	end
end

function Client.proxy(self, prefix)
	prefix = prefix or ""
	return setmetatable({}, {
		__call = function(proxy, ...)
			return self:request(prefix, {...})
		end,
		__index = function(proxy, name)
			return self:proxy(prefix .. name .. ".")
		end
	})
end