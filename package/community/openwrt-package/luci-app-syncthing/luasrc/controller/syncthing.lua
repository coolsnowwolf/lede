
module("luci.controller.syncthing", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/syncthing") then
		return
	end
	
	entry({"admin", "nas"}, firstchild(), "NAS", 44).dependent = false
	entry({"admin", "nas", "syncthing"}, cbi("syncthing"), _("Syncthing"), 10).dependent = true
	entry({"admin","nas","syncthing","status"},call("act_status")).leaf=true
end

function act_status()
  local e={}
  e.running=luci.sys.call("pgrep syncthing >/dev/null")==0
  luci.http.prepare_content("application/json")
  luci.http.write_json(e)
end
