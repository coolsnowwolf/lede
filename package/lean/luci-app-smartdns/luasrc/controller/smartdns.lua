--
-- Copyright (C) 2018-2020 Ruilin Peng (Nick) <pymumu@gmail.com>.
--
-- smartdns is free software: you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation, either version 3 of the License, or
-- (at your option) any later version.
--
-- smartdns is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with this program.  If not, see <http://www.gnu.org/licenses/>.

module("luci.controller.smartdns", package.seeall)
local smartdns = require "luci.model.smartdns"

function index()
	if not nixio.fs.access("/etc/config/smartdns") then
		return
	end

	local page
	page = entry({"admin", "services", "smartdns"}, cbi("smartdns/smartdns"), _("SmartDNS"), 60)
	page.dependent = true
	page = entry({"admin", "services", "smartdns", "status"}, call("act_status"))
	page.leaf = true
	page = entry({"admin", "services", "smartdns", "upstream"}, cbi("smartdns/upstream"), nil)
	page.leaf = true
end

local function is_running()
	return luci.sys.call("pidof smartdns >/dev/null") == 0
end

function act_status()
	local e={}
	local ipv6_server;
	local redirect_mode="none";

	e.ipv6_works = 2;
	e.ipv4_works = 2;
	e.ipv6_server = 1;
	e.dnsmasq_forward = 0;
	redirect_mode = smartdns.get_config_option("smartdns", "smartdns", "redirect", nil);
	if redirect_mode == "redirect" then
		e.redirect = 1
	elseif redirect_mode == "dnsmasq-upstream" then
		e.redirect = 2
	else
		e.redirect = 0
	end

	e.local_port = smartdns.get_config_option("smartdns", "smartdns", "port", nil);
	ipv6_server = smartdns.get_config_option("smartdns", "smartdns", "ipv6_server", nil);
	if e.redirect == 1 then 
		if e.local_port ~= nil and e.local_port ~= "53" then
			e.ipv4_works = luci.sys.call("iptables -t nat -nL PREROUTING 2>/dev/null | grep REDIRECT | grep dpt:53 | grep %q >/dev/null 2>&1" % e.local_port) == 0
			if ipv6_server == "1" then
				e.ipv6_works = luci.sys.call("ip6tables -t nat -nL PREROUTING 2>/dev/null| grep REDIRECT | grep dpt:53 | grep %q >/dev/null 2>&1" % e.local_port) == 0
			else 
				e.ipv6_works = 2
			end
		else
			e.redirect = 0
		end
	elseif e.redirect == 2 then
		local str;
		local dnsmasq_server = luci.sys.exec("uci get dhcp.@dnsmasq[0].server")
		if e.local_port ~= nil then
			str = "127.0.0.1#" .. e.local_port 
			if  string.sub(dnsmasq_server,1,string.len(str)) == str then 
				e.dnsmasq_forward = 1
			end
		end
	end
	e.running = is_running()

	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end
