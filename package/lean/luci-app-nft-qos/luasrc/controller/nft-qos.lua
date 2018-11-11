-- Copyright 2018 Rosy Song <rosysong@rosinson.com>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.nft-qos", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/nft-qos") then
		return
	end

	entry({"admin", "status", "realtime", "rate"},
		template("nft-qos/rate"), _("Rate"), 5).leaf = true
	entry({"admin", "status", "realtime", "rate_status"},
		call("action_rate")).leaf = true
	entry({"admin", "services", "nft-qos"}, cbi("nft-qos/nft-qos"),
		_("Qos over Nftables"), 60)
end

function _action_rate(rv, n)
	local c = io.popen("nft list chain inet nft-qos-monitor " .. n .. " 2>/dev/null")
	if c then
		for l in c:lines() do
			local _, i, p, b = l:match('^%s+ip ([^%s]+) ([^%s]+) counter packets (%d+) bytes (%d+)')
			if i and p and b then
				-- handle expression
				local r = {
					rule = {
					family = "inet",
					table = "nft-qos-monitor",
					chain = n,
					handle = 0,
					expr = {
							{ match = { right = i } },
							{ counter = { packets = p, bytes = b } }
						}
					}
				}
				rv[#rv + 1] = r
			end
		end
		c:close()
	end
end

function action_rate()
	luci.http.prepare_content("application/json")
	local data = { nftables = {} }
	_action_rate(data.nftables, "upload")
	_action_rate(data.nftables, "download")
	luci.http.write_json(data)
end
