module("luci.controller.netspeedtest", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/netspeedtest") then
		return
	end
	
	entry({"admin", "network", "netspeedtest"}, cbi("netspeedtest", {hideapplybtn=true, hidesavebtn=true, hideresetbtn=true}),_("Netspeedtest"),91).dependent = true
	entry({"admin", "network", "test_iperf0"}, post("test_iperf0"), nil).leaf = true
	entry({"admin", "network", "test_iperf1"}, post("test_iperf1"), nil).leaf = true

	entry({"admin", "network", "netspeedtest", "run"}, call("run"))
	entry({"admin", "network", "netspeedtest", "realtime_log"}, call("get_log")) 
end

function testlan(cmd, addr)
		luci.http.prepare_content("text/plain")
		local util = io.popen(cmd)
		if util then
			while true do
				local ln = util:read("*l")
				if not ln then break end
				luci.http.write(ln)
				luci.http.write("\n")
			end
			util:close()
		end

end

function testwan(cmd)
		local util = io.popen(cmd)
		util:close()
end

function test_iperf0(addr)
	testlan("iperf3 -s ", addr)
end

function test_iperf1(addr)
	luci.sys.call("killall iperf3")
end

function get_log()
	local fs = require "nixio.fs"
	local e = {}
	e.running = luci.sys.call("busybox ps -w | grep netspeedtest | grep -v grep >/dev/null") == 0
	e.log = fs.readfile("/var/log/netspeedtest.log") or ""
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end

function run()
	testwan("/etc/init.d/netspeedtest nstest ")
	luci.http.redirect(luci.dispatcher.build_url("admin","network","netspeedtest"))
end
