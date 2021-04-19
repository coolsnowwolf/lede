
-- Licensed to the public under the GNU General Public License v3.

module("luci.controller.mwan3helper", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/mwan3helper") then
		return
	end

	entry({"admin", "services", "mwan3helper"}, alias("admin", "services", "mwan3helper", "client"), _("MWAN3 Helper"), 300).dependent = true
	entry({"admin", "services", "mwan3helper", "client"}, cbi("mwan3helper/client"), _("Settings"), 10).leaf = true
	entry({"admin", "services", "mwan3helper", "lists"}, cbi("mwan3helper/list"), _("IPSet Lists"), 20).leaf = true
	entry({"admin", "services", "mwan3helper", "status"}, call("act_status")).leaf = true
end

function act_status()
	local e={}
	e.running=luci.sys.call("pgrep mwan3dns >/dev/null")==0
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end
