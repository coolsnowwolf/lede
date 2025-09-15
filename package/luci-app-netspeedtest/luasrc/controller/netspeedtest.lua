-- Copyright (C) 2020-2025  sirpdboy  <herboy2008@gmail.com> https://github.com/sirpdboy/netspeedtest

module("luci.controller.netspeedtest", package.seeall)
local http = require "luci.http"
local fs=require"nixio.fs"
local sys=require "luci.sys"
local uci = luci.model.uci.cursor()
name='netspeedtest'
function index()

    if not nixio.fs.access("/etc/config/netspeedtest") then return end

	local e = entry({"admin","network","netspeedtest"},alias("admin", "network", "netspeedtest", "speedtestlan"),_("Net Speedtest"),90)
	e.dependent = false
	e.acl_depends = { "luci-app-netspeedtest" }
	
	entry({"admin", "network", "netspeedtest", "speedtestlan"},cbi("netspeedtest/speedtestlan"),_("Lan Speedtest Web"),20).leaf = true
	entry({"admin", "network", "netspeedtest", "speedtestiperf3"},cbi("netspeedtest/speedtestiperf3", {hideapplybtn=true, hidesavebtn=true, hideresetbtn=true}),_("Lan Speedtest Iperf3"),30).leaf = true
        entry({"admin", "network", "netspeedtest", "speedtestwan"},cbi("netspeedtest/speedtestwan", {hideapplybtn=true, hidesavebtn=true, hideresetbtn=true}),_("Broadband speedtest"), 40).leaf = true
        entry({"admin", "network", "netspeedtest", "speedtestwanweb"},cbi("netspeedtest/speedtestwanweb", {hideapplybtn=true, hidesavebtn=true, hideresetbtn=true}),_("Broadband OpenSpeedtest"), 41).leaf = true
        entry({"admin", "network", "netspeedtest", "speedtestport"},cbi("netspeedtest/speedtestport", {hideapplybtn=true, hidesavebtn=true, hideresetbtn=true}),_("Server Port Latency Test"), 50).leaf = true
        entry({"admin", "network", "netspeedtest", "log"}, form("netspeedtest/log"), _("Log"), 60).leaf = true
	entry({"admin", "network", "netspeedtest", "test_port"}, call("test_port"))
	entry({"admin", "network", "iperf3_status"}, call("iperf3_status"))
	entry({"admin", "network", "test_iperf0"}, post("test_iperf0"), nil).leaf = true
	entry({"admin", "network", "test_iperf1"}, post("test_iperf1"), nil).leaf = true
	entry({"admin", "network", "netspeedtest", "speedtestwanrun"}, call("speedtestwanrun"))
	entry({"admin", "network", "netspeedtest", "netcheck"}, call("netcheck"))
	entry({"admin", "network", "netspeedtest", "dellog"},call("dellog"))
        entry({"admin", "network", "netspeedtest", "getlog"},call("getlog"))
	-- extra apis for parity with JS version
	entry({"admin", "network", "netspeedtest", "get_ookla_version"}, call("get_ookla_version")).leaf = true
	entry({"admin", "network", "netspeedtest", "get_speedtest_url"}, call("get_speedtest_url")).leaf = true
	entry({"admin", "network", "netspeedtest", "homebox_ctrl"}, post("homebox_ctrl")).leaf = true
end

function netcheck()
	http.prepare_content("text/plain; charset=utf-8")
	local f=io.open("/tmp/netspeedtest.log", "r+")
	local fdp=fs.readfile("/tmp/netspeedtestpos") or 0
	f:seek("set",fdp)
	local a=f:read(2048000) or ""
	fdp=f:seek()
	fs.writefile("/tmp/netspeedtestpos",tostring(fdp))
	f:close()
	http.write(a)
end

function speedtestwanrun()
	local cli = luci.http.formvalue('cli')
	local server_id = luci.http.formvalue('server_id') or ''
	local single = luci.http.formvalue('single') or ''
	uci:set(name, 'speedtestwan', 'speedtest_cli', cli)
	if server_id ~= '' then uci:set(name, 'speedtestwan', 'server_id', server_id) end
	if single ~= '' then uci:set(name, 'speedtestwan', 'single', single) end
	uci:commit(name)
	fs.writefile("/tmp/netspeedtestpos","0")
	http.prepare_content("application/json")
	http.write('')
	sys.exec(string.format("/etc/init.d/netspeedtest wantest " ..cli.. " > /tmp/netspeedtest.log 2>&1 &" ))
end

function test_port()
	local e = {}
	local domain = luci.http.formvalue('sdomain')
	local port = luci.http.formvalue('sport')
	local ip=sys.exec("echo "..domain.." | grep -E ^[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}$ || nslookup "..domain.." 2>/dev/null | grep Address | awk -F' ' '{print$NF}' | grep -E ^[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}$ | sed -n 1p")
	ip=sys.exec("echo -n "..ip)
	e.ping = luci.sys.exec(string.format("echo -n $(tcping -q -c 1 -i 1 -t 2 -p %s %s 2>&1  | grep -o 'time=[0-9]*.[0-9]*' | awk -F '=' '{print $2}') 2>/dev/null", port, ip))
	e.type = "tcping"
	if e.ping=="" then
		e.ping=sys.call("echo -n $(ping -c 1 -W 1 %q 2>&1 | grep -o 'time=[0-9]*.[0-9]*' | awk -F '=' '{print $2}') 2>/dev/null" % ip)
		e.type = "ping"
	end
	if e.ping=="" then e.ping="0" end
	sys.exec(string.format('echo -ne "\n 【$(date)】 服务器：%s -- 端口：%s -- TCP延时：%s ms \n">> /var/log/netspeedtest.log',domain,port,e.ping))
	uci:set(name, 'speedtestport', 'sdomain', domain)
	uci:set(name, 'speedtestport', 'sport', port)
	uci:set(name, 'speedtestport', 'tcpspeed', e.ping.." ms")
	uci:commit(name)
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end


function iperf3_status()
	local e={}
	e.run=sys.call("busybox ps -w | grep iperf3 | grep -v grep >/dev/null") == 0
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end

function testout(cmd, addr)
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

function test_iperf0(addr)
       sys.call("pgrep -f unblockneteasemusic | xargs kill -9 >/dev/null 2>&1 ")
       sys.call("/etc/init.d/unblockneteasemusic stop ")
       sys.call("/etc/init.d/unblockmusic stop ")
       testout("iperf3 -s ", addr)
end

function test_iperf1(addr)
	sys.call("pgrep -f iperf3 | xargs kill -9 >/dev/null 2>&1 ")
	sys.call("/etc/init.d/unblockneteasemusic restart")
	sys.call("/etc/init.d/unblockmusic restart")
end

function dellog()
	fs.writefile("/var/log/netspeedtest.log","")
	fs.writefile("/tmp/netspeedtest.log","")
	fs.writefile("/tmp/netspeedtestpos","0")
	pcall(function() nixio.fs.remove("/var/speedtesturl.tmp") end)
	http.prepare_content("application/json")
	http.write('')
end



function getlog()
	local content=""
	local function readfile(path)
		if fs.access(path) then
			local f=io.open(path,"r")
			if f then
				local d=f:read("*a") or ""
				f:close()
				return d
			end
		end
		return ""
	end
	content = readfile("/var/log/netspeedtest.log")
	local tmpc = readfile("/tmp/netspeedtest.log")
	if tmpc ~= "" then
		content = (content ~= "" and (content .. "\n") or "") .. tmpc
	end
	content = content:gsub("\n$", "")
	http.prepare_content("text/plain; charset=utf-8")
	http.write(content)
end

-- Return Ookla speedtest cli version info (best-effort)
function get_ookla_version()
	local ver = ""
	-- prefer downloaded ookla binary if exists
	if nixio.fs.access("/var/netspeedtest/speedtest") then
		ver = sys.exec("/var/netspeedtest/speedtest --version 2>/dev/null | head -n1") or ""
	end
	if ver == nil or ver == '' then
		ver = sys.exec("/usr/bin/speedtest --version 2>/dev/null | head -n1") or ""
	end
	http.prepare_content("application/json")
	local res = {
		version = (ver or ""):gsub("\n$",""),
		installed = (ver or "") ~= '' and (ver:lower():find("speedtest") ~= nil)
	}
	luci.http.write_json(res)
end

-- Return last speedtest share url if exists
function get_speedtest_url()
	local urlfile = "/var/speedtesturl.tmp"
	local content = ''
	if fs.access(urlfile) then
		local f = io.open(urlfile, "r")
		if f then content = f:read("*l") or '' f:close() end
	end
	http.prepare_content("application/json")
	luci.http.write_json({ url = content or '' })
end

-- Control Homebox service start/stop quickly from UI
function homebox_ctrl()
	local action = luci.http.formvalue('action') or ''
	if action == 'start' then
		sys.exec("/etc/init.d/netspeedtest start >/dev/null 2>&1 &")
	elseif action == 'stop' then
		sys.exec("/etc/init.d/netspeedtest stop >/dev/null 2>&1 &")
	end
	http.prepare_content("application/json")
	http.write('')
end

