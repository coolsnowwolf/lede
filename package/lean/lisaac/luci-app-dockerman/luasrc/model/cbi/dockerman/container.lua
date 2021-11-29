--[[
LuCI - Lua Configuration Interface
Copyright 2019 lisaac <https://github.com/lisaac/luci-app-dockerman>
]]--

require "luci.util"

local docker = require "luci.model.docker"
local dk = docker.new()

container_id = arg[1]
local action = arg[2] or "info"

local m, s, o
local images, networks, container_info, res

if not container_id then
	return
end

res = dk.containers:inspect({id = container_id})
if res.code < 300 then
	container_info = res.body
else
	return
end

local get_ports = function(d)
	local data

	if d.HostConfig and d.HostConfig.PortBindings then
		for inter, out in pairs(d.HostConfig.PortBindings) do
			data = (data and (data .. "<br />") or "") .. out[1]["HostPort"] .. ":" .. inter
		end
	end

	return data
end

local get_env = function(d)
	local data

	if d.Config and d.Config.Env then
		for _,v in ipairs(d.Config.Env) do
			data = (data and (data .. "<br />") or "") .. v
		end
	end

	return data
end

local get_command = function(d)
	local data

	if d.Config and d.Config.Cmd then
		for _,v in ipairs(d.Config.Cmd) do
			data = (data and (data .. " ") or "") .. v
		end
	end

	return data
end

local get_mounts = function(d)
	local data

	if d.Mounts then
		for _,v in ipairs(d.Mounts) do
			local v_sorce_d, v_dest_d
			local v_sorce = ""
			local v_dest = ""
			for v_sorce_d in v["Source"]:gmatch('[^/]+') do
				if v_sorce_d and #v_sorce_d > 12 then
					v_sorce = v_sorce .. "/" .. v_sorce_d:sub(1,12) .. "..."
				else
					v_sorce = v_sorce .."/".. v_sorce_d
				end
			end
			for v_dest_d in v["Destination"]:gmatch('[^/]+') do
				if v_dest_d and #v_dest_d > 12 then
					v_dest = v_dest .. "/" .. v_dest_d:sub(1,12) .. "..."
				else
					v_dest = v_dest .."/".. v_dest_d
				end
			end
			data = (data and (data .. "<br />") or "") .. v_sorce .. ":" .. v["Destination"] .. (v["Mode"] ~= "" and (":" .. v["Mode"]) or "")
		end
	end

	return data
end

local get_device = function(d)
	local data

	if d.HostConfig and d.HostConfig.Devices then
		for _,v in ipairs(d.HostConfig.Devices) do
			data = (data and (data .. "<br />") or "") .. v["PathOnHost"] .. ":" .. v["PathInContainer"] .. (v["CgroupPermissions"] ~= "" and (":" .. v["CgroupPermissions"]) or "")
		end
	end

	return data
end

local get_links = function(d)
	local data

	if d.HostConfig and d.HostConfig.Links then
		for _,v in ipairs(d.HostConfig.Links) do
			data = (data and (data .. "<br />") or "") .. v
		end
	end

	return data
end

local get_tmpfs = function(d)
	local data

	if d.HostConfig and d.HostConfig.Tmpfs then
		for k, v in pairs(d.HostConfig.Tmpfs) do
			data = (data and (data .. "<br />") or "") .. k .. (v~="" and ":" or "")..v
		end
	end

	return data
end

local get_dns = function(d)
	local data

	if d.HostConfig and d.HostConfig.Dns then
		for _, v in ipairs(d.HostConfig.Dns) do
			data = (data and (data .. "<br />") or "") .. v
		end
	end

	return data
end

local get_sysctl = function(d)
	local data

	if d.HostConfig and d.HostConfig.Sysctls then
		for k, v in pairs(d.HostConfig.Sysctls) do
			data = (data and (data .. "<br />") or "") .. k..":"..v
		end
	end

	return data
end

local get_networks = function(d)
	local data={}

	if d.NetworkSettings and d.NetworkSettings.Networks and type(d.NetworkSettings.Networks) == "table" then
		for k,v in pairs(d.NetworkSettings.Networks) do
			data[k] = v.IPAddress or ""
		end
	end

	return data
end


local start_stop_remove = function(m, cmd)
	local res

	docker:clear_status()
	docker:append_status("Containers: " .. cmd .. " " .. container_id .. "...")

	if cmd ~= "upgrade" then
		res = dk.containers[cmd](dk, {id = container_id})
	else
		res = dk.containers_upgrade(dk, {id = container_id})
	end

	if res and res.code >= 300 then
		docker:append_status("code:" .. res.code.." ".. (res.body.message and res.body.message or res.message))
		luci.http.redirect(luci.dispatcher.build_url("admin/docker/container/"..container_id))
	else
		docker:clear_status()
		if cmd ~= "remove" and cmd ~= "upgrade" then
			luci.http.redirect(luci.dispatcher.build_url("admin/docker/container/"..container_id))
		else
			luci.http.redirect(luci.dispatcher.build_url("admin/docker/containers"))
		end
	end
end

m=SimpleForm("docker",
	translatef("Docker - Container (%s)", container_info.Name:sub(2)),
	translate("On this page, the selected container can be managed."))
m.redirect = luci.dispatcher.build_url("admin/docker/containers")

s = m:section(SimpleSection)
s.template = "dockerman/apply_widget"
s.err=docker:read_status()
s.err=s.err and s.err:gsub("\n","<br />"):gsub(" ","&#160;")
if s.err then
	docker:clear_status()
end

s = m:section(Table,{{}})
s.notitle=true
s.rowcolors=false
s.template = "cbi/nullsection"

o = s:option(Button, "_start")
o.template = "dockerman/cbi/inlinebutton"
o.inputtitle=translate("Start")
o.inputstyle = "apply"
o.forcewrite = true
o.write = function(self, section)
	start_stop_remove(m,"start")
end

o = s:option(Button, "_restart")
o.template = "dockerman/cbi/inlinebutton"
o.inputtitle=translate("Restart")
o.inputstyle = "reload"
o.forcewrite = true
o.write = function(self, section)
	start_stop_remove(m,"restart")
end

o = s:option(Button, "_stop")
o.template = "dockerman/cbi/inlinebutton"
o.inputtitle=translate("Stop")
o.inputstyle = "reset"
o.forcewrite = true
o.write = function(self, section)
	start_stop_remove(m,"stop")
end

o = s:option(Button, "_kill")
o.template = "dockerman/cbi/inlinebutton"
o.inputtitle=translate("Kill")
o.inputstyle = "reset"
o.forcewrite = true
o.write = function(self, section)
	start_stop_remove(m,"kill")
end

o = s:option(Button, "_export")
o.template = "dockerman/cbi/inlinebutton"
o.inputtitle=translate("Export")
o.inputstyle = "apply"
o.forcewrite = true
o.write = function(self, section)
  luci.http.redirect(luci.dispatcher.build_url("admin/docker/container_export/"..container_id))
end

o = s:option(Button, "_upgrade")
o.template = "dockerman/cbi/inlinebutton"
o.inputtitle=translate("Upgrade")
o.inputstyle = "reload"
o.forcewrite = true
o.write = function(self, section)
	start_stop_remove(m,"upgrade")
end

o = s:option(Button, "_duplicate")
o.template = "dockerman/cbi/inlinebutton"
o.inputtitle=translate("Duplicate/Edit")
o.inputstyle = "add"
o.forcewrite = true
o.write = function(self, section)
	luci.http.redirect(luci.dispatcher.build_url("admin/docker/newcontainer/duplicate/"..container_id))
end

o = s:option(Button, "_remove")
o.template = "dockerman/cbi/inlinebutton"
o.inputtitle=translate("Remove")
o.inputstyle = "remove"
o.forcewrite = true
o.write = function(self, section)
	start_stop_remove(m,"remove")
end

s = m:section(SimpleSection)
s.template = "dockerman/container"

if action == "info" then
	res = dk.networks:list()
	if res.code < 300 then
		networks = res.body
	else
		return
	end
	m.submit = false
	m.reset  = false
	table_info = {
		["01name"] = {
			_key = translate("Name"),
			_value = container_info.Name:sub(2) or "-",
			_button=translate("Update")
		},
		["02id"] = {
			_key = translate("ID"),
			_value = container_info.Id or "-"
		},
		["03image"] = {
			_key = translate("Image"),
			_value = container_info.Config.Image .. "<br />" .. container_info.Image
		},
		["04status"] = {
			_key = translate("Status"),
			_value = container_info.State and container_info.State.Status or "-"
		},
		["05created"] = {
			_key = translate("Created"),
			_value = container_info.Created or "-"
		},
	}

	if container_info.State.Status == "running" then
		table_info["06start"] = {
			_key = translate("Start Time"),
			_value = container_info.State and container_info.State.StartedAt or "-"
		}
	else
		table_info["06start"] = {
			_key = translate("Finish Time"),
			_value = container_info.State and container_info.State.FinishedAt or "-"
		}
	end

	table_info["07healthy"] = {
		_key = translate("Healthy"),
		_value = container_info.State and container_info.State.Health and container_info.State.Health.Status or "-"
	}
	table_info["08restart"] = {
		_key = translate("Restart Policy"),
		_value = container_info.HostConfig and container_info.HostConfig.RestartPolicy and container_info.HostConfig.RestartPolicy.Name or "-",
		_button=translate("Update")
	}
	table_info["081user"] = {
		_key = translate("User"),
		_value = container_info.Config and (container_info.Config.User ~="" and container_info.Config.User or "-") or "-"
	}
	table_info["09mount"] = {
		_key = translate("Mount/Volume"),
		_value = get_mounts(container_info) or "-"
	}
	table_info["10cmd"] = {
		_key = translate("Command"),
		_value = get_command(container_info) or "-"
	}
	table_info["11env"] = {
		_key = translate("Env"),
		_value = get_env(container_info) or "-"
	}
	table_info["12ports"] = {
		_key = translate("Ports"),
		_value = get_ports(container_info) or "-"
	}
	table_info["13links"] = {
		_key = translate("Links"),
		_value = get_links(container_info) or "-"
	}
	table_info["14device"] = {
		_key = translate("Device"),
		_value = get_device(container_info) or "-"
	}
	table_info["15tmpfs"] = {
		_key = translate("Tmpfs"),
		_value = get_tmpfs(container_info) or "-"
	}
	table_info["16dns"] = {
		_key = translate("DNS"),
		_value = get_dns(container_info) or "-"
	}
	table_info["17sysctl"] = {
		_key = translate("Sysctl"),
		_value = get_sysctl(container_info) or "-"
	}

	info_networks = get_networks(container_info)
	list_networks = {}
	for _, v in ipairs (networks) do
		if v and v.Name then
			local parent = v.Options and v.Options.parent or nil
			local ip = v.IPAM and v.IPAM.Config and v.IPAM.Config[1] and v.IPAM.Config[1].Subnet or nil
			ipv6 =  v.IPAM and v.IPAM.Config and v.IPAM.Config[2] and v.IPAM.Config[2].Subnet or nil
			local network_name = v.Name .. " | " .. v.Driver  .. (parent and (" | " .. parent) or "") .. (ip and (" | " .. ip) or "").. (ipv6 and (" | " .. ipv6) or "")
			list_networks[v.Name] = network_name
		end
	end

	if type(info_networks)== "table" then
		for k,v in pairs(info_networks) do
			table_info["14network"..k] = {
				_key = translate("Network"),
				_value = k.. (v~="" and (" | ".. v) or ""),
				_button=translate("Disconnect")
			}
			list_networks[k]=nil
		end
	end

	table_info["15connect"] = {
		_key = translate("Connect Network"),
		_value = list_networks ,_opts = "",
		_button=translate("Connect")
	}

	s = m:section(Table,table_info)
	s.nodescr=true
	s.formvalue=function(self, section)
		return table_info
	end

	o = s:option(DummyValue, "_key", translate("Info"))
	o.width = "20%"

	o = s:option(ListValue, "_value")
	o.render = function(self, section, scope)
		if table_info[section]._key == translate("Name") then
			self:reset_values()
			self.template = "cbi/value"
			self.size = 30
			self.keylist = {}
			self.vallist = {}
			self.default=table_info[section]._value
			Value.render(self, section, scope)
		elseif table_info[section]._key == translate("Restart Policy") then
			self.template = "cbi/lvalue"
			self:reset_values()
			self.size = nil
			self:value("no", "No")
			self:value("unless-stopped", "Unless stopped")
			self:value("always", "Always")
			self:value("on-failure", "On failure")
			self.default=table_info[section]._value
			ListValue.render(self, section, scope)
		elseif table_info[section]._key == translate("Connect Network") then
			self.template = "cbi/lvalue"
			self:reset_values()
			self.size = nil
			for k,v in pairs(list_networks) do
					if k ~= "host" then
					self:value(k,v)
				end
			end
			self.default=table_info[section]._value
			ListValue.render(self, section, scope)
		else
			self:reset_values()
			self.rawhtml=true
			self.template = "cbi/dvalue"
			self.default=table_info[section]._value
			DummyValue.render(self, section, scope)
		end
	end
	o.forcewrite = true
	o.write = function(self, section, value)
		table_info[section]._value=value
	end
	o.validate = function(self, value)
		return value
	end

	o = s:option(Value, "_opts")
	o.forcewrite = true
	o.write = function(self, section, value)
		table_info[section]._opts=value
	end
	o.validate = function(self, value)
		return value
	end
	o.render = function(self, section, scope)
		if table_info[section]._key==translate("Connect Network") then
			self.template = "cbi/value"
			self.keylist = {}
			self.vallist = {}
			self.placeholder = "10.1.1.254"
			self.datatype = "ip4addr"
			self.default=table_info[section]._opts
			Value.render(self, section, scope)
		else
			self.rawhtml=true
			self.template = "cbi/dvalue"
			self.default=table_info[section]._opts
			DummyValue.render(self, section, scope)
		end
	end

	o = s:option(Button, "_button")
	o.forcewrite = true
	o.render = function(self, section, scope)
		if table_info[section]._button and table_info[section]._value ~= nil then
			self.inputtitle=table_info[section]._button
			self.template = "cbi/button"
			self.inputstyle = "edit"
			Button.render(self, section, scope)
		else
			self.template = "cbi/dvalue"
			self.default=""
			DummyValue.render(self, section, scope)
		end
	end
	o.write = function(self, section, value)
		local res

		docker:clear_status()

		if section == "01name" then
			docker:append_status("Containers: rename " .. container_id .. "...")
			local new_name = table_info[section]._value
			res = dk.containers:rename({
				id = container_id,
				query = {
					name=new_name
				}
			})
		elseif section == "08restart" then
			docker:append_status("Containers: update " .. container_id .. "...")
			local new_restart = table_info[section]._value
			res = dk.containers:update({
				id = container_id,
				body = {
					RestartPolicy = {
						Name = new_restart
					}
				}
			})
		elseif table_info[section]._key == translate("Network") then
			local _,_,leave_network

			_, _, leave_network = table_info[section]._value:find("(.-) | .+")
			leave_network = leave_network or table_info[section]._value
			docker:append_status("Network: disconnect " .. leave_network .. container_id .. "...")
			res = dk.networks:disconnect({
				name = leave_network,
				body = {
					Container = container_id
				}
			})
		elseif section == "15connect" then
			local connect_network = table_info[section]._value
			local network_opiton
			if connect_network ~= "none"
				and connect_network ~= "bridge"
				and connect_network ~= "host" then

				network_opiton = table_info[section]._opts ~= "" and {
					IPAMConfig={
						IPv4Address=table_info[section]._opts
					}
				} or nil
			end
			docker:append_status("Network: connect " .. connect_network .. container_id .. "...")
			res = dk.networks:connect({
				name = connect_network,
				body = {
					Container = container_id,
					EndpointConfig= network_opiton
				}
			})
		end

		if res and res.code > 300 then
			docker:append_status("code:" .. res.code.." ".. (res.body.message and res.body.message or res.message))
		else
			docker:clear_status()
		end
		luci.http.redirect(luci.dispatcher.build_url("admin/docker/container/"..container_id.."/info"))
	end
elseif action == "resources" then
	s = m:section(SimpleSection)
	o = s:option( Value, "cpus",
		translate("CPUs"),
		translate("Number of CPUs. Number is a fractional number. 0.000 means no limit."))
	o.placeholder = "1.5"
	o.rmempty = true
	o.datatype="ufloat"
	o.default = container_info.HostConfig.NanoCpus / (10^9)

	o = s:option(Value, "cpushares",
		translate("CPU Shares Weight"),
		translate("CPU shares relative weight, if 0 is set, the system will ignore the value and use the default of 1024."))
	o.placeholder = "1024"
	o.rmempty = true
	o.datatype="uinteger"
	o.default = container_info.HostConfig.CpuShares

	o = s:option(Value, "memory",
		translate("Memory"),
		translate("Memory limit (format: <number>[<unit>]). Number is a positive integer. Unit can be one of b, k, m, or g. Minimum is 4M."))
	o.placeholder = "128m"
	o.rmempty = true
	o.default = container_info.HostConfig.Memory ~=0 and ((container_info.HostConfig.Memory / 1024 /1024) .. "M") or 0

	o = s:option(Value, "blkioweight",
		translate("Block IO Weight"),
		translate("Block IO weight (relative weight) accepts a weight value between 10 and 1000."))
	o.placeholder = "500"
	o.rmempty = true
	o.datatype="uinteger"
	o.default = container_info.HostConfig.BlkioWeight

	m.handle = function(self, state, data)
		if state == FORM_VALID then
			local memory = data.memory
			if memory and memory ~= 0 then
				_,_,n,unit = memory:find("([%d%.]+)([%l%u]+)")
				if n then
					unit = unit and unit:sub(1,1):upper() or "B"
					if  unit == "M" then
						memory = tonumber(n) * 1024 * 1024
					elseif unit == "G" then
						memory = tonumber(n) * 1024 * 1024 * 1024
					elseif unit == "K" then
						memory = tonumber(n) * 1024
					else
						memory = tonumber(n)
					end
				end
			end

			request_body = {
				BlkioWeight = tonumber(data.blkioweight),
				NanoCPUs = tonumber(data.cpus)*10^9,
				Memory = tonumber(memory),
				CpuShares = tonumber(data.cpushares)
			}

			docker:write_status("Containers: update " .. container_id .. "...")
			local res = dk.containers:update({id = container_id, body = request_body})
			if res and res.code >= 300 then
				docker:append_status("code:" .. res.code.." ".. (res.body.message and res.body.message or res.message))
			else
				docker:clear_status()
			end
			luci.http.redirect(luci.dispatcher.build_url("admin/docker/container/"..container_id.."/resources"))
		end
	end

elseif action == "file" then
	m.submit = false
	m.reset  = false
	s= m:section(SimpleSection)
	s.template = "dockerman/container_file_manager"
	s.container = container_id
	m.redirect = nil
elseif action == "inspect" then
	s = m:section(SimpleSection)
	s.syslog = luci.jsonc.stringify(container_info, true)
	s.title = translate("Container Inspect")
	s.template = "dockerman/logs"
	m.submit = false
	m.reset  = false
elseif action == "logs" then
	local logs = ""
	local query ={
		stdout = 1,
		stderr = 1,
		tail = 1000
	}

	s = m:section(SimpleSection)

	logs = dk.containers:logs({id = container_id, query = query})
	if logs.code == 200 then
		s.syslog=logs.body
	else
		s.syslog="Get Logs ERROR\n"..logs.code..": "..logs.body
	end

	s.title=translate("Container Logs")
	s.template = "dockerman/logs"
	m.submit = false
	m.reset  = false
elseif action == "console" then
	m.submit = false
	m.reset  = false
	local cmd_docker = luci.util.exec("command -v docker"):match("^.+docker") or nil
	local cmd_ttyd = luci.util.exec("command -v ttyd"):match("^.+ttyd") or nil

	if cmd_docker and cmd_ttyd and container_info.State.Status == "running" then
		local cmd = "/bin/sh"
		local uid

		s = m:section(SimpleSection)

		o = s:option(Value, "command", translate("Command"))
		o:value("/bin/sh", "/bin/sh")
		o:value("/bin/ash", "/bin/ash")
		o:value("/bin/bash", "/bin/bash")
		o.default = "/bin/sh"
		o.forcewrite = true
		o.write = function(self, section, value)
			cmd = value
		end

		o = s:option(Value, "uid", translate("UID"))
		o.forcewrite = true
		o.write = function(self, section, value)
			uid = value
		end

		o = s:option(Button, "connect")
		o.render = function(self, section, scope)
			self.inputstyle = "add"
			self.title = " "
			self.inputtitle = translate("Connect")
			Button.render(self, section, scope)
		end
		o.write = function(self, section)
			local cmd_docker = luci.util.exec("command -v docker"):match("^.+docker") or nil
			local cmd_ttyd = luci.util.exec("command -v ttyd"):match("^.+ttyd") or nil

			if not cmd_docker or not cmd_ttyd or cmd_docker:match("^%s+$") or cmd_ttyd:match("^%s+$") then
				return
			end

			local pid = luci.util.trim(luci.util.exec("netstat -lnpt | grep :7682 | grep ttyd | tr -s ' ' | cut -d ' ' -f7 | cut -d'/' -f1"))
			if pid and pid ~= "" then
				luci.util.exec("kill -9 " .. pid)
			end

			local hosts
			local uci = (require "luci.model.uci").cursor()
			local remote = uci:get_bool("dockerd", "dockerman", "remote_endpoint") or false
			local host = nil
			local port = nil
			local socket = nil

			if remote then
				host = uci:get("dockerd", "dockerman", "remote_host") or nil
				port = uci:get("dockerd", "dockerman", "remote_port") or nil
			else
				socket = uci:get("dockerd", "dockerman", "socket_path") or "/var/run/docker.sock"
			end

			if remote and host and port then
				hosts = "tcp://" .. host .. ':'.. port
			elseif socket then
				hosts = "unix://" .. socket
			else
				return
			end

			if uid and uid ~= "" then
				uid = "-u " .. uid
			else
				uid = ""
			end

			local start_cmd = string.format('%s -d 2 --once -p 7682 %s -H "%s" exec -it %s %s %s&', cmd_ttyd, cmd_docker, hosts, uid, container_id, cmd)

			os.execute(start_cmd)

			o = s:option(DummyValue, "console")
			o.container_id = container_id
			o.template = "dockerman/container_console"
		end
	end
elseif action == "stats" then
	local response = dk.containers:top({id = container_id, query = {ps_args="-aux"}})
	local container_top

	if response.code == 200 then
		container_top=response.body
	else
		response = dk.containers:top({id = container_id})
		if response.code == 200 then
			container_top=response.body
		end
	end

	if type(container_top) == "table" then
		s = m:section(SimpleSection)
		s.container_id = container_id
		s.template = "dockerman/container_stats"
		table_stats = {
			cpu={
				key=translate("CPU Useage"),
				value='-'
			},
			memory={
				key=translate("Memory Useage"),
				value='-'
			}
		}

		container_top = response.body
		s = m:section(Table, table_stats, translate("Stats"))
		s:option(DummyValue, "key", translate("Stats")).width="33%"
		s:option(DummyValue, "value")
		top_section = m:section(Table, container_top.Processes, translate("TOP"))
		for i, v in ipairs(container_top.Titles) do
			top_section:option(DummyValue, i, translate(v))
		end
	end

	m.submit = false
	m.reset  = false
end

return m
