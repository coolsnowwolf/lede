-- Copyright 2014 Aedan Renner <chipdankly@gmail.com>
-- Copyright 2018 Florian Eckert <fe@dev.tdt.de>
-- Licensed to the public under the GNU General Public License v2.

module("luci.controller.mwan3", package.seeall)

sys = require "luci.sys"
ut = require "luci.util"

ip = "ip -4 "

function index()
	if not nixio.fs.access("/etc/config/mwan3") then
		return
	end

	entry({"admin", "status", "mwan"},
		alias("admin", "status", "mwan", "overview"),
		_("Load Balancing"), 600)

	entry({"admin", "status", "mwan", "overview"},
		template("mwan/status_interface"))
	entry({"admin", "status", "mwan", "detail"},
		template("mwan/status_detail"))
	entry({"admin", "status", "mwan", "diagnostics"},
		template("mwan/status_diagnostics"))
	entry({"admin", "status", "mwan", "troubleshooting"},
		template("mwan/status_troubleshooting"))
	entry({"admin", "status", "mwan", "interface_status"},
		call("mwan_Status"))
	entry({"admin", "status", "mwan", "detailed_status"},
		call("detailedStatus"))
	entry({"admin", "status", "mwan", "diagnostics_display"},
		call("diagnosticsData"), nil).leaf = true
	entry({"admin", "status", "mwan", "troubleshooting_display"},
		call("troubleshootingData"))


	entry({"admin", "network", "mwan"},
		alias("admin", "network", "mwan", "interface"),
		_("Load Balancing"), 600)

	entry({"admin", "network", "mwan", "globals"},
		cbi("mwan/globalsconfig"),
		_("Globals"), 5).leaf = true
	entry({"admin", "network", "mwan", "interface"},
		arcombine(cbi("mwan/interface"), cbi("mwan/interfaceconfig")),
		_("Interfaces"), 10).leaf = true
	entry({"admin", "network", "mwan", "member"},
		arcombine(cbi("mwan/member"), cbi("mwan/memberconfig")),
		_("Members"), 20).leaf = true
	entry({"admin", "network", "mwan", "policy"},
		arcombine(cbi("mwan/policy"), cbi("mwan/policyconfig")),
		_("Policies"), 30).leaf = true
	entry({"admin", "network", "mwan", "rule"},
		arcombine(cbi("mwan/rule"), cbi("mwan/ruleconfig")),
		_("Rules"), 40).leaf = true
	entry({"admin", "network", "mwan", "notify"},
		form("mwan/notify"),
		_("Notification"), 50).leaf = true
end

function mwan_Status()
	local status = ut.ubus("mwan3", "status", {})

	luci.http.prepare_content("application/json")
	if status ~= nil then
		luci.http.write_json(status)
	else
		luci.http.write_json({})
	end
end

function detailedStatus()
	local statusInfo = ut.trim(sys.exec("/usr/sbin/mwan3 status"))
	luci.http.prepare_content("text/plain")
	if statusInfo ~= "" then
		luci.http.write(statusInfo)
	else
		luci.http.write("Unable to get status information")
	end
end

function diagnosticsData(interface, task)
	function getInterfaceNumber(interface)
		local number = 0
		local interfaceNumber
		local uci = require "luci.model.uci".cursor()
		uci:foreach("mwan3", "interface",
			function (section)
				number = number+1
				if section[".name"] == interface then
					interfaceNumber = number
				end
			end
		)
		return interfaceNumber
	end

	function diag_command(cmd, device, addr)
		if addr and addr:match("^[a-zA-Z0-9%-%.:_]+$") then
			local util = io.popen(cmd %{ut.shellquote(device), ut.shellquote(addr)})
			if util then
				luci.http.write("Command:\n")
				luci.http.write(cmd %{ut.shellquote(device),
					ut.shellquote(addr)} .. "\n\n")
				luci.http.write("Result:\n")
				while true do
					local ln = util:read("*l")
					if not ln then break end
					luci.http.write(ln)
					luci.http.write("\n")
				end
				util:close()
			end
			return
		end
	end

	function get_gateway(interface)
		local gateway = nil
		local dump = nil

		dump = require("luci.util").ubus("network.interface.%s_4" % interface, "status", {})
		if not dump then
			dump = require("luci.util").ubus("network.interface.%s" % interface, "status", {})
		end

		if dump and dump.route then
			local _, route
			for _, route in ipairs(dump.route) do
				if dump.route[_].target == "0.0.0.0" then
					gateway = dump.route[_].nexthop
				end
			end
		end
		return gateway
	end

	local mArray = {}
	local results = ""
	local number = getInterfaceNumber(interface)

	local uci = require "luci.model.uci".cursor(nil, "/var/state")
	local nw = require "luci.model.network".init()
	local i18n = require "luci.i18n"
	local network = nw:get_network(interface)
	local device = network and network:get_interface()
	device = device:name()

	luci.http.prepare_content("text/plain")
	if device then
		if task == "ping_gateway" then
			local gateway = get_gateway(interface)
			if gateway ~= nil then
				diag_command("ping -I %s -c 5 -W 1 %s 2>&1", device, gateway)
			else
				luci.http.prepare_content("text/plain")
				luci.http.write(i18n.translatef("No gateway for interface %s found.", interface))
			end
		elseif task == "ping_trackips" then
			local trackips = uci:get("mwan3", interface, "track_ip")
			if #trackips > 0 then
				for i in pairs(trackips) do
					diag_command("ping -I %s -c 5 -W 1 %s 2>&1", device, trackips[i])
				end
			else
				luci.http.write(i18n.translatef("No tracking Hosts for interface %s defined.", interface))
			end
		elseif task == "check_rules" then
			local number = getInterfaceNumber(interface)
			local iif = 1000 + number
			local fwmark = 2000 + number
			local iif_rule  = sys.exec(string.format("ip rule | grep %d", iif))
			local fwmark_rule = sys.exec(string.format("ip rule | grep %d", fwmark))
			if iif_rule ~= "" and fwmark_rule ~= "" then
				luci.http.write(i18n.translatef("All required IP rules for interface %s found", interface))
				luci.http.write("\n")
				luci.http.write(fwmark_rule)
				luci.http.write(iif_rule)
			elseif iif_rule == "" and fwmark_rule ~= "" then
				luci.http.write(i18n.translatef("Only one IP rules for interface %s found", interface))
				luci.http.write("\n")
				luci.http.write(fwmark_rule)
			elseif iif_rule ~= "" and fwmark_rule == "" then
				luci.http.write(i18n.translatef("Only one IP rules for interface %s found", interface))
				luci.http.write("\n")
				luci.http.write(iif_rule)
			else
				luci.http.write(i18n.translatef("Missing both IP rules for interface %s", interface))
			end
		elseif task == "check_routes" then
			local number = getInterfaceNumber(interface)
			local routeTable = sys.exec(string.format("ip route list table %s", number))
			if routeTable ~= "" then
				luci.http.write(i18n.translatef("Routing table %s for interface %s found", number, interface))
				luci.http.write("\n")
				luci.http.write(routeTable)
			else
				luci.http.write(i18n.translatef("Routing table %s for interface %s not found", number, interface))
			end
		elseif task == "hotplug_ifup" then
			os.execute(string.format("/usr/sbin/mwan3 ifup %s", ut.shellquote(interface)))
			luci.http.write(string.format("Hotplug ifup sent to interface %s", interface))
		elseif task == "hotplug_ifdown" then
			os.execute(string.format("/usr/sbin/mwan3 ifdown %s", ut.shellquote(interface)))
			luci.http.write(string.format("Hotplug ifdown sent to interface %s", interface))
		else
			luci.http.write("Unknown task")
		end
	else
		luci.http.write(string.format("Unable to perform diagnostic tests on %s.", interface))
		luci.http.write("\n")
		luci.http.write("There is no physical or virtual device associated with this interface.")
	end
end

function troubleshootingData()
	local ver = require "luci.version"
	local dash = "-------------------------------------------------"

	luci.http.prepare_content("text/plain")

	luci.http.write("\n")
	luci.http.write("\n")
	luci.http.write("Software-Version")
	luci.http.write("\n")
	luci.http.write(dash)
	luci.http.write("\n")
	if ver.distversion then
		luci.http.write(string.format("OpenWrt - %s", ver.distversion))
		luci.http.write("\n")
	else
		luci.http.write("OpenWrt - unknown")
		luci.http.write("\n")
	end

	if ver.luciversion then
		luci.http.write(string.format("LuCI - %s", ver.luciversion))
		luci.http.write("\n")
	else
		luci.http.write("LuCI - unknown")
		luci.http.write("\n")
	end

	luci.http.write("\n")
	luci.http.write("\n")
	local output = ut.trim(sys.exec("ip a show"))
	luci.http.write("Output of \"ip a show\"")
	luci.http.write("\n")
	luci.http.write(dash)
	luci.http.write("\n")
	if output ~= "" then
		luci.http.write(output)
		luci.http.write("\n")
	else
		luci.http.write("No data found")
		luci.http.write("\n")
	end

	luci.http.write("\n")
	luci.http.write("\n")
	local output = ut.trim(sys.exec("ip route show"))
	luci.http.write("Output of \"ip route show\"")
	luci.http.write("\n")
	luci.http.write(dash)
	luci.http.write("\n")
	if output ~= "" then
		luci.http.write(output)
		luci.http.write("\n")
	else
		luci.http.write("No data found")
		luci.http.write("\n")
	end

	luci.http.write("\n")
	luci.http.write("\n")
	local output = ut.trim(sys.exec("ip rule show"))
	luci.http.write("Output of \"ip rule show\"")
	luci.http.write("\n")
	luci.http.write(dash)
	luci.http.write("\n")
	if output ~= "" then
		luci.http.write(output)
		luci.http.write("\n")
	else
		luci.http.write("No data found")
		luci.http.write("\n")
	end

	luci.http.write("\n")
	luci.http.write("\n")
	luci.http.write("Output of \"ip route list table 1-250\"")
	luci.http.write("\n")
	luci.http.write(dash)
	luci.http.write("\n")
	for i=1,250 do
		local output = ut.trim(sys.exec(string.format("ip route list table %d", i)))
		if output ~= "" then
			luci.http.write(string.format("Table %s: ", i))
			luci.http.write(output)
			luci.http.write("\n")
		end
	end

	luci.http.write("\n")
	luci.http.write("\n")
	local output = ut.trim(sys.exec("iptables -L -t mangle -v -n"))
	luci.http.write("Output of \"iptables -L -t mangle -v -n\"")
	luci.http.write("\n")
	luci.http.write(dash)
	luci.http.write("\n")
	if output ~= "" then
		luci.http.write(output)
		luci.http.write("\n")
	else
		luci.http.write("No data found")
		luci.http.write("\n")
	end
end
