#!/usr/bin/env lua

require "luci.util"
docker = require "luci.docker"
uci = (require "luci.model.uci").cursor()
dk = docker.new({socket_path = "/var/run/docker.sock"})

if dk:_ping().code ~= 200 then
	return
end

allowed_container = uci:get("dockerd", "dockerman", "ac_allowed_container")
if not allowed_container or next(allowed_container)==nil then
	return
end
-- for i,v in ipairs(allowed_container) do
-- 	print(v)
-- end
containers_list = dk.containers:list({
	query={
		filters={
			id=allowed_container
		}
	}
}).body

-- allowed_ip = {}
for i, v in ipairs(containers_list) do
	if v.NetworkSettings and v.NetworkSettings.Networks and v.NetworkSettings.Networks.bridge and v.NetworkSettings.Networks.bridge.IPAddress and v.NetworkSettings.Networks.bridge.IPAddress ~= "" then
		-- print(v.NetworkSettings.Networks.bridge.IPAddress)
		luci.util.exec("iptables -I DOCKER-MAN -d "..v.NetworkSettings.Networks.bridge.IPAddress.." -o docker0 -j ACCEPT")
	end
end
