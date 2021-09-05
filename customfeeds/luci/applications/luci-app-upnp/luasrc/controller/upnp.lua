-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.upnp", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/upnpd") then
		return
	end

	local page

	page = entry({"admin", "services", "upnp"}, cbi("upnp/upnp"), _("UPnP"))
	page.dependent = true

	entry({"admin", "services", "upnp", "status"}, call("act_status")).leaf = true
	entry({"admin", "services", "upnp", "delete"}, post("act_delete")).leaf = true
end

function act_status()
	local uci = luci.model.uci.cursor()
	local lease_file = uci:get("upnpd", "config", "upnp_lease_file")

	local ipt = io.popen("iptables --line-numbers -t nat -xnvL MINIUPNPD 2>/dev/null")
	if ipt then
		local upnpf = lease_file and io.open(lease_file, "r")
		local fwd = { }
		while true do
			local ln = ipt:read("*l")
			if not ln then
				break
			elseif ln:match("^%d+") then
				local num, proto, extport, intaddr, intport =
					ln:match("^(%d+).-([a-z]+).-dpt:(%d+) to:(%S-):(%d+)")
				local descr = ""

				if num and proto and extport and intaddr and intport then
					num     = tonumber(num)
					extport = tonumber(extport)
					intport = tonumber(intport)

					if upnpf then
						local uln = upnpf:read("*l")
						if uln then descr = uln:match(string.format("^%s:%d:%s:%d:%%d*:(.*)$", proto:upper(), extport, intaddr, intport)) end
						if not descr then descr = "" end
					end

					fwd[#fwd+1] = {
						num     = num,
						proto   = proto:upper(),
						extport = extport,
						intaddr = intaddr,
						intport = intport,
						descr = descr
					}
				end
			end
		end

		if upnpf then upnpf:close() end
		ipt:close()

		luci.http.prepare_content("application/json")
		luci.http.write_json(fwd)
	end
end

function act_delete(num)
	local idx = tonumber(num)
	local uci = luci.model.uci.cursor()

	if idx and idx > 0 then
		luci.sys.call("iptables -t filter -D MINIUPNPD %d 2>/dev/null" % idx)
		luci.sys.call("iptables -t nat -D MINIUPNPD %d 2>/dev/null" % idx)

		local lease_file = uci:get("upnpd", "config", "upnp_lease_file")
		if lease_file and nixio.fs.access(lease_file) then
			luci.sys.call("sed -i -e '%dd' %s" %{ idx, luci.util.shellquote(lease_file) })
		end

		luci.http.status(200, "OK")
		return
	end

	luci.http.status(400, "Bad request")
end
