
module("luci.controller.adbyby", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/adbyby") then
		return
	end
	
	entry({"admin", "services", "adbyby"}, alias("admin", "services", "adbyby", "base"),_("ADBYBY Plus +"), 9).dependent = true
	
	entry({"admin", "services", "adbyby", "base"}, cbi("adbyby/base"), _("Base Setting"), 10).leaf=true
	entry({"admin", "services", "adbyby", "advanced"}, cbi("adbyby/advanced"), _("Advance Setting"), 20).leaf=true
	entry({"admin", "services", "adbyby", "help"}, form("adbyby/help"), _("Plus+ Domain List"), 30).leaf=true
	
	
	entry({"admin","services","adbyby","status"},call("act_status")).leaf=true
end

function act_status()
  local e={}
  e.running=luci.sys.call("pgrep adbyby >/dev/null")==0
  luci.http.prepare_content("application/json")
  luci.http.write_json(e)
end
