-- Copyright 2017 Xingwang Liao <kuoruan@gmail.com>
-- Licensed to the public under the Apache License 2.0.

module("luci.model.qos_gargoyle", package.seeall)

function has_ndpi()
	return luci.sys.call("lsmod | cut -d ' ' -f1 | grep -q 'xt_ndpi'") == 0
end

function cbi_add_dpi_protocols(field)
	local util = require "luci.util"

	local dpi_protocols = {}

	for line in util.execi("iptables -m ndpi --help 2>/dev/null | grep '^--'") do
		local _, _, protocol, name = line:find("%-%-([^%s]+) Match for ([^%s]+)")

		if protocol and name then
			dpi_protocols[protocol] = name
		end
	end

	for p, n in util.kspairs(dpi_protocols) do
		field:value(p, n)
	end
end

function get_wan()
	local net = require "luci.model.network".init()
	return net:get_wannet()
end
