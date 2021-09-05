-- Copyright 2008 Freifunk Leipzig / Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

-- This class contains functions to parse, compare and format http dates.
module("luci.http.date", package.seeall)

require("luci.sys.zoneinfo")


MONTHS = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug",
	"Sep", "Oct", "Nov", "Dec"
}

function tz_offset(tz)

	if type(tz) == "string" then

		-- check for a numeric identifier
		local s, v = tz:match("([%+%-])([0-9]+)")
		if s == '+' then s = 1 else s = -1 end
		if v then v = tonumber(v) end

		if s and v then
			return s * 60 * ( math.floor( v / 100 ) * 60 + ( v % 100 ) )

		-- lookup symbolic tz
		elseif luci.sys.zoneinfo.OFFSET[tz:lower()] then
			return luci.sys.zoneinfo.OFFSET[tz:lower()]
		end

	end

	-- bad luck
	return 0
end

function to_unix(date)

	local wd, day, mon, yr, hr, min, sec, tz = date:match(
		"([A-Z][a-z][a-z]), ([0-9]+) " ..
		"([A-Z][a-z][a-z]) ([0-9]+) " ..
		"([0-9]+):([0-9]+):([0-9]+) " ..
		"([A-Z0-9%+%-]+)"
	)

	if day and mon and yr and hr and min and sec then
		-- find month
		local month = 1
		for i = 1, 12 do
			if MONTHS[i] == mon then
				month = i
				break
			end
		end

		-- convert to epoch time
		return tz_offset(tz) + os.time( {
			year  = yr,
			month = month,
			day   = day,
			hour  = hr,
			min   = min,
			sec   = sec
		} )
	end

	return 0
end

function to_http(time)
	return os.date( "%a, %d %b %Y %H:%M:%S GMT", time )
end

function compare(d1, d2)

	if d1:match("[^0-9]") then d1 = to_unix(d1) end
	if d2:match("[^0-9]") then d2 = to_unix(d2) end

	if d1 == d2 then
		return 0
	elseif d1 < d2 then
		return -1
	else
		return 1
	end
end
