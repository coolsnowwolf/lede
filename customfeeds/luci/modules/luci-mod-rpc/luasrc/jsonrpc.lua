-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

module("luci.jsonrpc", package.seeall)
require "luci.json"

function resolve(mod, method)
	local path = luci.util.split(method, ".")
	
	for j=1, #path-1 do
		if not type(mod) == "table" then
			break
		end
		mod = rawget(mod, path[j])
		if not mod then
			break
		end
	end
	mod = type(mod) == "table" and rawget(mod, path[#path]) or nil
	if type(mod) == "function" then
		return mod
	end
end

function handle(tbl, rawsource, ...)
	local decoder = luci.json.Decoder()
	local stat = luci.ltn12.pump.all(rawsource, decoder:sink())
	local json = decoder:get()
	local response
	local success = false
	
	if stat then
		if type(json.method) == "string"
		 and (not json.params or type(json.params) == "table") then
			local method = resolve(tbl, json.method)
			if method then
				response = reply(json.jsonrpc, json.id,
				 proxy(method, unpack(json.params or {})))
			else
		 		response = reply(json.jsonrpc, json.id,
			 	 nil, {code=-32601, message="Method not found."})
			end
		else
			response = reply(json.jsonrpc, json.id,
			 nil, {code=-32600, message="Invalid request."})
		end
	else
		response = reply("2.0", nil,
		 nil, {code=-32700, message="Parse error."})
	end

	return luci.json.Encoder(response, ...):source()
end

function reply(jsonrpc, id, res, err)
	require "luci.json"
	id = id or luci.json.null
	
	-- 1.0 compatibility
	if jsonrpc ~= "2.0" then
		jsonrpc = nil
		res = res or luci.json.null
		err = err or luci.json.null
	end
	
	return {id=id, result=res, error=err, jsonrpc=jsonrpc}
end

function proxy(method, ...)
	local res = {luci.util.copcall(method, ...)}
	local stat = table.remove(res, 1)
	
	if not stat then
		return nil, {code=-32602, message="Invalid params.", data=table.remove(res, 1)} 
	else
		if #res <= 1 then
			return res[1] or luci.json.null
		else
			return res
		end
	end
end
