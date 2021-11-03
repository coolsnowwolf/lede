module("luci.controller.vlmcsd", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/vlmcsd") then
		return
	end

	entry({"admin", "services", "vlmcsd"}, alias("admin", "services", "vlmcsd", "basic"), _("KMS Server"), 100).dependent = true
	entry({"admin", "services", "vlmcsd", "basic"}, cbi("vlmcsd/basic"), _("Basic Setting"), 1).leaf = true
	entry({"admin", "services", "vlmcsd", "config"}, cbi("vlmcsd/config"), _("Config File"), 2).leaf = true
	entry({"admin", "services", "vlmcsd", "status"}, call("act_status")).leaf = true
end

function act_status()
	local e={}
	e.running=luci.sys.call("pgrep vlmcsd >/dev/null")==0
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end
