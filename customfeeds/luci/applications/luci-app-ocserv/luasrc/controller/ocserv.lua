-- Copyright 2014 Nikos Mavrogiannopoulos <n.mavrogiannopoulos@gmail.com>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.ocserv", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/ocserv") then
		return
	end

	local page

	page = entry({"admin", "services", "ocserv"}, alias("admin", "services", "ocserv", "main"),
		_("OpenConnect VPN"))
	page.dependent = true
	
	page = entry({"admin", "services", "ocserv", "main"},
		cbi("ocserv/main"),
		_("Server Settings"), 200)
	page.dependent = true

	page = entry({"admin", "services", "ocserv", "users"},
		cbi("ocserv/users"),
		_("User Settings"), 300)
	page.dependent = true

	entry({"admin", "services", "ocserv", "status"},
		call("ocserv_status")).leaf = true

	entry({"admin", "services", "ocserv", "disconnect"},
		post("ocserv_disconnect")).leaf = true

end

function ocserv_status()
	local ipt = io.popen("/usr/bin/occtl show users");

	if ipt then

		local fwd = { }
		while true do

			local ln = ipt:read("*l")
			if not ln then break end
		
			local id, user, group, vpn_ip, ip, device, time, cipher, status = 
				ln:match("^%s*(%d+)%s+([-_%w]+)%s+([%(%)%.%*-_%w]+)%s+([%:%.-_%w]+)%s+([%:%.-_%w]+)%s+([%:%.-_%w]+)%s+([%:%.-_%w]+)%s+([%(%)%:%.-_%w]+)%s+([%:%.-_%w]+).*")
			if id then
				fwd[#fwd+1] = {
					id = id,
					user = user,
					group = group,
					vpn_ip = vpn_ip,
					ip = ip,
					device = device,
					time = time,
					cipher = cipher,
					status = status
				}
			end
		end
		ipt:close()
		luci.http.prepare_content("application/json")
		luci.http.write_json(fwd)
	end
end

function ocserv_disconnect(num)
	local idx = tonumber(num)

	if idx and idx > 0 then
		luci.sys.call("/usr/bin/occtl disconnect id %d" % idx)
		luci.http.status(200, "OK")

		return
	end
	luci.http.status(400, "Bad request")
end
