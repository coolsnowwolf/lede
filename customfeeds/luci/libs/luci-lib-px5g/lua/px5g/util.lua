--[[
 * px5g - Embedded x509 key and certificate generator based on PolarSSL
 *
 *   Copyright (C) 2009 Steven Barth <steven@midlink.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License, version 2.1 as published by the Free Software Foundation.
]]--

local nixio = require "nixio"
local table = require "table"

module "px5g.util"

local preamble = {
	key = "-----BEGIN RSA PRIVATE KEY-----",
	cert = "-----BEGIN CERTIFICATE-----",
	request = "-----BEGIN CERTIFICATE REQUEST-----"
}

local postamble = {
	key = "-----END RSA PRIVATE KEY-----",
	cert = "-----END CERTIFICATE-----",
	request = "-----END CERTIFICATE REQUEST-----"
}

function der2pem(data, type)
	local b64 = nixio.bin.b64encode(data)
	
	local outdata = {preamble[type]}
	for i = 1, #b64, 64 do
		outdata[#outdata + 1] = b64:sub(i, i + 63) 
	end
	outdata[#outdata + 1] = postamble[type]
	outdata[#outdata + 1] = ""
	
	return table.concat(outdata, "\n")
end

function pem2der(data)
	local b64 = data:gsub({["\n"] = "", ["%-%-%-%-%-.-%-%-%-%-%-"] = ""})
	return nixio.bin.b64decode(b64)
end