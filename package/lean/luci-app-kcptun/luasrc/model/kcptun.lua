-- Copyright 2016-2017 Xingwang Liao <kuoruan@gmail.com>
-- Licensed to the public under the Apache License 2.0.

local fs   = require "nixio.fs"
local sys  = require "luci.sys"
local uci  = require "luci.model.uci".cursor()
local util = require "luci.util"
local i18n = require "luci.i18n"

module("luci.model.kcptun", package.seeall)

local kcptun_api = "https://api.github.com/repos/xtaci/kcptun/releases/latest"
local luci_api = "https://api.github.com/repos/kuoruan/luci-app-kcptun/releases/latest"

local wget = "/usr/bin/wget"
local wget_args = { "--no-check-certificate", "--quiet", "--timeout=10", "--tries=2" }
local command_timeout = 40

local function _unpack(t, i)
	i = i or 1
	if t[i] ~= nil then
		return t[i], _unpack(t, i + 1)
	end
end

local function exec(cmd, args, writer, timeout)
	local os = require "os"
	local nixio = require "nixio"

	local fdi, fdo = nixio.pipe()
	local pid = nixio.fork()

	if pid > 0 then
		fdo:close()

		if writer or timeout then
			local starttime = os.time()
			while true do
				if timeout and os.difftime(os.time(), starttime) >= timeout then
					nixio.kill(pid, nixio.const.SIGTERM)
					return 1
				end

				if writer then
					local buffer = fdi:read(2048)
					if buffer and #buffer > 0 then
						writer(buffer)
					end
				end

				local wpid, stat, code = nixio.waitpid(pid, "nohang")

				if wpid and stat == "exited" then
					return code
				end

				if not writer and timeout then
					nixio.nanosleep(1)
				end
			end
		else
			local wpid, stat, code = nixio.waitpid(pid)
			return wpid and stat == "exited" and code
		end
	elseif pid == 0 then
		nixio.dup(fdo, nixio.stdout)
		fdi:close()
		fdo:close()
		nixio.exece(cmd, args, nil)
		nixio.stdout:close()
		os.exit(1)
	end
end

local function compare_versions(ver1, comp, ver2)
	local table = table

	local av1 = util.split(ver1, "[%.%-]", nil, true)
	local av2 = util.split(ver2, "[%.%-]", nil, true)

	local max = table.getn(av1)
	local n2 = table.getn(av2)
	if (max < n2) then
		max = n2
	end

	for i = 1, max, 1  do
		local s1 = av1[i] or ""
		local s2 = av2[i] or ""

		if comp == "~=" and (s1 ~= s2) then return true end
		if (comp == "<" or comp == "<=") and (s1 < s2) then return true end
		if (comp == ">" or comp == ">=") and (s1 > s2) then return true end
		if (s1 ~= s2) then return false end
	end

	return not (comp == "<" or comp == ">")
end

local function auto_get_arch()
	local arch = nixio.uname().machine or ""

	if arch == "mips" then
		if fs.access("/usr/lib/os-release") then
			arch = sys.exec("grep 'LEDE_BOARD' /usr/lib/os-release | grep -oE 'ramips|ar71xx'")
		elseif fs.access("/etc/openwrt_release") then
			arch = sys.exec("grep 'DISTRIB_TARGET' /etc/openwrt_release | grep -oE 'ramips|ar71xx'")
		end
	end

	return util.trim(arch)
end

local function get_file_info(arch)
	local file_tree = ""
	local sub_version = ""

	if arch == "x86_64" then
		file_tree = "amd64"
	elseif arch == "ramips" then
		file_tree = "mipsle"
	elseif arch == "ar71xx" then
		file_tree = "mips"
	elseif arch:match("^i[%d]86$") then
		file_tree = "386"
	elseif arch:match("^armv[5-8]") then
		file_tree = "arm"
		sub_version = arch:match("[5-8]")
	end

	return file_tree, sub_version
end

local function get_api_json(url)
	local jsonc = require "luci.jsonc"

	local output = { }
	exec(wget, { "-O-", url, _unpack(wget_args) },
		function(chunk) output[#output + 1] = chunk end)

	local json_content = util.trim(table.concat(output))

	if json_content == "" then
		return { }
	end

	return jsonc.parse(json_content) or { }
end

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

function check_kcptun(arch)
	if not arch or arch == "" then
		arch = auto_get_arch()
	end

	local file_tree, sub_version = get_file_info(arch)

	if file_tree == "" then
		return {
			code = 1,
			error = i18n.translate("Can't determine ARCH, or ARCH not supported. Please select manually.")
		}
	end

	local json = get_api_json(kcptun_api)

	if json.tag_name == nil then
		return {
			code = 1,
			error = i18n.translate("Get remote version info failed.")
		}
	end

	local remote_version = json.tag_name:match("[^v]+")

	local client_file = get_config_option("client_file")

	local needs_update = compare_versions(get_kcptun_version(client_file), "<", remote_version)
	local html_url, download_url

	if needs_update then
		html_url = json.html_url
		for _, v in ipairs(json.assets) do
			if v.name and v.name:match("linux%-" .. file_tree) then
				download_url = v.browser_download_url
				break
			end
		end
	end

	if needs_update and not download_url then
		return {
			code = 1,
			version = remote_version,
			html_url = html_url,
			error = i18n.translate("New version found, but failed to get new version download url.")
		}
	end

	return {
		code = 0,
		update = needs_update,
		version = remote_version,
		url = {
			html = html_url,
			download = download_url
		},
		type = file_tree .. sub_version
	}
end

function check_luci()
	local json = get_api_json(luci_api)

	if json.tag_name == nil then
		return {
			code = 1,
			error = i18n.translate("Get remote version info failed.")
		}
	end

	local remote_version = json.tag_name:match("[^v]+")

	local needs_update = compare_versions(get_luci_version(), "<", remote_version)
	local html_url, luci_url
	local i18n_urls = { }

	if needs_update then
		html_url = json.html_url
		for _, v in ipairs(json.assets) do
			local n = v.name
			if n then
				if n:match("luci%-app%-kcptun") then
					luci_url = v.browser_download_url
				elseif n:match("luci%-i18n%-kcptun") then
					i18n_urls[#i18n_urls + 1] = v.browser_download_url
				end
			end
		end
	end

	if needs_update and not luci_url then
		return {
			code = 1,
			version = remote_version,
			html_url = html_url,
			error = i18n.translate("New version found, but failed to get new version download url.")
		}
	end

	return {
		code = 0,
		update = needs_update,
		version = remote_version,
		url = {
			html = html_url,
			luci = luci_url,
			i18n = i18n_urls
		}
	}
end

function download_kcptun(url)
	if not url or url == "" then
		return {
			code = 1,
			error = i18n.translate("Download url is required.")
		}
	end

	sys.call("/bin/rm -f /tmp/kcptun_download.*")

	local tmp_file = util.trim(util.exec("mktemp -u -t kcptun_download.XXXXXX"))

	local result = exec(wget, {
		"-O", tmp_file, url, _unpack(wget_args) }, nil, command_timeout) == 0

	if not result then
		exec("/bin/rm", { "-f", tmp_file })
		return {
			code = 1,
			error = i18n.translatef("File download failed or timed out: %s", url)
		}
	end

	return {
		code = 0,
		file = tmp_file
	}
end

function extract_kcptun(file, subfix)
	if not file or file == "" or not fs.access(file) then
		return {
			code = 1,
			error = i18n.translate("File path required.")
		}
	end

	sys.call("/bin/rm -rf /tmp/kcptun_extract.*")
	local tmp_dir = util.trim(util.exec("mktemp -d -t kcptun_extract.XXXXXX"))

	local output = { }
	exec("/bin/tar", { "-C", tmp_dir, "-zxvf", file },
		function(chunk) output[#output + 1] = chunk end)

	local files = util.split(table.concat(output))

	exec("/bin/rm", { "-f", file })

	local new_file = nil
	for _, f in pairs(files) do
		if f:match("client_linux_%s" % subfix) then
			new_file = tmp_dir .. "/" .. util.trim(f)
			break
		end
	end

	if not new_file then
		for _, f in pairs(files) do
			if f:match("client_") then
				new_file = tmp_dir .. "/" .. util.trim(f)
				break
			end
		end
	end

	if not new_file then
		exec("/bin/rm", { "-rf", tmp_dir })
		return {
			code = 1,
			error = i18n.translatef("Can't find client in file: %s", file)
		}
	end

	return {
		code = 0,
		file = new_file
	}
end

function move_kcptun(file)
	if not file or file == "" or not fs.access(file) then
		sys.call("/bin/rm -rf /tmp/kcptun_extract.*")
		return {
			code = 1,
			error = i18n.translate("Client file is required.")
		}
	end

	local version = get_kcptun_version(file)
	if version == "" then
		sys.call("/bin/rm -rf /tmp/kcptun_extract.*")
		return {
			code = 1,
			error = i18n.translate("The client file is not suitable for current device. Please reselect ARCH.")
		}
	end

	local client_file = get_config_option("client_file", "/var/kcptun_client")
	local client_file_bak

	if fs.access(client_file) then
		client_file_bak = client_file .. ".bak"
		exec("/bin/mv", { "-f", client_file, client_file_bak })
	end

	local result = exec("/bin/mv", { "-f", file, client_file }, nil, command_timeout) == 0

	if not result or not fs.access(client_file) then
		sys.call("/bin/rm -rf /tmp/kcptun_extract.*")
		if client_file_bak then
			exec("/bin/mv", { "-f", client_file_bak, client_file })
		end
		return {
			code = 1,
			error = i18n.translatef("Can't move new file to path: %s", client_file)
		}
	end

	exec("/bin/chmod", { "755", client_file })

	if client_file_bak then
		exec("/bin/rm", { "-f", client_file_bak })
	end

	sys.call("/bin/rm -rf /tmp/kcptun_extract.*")

	uci:set("kcptun", "general", "client_file", client_file)
	uci:commit("kcptun")

	return { code = 0 }
end

function update_luci(url, save)
	if not url or url == "" then
		return {
			code = 1,
			error = i18n.translate("Download url is required.")
		}
	end

	sys.call("/bin/rm -f /tmp/luci_kcptun.*.ipk")

	local tmp_file = util.trim(util.exec("mktemp -u -t luci_kcptun.XXXXXX")) .. ".ipk"

	local result = exec("/usr/bin/wget", {
		"-O", tmp_file, url, _unpack(wget_args) }, nil, command_timeout) == 0

	if not result then
		exec("/bin/rm", { "-f", tmp_file })
		return {
			code = 1,
			error = i18n.translatef("File download failed or timed out: %s", url)
		}
	end

	local opkg_args = { "--force-downgrade", "--force-reinstall" }

	if save ~= "true" then
		opkg_args[#opkg_args + 1] = "--force-maintainer"
	end

	result = exec("/bin/opkg", { "install", tmp_file, _unpack(opkg_args) }) == 0

	if not result then
		exec("/bin/rm", { "-f", tmp_file })
		return {
			code = 1,
			error = i18n.translate("Package update failed.")
		}
	end

	exec("/bin/rm", { "-f", tmp_file })
	exec("/bin/rm", { "-rf", "/tmp/luci-indexcache", "/tmp/luci-modulecache" })

	return { code = 0 }
end
