module("luci.model.cbi.passwall.api.xray", package.seeall)
local api = require "luci.model.cbi.passwall.api.api"
local fs = api.fs
local sys = api.sys
local util = api.util
local i18n = api.i18n

local xray_api = "https://api.github.com/repos/XTLS/Xray-core/releases?per_page=1"
local is_armv7 = false
local app_path = api.get_xray_path() or ""

function check_path()
    if app_path == "" then
        return {
            code = 1,
            error = i18n.translatef("You did not fill in the %s path. Please save and apply then update manually.", "Xray")
        }
    end
    return {
        code = 0
    }
end

function to_check(arch)
    local result = check_path()
    if result.code ~= 0 then
        return result
    end

    if not arch or arch == "" then arch = api.auto_get_arch() end

    local file_tree, sub_version = api.get_file_info(arch)

    if file_tree == "" then
        return {
            code = 1,
            error = i18n.translate("Can't determine ARCH, or ARCH not supported.")
        }
    end

    if file_tree == "amd64" then file_tree = "64" end
    if file_tree == "386" then file_tree = "32" end
    if file_tree == "mipsle" then file_tree = "mips32le" end
    if file_tree == "mips" then file_tree = "mips32" end
    if file_tree == "arm" then file_tree = "arm32" end

    local json = api.get_api_json(xray_api)

    if #json > 0 then
        json = json[1]
    end

    if json.tag_name == nil then
        return {
            code = 1,
            error = i18n.translate("Get remote version info failed.")
        }
    end

    local now_version = api.get_xray_version()
    local remote_version = json.tag_name
    local needs_update = api.compare_versions(now_version:match("[^v]+"), "<", remote_version:match("[^v]+"))
    local html_url, download_url

    if needs_update then
        html_url = json.html_url
        for _, v in ipairs(json.assets) do
            if v.name and v.name:match("linux%-" .. file_tree .. (sub_version ~= "" and ".+" .. sub_version or "")) then
                download_url = v.browser_download_url
                break
            end
        end
    end

    if needs_update and not download_url then
        return {
            code = 1,
            now_version = now_version,
            version = remote_version,
            html_url = html_url,
            error = i18n.translate("New version found, but failed to get new version download url.")
        }
    end

    return {
        code = 0,
        update = needs_update,
        now_version = now_version,
        version = remote_version,
        url = {html = html_url, download = download_url}
    }
end

function to_download(url)
    local result = check_path()
    if result.code ~= 0 then
        return result
    end

    if not url or url == "" then
        return {code = 1, error = i18n.translate("Download url is required.")}
    end

    sys.call("/bin/rm -f /tmp/xray_download.*")

    local tmp_file = util.trim(util.exec("mktemp -u -t xray_download.XXXXXX"))

    result = api.exec(api.curl, {api._unpack(api.curl_args), "-o", tmp_file, url}, nil, api.command_timeout) == 0

    if not result then
        api.exec("/bin/rm", {"-f", tmp_file})
        return {
            code = 1,
            error = i18n.translatef("File download failed or timed out: %s", url)
        }
    end

    return {code = 0, file = tmp_file}
end

function to_extract(file, subfix)
    local result = check_path()
    if result.code ~= 0 then
        return result
    end

    if not file or file == "" or not fs.access(file) then
        return {code = 1, error = i18n.translate("File path required.")}
    end

    if sys.exec("echo -n $(opkg list-installed | grep -c unzip)") ~= "1" then
        api.exec("/bin/rm", {"-f", file})
        return {
            code = 1,
            error = i18n.translate("Not installed unzip, Can't unzip!")
        }
    end

    sys.call("/bin/rm -rf /tmp/xray_extract.*")
    local tmp_dir = util.trim(util.exec("mktemp -d -t xray_extract.XXXXXX"))

    local output = {}
    api.exec("/usr/bin/unzip", {"-o", file, "xray", "-d", tmp_dir},
             function(chunk) output[#output + 1] = chunk end)

    local files = util.split(table.concat(output))

    api.exec("/bin/rm", {"-f", file})

    return {code = 0, file = tmp_dir}
end

function to_move(file)
    local result = check_path()
    if result.code ~= 0 then
        return result
    end

    if not file or file == "" then
        sys.call("/bin/rm -rf /tmp/xray_extract.*")
        return {code = 1, error = i18n.translate("Client file is required.")}
    end

    local flag = sys.call('pgrep -af "passwall/.*xray" >/dev/null')
    if flag == 0 then
        sys.call("/etc/init.d/passwall stop")
    end
    result = api.exec("/bin/mv", { "-f", file .. "/xray", app_path }, nil, api.command_timeout) == 0
    sys.call("/bin/rm -rf /tmp/xray_extract.*")
    if not result or not fs.access(app_path) then
        if flag == 0 then
            sys.call("/etc/init.d/passwall restart >/dev/null 2>&1 &")
        end
        if #app_path > 1 then
            sys.call("/bin/rm -rf " .. app_path)
        end
        return {
            code = 1,
            error = i18n.translatef("Can't move new file to path: %s", app_path)
        }
    end
    
    api.chmod_755(app_path)
    if flag == 0 then
        sys.call("/etc/init.d/passwall restart >/dev/null 2>&1 &")
    end

    return {code = 0}
end
