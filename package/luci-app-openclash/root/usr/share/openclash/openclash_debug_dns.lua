#!/usr/bin/lua

require "nixio"
require "luci.util"
require "luci.sys"
local uci = require("luci.model.uci").cursor()
local json = require "luci.jsonc"
local datatype = require "luci.cbi.datatypes"
local addr = arg[1]

local function debug_dns()
	local info, ip, host
	ip = luci.sys.exec("uci -q get network.lan.ipaddr |awk -F '/' '{print $1}' 2>/dev/null |tr -d '\n'")
	if not ip or ip == "" then
		ip = luci.sys.exec("ip address show $(uci -q -p /tmp/state get network.lan.ifname) | grep -w 'inet'  2>/dev/null |grep -Eo 'inet [0-9\.]+' | awk '{print $2}' | tr -d '\n'")
	end
	if not ip or ip == "" then
		ip = luci.sys.exec("ip addr show 2>/dev/null | grep -w 'inet' | grep 'global' | grep 'brd' | grep -Eo 'inet [0-9\.]+' | awk '{print $2}' | head -n 1 | tr -d '\n'")
	end
	local port = uci:get("openclash", "config", "cn_port")
	local passwd = uci:get("openclash", "config", "dashboard_password") or ""

	if datatype.hostname(addr) and ip and port then
		info = luci.sys.exec(string.format('curl -sL -m 3 -H "Content-Type: application/json" -H "Authorization: Bearer %s" -XGET http://"%s":"%s"/dns/query?name="%s"', passwd, ip, port, addr))
		if info then
			info = json.parse(info)
		end
		if info then
			print("Status: "..(info.Status))
			print("TC: "..tostring(info.TC))
			print("RD: "..tostring(info.RD))
			print("RA: "..tostring(info.RA))
			print("AD: "..tostring(info.AD))
			print("CD: "..tostring(info.CD))
			print("")
			print("Question: ")
			for _, v in pairs(info.Question) do
				print("  Name: "..(v.Name))
				print("  Qtype: "..(v.Qtype))
				print("  Qclass: "..(v.Qclass))
				print("")
			end
			if info.Answer then
				print("Answer: ")
				for _, v in pairs(info.Answer) do
					print("  TTL: "..(v.TTL))
					print("  data: "..(v.data:gsub("\n?", "")))
					print("  name: "..(v.name))
					print("  type: "..(v.type))
					print("")
				end
			end
			if info.Additional then
				print("Additional: ")
				for _, v in pairs(info.Additional) do
					print("  TTL: "..(v.TTL))
					print("  data: "..(v.data:gsub("\n?", "")))
					print("  name: "..(v.name))
					print("  type: "..(v.type))
					print("")
				end
			end
			if info.Authority then
				print("Authority: ")
				for _, v in pairs(info.Authority) do
					print("  TTL: "..(v.TTL))
					print("  data: "..(v.data:gsub("\n?", "")))
					print("  name: "..(v.name))
					print("  type: "..(v.type))
					print("")
				end
			end
		end
	end
	os.exit(0)
end

debug_dns()