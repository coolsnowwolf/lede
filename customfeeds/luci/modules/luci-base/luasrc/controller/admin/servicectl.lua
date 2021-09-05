-- Copyright 2010 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.admin.servicectl", package.seeall)

function index()
	entry({"servicectl"}, alias("servicectl", "status")).sysauth = "root"
	entry({"servicectl", "status"}, call("action_status")).leaf = true
	entry({"servicectl", "restart"}, post("action_restart")).leaf = true
end

function action_status()
	local data = nixio.fs.readfile("/var/run/luci-reload-status")
	if data then
		luci.http.write("/etc/config/")
		luci.http.write(data)
	else
		luci.http.write("finish")
	end
end

function action_restart(args)
	local uci = require "luci.model.uci".cursor()
	if args then
		local service
		local services = { }

		for service in args:gmatch("[%w_-]+") do
			services[#services+1] = service
		end

		local command = uci:apply(services, true)
		if nixio.fork() == 0 then
			local i = nixio.open("/dev/null", "r")
			local o = nixio.open("/dev/null", "w")

			nixio.dup(i, nixio.stdin)
			nixio.dup(o, nixio.stdout)

			i:close()
			o:close()

			nixio.exec("/bin/sh", unpack(command))
		else
			luci.http.write("OK")
			os.exit(0)
		end
	end
end
