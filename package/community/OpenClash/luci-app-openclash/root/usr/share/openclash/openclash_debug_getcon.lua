#!/usr/bin/lua

require "nixio"
require "luci.util"
require "luci.sys"
local uci = require("luci.model.uci").cursor()
local fs = require "luci.openclash"
local json = require "luci.jsonc"
local datatype = require "luci.cbi.datatypes"
local addr = arg[1]

local function s(e)
local t=0
local a={' KB',' MB',' GB',' TB'}
repeat
e=e/1024
t=t+1
until(e<=1024)
return string.format("%.1f",e)..a[t]
end

local function debug_getcon()
	local info, ip, host, diag_info
	ip = luci.sys.exec("uci -q get network.lan.ipaddr |awk -F '/' '{print $1}' 2>/dev/null |tr -d '\n'")
	if not ip or ip == "" then
		ip = luci.sys.exec("ip addr show 2>/dev/null | grep -w 'inet' | grep 'global' | grep 'brd' | grep -Eo 'inet [0-9\.]+' | awk '{print $2}' | head -n 1 | tr -d '\n'")
	end
	local port = uci:get("openclash", "config", "cn_port")
	local passwd = uci:get("openclash", "config", "dashboard_password") or ""
	if ip and port then
		info = luci.sys.exec(string.format('curl -sL -m 3 -H "Content-Type: application/json" -H "Authorization: Bearer %s" -XGET http://"%s":"%s"/connections', passwd, ip, port))
		if info then
			info = json.parse(info)
		end
		if info then
			for i = 1, #(info.connections) do
				if info.connections[i].metadata.host == "" then
					host = "Empty"
				else
					host = info.connections[i].metadata.host
				end
				if not addr then
					luci.sys.exec(string.format('echo "%s. SourceIP:【%s】 - Host:【%s】 - DestinationIP:【%s】 - Network:【%s】 - RulePayload:【%s】 - Lastchain:【%s】" >> /tmp/openclash_debug.log', i, (info.connections[i].metadata.sourceIP), host, (info.connections[i].metadata.destinationIP), (info.connections[i].metadata.network), (info.connections[i].rulePayload),(info.connections[i].chains[1])))
				else
					if datatype.hostname(addr) and string.lower(addr) == host  or datatype.ipaddr(addr) and addr == (info.connections[i].metadata.destinationIP) then
						print("id: "..(info.connections[i].id))
						print("start: "..(info.connections[i].start))
						print("download: "..s(info.connections[i].download))
						print("upload: "..s(info.connections[i].upload))
						print("rule: "..(info.connections[i].rule))
						print("rulePayload: "..(info.connections[i].rulePayload))
						print("chains: ")
						for o = 1, #(info.connections[i].chains) do
							print("  "..o..": "..(info.connections[i].chains[o]))
						end
						print("metadata: ")
						print("  sourceIP: "..(info.connections[i].metadata.sourceIP))
						print("  sourcePort: "..(info.connections[i].metadata.sourcePort))
						print("  host: "..host)
						print("  destinationIP: "..(info.connections[i].metadata.destinationIP))
						print("  destinationPort: "..(info.connections[i].metadata.destinationPort))
						print("  network: "..(info.connections[i].metadata.network))
						print("  type: "..(info.connections[i].metadata.type))
						print("")
					end
				end
			end
		end
	end
	os.exit(0)
end

debug_getcon()