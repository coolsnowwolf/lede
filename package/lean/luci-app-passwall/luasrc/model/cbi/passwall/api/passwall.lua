module("luci.model.cbi.passwall.api.passwall", package.seeall)
local fs = require "nixio.fs"
local sys = require "luci.sys"
local util = require "luci.util"
local i18n = require "luci.i18n"
local api = require "luci.model.cbi.passwall.api.api"

local luci_api =
    "https://api.github.com/repos/lienol/openwrt-package/releases/latest"

function get_luci_version()
    local ipkg = require "luci.model.ipkg"
    local package_name = "luci-app-passwall"
    local package_info = ipkg.info(package_name) or {}
    if next(package_info) ~= nil then
        return package_info[package_name]["Version"]
    end
    return ""
end

function to_check()
    local json = api.get_api_json(luci_api)

    if json.tag_name == nil then
        return {
            code = 1,
            error = i18n.translate("Get remote version info failed.")
        }
    end

    local remote_version = json.tag_name:match("[^v]+")

    local needs_update = api.compare_versions(get_luci_version(client_file),
                                              "<", remote_version)
    local html_url, download_url

    if needs_update then
        html_url = json.html_url
        for _, v in ipairs(json.assets) do
            local n = v.name
            if n then
                if n:match("luci%-app%-passwall") then
                    download_url = v.browser_download_url
                end
            end
        end
    end

    if needs_update and not download_url then
        return {
            code = 1,
            now_version = get_luci_version(),
            version = remote_version,
            html_url = html_url,
            error = i18n.translate(
                "New version found, but failed to get new version download url.")
        }
    end

    return {
        code = 0,
        update = needs_update,
        now_version = get_luci_version(),
        version = remote_version,
        url = {html = html_url, download = download_url}
    }
end

function update_luci(url, save)
    if not url or url == "" then
        return {code = 1, error = i18n.translate("Download url is required.")}
    end

    sys.call("/bin/rm -f /tmp/luci_passwall.*.ipk")

    local tmp_file =
        util.trim(util.exec("mktemp -u -t luci_passwall.XXXXXX")) .. ".ipk"

    local result = api.exec(api.wget,
                            {"-O", tmp_file, url, api._unpack(api.wget_args)},
                            nil, api.command_timeout) == 0

    if not result then
        api.exec("/bin/rm", {"-f", tmp_file})
        return {
            code = 1,
            error = i18n.translatef("File download failed or timed out: %s", url)
        }
    end

    api.exec("/bin/opkg", {"remove", "luci-app-passwall"})

    local opkg_args = {"--force-downgrade", "--force-reinstall"}

    if save ~= "true" then opkg_args[#opkg_args + 1] = "--force-maintainer" end

    result =
        api.exec("/bin/opkg", {"install", tmp_file, api._unpack(opkg_args)}) ==
            0

    if not result then
        api.exec("/bin/rm", {"-f", tmp_file})
        return {code = 1, error = i18n.translate("Package update failed.")}
    end

    api.exec("/bin/rm", {"-f", tmp_file})
    api.exec("/bin/rm", {"-rf", "/tmp/luci-indexcache", "/tmp/luci-modulecache"})

    return {code = 0}
end
