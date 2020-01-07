module("luci.controller.nps",package.seeall)
function index()
if not nixio.fs.access("/etc/config/nps")then
return
end
local e
e=entry({"admin","services","nps"},cbi("nps"),_("Nps Setting"),100)
e.i18n="nps"
e.dependent=true
entry({"admin","services","nps","status"},call("status")).leaf=true
end
function status()
local e={}
e.running=luci.sys.call("pgrep npc > /dev/null")==0
luci.http.prepare_content("application/json")
luci.http.write_json(e)
end
