module("luci.passwall.api", package.seeall)
local com = require "luci.passwall.com"
bin = require "nixio".bin
fs = require "nixio.fs"
sys = require "luci.sys"
uci = require"luci.model.uci".cursor()
util = require "luci.util"
datatypes = require "luci.cbi.datatypes"
jsonc = require "luci.jsonc"
i18n = require "luci.i18n"

appname = "passwall"
curl_args = { "-skfL", "--connect-timeout 3", "--retry 3", "-m 60" }
command_timeout = 300
OPENWRT_ARCH = nil
DISTRIB_ARCH = nil

LOG_FILE = "/tmp/log/" .. appname .. ".log"
CACHE_PATH = "/tmp/etc/" .. appname .. "_tmp"

function log(...)
	local result = os.date("%Y-%m-%d %H:%M:%S: ") .. table.concat({...}, " ")
	local f, err = io.open(LOG_FILE, "a")
	if f and err == nil then
		f:write(result .. "\n")
		f:close()
	end
end

function exec_call(cmd)
	local process = io.popen(cmd .. '; echo -e "\n$?"')
	local lines = {}
	local result = ""
	local return_code
	for line in process:lines() do
		lines[#lines + 1] = line
	end
	process:close()
	if #lines > 0 then
		return_code = lines[#lines]
		for i = 1, #lines - 1 do
			result = result .. lines[i] .. ((i == #lines - 1) and "" or "\n")
		end
	end
	return tonumber(return_code), trim(result)
end

function base64Decode(text)
	local raw = text
	if not text then return '' end
	text = text:gsub("%z", "")
	text = text:gsub("%c", "")
	text = text:gsub("_", "/")
	text = text:gsub("-", "+")
	local mod4 = #text % 4
	text = text .. string.sub('====', mod4 + 1)
	local result = nixio.bin.b64decode(text)
	if result then
		return result:gsub("%z", "")
	else
		return raw
	end
end

function curl_base(url, file, args)
	if not args then args = {} end
	if file then
		args[#args + 1] = "-o " .. file
	end
	local cmd = string.format('curl %s "%s"', table_join(args), url)
	return exec_call(cmd)
end

function curl_proxy(url, file, args)
	--使用代理
	local socks_server = luci.sys.exec("[ -f /tmp/etc/passwall/TCP_SOCKS_server ] && echo -n $(cat /tmp/etc/passwall/TCP_SOCKS_server) || echo -n ''")
	if socks_server ~= "" then
		if not args then args = {} end
		local tmp_args = clone(args)
		tmp_args[#tmp_args + 1] = "-x socks5h://" .. socks_server
		return curl_base(url, file, tmp_args)
	end
	return nil, nil
end

function curl_logic(url, file, args)
	local return_code, result = curl_proxy(url, file, args)
	if not return_code or return_code ~= 0 then
		return_code, result = curl_base(url, file, args)
	end
	return return_code, result
end

function url(...)
	local url = string.format("admin/services/%s", appname)
	local args = { ... }
	for i, v in pairs(args) do
		if v ~= "" then
			url = url .. "/" .. v
		end
	end
	return require "luci.dispatcher".build_url(url)
end

function trim(s)
	return (s:gsub("^%s*(.-)%s*$", "%1"))
end

-- 分割字符串
function split(full, sep)
	if full then
		full = full:gsub("%z", "") -- 这里不是很清楚 有时候结尾带个\0
		local off, result = 1, {}
		while true do
			local nStart, nEnd = full:find(sep, off)
			if not nEnd then
				local res = string.sub(full, off, string.len(full))
				if #res > 0 then -- 过滤掉 \0
					table.insert(result, res)
				end
				break
			else
				table.insert(result, string.sub(full, off, nStart - 1))
				off = nEnd + 1
			end
		end
		return result
	end
	return {}
end

function is_exist(table, value)
	for index, k in ipairs(table) do
		if k == value then
			return true
		end
	end
	return false
end

function repeat_exist(table, value)
	local count = 0
	for index, k in ipairs(table) do
		if k:find("-") and k == value then
			count = count + 1
		end
	end
	if count > 1 then
		return true
	end
	return false
end

function remove(...)
	for index, value in ipairs({...}) do
		if value and #value > 0 and value ~= "/" then
			sys.call(string.format("rm -rf %s", value))
		end
	end
end

function is_install(package)
	if package and #package > 0 then
		return sys.call(string.format('opkg list-installed | grep "%s" > /dev/null 2>&1', package)) == 0
	end
	return false
end

function get_args(arg)
	local var = {}
	for i, arg_k in pairs(arg) do
		if i > 0 then
			local v = arg[i + 1]
			if v then
				if repeat_exist(arg, v) == false then
					var[arg_k] = v
				end
			end
		end
	end
	return var
end

function get_function_args(arg)
	local var = nil
	if arg and #arg > 1 then
		local param = {}
		for i = 2, #arg do
			param[#param + 1] = arg[i]
		end
		var = get_args(param)
	end
	return var
end

function strToTable(str)
	if str == nil or type(str) ~= "string" then
		return {}
	end

	return loadstring("return " .. str)()
end

function is_normal_node(e)
	if e and e.type and e.protocol and (e.protocol == "_balancing" or e.protocol == "_shunt" or e.protocol == "_iface") then
		return false
	end
	return true
end

function is_special_node(e)
	return is_normal_node(e) == false
end

function is_ip(val)
	if is_ipv6(val) then
		val = get_ipv6_only(val)
	end
	return datatypes.ipaddr(val)
end

function is_ipv6(val)
	local str = val
	local address = val:match('%[(.*)%]')
	if address then
		str = address
	end
	if datatypes.ip6addr(str) then
		return true
	end
	return false
end

function is_ipv6addrport(val)
	if is_ipv6(val) then
		local address, port = val:match('%[(.*)%]:([^:]+)$')
		if port then
			return datatypes.port(port)
		end
	end
	return false
end

function get_ipv6_only(val)
	local result = ""
	if is_ipv6(val) then
		result = val
		if val:match('%[(.*)%]') then
			result = val:match('%[(.*)%]')
		end
	end
	return result
end

function get_ipv6_full(val)
	local result = ""
	if is_ipv6(val) then
		result = val
		if not val:match('%[(.*)%]') then
			result = "[" .. result .. "]"
		end
	end
	return result
end

function get_ip_type(val)
	if is_ipv6(val) then
		return "6"
	elseif datatypes.ip4addr(val) then
		return "4"
	end
	return ""
end

function is_mac(val)
	return datatypes.macaddr(val)
end

function ip_or_mac(val)
	if val then
		if get_ip_type(val) == "4" then
			return "ip"
		end
		if is_mac(val) then
			return "mac"
		end
	end
	return ""
end

function iprange(val)
	if val then
		local ipStart, ipEnd = val:match("^([^/]+)-([^/]+)$")
		if (ipStart and datatypes.ip4addr(ipStart)) and (ipEnd and datatypes.ip4addr(ipEnd)) then
			return true
		end
	end
	return false
end

function get_domain_from_url(url)
	local domain = string.match(url, "//([^/]+)")
	if domain then
		return domain
	end
	return url
end

function get_valid_nodes()
	local nodes_ping = uci_get_type("global_other", "nodes_ping") or ""
	local nodes = {}
	uci:foreach(appname, "nodes", function(e)
		e.id = e[".name"]
		if e.type and e.remarks then
			if e.protocol and (e.protocol == "_balancing" or e.protocol == "_shunt" or e.protocol == "_iface") then
				e["remark"] = "%s：[%s] " % {e.type .. " " .. i18n.translatef(e.protocol), e.remarks}
				e["node_type"] = "special"
				nodes[#nodes + 1] = e
			end
			if e.port and e.address then
				local address = e.address
				if is_ip(address) or datatypes.hostname(address) then
					local type = e.type
					if (type == "sing-box" or type == "Xray") and e.protocol then
						local protocol = e.protocol
						if protocol == "vmess" then
							protocol = "VMess"
						elseif protocol == "vless" then
							protocol = "VLESS"
						else
							protocol = protocol:gsub("^%l",string.upper)
						end
						type = type .. " " .. protocol
					end
					if is_ipv6(address) then address = get_ipv6_full(address) end
					e["remark"] = "%s：[%s]" % {type, e.remarks}
					if nodes_ping:find("info") then
						e["remark"] = "%s：[%s] %s:%s" % {type, e.remarks, address, e.port}
					end
					e.node_type = "normal"
					nodes[#nodes + 1] = e
				end
			end
		end
	end)
	return nodes
end

function get_node_remarks(n)
	local remarks = ""
	if n then
		if n.protocol and (n.protocol == "_balancing" or n.protocol == "_shunt" or n.protocol == "_iface") then
			remarks = "%s：[%s] " % {n.type .. " " .. i18n.translatef(n.protocol), n.remarks}
		else
			local type2 = n.type
			if (n.type == "sing-box" or n.type == "Xray") and n.protocol then
				local protocol = n.protocol
				if protocol == "vmess" then
					protocol = "VMess"
				elseif protocol == "vless" then
					protocol = "VLESS"
				else
					protocol = protocol:gsub("^%l",string.upper)
				end
				type2 = type2 .. " " .. protocol
			end
			remarks = "%s：[%s]" % {type2, n.remarks}
		end
	end
	return remarks
end

function get_full_node_remarks(n)
	local remarks = get_node_remarks(n)
	if #remarks > 0 then
		if n.address and n.port then
			remarks = remarks .. " " .. n.address .. ":" .. n.port
		end
	end
	return remarks
end

function gen_uuid(format)
	local uuid = sys.exec("echo -n $(cat /proc/sys/kernel/random/uuid)")
	if format == nil then
		uuid = string.gsub(uuid, "-", "")
	end
	return uuid
end

function gen_short_uuid()
	return sys.exec("echo -n $(head /dev/urandom | tr -dc A-Za-z0-9 | head -c 8)")
end

function uci_get_type(type, config, default)
	local value = uci:get_first(appname, type, config, default) or sys.exec("echo -n $(uci -q get " .. appname .. ".@" .. type .."[0]." .. config .. ")")
	if (value == nil or value == "") and (default and default ~= "") then
		value = default
	end
	return value
end

function uci_get_type_id(id, config, default)
	local value = uci:get(appname, id, config, default) or sys.exec("echo -n $(uci -q get " .. appname .. "." .. id .. "." .. config .. ")")
	if (value == nil or value == "") and (default and default ~= "") then
		value = default
	end
	return value
end

local function chmod_755(file)
	if file and file ~= "" then
		if not fs.access(file, "rwx", "rx", "rx") then
			fs.chmod(file, 755)
		end
	end
end

function get_customed_path(e)
	return uci_get_type("global_app", e .. "_file")
end

function finded_com(e)
	local bin = get_app_path(e)
	if not bin then return end
	local s = luci.sys.exec('echo -n $(type -t -p "%s" | head -n1)' % { bin })
	if s == "" then
		s = nil
	end
	return s
end

function finded(e)
	return luci.sys.exec('echo -n $(type -t -p "/bin/%s" -p "/usr/bin/%s" "%s" | head -n1)' % {e, e, e})
end

function is_finded(e)
	return finded(e) ~= "" and true or false
end

function clone(org)
	local function copy(org, res)
		for k,v in pairs(org) do
			if type(v) ~= "table" then
				res[k] = v;
			else
				res[k] = {};
				copy(v, res[k])
			end
		end
	end

	local res = {}
	copy(org, res)
	return res
end

local function get_bin_version_cache(file, cmd)
	sys.call("mkdir -p /tmp/etc/passwall_tmp")
	if fs.access(file) then
		chmod_755(file)
		local md5 = sys.exec("echo -n $(md5sum " .. file .. " | awk '{print $1}')")
		if fs.access("/tmp/etc/passwall_tmp/" .. md5) then
			return sys.exec("echo -n $(cat /tmp/etc/passwall_tmp/%s)" % md5)
		else
			local version = sys.exec(string.format("echo -n $(%s %s)", file, cmd))
			if version and version ~= "" then
				sys.call("echo '" .. version .. "' > " .. "/tmp/etc/passwall_tmp/" .. md5)
				return version
			end
		end
	end
	return ""
end

function get_app_path(app_name)
	if com[app_name] then
		local def_path = com[app_name].default_path
		local path = uci_get_type("global_app", app_name:gsub("%-","_") .. "_file")
		path = path and (#path>0 and path or def_path) or def_path
		return path
	end
end

function get_app_version(app_name, file)
	if file == nil then file = get_app_path(app_name) end
	return get_bin_version_cache(file, com[app_name].cmd_version)
end

local function is_file(path)
	if path and #path > 1 then
		if sys.exec('[ -f "%s" ] && echo -n 1' % path) == "1" then
			return true
		end
	end
	return nil
end

local function is_dir(path)
	if path and #path > 1 then
		if sys.exec('[ -d "%s" ] && echo -n 1' % path) == "1" then
			return true
		end
	end
	return nil
end

local function get_final_dir(path)
	if is_dir(path) then
		return path
	else
		return get_final_dir(fs.dirname(path))
	end
end

local function get_free_space(dir)
	if dir == nil then dir = "/" end
	if sys.call("df -k " .. dir .. " >/dev/null 2>&1") == 0 then
		return tonumber(sys.exec("echo -n $(df -k " .. dir .. " | awk 'NR>1' | awk '{print $4}')"))
	end
	return 0
end

local function get_file_space(file)
	if file == nil then return 0 end
	if fs.access(file) then
		return tonumber(sys.exec("echo -n $(du -k " .. file .. " | awk '{print $1}')"))
	end
	return 0
end

function _unpack(t, i)
	i = i or 1
	if t[i] ~= nil then return t[i], _unpack(t, i + 1) end
end

function table_join(t, s)
	if not s then
		s = " "
	end
	local str = ""
	for index, value in ipairs(t) do
		str = str .. t[index] .. (index == #t and "" or s)
	end
	return str
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

				if wpid and stat == "exited" then return code end

				if not writer and timeout then nixio.nanosleep(1) end
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

function compare_versions(ver1, comp, ver2)
	local table = table

	if not ver1 then ver1 = "" end
	if not ver2 then ver2 = "" end

	local av1 = util.split(ver1, "[%.%-]", nil, true)
	local av2 = util.split(ver2, "[%.%-]", nil, true)

	local max = table.getn(av1)
	local n2 = table.getn(av2)
	if (max < n2) then max = n2 end

	for i = 1, max, 1 do
		local s1 = tonumber(av1[i] or 0) or 0
		local s2 = tonumber(av2[i] or 0) or 0

		if comp == "~=" and (s1 ~= s2) then return true end
		if (comp == "<" or comp == "<=") and (s1 < s2) then return true end
		if (comp == ">" or comp == ">=") and (s1 > s2) then return true end
		if (s1 ~= s2) then return false end
	end

	return not (comp == "<" or comp == ">")
end

local function auto_get_arch()
	local arch = nixio.uname().machine or ""
	if not OPENWRT_ARCH and fs.access("/usr/lib/os-release") then
		OPENWRT_ARCH = sys.exec("echo -n $(grep 'OPENWRT_ARCH' /usr/lib/os-release | awk -F '[\\042\\047]' '{print $2}')")
		if OPENWRT_ARCH == "" then OPENWRT_ARCH = nil end
	end
	if not DISTRIB_ARCH and fs.access("/etc/openwrt_release") then
		DISTRIB_ARCH = sys.exec("echo -n $(grep 'DISTRIB_ARCH' /etc/openwrt_release | awk -F '[\\042\\047]' '{print $2}')")
		if DISTRIB_ARCH == "" then DISTRIB_ARCH = nil end
	end

	if arch:match("^i[%d]86$") then
		arch = "x86"
	elseif arch:match("armv5") then  -- armv5l
		arch = "armv5"
	elseif arch:match("armv6") then
		arch = "armv6"
	elseif arch:match("armv7") then  -- armv7l
		arch = "armv7"
	end

	if OPENWRT_ARCH or DISTRIB_ARCH then
		if arch == "mips" then
			if OPENWRT_ARCH and OPENWRT_ARCH:match("mipsel") == "mipsel"
			or DISTRIB_ARCH and DISTRIB_ARCH:match("mipsel") == "mipsel" then
				arch = "mipsel"
			end
		elseif arch == "armv7" then
			if OPENWRT_ARCH and not OPENWRT_ARCH:match("vfp") and not OPENWRT_ARCH:match("neon")
			or DISTRIB_ARCH and not DISTRIB_ARCH:match("vfp") and not DISTRIB_ARCH:match("neon") then
				arch = "armv5"
			end
		end
	end

	return util.trim(arch)
end

function parseURL(url)
	if not url or url == "" then
		return nil
	end
	local pattern = "^(%w+)://"
	local protocol = url:match(pattern)

	if not protocol then
		--error("Invalid URL: " .. url)
		return nil
	end

	local auth_host_port = url:sub(#protocol + 4)
	local auth_pattern = "^([^@]+)@"
	local auth = auth_host_port:match(auth_pattern)
	local username, password

	if auth then
		username, password = auth:match("^([^:]+):([^:]+)$")
		auth_host_port = auth_host_port:sub(#auth + 2)
	end

	local host, port = auth_host_port:match("^([^:]+):(%d+)$")

	if not host or not port then
		--error("Invalid URL: " .. url)
		return nil
	end

	return {
		protocol = protocol,
		username = username,
		password = password,
		host = host,
		port = tonumber(port)
	}
end

local default_file_tree = {
	x86_64  = "amd64",
	x86     = "386",
	aarch64 = "arm64",
	mips    = "mips",
	mipsel  = "mipsle",
	armv5   = "arm.*5",
	armv6   = "arm.*6[^4]*",
	armv7   = "arm.*7",
	armv8   = "arm64"
}

local function get_api_json(url)
	local jsonc = require "luci.jsonc"
	local return_code, content = curl_logic(url, nil, curl_args)
	if return_code ~= 0 or content == "" then return {} end
	return jsonc.parse(content) or {}
end

local function check_path(app_name)
	local path = get_app_path(app_name) or ""
	if path == "" then
		return {
			code = 1,
			error = i18n.translatef("You did not fill in the %s path. Please save and apply then update manually.", app_name)
		}
	end
	return {
		code = 0,
		app_path = path
	}
end

function to_check(arch, app_name)
	local result = check_path(app_name)
	if result.code ~= 0 then
		return result
	end

	if not arch or arch == "" then arch = auto_get_arch() end

	local file_tree = com[app_name].file_tree[arch] or default_file_tree[arch] or ""

	if file_tree == "" then
		return {
			code = 1,
			error = i18n.translate("Can't determine ARCH, or ARCH not supported.")
		}
	end

	local local_version = get_app_version(app_name)
	local match_file_name = string.format(com[app_name].match_fmt_str, file_tree)
	local json = get_api_json(com[app_name]:get_url())

	if #json > 0 then
		json = json[1]
	end

	if json.tag_name == nil then
		return {
			code = 1,
			error = i18n.translate("Get remote version info failed.")
		}
	end

	local remote_version = json.tag_name
	if com[app_name].remote_version_str_replace then
		remote_version = remote_version:gsub(com[app_name].remote_version_str_replace, "")
	end
	local has_update = compare_versions(local_version:match("[^v]+"), "<", remote_version:match("[^v]+"))

	if not has_update then
		return {
			code = 0,
			local_version = local_version,
			remote_version = remote_version
		}
	end

	local asset = {}
	for _, v in ipairs(json.assets) do
		if v.name and v.name:match(match_file_name) then
			asset = v
			break
		end
	end
	if not asset.browser_download_url then
		return {
			code = 1,
			local_version = local_version,
			remote_version = remote_version,
			html_url = json.html_url,
			data = asset,
			error = i18n.translate("New version found, but failed to get new version download url.")
		}
	end

	return {
		code = 0,
		has_update = true,
		local_version = local_version,
		remote_version = remote_version,
		html_url = json.html_url,
		data = asset
	}
end

function to_download(app_name, url, size)
	local result = check_path(app_name)
	if result.code ~= 0 then
		return result
	end

	if not url or url == "" then
		return {code = 1, error = i18n.translate("Download url is required.")}
	end

	sys.call("/bin/rm -f /tmp/".. app_name .."_download.*")

	local tmp_file = util.trim(util.exec("mktemp -u -t ".. app_name .."_download.XXXXXX"))

	if size then
		local kb1 = get_free_space("/tmp")
		if tonumber(size) > tonumber(kb1) then
			return {code = 1, error = i18n.translatef("%s not enough space.", "/tmp")}
		end
	end

	local return_code, result = curl_logic(url, tmp_file, curl_args)
	result = return_code == 0

	if not result then
		exec("/bin/rm", {"-f", tmp_file})
		return {
			code = 1,
			error = i18n.translatef("File download failed or timed out: %s", url)
		}
	end

	return {code = 0, file = tmp_file, zip = com[app_name].zipped }
end

function to_extract(app_name, file, subfix)
	local result = check_path(app_name)
	if result.code ~= 0 then
		return result
	end

	if not file or file == "" or not fs.access(file) then
		return {code = 1, error = i18n.translate("File path required.")}
	end

	local tools_name
	if com[app_name].zipped then
		if not com[app_name].zipped_suffix or com[app_name].zipped_suffix == "zip" then
			tools_name = "unzip"
		end
		if com[app_name].zipped_suffix and com[app_name].zipped_suffix == "tar.gz" then
			tools_name = "tar"
		end
		if tools_name then
			if sys.exec("echo -n $(command -v %s)" % { tools_name }) == "" then
				exec("/bin/rm", {"-f", file})
				return {
					code = 1,
					error = i18n.translate("Not installed %s, Can't unzip!" % { tools_name })
				}
			end
		end
	end

	sys.call("/bin/rm -rf /tmp/".. app_name .."_extract.*")

	local new_file_size = get_file_space(file)
	local tmp_free_size = get_free_space("/tmp")
	if tmp_free_size <= 0 or tmp_free_size <= new_file_size then
		return {code = 1, error = i18n.translatef("%s not enough space.", "/tmp")}
	end

	local tmp_dir = util.trim(util.exec("mktemp -d -t ".. app_name .."_extract.XXXXXX"))

	local output = {}

	if tools_name then
		if tools_name == "unzip" then
			local bin = sys.exec("echo -n $(command -v unzip)")
			exec(bin, {"-o", file, app_name, "-d", tmp_dir}, function(chunk) output[#output + 1] = chunk end)
		elseif tools_name == "tar" then
			local bin = sys.exec("echo -n $(command -v tar)")
			if com[app_name].zipped_suffix == "tar.gz" then
				exec(bin, {"-zxf", file, "-C", tmp_dir}, function(chunk) output[#output + 1] = chunk end)
				sys.call("/bin/mv -f " .. tmp_dir .. "/*/" .. com[app_name].name:lower() .. " " .. tmp_dir)
			end
		end
	end

	local files = util.split(table.concat(output))

	exec("/bin/rm", {"-f", file})

	return {code = 0, file = tmp_dir}
end

function to_move(app_name,file)
	local result = check_path(app_name)
	if result.code ~= 0 then
		return result
	end

	local app_path = result.app_path
	local bin_path = file
	local cmd_rm_tmp = "/bin/rm -rf /tmp/" .. app_name .. "_download.*"
	if fs.stat(file, "type") == "dir" then
		bin_path = file .. "/" .. com[app_name].name:lower()
		cmd_rm_tmp = "/bin/rm -rf /tmp/" .. app_name .. "_extract.*"
	end

	if not file or file == "" then
		sys.call(cmd_rm_tmp)
		return {code = 1, error = i18n.translate("Client file is required.")}
	end

	local new_version = get_app_version(app_name, bin_path)
	if new_version == "" then
		sys.call(cmd_rm_tmp)
		return {
			code = 1,
			error = i18n.translate("The client file is not suitable for current device.")..app_name.."__"..bin_path
		}
	end

	local flag = sys.call('pgrep -af "passwall/.*'.. app_name ..'" >/dev/null')
	if flag == 0 then
		sys.call("/etc/init.d/passwall stop")
	end

	local old_app_size = 0
	if fs.access(app_path) then
		old_app_size = get_file_space(app_path)
	end
	local new_app_size = get_file_space(bin_path)
	local final_dir = get_final_dir(app_path)
	local final_dir_free_size = get_free_space(final_dir)
	if final_dir_free_size > 0 then
		final_dir_free_size = final_dir_free_size + old_app_size
		if new_app_size > final_dir_free_size then
			sys.call(cmd_rm_tmp)
			return {code = 1, error = i18n.translatef("%s not enough space.", final_dir)}
		end
	end

	result = exec("/bin/mv", { "-f", bin_path, app_path }, nil, command_timeout) == 0

	sys.call(cmd_rm_tmp)
	if flag == 0 then
		sys.call("/etc/init.d/passwall restart >/dev/null 2>&1 &")
	end

	if not result or not fs.access(app_path) then
		return {
			code = 1,
			error = i18n.translatef("Can't move new file to path: %s", app_path)
		}
	end

	return {code = 0}
end

function get_version()
	return sys.exec("echo -n $(opkg list-installed luci-app-passwall |awk '{print $3}')")
end

function to_check_self()
	local url = "https://raw.githubusercontent.com/xiaorouji/openwrt-passwall/main/luci-app-passwall/Makefile"
	local tmp_file = "/tmp/passwall_makefile"
	local return_code, result = curl_logic(url, tmp_file, curl_args)
	result = return_code == 0
	if not result then
		exec("/bin/rm", {"-f", tmp_file})
		return {
			code = 1,
			error = i18n.translatef("Failed")
		}
	end
	local local_version  = get_version()
	local remote_version = sys.exec("echo -n $(grep 'PKG_VERSION' /tmp/passwall_makefile|awk -F '=' '{print $2}')")

	local has_update = compare_versions(local_version, "<", remote_version)
	if not has_update then
		return {
			code = 0,
			local_version = local_version,
			remote_version = remote_version
		}
	end
	return {
		code = 1,
		has_update = true,
		local_version = local_version,
		remote_version = remote_version,
		error = i18n.translatef("The latest version: %s, currently does not support automatic update, if you need to update, please compile or download the ipk and then manually install.", remote_version)
	}
end

function is_js_luci()
	return sys.call('[ -f "/www/luci-static/resources/uci.js" ]') == 0
end

function set_apply_on_parse(map)
	if is_js_luci() == true then
		map.apply_on_parse = false
		map.on_after_apply = function(self)
			if self.redirect then
				os.execute("sleep 1")
				luci.http.redirect(self.redirect)
			end
		end
	end
end

function luci_types(id, m, s, type_name, option_prefix)
	local rewrite_option_table = {}
	for key, value in pairs(s.fields) do
		if key:find(option_prefix) == 1 then
			if not s.fields[key].not_rewrite then
				if s.fields[key].rewrite_option then
					if not rewrite_option_table[s.fields[key].rewrite_option] then
						rewrite_option_table[s.fields[key].rewrite_option] = 1
					else
						rewrite_option_table[s.fields[key].rewrite_option] = rewrite_option_table[s.fields[key].rewrite_option] + 1
					end
				end

				s.fields[key].cfgvalue = function(self, section)
					if self.rewrite_option then
						return m:get(section, self.rewrite_option)
					else
						if self.option:find(option_prefix) == 1 then
							return m:get(section, self.option:sub(1 + #option_prefix))
						end
					end
				end
				s.fields[key].write = function(self, section, value)
					if s.fields["type"]:formvalue(id) == type_name then
						if self.rewrite_option then
							m:set(section, self.rewrite_option, value)
						else
							if self.option:find(option_prefix) == 1 then
								m:set(section, self.option:sub(1 + #option_prefix), value)
							end
						end
					end
				end
				s.fields[key].remove = function(self, section)
					if s.fields["type"]:formvalue(id) == type_name then
						if self.rewrite_option and rewrite_option_table[self.rewrite_option] == 1 then
							m:del(section, self.rewrite_option)
						else
							if self.option:find(option_prefix) == 1 then
								m:del(section, self.option:sub(1 + #option_prefix))
							end
						end
					end
				end
			end

			local deps = s.fields[key].deps
			if #deps > 0 then
				for index, value in ipairs(deps) do
					deps[index]["type"] = type_name
				end
			else
				s.fields[key]:depends({ type = type_name })
			end
		end
	end
end
