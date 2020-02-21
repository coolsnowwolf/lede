module("luci.controller.mia", package.seeall)

function index()
    if not nixio.fs.access("/etc/config/mia") then return end

    entry({"admin", "control"}, firstchild(), "Control", 44).dependent = false
    entry({"admin", "control", "mia"}, cbi("mia"), _("时间控制"), 10).dependent =
        true
    entry({"admin", "control", "mia", "status"}, call("status")).leaf = true
end

function status()
    local e = {}
    e.status = luci.sys.call("iptables -L FORWARD |grep MIA >/dev/null") == 0
    luci.http.prepare_content("application/json")
    luci.http.write_json(e)
end
