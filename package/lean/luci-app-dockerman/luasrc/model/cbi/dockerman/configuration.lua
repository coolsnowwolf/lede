--[[
LuCI - Lua Configuration Interface
Copyright 2021 Florian Eckert <fe@dev.tdt.de>
Copyright 2021 lisaac <lisaac.cn@gmail.com>
]]--

local uci = (require "luci.model.uci").cursor()

local m, s, o

m = Map("dockerd",
	translate("Docker - Configuration"),
	translate("DockerMan is a simple docker manager client for LuCI"))

if nixio.fs.access("/usr/bin/dockerd") and not m.uci:get_bool("dockerd", "dockerman", "remote_endpoint")  then
	s = m:section(NamedSection, "globals", "section", translate("Docker Daemon settings"))

	o = s:option(Flag, "auto_start", translate("Auto start"))
	o.rmempty = false
	o.write = function(self, section, value)
		if value == "1" then
			luci.util.exec("/etc/init.d/dockerd enable")
		else
			luci.util.exec("/etc/init.d/dockerd disable")
		end
		m.uci:set("dockerd", "globals", "auto_start", value)
	end

	o = s:option(Value, "data_root",
		translate("Docker Root Dir"))
	o.placeholder = "/opt/docker/"
	o:depends("remote_endpoint", 0)

	o = s:option(Value, "bip",
		translate("Default bridge"),
		translate("Configure the default bridge network"))
	o.placeholder = "172.17.0.1/16"
	o.datatype = "ipaddr"
	o:depends("remote_endpoint", 0)

	o = s:option(DynamicList, "registry_mirrors",
		translate("Registry Mirrors"),
		translate("It replaces the daemon registry mirrors with a new set of registry mirrors"))
	o:value("https://hub-mirror.c.163.com", "https://hub-mirror.c.163.com")
	o:depends("remote_endpoint", 0)
	o.forcewrite = true

	o = s:option(ListValue, "log_level",
		translate("Log Level"),
		translate('Set the logging level'))
	o:value("debug", translate("Debug"))
	o:value("", translate("Info")) -- This is the default debug level from the deamon is optin is not set
	o:value("warn", translate("Warning"))
	o:value("error", translate("Error"))
	o:value("fatal", translate("Fatal"))
	o.rmempty = true
	o:depends("remote_endpoint", 0)

	o = s:option(DynamicList, "hosts",
		translate("Client connection"),
		translate('Specifies where the Docker daemon will listen for client connections (default: unix:///var/run/docker.sock)'))
	o:value("unix:///var/run/docker.sock", "unix:///var/run/docker.sock")
	o:value("tcp://0.0.0.0:2375", "tcp://0.0.0.0:2375")
	o.rmempty = true
	o:depends("remote_endpoint", 0)
end

s = m:section(NamedSection, "dockerman", "section", translate("DockerMan settings"))
s:tab("ac", translate("Access Control"))
s:tab("dockerman", translate("DockerMan"))

o = s:taboption("dockerman", Flag, "remote_endpoint",
	translate("Remote Endpoint"),
	translate("Connect to remote docker endpoint"))
o.rmempty = false
o.validate = function(self, value, sid)
	local res = luci.http.formvaluetable("cbid.dockerd")
	if res["dockerman.remote_endpoint"] == "1" then
	 if res["dockerman.remote_port"] and res["dockerman.remote_port"] ~= "" and res["dockerman.remote_host"] and res["dockerman.remote_host"] ~= "" then
			return 1
		else
			return nil, translate("Please input the PORT or HOST IP of remote docker instance!")
		end
	else
		if not res["dockerman.socket_path"] then
			return nil, translate("Please input the SOCKET PATH of docker daemon!")
		end
	end
	return 0
end

o = s:taboption("dockerman", Value, "socket_path",
	translate("Docker Socket Path"))
o.default = "/var/run/docker.sock"
o.placeholder = "/var/run/docker.sock"
o:depends("remote_endpoint", 0)

o = s:taboption("dockerman", Value, "remote_host",
	translate("Remote Host"),
	translate("Host or IP Address for the connection to a remote docker instance"))
o.datatype = "host"
o.placeholder = "10.1.1.2"
o:depends("remote_endpoint", 1)

o = s:taboption("dockerman", Value, "remote_port",
	translate("Remote Port"))
o.placeholder = "2375"
o.datatype = "port"
o:depends("remote_endpoint", 1)

-- o = s:taboption("dockerman", Value, "status_path", translate("Action Status Tempfile Path"), translate("Where you want to save the docker status file"))
-- o = s:taboption("dockerman", Flag, "debug", translate("Enable Debug"), translate("For debug, It shows all docker API actions of luci-app-dockerman in Debug Tempfile Path"))
-- o.enabled="true"
-- o.disabled="false"
-- o = s:taboption("dockerman", Value, "debug_path", translate("Debug Tempfile Path"), translate("Where you want to save the debug tempfile"))

if nixio.fs.access("/usr/bin/dockerd") and not m.uci:get_bool("dockerd", "dockerman", "remote_endpoint")  then
	o = s:taboption("ac", DynamicList, "ac_allowed_interface", translate("Allowed access interfaces"), translate("Which interface(s) can access containers under the bridge network, fill-in Interface Name"))
	local interfaces = luci.sys and luci.sys.net and luci.sys.net.devices() or {}
	for i, v in ipairs(interfaces) do
		o:value(v, v)
	end
	o = s:taboption("ac", DynamicList, "ac_allowed_ports", translate("Ports allowed to be accessed"), translate("Which Port(s) can be accessed, it's not restricted by the Allowed Access interfaces configuration. Use this configuration with caution!"))
	o.placeholder = "8080/tcp"
	local docker = require "luci.model.docker"
	local containers, res, lost_state
	local dk = docker.new()
	if dk:_ping().code ~= 200 then
		lost_state = true
	else
		lost_state = false
		res = dk.containers:list()
		if res and res.code and res.code < 300 then
			containers = res.body
		end
	end

	-- allowed_container.placeholder = "container name_or_id"
	if containers then
		for i, v in ipairs(containers) do
			if	v.State == "running" and v.Ports then
				for _, port in ipairs(v.Ports) do
					if port.PublicPort and port.IP and not string.find(port.IP,":")  then
						o:value(port.PublicPort.."/"..port.Type, v.Names[1]:sub(2) .. " | " .. port.PublicPort .. " | " .. port.Type)
					end
				end
			end
		end
	end
end

return m
