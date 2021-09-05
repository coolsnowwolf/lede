-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Licensed to the public under the Apache License 2.0.

exectime = os.clock()
module("luci.sgi.cgi", package.seeall)
local ltn12 = require("luci.ltn12")
require("nixio.util")
require("luci.http")
require("luci.sys")
require("luci.dispatcher")

-- Limited source to avoid endless blocking
local function limitsource(handle, limit)
	limit = limit or 0
	local BLOCKSIZE = ltn12.BLOCKSIZE

	return function()
		if limit < 1 then
			handle:close()
			return nil
		else
			local read = (limit > BLOCKSIZE) and BLOCKSIZE or limit
			limit = limit - read

			local chunk = handle:read(read)
			if not chunk then handle:close() end
			return chunk
		end
	end
end

function run()
	local r = luci.http.Request(
		luci.sys.getenv(),
		limitsource(io.stdin, tonumber(luci.sys.getenv("CONTENT_LENGTH"))),
		ltn12.sink.file(io.stderr)
	)
	
	local x = coroutine.create(luci.dispatcher.httpdispatch)
	local hcache = ""
	local active = true
	
	while coroutine.status(x) ~= "dead" do
		local res, id, data1, data2 = coroutine.resume(x, r)

		if not res then
			print("Status: 500 Internal Server Error")
			print("Content-Type: text/plain\n")
			print(id)
			break;
		end

		if active then
			if id == 1 then
				io.write("Status: " .. tostring(data1) .. " " .. data2 .. "\r\n")
			elseif id == 2 then
				hcache = hcache .. data1 .. ": " .. data2 .. "\r\n"
			elseif id == 3 then
				io.write(hcache)
				io.write("\r\n")
			elseif id == 4 then
				io.write(tostring(data1 or ""))
			elseif id == 5 then
				io.flush()
				io.close()
				active = false
			elseif id == 6 then
				data1:copyz(nixio.stdout, data2)
				data1:close()
			end
		end
	end
end
