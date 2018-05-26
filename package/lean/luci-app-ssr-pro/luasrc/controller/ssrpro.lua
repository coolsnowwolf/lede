module("luci.controller.ssrpro", package.seeall)
function index()
		if not nixio.fs.access("/etc/config/ssrpro") then
		return
	end
	local page
	page = entry({"admin", "services", "ssrpro"}, cbi("ssrpro"), _("ShadowsocksR Pro"))
	page.dependent = true
	entry({"admin","services","ssrpro","status"},call("act_status")).leaf=true
end

function act_status()
  local e={}
  e.running=luci.sys.call("pgrep ssr-redir >/dev/null")==0
  luci.http.prepare_content("application/json")
  luci.http.write_json(e)
end
