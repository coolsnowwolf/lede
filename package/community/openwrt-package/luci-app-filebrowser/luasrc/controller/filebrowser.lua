-- Copyright 2018-2020 Lienol <lawlienol@gmail.com>
module("luci.controller.filebrowser", package.seeall)

local http = require "luci.http"
local api = require "luci.model.cbi.filebrowser.api"

function index()
    if not nixio.fs.access("/etc/config/filebrowser") then return end

    entry({"admin", "nas"}, firstchild(), "NAS", 44).dependent = false
    entry({"admin", "nas", "filebrowser"}, cbi("filebrowser/settings"),
          _("File Browser"), 2).dependent = true

    entry({"admin", "nas", "filebrowser", "check"}, call("action_check")).leaf =
        true
    entry({"admin", "nas", "filebrowser", "download"}, call("action_download")).leaf =
        true
    entry({"admin", "nas", "filebrowser", "status"}, call("act_status")).leaf =
        true
    entry({"admin", "nas", "filebrowser", "get_log"}, call("get_log")).leaf =
        true
    entry({"admin", "nas", "filebrowser", "clear_log"}, call("clear_log")).leaf =
        true
end

local function http_write_json(content)
    http.prepare_content("application/json")
    http.write_json(content or {code = 1})
end

function act_status()
    local e = {}
    e.status = luci.sys.call(
                   "ps -w | grep -v grep | grep 'filebrowser -a 0.0.0.0' >/dev/null") ==
                   0
    http_write_json(e)
end

function action_check()
    local json = api.to_check()
    http_write_json(json)
end

function action_download()
    local json = nil
    local task = http.formvalue("task")
    if task == "extract" then
        json = api.to_extract(http.formvalue("file"))
    elseif task == "move" then
        json = api.to_move(http.formvalue("file"))
    else
        json = api.to_download(http.formvalue("url"))
    end
    http_write_json(json)
end

function get_log()
    luci.http.write(luci.sys.exec(
                        "[ -f '/var/log/filebrowser.log' ] && cat /var/log/filebrowser.log"))
end
function clear_log() luci.sys.call("echo '' > /var/log/filebrowser.log") end
