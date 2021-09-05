module("luci.model.cbi.bypass.trojan_go", package.seeall)
local fs = require "nixio.fs"
local sys = require "luci.sys"
local util = require "luci.util"
local i18n = require "luci.i18n"
local api = require "luci.model.cbi.bypass.api"

local trojan_go_api = "https://api.github.com/repos/p4gefau1t/trojan-go/releases?per_page=1"

function to_check(arch)
    local app_path = api.get_trojan_go_path() or ""
    if app_path == "" then
        return {
            code = 1,
            error = i18n.translatef("You did not fill in the %s path. Please save and apply then update manually.", "Trojan-GO")
        }
    end
    if not arch or arch == "" then arch = api.auto_get_arch() end

    local file_tree, sub_version = api.get_file_info(arch)

    if file_tree == "" then
        return {
            code = 1,
            error = i18n.translate("Can't determine ARCH, or ARCH not supported.")
        }
    end

	if file_tree == "mips" then file_tree = "mips%-hardfloat" end
    if file_tree == "mipsle" then file_tree = "mipsle%-hardfloat" end
    if file_tree == "arm64" then
        file_tree = "armv8"
    else
        if sub_version and sub_version:match("^[5-8]$") then file_tree = file_tree .. "v" .. sub_version end
    end

    local json = api.get_api_json(trojan_go_api)

    if #json > 0 then
        json = json[1]
    end

    if json.tag_name == nil then
        return {
            code = 1,
            error = i18n.translate("Get remote version info failed.")
        }
    end

    local now_version = api.get_trojan_go_version()
    local remote_version = json.tag_name
    local needs_update = api.compare_versions(now_version:match("[^v]+"), "<", remote_version:match("[^v]+"))
    local html_url, download_url

    if needs_update then
        html_url = json.html_url
        for _, v in ipairs(json.assets) do
            if v.name and v.name:match("linux%-" .. file_tree .. "%.zip") then
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
            error = i18n.translate("New version found, but failed to get new version download url.") .. " [linux-" .. file_tree .. ".zip]"
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
    local app_path = api.get_trojan_go_path() or ""
    if app_path == "" then
        return {
            code = 1,
            error = i18n.translatef("You did not fill in the %s path. Please save and apply then update manually.", "Trojan-GO")
        }
    end
    if not url or url == "" then
        return {code = 1, error = i18n.translate("Download url is required.")}
    end

    sys.call("/bin/rm -f /tmp/trojan-go_download.*")

    local tmp_file = util.trim(util.exec("mktemp -u -t trojan-go_download.XXXXXX"))

    local result = api.exec(api.curl, {api._unpack(api.curl_args), "-o", tmp_file, url}, nil, api.command_timeout) == 0

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
    local app_path = api.get_trojan_go_path() or ""
    if app_path == "" then
        return {
            code = 1,
            error = i18n.translatef("You did not fill in the %s path. Please save and apply then update manually.", "Trojan-GO")
        }
    end
    if sys.exec("echo -n $(opkg list-installed | grep -c unzip)") ~= "1" then
        api.exec("/bin/rm", {"-f", file})
        return {
            code = 1,
            error = i18n.translate("Not installed unzip, Can't unzip!")
        }
    end

    if not file or file == "" or not fs.access(file) then
        return {code = 1, error = i18n.translate("File path required.")}
    end

    sys.call("/bin/rm -rf /tmp/trojan-go_extract.*")
    local tmp_dir = util.trim(util.exec("mktemp -d -t trojan-go_extract.XXXXXX"))

    local output = {}
    api.exec("/usr/bin/unzip", {"-o", file, "-d", tmp_dir},
             function(chunk) output[#output + 1] = chunk end)

    local files = util.split(table.concat(output))

    api.exec("/bin/rm", {"-f", file})

    return {code = 0, file = tmp_dir}
end

function to_move(file)
    local app_path = api.get_trojan_go_path() or ""
    if app_path == "" then
        return {
            code = 1,
            error = i18n.translatef("You did not fill in the %s path. Please save and apply then update manually.", "Trojan-GO")
        }
    end
    if not file or file == "" then
        sys.call("/bin/rm -rf /tmp/trojan-go_extract.*")
        return {code = 1, error = i18n.translate("Client file is required.")}
    end

    local app_path_bak

    if fs.access(app_path) then
        app_path_bak = app_path .. ".bak"
        api.exec("/bin/mv", {"-f", app_path, app_path_bak})
    end

    local result = api.exec("/bin/mv", { "-f", file .. "/trojan-go", app_path }, nil, api.command_timeout) == 0
    sys.call("/bin/rm -rf /tmp/trojan-go_extract.*")
    if not result or not fs.access(app_path) then
        return {
            code = 1,
            error = i18n.translatef("Can't move new file to path: %s", app_path)
        }
    end

    api.exec("/bin/chmod", {"-R", "755", app_path})

    return {code = 0}
end
