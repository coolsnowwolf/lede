local fs = require "nixio.fs"
local sys = require "luci.sys"
local uci = require"luci.model.uci".cursor()
local util = require "luci.util"
local i18n = require "luci.i18n"

module("luci.model.cbi.kodexplorer.api", package.seeall)

local appname = "kodexplorer"
local api_url = "https://api.kodcloud.com/?app/version"

local wget = "/usr/bin/wget"
local wget_args = { "--no-check-certificate", "--quiet", "--timeout=10", "--tries=2" }
local command_timeout = 300

local function _unpack(t, i)
    i = i or 1
    if t[i] ~= nil then return t[i], _unpack(t, i + 1) end
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

function get_project_directory()
    return uci:get(appname, "@global[0]", "project_directory") or "/tmp/kodcloud"
end

function get_version()
    local version = get_project_directory() .. "/config/version.php"
    return sys.exec(string.format("echo -n $(cat %s 2>/dev/null | grep \"'KOD_VERSION'\" | cut -d \"'\" -f 4)", version))
end

local function compare_versions(ver1, comp, ver2)
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

local function get_api_json(url)
    local jsonc = require "luci.jsonc"

    local output = {}
    -- exec(wget, { "-O-", url, _unpack(wget_args) },
    --	function(chunk) output[#output + 1] = chunk end)
    -- local json_content = util.trim(table.concat(output))

    local json_content = sys.exec(wget .. " --no-check-certificate --timeout=10 -t 1 -O- " .. url)

    if json_content == "" then return {} end

    return jsonc.parse(json_content) or {}
end

function to_check()
    local json = get_api_json(api_url)
    return json
end

function to_download(url)
    if not url or url == "" then
        return {code = 1, error = i18n.translate("Download url is required.")}
    end

    sys.call("/bin/rm -f /tmp/kodcloud_download.*")

    local tmp_file = util.trim(util.exec("mktemp -u -t kodcloud_download.XXXXXX"))

    local result = exec(wget, {"-O", tmp_file, url, _unpack(wget_args)}, nil, command_timeout) == 0

    if not result then
        exec("/bin/rm", {"-f", tmp_file})
        return {
            code = 1,
            error = i18n.translatef("File download failed or timed out: %s", url)
        }
    end

    return {code = 0, file = tmp_file}
end

function to_extract(file)
    if not file or file == "" or not fs.access(file) then
        return {code = 1, error = i18n.translate("File path required.")}
    end

    sys.call("/bin/rm -rf /tmp/kodcloud_extract.*")
    local tmp_dir = util.trim(util.exec("mktemp -d -t kodcloud_extract.XXXXXX"))

    local output = {}
    exec("/usr/bin/unzip", {"-o", file, "-d", tmp_dir}, function(chunk) output[#output + 1] = chunk end)

    local files = util.split(table.concat(output))

    exec("/bin/rm", {"-f", file})

    return {code = 0, file = tmp_dir}
end

function to_move(file)
    if not file or file == "" or not fs.access(file) then
        sys.call("/bin/rm -rf /tmp/kodcloud_extract.*")
        return {code = 1, error = i18n.translate("Client file is required.")}
    end

    local client_file = get_project_directory()
    sys.call("mkdir -p " .. client_file)
    sys.call("cp -R " .. file .. "/* " .. client_file)
    sys.call("/bin/rm -rf /tmp/kodcloud_extract.*")

    return {code = 0}
end
