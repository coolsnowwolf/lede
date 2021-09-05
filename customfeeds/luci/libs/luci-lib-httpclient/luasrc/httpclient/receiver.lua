-- Copyright 2009 Steven Barth <steven@midlink.org>
-- Licensed to the public under the Apache License 2.0.

require "nixio.util"
local nixio = require "nixio"
local httpc = require "luci.httpclient"
local ltn12 = require "luci.ltn12"

local print, tonumber, require, unpack = print, tonumber, require, unpack

module "luci.httpclient.receiver"

local function prepare_fd(target)
	-- Open fd for appending
	local oflags = nixio.open_flags("wronly", "creat")
	local file, code, msg = nixio.open(target, oflags)
	if not file then
		return file, code, msg
	end
	
	-- Acquire lock
	local stat, code, msg = file:lock("tlock")
	if not stat then
		return stat, code, msg
	end
	
	file:seek(0, "end") 
	
	return file
end

local function splice_async(sock, pipeout, pipein, file, cb)
	local ssize = 65536
	local smode = nixio.splice_flags("move", "more", "nonblock")

	-- Set pipe non-blocking otherwise we might end in a deadlock
	local stat, code, msg = pipein:setblocking(false)
	if stat then
		stat, code, msg = pipeout:setblocking(false)
	end
	if not stat then
		return stat, code, msg
	end
	
	
	local pollsock = {
		{fd=sock, events=nixio.poll_flags("in")}
	}
	
	local pollfile = {
		{fd=file, events=nixio.poll_flags("out")}
	}
	
	local done
	local active -- Older splice implementations sometimes don't detect EOS
	
	repeat
		active = false
		
		-- Socket -> Pipe
		repeat
			nixio.poll(pollsock, 15000)
		
			stat, code, msg = nixio.splice(sock, pipeout, ssize, smode)
			if stat == nil then
				return stat, code, msg
			elseif stat == 0 then
				done = true
				break
			elseif stat then
				active = true
			end
		until stat == false
		
		-- Pipe -> File
		repeat
			nixio.poll(pollfile, 15000)
		
			stat, code, msg = nixio.splice(pipein, file, ssize, smode)
			if stat == nil then
				return stat, code, msg
			elseif stat then
				active = true
			end
		until stat == false
		
		if cb then
			cb(file)
		end
		
		if not active then
			-- We did not splice any data, maybe EOS, fallback to default
			return false
		end
	until done
	
	pipein:close()
	pipeout:close()
	sock:close()
	file:close()
	return true
end

local function splice_sync(sock, pipeout, pipein, file, cb)
	local os = require "os"
	local ssize = 65536
	local smode = nixio.splice_flags("move", "more")
	local stat
	
	-- This is probably the only forking http-client ;-)
	local pid, code, msg = nixio.fork()
	if not pid then
		return pid, code, msg
	elseif pid == 0 then
		pipein:close()
		file:close()

		repeat
			stat, code = nixio.splice(sock, pipeout, ssize, smode)
		until not stat or stat == 0
	
		pipeout:close()
		sock:close()
		os.exit(stat or code)
	else
		pipeout:close()
		sock:close()
		
		repeat
			stat, code, msg = nixio.splice(pipein, file, ssize, smode)
			if cb then
				cb(file)
			end
		until not stat or stat == 0
		
		pipein:close()
		file:close()
		
		if not stat then
			nixio.kill(pid, 15)
			nixio.wait(pid)
			return stat, code, msg
		else
			pid, msg, code = nixio.wait(pid)
			if msg == "exited" then
				if code == 0 then
					return true
				else
					return nil, code, nixio.strerror(code)
				end
			else
				return nil, -0x11, "broken pump"
			end
		end
	end
end

function request_to_file(uri, target, options, cbs)
	options = options or {}
	cbs = cbs or {}
	options.headers = options.headers or {}
	local hdr = options.headers
	local file, code, msg
	
	if target then
		file, code, msg = prepare_fd(target)
		if not file then
			return file, code, msg
		end
	
		local off = file:tell()
		
		-- Set content range
		if off > 0 then
			hdr.Range = hdr.Range or ("bytes=" .. off .. "-")  
		end
	end
	
	local code, resp, buffer, sock = httpc.request_raw(uri, options)
	if not code then
		-- No success
		if file then
			file:close()
		end
		return code, resp, buffer
	elseif hdr.Range and code ~= 206 then
		-- We wanted a part but we got the while file
		sock:close()
		if file then
			file:close()
		end
		return nil, -4, code, resp
	elseif not hdr.Range and code ~= 200 then
		-- We encountered an error
		sock:close()
		if file then
			file:close()
		end
		return nil, -4, code, resp
	end
	
	if cbs.on_header then
		local stat = {cbs.on_header(file, code, resp)}
		if stat[1] == false then
			if file then
				file:close()
			end
			sock:close()
			return unpack(stat)
		elseif stat[2] then
			file = file and stat[2]
		end
	end
	
	if not file then
		return nil, -5, "no target given"
	end

	local chunked = resp.headers["Transfer-Encoding"] == "chunked"
	local stat

	-- Write the buffer to file
	file:writeall(buffer)
	
	repeat
		if not options.splice or not sock:is_socket() or chunked then
			break
		end
		
		-- This is a plain TCP socket and there is no encoding so we can splice
	
		local pipein, pipeout, msg = nixio.pipe()
		if not pipein then
			sock:close()
			file:close()
			return pipein, pipeout, msg
		end
		
		
		-- Adjust splice values
		local ssize = 65536
		local smode = nixio.splice_flags("move", "more")
		
		-- Splicing 512 bytes should never block on a fresh pipe
		local stat, code, msg = nixio.splice(sock, pipeout, 512, smode)
		if stat == nil then
			break
		end
		
		-- Now do the real splicing
		local cb = cbs.on_write
		if options.splice == "asynchronous" then
			stat, code, msg = splice_async(sock, pipeout, pipein, file, cb)
		elseif options.splice == "synchronous" then
			stat, code, msg = splice_sync(sock, pipeout, pipein, file, cb)
		else
			break
		end
		
		if stat == false then
			break
		end

		return stat, code, msg
	until true
	
	local src = chunked and httpc.chunksource(sock) or sock:blocksource()
	local snk = file:sink()
	
	if cbs.on_write then
		src = ltn12.source.chain(src, function(chunk)
			cbs.on_write(file)
			return chunk
		end)
	end
	
	-- Fallback to read/write
	stat, code, msg = ltn12.pump.all(src, snk)

	file:close()
	sock:close()
	return stat and true, code, msg
end

