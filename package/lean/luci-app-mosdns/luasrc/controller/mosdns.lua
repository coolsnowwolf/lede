module("luci.controller.mosdns", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/mosdns") then
		return
	end

	local page = entry({"admin", "services", "mosdns"}, alias("admin", "services", "mosdns", "basic"), _("MosDNS"), 30)
	page.dependent = true
	page.acl_depends = { "luci-app-mosdns" }

	entry({"admin", "services", "mosdns", "basic"}, cbi("mosdns/basic"), _("Basic Setting"), 1).leaf = true
	entry({"admin", "services", "mosdns", "rule_list"}, cbi("mosdns/rule_list"), _("Rule List"), 2).leaf = true
	entry({"admin", "services", "mosdns", "update"}, cbi("mosdns/update"), _("Geodata Update"), 3).leaf = true
	entry({"admin", "services", "mosdns", "log"}, cbi("mosdns/log"), _("Logs"), 4).leaf = true
	entry({"admin", "services", "mosdns", "status"}, call("act_status")).leaf = true
	entry({"admin", "services", "mosdns", "get_log"}, call("get_log")).leaf = true
	entry({"admin", "services", "mosdns", "clear_log"}, call("clear_log")).leaf = true
	entry({"admin", "services", "mosdns", "geo_update"}, call("geo_update")).leaf = true
end

function act_status()
	local e = {}
	e.running = luci.sys.call("pgrep -f mosdns >/dev/null") == 0
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end

function get_log()
	luci.http.write(luci.sys.exec("cat $(/usr/share/mosdns/mosdns.sh logfile)"))
end

function clear_log()
	luci.sys.call("cat /dev/null > $(/usr/share/mosdns/mosdns.sh logfile)")
end

function geo_update()
	local e = {}
	e.updating = luci.sys.call("/usr/share/mosdns/mosdns.sh geodata >/dev/null") == 0
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end
