local debug = require "debug"
local io = require "io"
local collectgarbage, floor = collectgarbage, math.floor

module "luci.debug"
__file__ = debug.getinfo(1, 'S').source:sub(2)

-- Enables the memory tracer with given flags and returns a function to disable the tracer again
function trap_memtrace(flags, dest)
	flags = flags or "clr"
	local tracefile = io.open(dest or "/tmp/memtrace", "w")
	local peak = 0

	local function trap(what, line)
		local info = debug.getinfo(2, "Sn")
		local size = floor(collectgarbage("count"))
		if size > peak then
			peak = size
		end
		if tracefile then
			tracefile:write(
				"[", what, "] ", info.source, ":", (line or "?"), "\t",
				(info.namewhat or ""), "\t",
				(info.name or ""), "\t",
				size, " (", peak, ")\n"
			)
		end
	end

	debug.sethook(trap, flags)

	return function()
		debug.sethook()
		tracefile:close()
	end
end

