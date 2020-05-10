module("luci.model.cbi.passwall.api.api", package.seeall)
local fs = require "nixio.fs"
local sys = require "luci.sys"
local uci = require"luci.model.uci".cursor()
local util = require "luci.util"
local i18n = require "luci.i18n"

appname = "passwall"
wget = "/usr/bin/wget"
wget_args = {"--no-check-certificate", "--quiet", "--timeout=100", "--tries=3"}
command_timeout = 300
LEDE_BOARD = nil
DISTRIB_TARGET = nil

function uci_get_type(type, config, default)
    local value = uci:get_first(appname, type, config, default) or sys.exec(
                      "echo -n `uci -q get " .. appname .. ".@" .. type ..
                          "[0]." .. config .. "`")
    if (value == nil or value == "") and (default and default ~= "") then
        value = default
    end
    return value
end

function uci_get_type_id(id, config, default)
    local value = uci:get(appname, id, config, default) or
                      sys.exec("echo -n `uci -q get " .. appname .. "." .. id ..
                                   "." .. config .. "`")
    if (value == nil or value == "") and (default and default ~= "") then
        value = default
    end
    return value
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

    local av1 = util.split(ver1, "[%.%-]", nil, true)
    local av2 = util.split(ver2, "[%.%-]", nil, true)

    local max = table.getn(av1)
    local n2 = table.getn(av2)
    if (max < n2) then max = n2 end

    for i = 1, max, 1 do
        local s1 = av1[i] or ""
        local s2 = av2[i] or ""

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
        LEDE_BOARD = sys.exec(
                         "echo -n `grep 'LEDE_BOARD' /usr/lib/os-release | awk -F '[\\042\\047]' '{print $2}'`")
    end
    if fs.access("/etc/openwrt_release") then
        DISTRIB_TARGET = sys.exec(
                             "echo -n `grep 'DISTRIB_TARGET' /etc/openwrt_release | awk -F '[\\042\\047]' '{print $2}'`")
    end

    if arch == "mips" then
        if LEDE_BOARD and LEDE_BOARD ~= "" then
            if string.match(LEDE_BOARD, "ramips") == "ramips" then
                arch = "ramips"
            else
                arch = sys.exec("echo '" .. LEDE_BOARD ..
                                    "' | grep -oE 'ramips|ar71xx'")
            end
        elseif DISTRIB_TARGET and DISTRIB_TARGET ~= "" then
            if string.match(DISTRIB_TARGET, "ramips") == "ramips" then
                arch = "ramips"
            else
                arch = sys.exec("echo '" .. DISTRIB_TARGET ..
                                    "' | grep -oE 'ramips|ar71xx'")
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

    local output = {}
    -- exec(wget, { "-O-", url, _unpack(wget_args) },
    --	function(chunk) output[#output + 1] = chunk end)
    -- local json_content = util.trim(table.concat(output))

    local json_content = luci.sys.exec(wget ..
                                           " --no-check-certificate --timeout=10 -t 1 -O- " ..
                                           url)

    if json_content == "" then return {} end

    return jsonc.parse(json_content) or {}
end
