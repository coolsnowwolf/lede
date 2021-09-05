#!/usr/bin/lua

require 'luci.sys'
local jsonc = require "luci.jsonc"

local function get_config_path()
    return luci.sys.exec("echo -n $(cat /usr/share/ssr_mudb_server/userapiconfig.py | grep 'MUDB_FILE' | cut -d \"'\" -f 2)")
end

local function get_config_json()
    return luci.sys.exec("cat " .. get_config_path()) or "[]"
end

local json = jsonc.parse(get_config_json())
if json then
	for index = 1, table.maxn(json) do
		local o = json[index]
		if o.enable == 1 then
			luci.sys.call(string.format("iptables -A SSR_MUDB-SERVER -p tcp --dport %s -m comment --comment %s -j ACCEPT", o.port, o.user))
			luci.sys.call(string.format("iptables -A SSR_MUDB-SERVER -p udp --dport %s -m comment --comment %s -j ACCEPT", o.port, o.user))
		end
	end
end