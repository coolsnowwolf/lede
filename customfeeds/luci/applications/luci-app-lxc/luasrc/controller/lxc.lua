--[[

LuCI LXC module

Copyright (C) 2014, Cisco Systems, Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

Author: Petar Koretic <petar.koretic@sartura.hr>

]]--

local uci = require "luci.model.uci"
local util = require "luci.util"
local nixio = require "nixio"

module("luci.controller.lxc", package.seeall)

function fork_exec(command)
	local pid = nixio.fork()
	if pid > 0 then
		return
	elseif pid == 0 then
		-- change to root dir
		nixio.chdir("/")

		-- patch stdin, out, err to /dev/null
		local null = nixio.open("/dev/null", "w+")
		if null then
			nixio.dup(null, nixio.stderr)
			nixio.dup(null, nixio.stdout)
			nixio.dup(null, nixio.stdin)
			if null:fileno() > 2 then
				null:close()
			end
		end

		-- replace with target command
		nixio.exec("/bin/sh", "-c", command)
	end
end

function index()
	page = node("admin", "services", "lxc")
	page.target = cbi("lxc")
	page.title = _("LXC Containers")
	page.order = 70

	page = entry({"admin", "services", "lxc_create"}, call("lxc_create"), nil)
	page.leaf = true

	page = entry({"admin", "services", "lxc_action"}, call("lxc_action"), nil)
	page.leaf = true

	page = entry({"admin", "services", "lxc_get_downloadable"}, call("lxc_get_downloadable"), nil)
	page.leaf = true

	page = entry({"admin", "services", "lxc_configuration_get"}, call("lxc_configuration_get"), nil)
	page.leaf = true

	page = entry({"admin", "services", "lxc_configuration_set"}, call("lxc_configuration_set"), nil)
	page.leaf = true

end

function lxc_get_downloadable()
	local target = lxc_get_arch_target()
	local templates = {}

	local f = io.popen('sh /usr/share/lxc/templates/lxc-download --list --no-validate --server %s'
		% util.shellquote(uci.cursor():get("lxc", "lxc", "url")), 'r')

	local line
	for line in f:lines() do
		local dist, version, dist_target = line:match("^(%S+)%s+(%S+)%s+(%S+)%s+default%s+%S+$")
		if dist and version and dist_target == target then
			templates[#templates+1] = "%s:%s" %{ dist, version }
		end
	end

	f:close()

	luci.http.prepare_content("application/json")
	luci.http.write_json(templates)
end

function lxc_create(lxc_name, lxc_template)
	luci.http.prepare_content("text/plain")

	if not pcall(dofile, "/etc/openwrt_release") then
		return luci.http.write("1")
	end

	local lxc_dist, lxc_release = lxc_template:match("^(.+):(.+)$")

	luci.http.write(util.ubus("lxc", "create", {
		name = lxc_name,
		template = "download",
		args = {
			"--server", uci.cursor():get("lxc", "lxc", "url"),
			"--no-validate",
			"--dist", lxc_dist,
			"--release", lxc_release,
			"--arch", lxc_get_arch_target()
		}
	}))
end

function lxc_action(lxc_action, lxc_name)
	local data, ec = util.ubus("lxc", lxc_action, lxc_name and { name = lxc_name } or {})

	luci.http.prepare_content("application/json")
	luci.http.write_json(ec and {} or data)
end

function lxc_get_config_path()
	local f = io.open("/etc/lxc/lxc.conf", "r")
	local content = f:read("*all")
	f:close()
	local ret = content:match('^%s*lxc.lxcpath%s*=%s*([^%s]*)')
	if ret then
		return ret .. "/"
	else
		return "/srv/lxc/"
	end
end

function lxc_configuration_get(lxc_name)
	luci.http.prepare_content("text/plain")

	local f = io.open(lxc_get_config_path() .. lxc_name .. "/config", "r")
	local content = f:read("*all")
	f:close()

	luci.http.write(content)
end

function lxc_configuration_set(lxc_name)
	luci.http.prepare_content("text/plain")

	local lxc_configuration = luci.http.formvalue("lxc_configuration")

	if lxc_configuration == nil then
		return luci.http.write("1")
	end

	local f, err = io.open(lxc_get_config_path() .. lxc_name .. "/config","w+")
	if not f then
		return luci.http.write("2")
	end

	f:write(lxc_configuration)
	f:close()

	luci.http.write("0")
end

function lxc_get_arch_target()
	local target = nixio.uname().machine
	local target_map = {
		armv5  = "armel",
		armv6  = "armel",
		armv7  = "armhf",
		armv8  = "arm64",
		x86_64 = "amd64"
	}

	local k, v
	for k, v in pairs(target_map) do
		if target:find(k) then
			return v
		end
	end

	return target
end
