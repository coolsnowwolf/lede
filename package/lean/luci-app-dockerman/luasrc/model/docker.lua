--[[
LuCI - Lua Configuration Interface
Copyright 2019 lisaac <https://github.com/lisaac/luci-app-dockerman>
]]--

local docker = require "luci.docker"
local fs = require "nixio.fs"
local uci = (require "luci.model.uci").cursor()

local _docker = {}
_docker.options = {}

--pull image and return iamge id
local update_image = function(self, image_name)
	local json_stringify = luci.jsonc and luci.jsonc.stringify
	_docker:append_status("Images: " .. "pulling" .. " " .. image_name .. "...\n")
	local res = self.images:create({query = {fromImage=image_name}}, _docker.pull_image_show_status_cb)

	if res and res.code and res.code == 200 and (#res.body > 0 and not res.body[#res.body].error and res.body[#res.body].status and (res.body[#res.body].status == "Status: Downloaded newer image for ".. image_name)) then
		_docker:append_status("done\n")
	else
		res.body.message = res.body[#res.body] and res.body[#res.body].error or (res.body.message or res.message)
	end

	new_image_id = self.images:inspect({name = image_name}).body.Id
	return new_image_id, res
end

local table_equal = function(t1, t2)
	if not t1 then
		return true
	end

	if not t2 then
		return false
	end

	if #t1 ~= #t2 then
		return false
	end

	for i, v in ipairs(t1) do
		if t1[i] ~= t2[i] then
			return false
		end
	end

	return true
end

local table_subtract = function(t1, t2)
	if not t1 or next(t1) == nil then
		return nil
	end

	if not t2 or next(t2) == nil then
		return t1
	end

	local res = {}
	for _, v1 in ipairs(t1) do
		local found = false
		for _, v2 in ipairs(t2) do
			if v1 == v2 then
				found= true
				break
			end
		end
		if not found then
			table.insert(res, v1)
		end
	end

	return next(res) == nil and nil or res
end

local map_subtract = function(t1, t2)
	if not t1 or next(t1) == nil then
		return nil
	end

	if not t2 or next(t2) == nil then
		return t1
	end

	local res = {}
	for k1, v1 in pairs(t1) do
		local found = false
		for k2, v2 in ipairs(t2) do
			if k1 == k2 and luci.util.serialize_data(v1) == luci.util.serialize_data(v2) then
				found= true
				break
			end
		end

		if not found then
			res[k1] = v1
		end
	end

	return next(res) ~= nil and res or nil
end

_docker.clear_empty_tables = function ( t )
	local k, v

	if next(t) == nil then
		t = nil
	else
		for k, v in pairs(t) do
			if type(v) == 'table' then
				t[k] = _docker.clear_empty_tables(v)
			end
		end
	end

	return t
end

local get_config = function(container_config, image_config)
	local config = container_config.Config
	local old_host_config = container_config.HostConfig
	local old_network_setting = container_config.NetworkSettings.Networks or {}

	if config.WorkingDir == image_config.WorkingDir then
		config.WorkingDir = ""
	end

	if config.User == image_config.User then
		config.User = ""
	end

	if table_equal(config.Cmd, image_config.Cmd) then
		config.Cmd = nil
	end

	if table_equal(config.Entrypoint, image_config.Entrypoint) then
		config.Entrypoint = nil
	end

	if table_equal(config.ExposedPorts, image_config.ExposedPorts) then
		config.ExposedPorts = nil
	end

	config.Env = table_subtract(config.Env, image_config.Env)
	config.Labels = table_subtract(config.Labels, image_config.Labels)
	config.Volumes = map_subtract(config.Volumes, image_config.Volumes)

	if old_host_config.PortBindings and next(old_host_config.PortBindings) ~= nil then
		config.ExposedPorts = {}
		for p, v in pairs(old_host_config.PortBindings) do
			config.ExposedPorts[p] = { HostPort=v[1] and v[1].HostPort }
		end
	end

	local network_setting = {}
	local multi_network = false
	local extra_network = {}

	for k, v in pairs(old_network_setting) do
		if multi_network then
			extra_network[k] = v
		else
			network_setting[k] = v
		end
		multi_network = true
	end

	local host_config = old_host_config
	host_config.Mounts = {}
	for i, v in ipairs(container_config.Mounts) do
		if v.Type == "volume" then
			table.insert(host_config.Mounts, {
				Type = v.Type,
				Target = v.Destination,
				Source = v.Source:match("([^/]+)\/_data"),
				BindOptions = (v.Type == "bind") and {Propagation = v.Propagation} or nil,
				ReadOnly = not v.RW
			})
		end
	end

	local create_body = config
	create_body["HostConfig"] = host_config
	create_body["NetworkingConfig"] = {EndpointsConfig = network_setting}
	create_body = _docker.clear_empty_tables(create_body) or {}
	extra_network = _docker.clear_empty_tables(extra_network) or {}

	return create_body, extra_network
end

local upgrade = function(self, request)
	_docker:clear_status()

	local container_info = self.containers:inspect({id = request.id})

	if container_info.code > 300 and type(container_info.body) == "table" then
		return container_info
	end

	local image_name = container_info.body.Config.Image
	if not image_name:match(".-:.+") then
		image_name = image_name .. ":latest"
	end

	local old_image_id = container_info.body.Image
	local container_name = container_info.body.Name:sub(2)

	local image_id, res = update_image(self, image_name)
	if res and res.code and res.code ~= 200 then
		return res
	end

	if image_id == old_image_id then
		return {code = 305, body = {message = "Already up to date"}}
	end

	local t = os.date("%Y%m%d%H%M%S")
	_docker:append_status("Container: rename" .. " " .. container_name .. " to ".. container_name .. "_old_".. t .. "...")
	res = self.containers:rename({name = container_name, query = { name = container_name .. "_old_" ..t }})
	if res and res.code and res.code < 300 then
		_docker:append_status("done\n")
	else
		return res
	end

	local image_config = self.images:inspect({id = old_image_id}).body.Config
	local create_body, extra_network = get_config(container_info.body, image_config)

	-- create new container
	_docker:append_status("Container: Create" .. " " .. container_name .. "...")
	create_body = _docker.clear_empty_tables(create_body)
	res = self.containers:create({name = container_name, body = create_body})
	if res and res.code and res.code > 300 then
		return res
	end
	_docker:append_status("done\n")

	-- extra networks need to network connect action
	for k, v in pairs(extra_network) do
		_docker:append_status("Networks: Connect" .. " " .. container_name .. "...")
		res = self.networks:connect({id = k, body = {Container = container_name, EndpointConfig = v}})
		if res and res.code and res.code > 300 then
			return res
		end
		_docker:append_status("done\n")
	end

	_docker:append_status("Container: " .. "Stop" .. " " .. container_name .. "_old_".. t .. "...")
	res = self.containers:stop({name = container_name .. "_old_" ..t })
	if res and res.code and res.code < 305 then
		_docker:append_status("done\n")
	else
		return res
	end

	_docker:append_status("Container: " .. "Start" .. " " .. container_name .. "...")
	res = self.containers:start({name = container_name})
	if res and res.code and res.code < 305 then
		_docker:append_status("done\n")
	else
		return res
	end

	_docker:clear_status()
	return res
end

local duplicate_config = function (self, request)
	local container_info = self.containers:inspect({id = request.id})
	if container_info.code > 300 and type(container_info.body) == "table" then
		return nil
	end

	local old_image_id = container_info.body.Image
	local image_config = self.images:inspect({id = old_image_id}).body.Config

	return get_config(container_info.body, image_config)
end

_docker.new = function()
	local host = nil
	local port = nil
	local socket_path = nil
	local debug_path = nil

	if uci:get_bool("dockerd", "dockerman", "remote_endpoint") then
		host = uci:get("dockerd", "dockerman", "remote_host") or nil
		port = uci:get("dockerd", "dockerman", "remote_port") or nil
	else
		socket_path = uci:get("dockerd", "dockerman", "socket_path") or "/var/run/docker.sock"
	end

	local debug = uci:get_bool("dockerd", "dockerman", "debug")
	if debug then
		debug_path = uci:get("dockerd", "dockerman", "debug_path") or "/tmp/.docker_debug"
	end

	local status_path = uci:get("dockerd", "dockerman", "status_path") or "/tmp/.docker_action_status"

	_docker.options = {
		host = host,
		port = port,
		socket_path = socket_path,
		debug = debug,
		debug_path = debug_path,
		status_path = status_path
	}

	local _new = docker.new(_docker.options)
	_new.containers_upgrade = upgrade
	_new.containers_duplicate_config = duplicate_config

	return _new
end

_docker.options.status_path = uci:get("dockerd", "dockerman", "status_path") or "/tmp/.docker_action_status"

_docker.append_status=function(self,val)
	if not val then
		return
	end
	local file_docker_action_status=io.open(self.options.status_path, "a+")
	file_docker_action_status:write(val)
	file_docker_action_status:close()
end

_docker.write_status=function(self,val)
	if not val then
		return
	end
	local file_docker_action_status=io.open(self.options.status_path, "w+")
	file_docker_action_status:write(val)
	file_docker_action_status:close()
end

_docker.read_status=function(self)
	return fs.readfile(self.options.status_path)
end

_docker.clear_status=function(self)
	fs.remove(self.options.status_path)
end

local status_cb = function(res, source, handler)
	res.body = res.body or {}
	while true do
		local chunk = source()
		if chunk then
			--standard output to res.body
			table.insert(res.body, chunk)
			handler(chunk)
		else
			return
		end
	end
end

--{"status":"Pulling from library\/debian","id":"latest"}
--{"status":"Pulling fs layer","progressDetail":[],"id":"50e431f79093"}
--{"status":"Downloading","progressDetail":{"total":50381971,"current":2029978},"id":"50e431f79093","progress":"[==>                                                ]   2.03MB\/50.38MB"}
--{"status":"Download complete","progressDetail":[],"id":"50e431f79093"}
--{"status":"Extracting","progressDetail":{"total":50381971,"current":17301504},"id":"50e431f79093","progress":"[=================>                                 ]   17.3MB\/50.38MB"}
--{"status":"Pull complete","progressDetail":[],"id":"50e431f79093"}
--{"status":"Digest: sha256:a63d0b2ecbd723da612abf0a8bdb594ee78f18f691d7dc652ac305a490c9b71a"}
--{"status":"Status: Downloaded newer image for debian:latest"}
_docker.pull_image_show_status_cb = function(res, source)
	return status_cb(res, source, function(chunk)
		local json_parse = luci.jsonc.parse
		local step = json_parse(chunk)
		if type(step) == "table" then
			local buf = _docker:read_status()
			local num = 0
			local str = '\t' .. (step.id and (step.id .. ": ") or "") .. (step.status and step.status or "")  .. (step.progress and (" " .. step.progress) or "").."\n"
			if step.id then
				buf, num = buf:gsub("\t"..step.id .. ": .-\n", str)
			end
			if num == 0 then
				buf = buf .. str
			end
			_docker:write_status(buf)
		end
	end)
end

--{"status":"Downloading from https://downloads.openwrt.org/releases/19.07.0/targets/x86/64/openwrt-19.07.0-x86-64-generic-rootfs.tar.gz"}
--{"status":"Importing","progressDetail":{"current":1572391,"total":3821714},"progress":"[====================\u003e                              ]  1.572MB/3.822MB"}
--{"status":"sha256:d5304b58e2d8cc0a2fd640c05cec1bd4d1229a604ac0dd2909f13b2b47a29285"}
_docker.import_image_show_status_cb = function(res, source)
	return status_cb(res, source, function(chunk)
		local json_parse = luci.jsonc.parse
		local step = json_parse(chunk)
		if type(step) == "table" then
			local buf = _docker:read_status()
			local num = 0
			local str = '\t' .. (step.status and step.status or "") .. (step.progress and (" " .. step.progress) or "").."\n"
			if step.status then
				buf, num = buf:gsub("\t"..step.status .. " .-\n", str)
			end
			if num == 0 then
				buf = buf .. str
			end
			_docker:write_status(buf)
		end
	end)
end

_docker.create_macvlan_interface = function(name, device, gateway, subnet)
	if not fs.access("/etc/config/network") or not fs.access("/etc/config/firewall") then
		return
	end

	if uci:get_bool("dockerd", "dockerman", "remote_endpoint") then
		return
	end

	local ip = require "luci.ip"
	local if_name = "docker_"..name
	local dev_name = "macvlan_"..name
	local net_mask = tostring(ip.new(subnet):mask())
	local lan_interfaces

	-- add macvlan device
	uci:delete("network", dev_name)
	uci:set("network", dev_name, "device")
	uci:set("network", dev_name, "name", dev_name)
	uci:set("network", dev_name, "ifname", device)
	uci:set("network", dev_name, "type", "macvlan")
	uci:set("network", dev_name, "mode", "bridge")

	-- add macvlan interface
	uci:delete("network", if_name)
	uci:set("network", if_name, "interface")
	uci:set("network", if_name, "proto", "static")
	uci:set("network", if_name, "ifname", dev_name)
	uci:set("network", if_name, "ipaddr", gateway)
	uci:set("network", if_name, "netmask", net_mask)
	uci:foreach("firewall", "zone", function(s)
		if s.name == "lan" then
			local interfaces
			if type(s.network) == "table" then
				interfaces = table.concat(s.network, " ")
				uci:delete("firewall", s[".name"], "network")
			else
				interfaces = s.network and s.network or ""
			end
			interfaces = interfaces .. " " .. if_name
			interfaces = interfaces:gsub("%s+", " ")
			uci:set("firewall", s[".name"], "network", interfaces)
		end
	end)

	uci:commit("firewall")
	uci:commit("network")

	os.execute("ifup " .. if_name)
end

_docker.remove_macvlan_interface = function(name)
	if not fs.access("/etc/config/network") or not fs.access("/etc/config/firewall") then
		return
	end

	if uci:get_bool("dockerd", "dockerman", "remote_endpoint") then
		return
	end

	local if_name = "docker_"..name
	local dev_name = "macvlan_"..name
	uci:foreach("firewall", "zone", function(s)
		if s.name == "lan" then
			local interfaces
			if type(s.network) == "table" then
				interfaces = table.concat(s.network, " ")
			else
				interfaces = s.network and s.network or ""
			end
			interfaces = interfaces and interfaces:gsub(if_name, "")
			interfaces = interfaces and interfaces:gsub("%s+", " ")
			uci:set("firewall", s[".name"], "network", interfaces)
		end
	end)

	uci:delete("network", dev_name)
	uci:delete("network", if_name)
	uci:commit("network")
	uci:commit("firewall")

	os.execute("ip link del " .. if_name)
end

_docker.byte_format = function (byte)
	if not byte then return 'NaN' end
	local suff = {"B", "KB", "MB", "GB", "TB"}
	for i=1, 5 do
		if byte > 1024 and i < 5 then
			byte = byte / 1024
		else
			return string.format("%.2f %s", byte, suff[i])
		end
	end
end

return _docker
