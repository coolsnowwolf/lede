-- Copyright 2016-2020 Xingwang Liao <kuoruan@gmail.com>
-- Licensed to the public under the Apache License 2.0.

local fs   = require "nixio.fs"
local sys  = require "luci.sys"
local uci  = require "luci.model.uci".cursor()
local util = require "luci.util"

module("luci.model.kcptun", package.seeall)

function get_config_option(option, default)
	return uci:get("kcptun", "general", option) or default
end

function get_current_log_file(type)
	local log_folder = get_config_option("log_folder", "/var/log/kcptun")
	return "%s/%s.%s.log" % { log_folder, type, "general" }
end

function is_running(client)
	if client and client ~= "" then
		local file_name = client:match(".*/([^/]+)$") or ""
		if file_name ~= "" then
			return sys.call("pidof %s >/dev/null" % file_name) == 0
		end
	end

	return false
end

function get_kcptun_version(file)
	if file and file ~= "" then
		if not fs.access(file, "rwx", "rx", "rx") then
			fs.chmod(file, 755)
		end

		local info = util.trim(sys.exec("%s -v 2>/dev/null" % file))

		if info ~= "" then
			local tb = util.split(info, "%s+", nil, true)
			return tb[1] == "kcptun" and tb[3] or ""
		end
	end

	return ""
end

function get_luci_version()
	local ipkg  = require "luci.model.ipkg"

	local package_name = "luci-app-kcptun"
	local package_info = ipkg.info(package_name) or {}

	if next(package_info) ~= nil then
		return package_info[package_name]["Version"]
	end
	return ""
end
