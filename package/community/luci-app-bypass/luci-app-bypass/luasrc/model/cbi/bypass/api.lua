module("luci.model.cbi.bypass.api", package.seeall)
local fs = require "nixio.fs"
local sys = require "luci.sys"
local uci = require"luci.model.uci".cursor()
local util = require "luci.util"
local datatypes = require "luci.cbi.datatypes"
local i18n = require "luci.i18n"

appname = "bypass"
curl = "/usr/bin/curl"
curl_args = {"-skL", "--connect-timeout 3", "--retry 3", "-m 60"}
wget = "/usr/bin/wget"
wget_args = {"--no-check-certificate", "--quiet", "--timeout=100", "--tries=3"}
command_timeout = 300
LEDE_BOARD = nil
DISTRIB_TARGET = nil

function is_exist(table, value)
    for index, k in ipairs(table) do
        if k == value then
            return true
        end
    end
    return false
end

function get_args(arg, myarg)
    local var = {}
    for i, arg_k in pairs(arg) do
        if i > 0 then
            if is_exist(myarg, arg_k) == true then
                local v = arg[i + 1]
                if v then
                    if is_exist(myarg, v) == false then
                        var[arg_k] = v
                    end
                end
            end
        end
    end
    return var
end

function chmod_755(file)
    if file and file ~= "" then
        if not fs.access(file, "rwx", "rx", "rx") then
            fs.chmod(file, 755)
        end
    end
end

function is_finded(e)
    return luci.sys.exec('which "%s"' % e) ~= "" and true or false
end

function get_xray_path()
    local path = "/usr/bin/xray"
    return path
end

function get_xray_version(file)
    if file == nil then file = get_xray_path() end
    chmod_755(file)
    if fs.access(file) then
        if file == get_xray_path() then
            local md5 = sys.exec("echo -n $(md5sum " .. file .. " | awk '{print $1}')")
            if fs.access("/tmp/psw_" .. md5) then
                return sys.exec("echo -n $(cat /tmp/psw_%s)" % md5)
            else
                local version = sys.exec("echo -n $(%s -version | awk '{print $2}' | sed -n 1P)" % file)
                sys.call("echo '" .. version .. "' > " .. "/tmp/psw_" .. md5)
                return version
            end
        else
            return sys.exec("echo -n $(%s -version | awk '{print $2}' | sed -n 1P)" % file)
        end
    end
    return ""
end

function get_trojan_go_path()
    local path = "/usr/bin/trojan-go"
    return path
end

function get_trojan_go_version(file)
    if file == nil then file = get_trojan_go_path() end
    chmod_755(file)
    if fs.access(file) then
        if file == get_trojan_go_path() then
            local md5 = sys.exec("echo -n $(md5sum " .. file .. " | awk '{print $1}')")
            if fs.access("/tmp/psw_" .. md5) then
                return sys.exec("echo -n $(cat /tmp/psw_%s)" % md5)
            else
                local version = sys.exec("echo -n $(%s -version | awk '{print $2}' | sed -n 1P)" % file)
                sys.call("echo '" .. version .. "' > " .. "/tmp/psw_" .. md5)
                return version
            end
        else
            return sys.exec("echo -n $(%s -version | awk '{print $2}' | sed -n 1P)" % file)
        end
    end
    return ""
end

function get_free_space(dir)
    if dir == nil then dir = "/" end
    if sys.call("df -k " .. dir .. " >/dev/null") == 0 then
        return tonumber(sys.exec("echo -n $(df -k " .. dir .. " | awk 'NR>1' | awk '{print $4}')"))
    end
    return 0
end

function get_file_space(file)
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

function exec(cmd, args, writer, timeout)
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

function auto_get_arch()
    local arch = nixio.uname().machine or ""
    if fs.access("/usr/lib/os-release") then
        LEDE_BOARD = sys.exec("echo -n $(grep 'LEDE_BOARD' /usr/lib/os-release | awk -F '[\\042\\047]' '{print $2}')")
    end
    if fs.access("/etc/openwrt_release") then
        DISTRIB_TARGET = sys.exec("echo -n $(grep 'DISTRIB_TARGET' /etc/openwrt_release | awk -F '[\\042\\047]' '{print $2}')")
    end

    if arch == "mips" then
        if LEDE_BOARD and LEDE_BOARD ~= "" then
            if string.match(LEDE_BOARD, "ramips") == "ramips" then
                arch = "ramips"
            else
                arch = sys.exec("echo '" .. LEDE_BOARD .. "' | grep -oE 'ramips|ar71xx'")
            end
        elseif DISTRIB_TARGET and DISTRIB_TARGET ~= "" then
            if string.match(DISTRIB_TARGET, "ramips") == "ramips" then
                arch = "ramips"
            else
                arch = sys.exec("echo '" .. DISTRIB_TARGET .. "' | grep -oE 'ramips|ar71xx'")
            end
        end
    end

    return util.trim(arch)
end

function get_file_info(arch)
    local file_tree = ""
    local sub_version = ""

    if arch == "x86_64" then
        file_tree = "amd64"
    elseif arch == "aarch64" then
        file_tree = "arm64"
    elseif arch == "ramips" then
        file_tree = "mipsle"
    elseif arch == "ar71xx" then
        file_tree = "mips"
    elseif arch:match("^i[%d]86$") then
        file_tree = "386"
    elseif arch:match("^armv[5-8]") then
        file_tree = "arm"
        sub_version = arch:match("[5-8]")
        if LEDE_BOARD and string.match(LEDE_BOARD, "bcm53xx") == "bcm53xx" then
            sub_version = "5"
        elseif DISTRIB_TARGET and string.match(DISTRIB_TARGET, "bcm53xx") ==
            "bcm53xx" then
            sub_version = "5"
        end
        sub_version = "5"
    end

    return file_tree, sub_version
end

function get_api_json(url)
    local jsonc = require "luci.jsonc"
    local json_content = luci.sys.exec(curl .. " " .. _unpack(curl_args) .. " " .. url)
    if json_content == "" then return {} end
    return jsonc.parse(json_content) or {}
end
