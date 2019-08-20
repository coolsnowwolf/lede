
module("luci.controller.unblockmusic", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/unblockmusic") then
		return
	end

	entry({"admin", "services", "unblockmusic"},firstchild(), _("解锁网易云灰色歌曲"), 50).dependent = false
	
	entry({"admin", "services", "unblockmusic", "general"},cbi("unblockmusic"), _("Base Setting"), 1)
	entry({"admin", "services", "unblockmusic", "log"},form("unblockmusiclog"), _("Log"), 2)
  
	entry({"admin", "services", "unblockmusic", "status"},call("act_status")).leaf=true
end

function act_status()
  local e={}
  e.running=luci.sys.call("ps | grep app.js | grep -v grep >/dev/null")==0
  luci.http.prepare_content("application/json")
  luci.http.write_json(e)
end
