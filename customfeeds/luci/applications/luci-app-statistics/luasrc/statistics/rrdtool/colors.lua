-- Copyright 2008 Freifunk Leipzig / Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

module("luci.statistics.rrdtool.colors", package.seeall)

require("luci.util")


Instance = luci.util.class()

function Instance.from_string( self, s )
	return {
		tonumber(s:sub(1,2), 16),
		tonumber(s:sub(3,4), 16),
		tonumber(s:sub(5,6), 16)
	}
end

function Instance.to_string( self, c )
	return string.format(
		"%02x%02x%02x",
		math.floor(c[1]),
		math.floor(c[2]),
		math.floor(c[3])
	)
end

function Instance.random( self )
	local r   = math.random(255)
	local g   = math.random(255)
	local min = 0
	local max = 255

	if ( r + g ) < 255 then
		min = 255 - r - g
	else
		max = 511 - r - g
	end

	local b = min + math.floor( math.random() * ( max - min ) )

	return { r, g, b }
end

function Instance.faded( self, fg, opts )
	opts = opts or {}
	opts.background = opts.background or { 255, 255, 255 }
	opts.alpha      = opts.alpha      or 0.25

	if type(opts.background) == "string" then
		opts.background = _string_to_color(opts.background)
	end

	local bg = opts.background

	return {
		( opts.alpha * fg[1] ) + ( ( 1.0 - opts.alpha ) * bg[1] ),
		( opts.alpha * fg[2] ) + ( ( 1.0 - opts.alpha ) * bg[2] ),
		( opts.alpha * fg[3] ) + ( ( 1.0 - opts.alpha ) * bg[3] )
	}
end
