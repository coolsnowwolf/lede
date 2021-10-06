module("luci.controller.uuplugin",package.seeall)

function index()
	if not nixio.fs.access("/etc/config/uuplugin") then
		return
	end

	entry({"admin", "services", "uuplugin"}, cbi("uuplugin/uuplugin"), ("UU GameAcc"), 99).dependent = true
	entry({"admin", "services", "uuplugin", "status"}, call("act_status")).leaf = true
end

function act_status()
	local e={}
	e.running=luci.sys.call("pgrep -f uuplugin >/dev/null")==0
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end
