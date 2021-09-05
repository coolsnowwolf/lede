-- Copyright 2011 Manuel Munz <freifunk at somakoma dot de>
-- Licensed to the public under the Apache License 2.0.

module("luci.tools.olsr", package.seeall)

function etx_color(etx)
	local color = "#bb3333"
	if etx == 0 then
		color = "#bb3333"
	elseif etx < 2 then
		color = "#00cc00"
	elseif etx < 4 then
		color = "#ffcb05"
	elseif etx < 10 then
		color = "#ff6600"
	end
	return color
end

function snr_color(snr)
	local color = "#bb3333"
	if snr == 0 then
		color = "#bb3333"
	elseif snr > 30 then
		color = "#00cc00"
	elseif snr > 20 then
		color = "#ffcb05"
	elseif snr > 5 then
		color = "#ff6600"
	end
	return color
end

