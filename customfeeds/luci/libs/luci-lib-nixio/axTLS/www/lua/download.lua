#!/usr/local/bin/lua

require"luasocket"

function receive (connection)
	connection:settimeout(0)
	local s, status = connection:receive (2^10)
	if status == "timeout" then
		coroutine.yield (connection)
	end
	return s, status
end

function download (host, file, outfile)
	--local f = assert (io.open (outfile, "w"))
	local c = assert (socket.connect (host, 80))
	c:send ("GET "..file.." HTTP/1.0\r\n\r\n")
	while true do
		local s, status = receive (c)
		--f:write (s)
		if status == "closed" then
			break
		end
	end
	c:close()
	--f:close()
end

local threads = {}
function get (host, file, outfile)
	print (string.format ("Downloading %s from %s to %s", file, host, outfile))
	local co = coroutine.create (function ()
		return download (host, file, outfile)
	end)
	table.insert (threads, co)
end

function dispatcher ()
	while true do
		local n = table.getn (threads)
		if n == 0 then
			break
		end
		local connections = {}
		for i = 1, n do
			local status, res = coroutine.resume (threads[i])
			if not res then
				table.remove (threads, i)
				break
			else
				table.insert (connections, res)
			end
		end
		if table.getn (connections) == n then
			socket.select (connections)
		end
	end
end

local url = arg[1]
if not url then
	print (string.format ("usage: %s url [times]", arg[0]))
	os.exit()
end
local times = arg[2] or 5

url = string.gsub (url, "^http.?://", "")
local _, _, host, file = string.find (url, "^([^/]+)(/.*)")
local _, _, fn = string.find (file, "([^/]+)$")

for i = 1, times do
	get (host, file, fn..i)
end

dispatcher ()
