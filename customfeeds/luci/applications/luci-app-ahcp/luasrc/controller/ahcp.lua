-- Copyright 2011 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.ahcp", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/ahcpd") then
		return
	end

	entry({"admin", "network", "ahcpd"}, cbi("ahcp"), _("AHCP Server"), 90)
	entry({"admin", "network", "ahcpd", "status"}, call("ahcp_status"))
end

function ahcp_status()
	local nfs = require "nixio.fs"
	local uci = require "luci.model.uci".cursor()
	local lsd = uci:get_first("ahcpd", "ahcpd", "lease_dir") or "/var/lib/leases"
	local idf = uci:get_first("ahcpd", "ahcpd", "id_file")   or "/var/lib/ahcpd-unique-id"

	local rv = {
		uid    = "00:00:00:00:00:00:00:00",
		leases = { }
	}

	idf = nfs.readfile(idf)
	if idf and #idf == 8 then
		rv.uid = "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X" %{ idf:byte(1, 8) }
	end

	local itr = nfs.dir(lsd)
	if itr then
		local addr
		for addr in itr do
			if addr:match("^%d+%.%d+%.%d+%.%d+$") then
				local s = nfs.stat(lsd .. "/" .. addr)
				rv.leases[#rv.leases+1] = {
					addr = addr,
					age  = s and (os.time() - s.mtime) or 0
				}
			end
		end
	end

	table.sort(rv.leases, function(a, b) return a.age < b.age end)

	luci.http.prepare_content("application/json")
	luci.http.write_json(rv)
end
