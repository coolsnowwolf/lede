module("luci.model.cbi.passwall.api.brook", package.seeall)
local fs = require "nixio.fs"
local sys = require "luci.sys"
local util = require "luci.util"
local i18n = require "luci.i18n"
local api = require "luci.model.cbi.passwall.api.api"

local brook_api =
    "https://api.github.com/repos/txthinking/brook/releases/latest"

function get_brook_file_path()
    return api.uci_get_type("global_app", "brook_file")
end

function get_brook_version(file)
    if file == nil then file = get_brook_file_path() end

    if file and file ~= "" then
        if not fs.access(file, "rwx", "rx", "rx") then
            fs.chmod(file, 755)
        end

        local info = util.trim(sys.exec("%s -v 2>/dev/null" % file))

        if info ~= "" then
            local tb = util.split(info, "%s+", nil, true)
            return tb[1] == "Brook" and tb[3] or ""
        end
    end

    return ""
end

function to_check(arch)
    if not arch or arch == "" then arch = api.auto_get_arch() end

    local file_tree, sub_version = api.get_file_info(arch)

    if file_tree == "" then
        return {
            code = 1,
            error = i18n.translate(
                "Can't determine ARCH, or ARCH not supported.")
        }
    end

    file_tree = "_linux_" .. file_tree
    if file_tree == "_linux_amd64" then file_tree = "" end

    local json = api.get_api_json(brook_api)

    if json.tag_name == nil then
        return {
            code = 1,
            error = i18n.translate("Get remote version info failed.")
        }
    end

    local remote_version = json.tag_name:match("[^v]+")

    local client_file = get_brook_file_path()

    local needs_update = api.compare_versions(get_brook_version(client_file),
                                              "<", remote_version)
    local html_url, download_url

    if needs_update then
        html_url = json.html_url
        for _, v in ipairs(json.assets) do
            if v.name and v.name:match(file_tree .. sub_version) then
                download_url = v.browser_download_url
                break
            end
        end
    end

    if needs_update and not download_url then
        return {
            code = 1,
            now_version = get_brook_version(client_file),
            version = remote_version,
            html_url = html_url,
            error = i18n.translate(
                "New version found, but failed to get new version download url.")
        }
    end

    return {
        code = 0,
        update = needs_update,
        now_version = get_brook_version(client_file),
        version = remote_version,
        url = {html = html_url, download = download_url}
    }
end

function to_download(url)
    if not url or url == "" then
        return {code = 1, error = i18n.translate("Download url is required.")}
    end

    sys.call("/bin/rm -f /tmp/brook_download.*")

    local tmp_file = util.trim(util.exec("mktemp -u -t brook_download.XXXXXX"))

    local result = api.exec(api.wget, {"-O", tmp_file, url, api._unpack(api.wget_args)},
                            nil, api.command_timeout) == 0

    if not result then
        api.exec("/bin/rm", {"-f", tmp_file})
        return {
            code = 1,
            error = i18n.translatef("File download failed or timed out: %s", url)
        }
    end

    return {code = 0, file = tmp_file}
end

function to_move(file)
    if not file or file == "" or not fs.access(file) then
        sys.call("/bin/rm -rf /tmp/brook_download.*")
        return {code = 1, error = i18n.translate("Client file is required.")}
    end

    local version = get_brook_version(file)
    if version == "" then
        sys.call("/bin/rm -rf /tmp/brook_download.*")
        return {
            code = 1,
            error = i18n.translate(
                "The client file is not suitable for current device.")
        }
    end

    local client_file = get_brook_file_path()
    local client_file_bak

    if fs.access(client_file) then
        client_file_bak = client_file .. ".bak"
        api.exec("/bin/mv", {"-f", client_file, client_file_bak})
    end

    local result = api.exec("/bin/mv", {"-f", file, client_file}, nil,
                            api.command_timeout) == 0

    if not result or not fs.access(client_file) then
        sys.call("/bin/rm -rf /tmp/brook_download.*")
        if client_file_bak then
            api.exec("/bin/mv", {"-f", client_file_bak, client_file})
        end
        return {
            code = 1,
            error = i18n.translatef("Can't move new file to path: %s",
                                    client_file)
        }
    end

    api.exec("/bin/chmod", {"755", client_file})

    if client_file_bak then api.exec("/bin/rm", {"-f", client_file_bak}) end

    sys.call("/bin/rm -rf /tmp/brook_download.*")

    return {code = 0}
end
