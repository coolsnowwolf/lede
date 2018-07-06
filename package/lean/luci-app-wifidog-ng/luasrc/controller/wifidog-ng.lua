
module("luci.controller.wifidog-ng", package.seeall)
local json = require "luci.json"

function index()
    if not nixio.fs.access("/etc/config/wifidog-ng") then
        return
    end

    local page
    page = entry({"admin", "services", "wifidog-ng"}, cbi("wifidog-ng"), _("WifiDog-ng"))
    page.dependent = true
    entry({"admin", "services", "wifidog-ng", "getClientList"}, call("getClientList"))
end

function getClientList()
    local result = {}
    local running = luci.sys.call("pgrep wifidog-ng >/dev/null")==0
    if running then
        local term = luci.sys.exec("ubus call wifidog-ng term '{\"action\":\"show\"}'")
        result = json.decode(term)
    end
	
    result["running"] = running
    luci.http.prepare_content("application/json")
    luci.http.write_json(result)
end