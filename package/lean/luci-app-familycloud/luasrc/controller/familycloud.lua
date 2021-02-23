
module("luci.controller.familycloud", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/familycloud") then
		return
	end

	entry({"admin", "services", "familycloud"},firstchild(), _("天翼家庭云/云盘提速"), 80).dependent = false
	
	entry({"admin", "services", "familycloud", "general"},cbi("familycloud/familycloud"), _("Base Setting"), 1)
        entry({"admin", "services", "familycloud", "log"},form("familycloud/familycloudlog"), _("Log"), 2)
  
	entry({"admin","services","familycloud","status"},call("act_status")).leaf=true
end

function act_status()
  local e={}
  e.running=luci.sys.call("ps | grep speedup.sh | grep -v grep >/dev/null")==0
  luci.http.prepare_content("application/json")
  luci.http.write_json(e)
end

