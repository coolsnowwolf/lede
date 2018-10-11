module("luci.controller.v2raypro", package.seeall)
function index()
		if not nixio.fs.access("/etc/config/v2raypro") then
		return
	end
	local page
	page = entry({"admin", "services", "v2raypro"}, cbi("v2raypro"), _("V2Ray Pro"))
	page.dependent = true
	entry({"admin","services","v2raypro","status"},call("act_status")).leaf=true
end

function act_status()
  local e={}
  e.running=luci.sys.call("pgrep v2ray >/dev/null")==0
  luci.http.prepare_content("application/json")
  luci.http.write_json(e)
end
