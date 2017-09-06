module("luci.controller.ssr_server",package.seeall)

function index()
if not nixio.fs.access("/etc/config/ssr_server")then
  return
end

entry({"admin", "vpn"}, firstchild(), "VPN", 45).dependent = false
	
local page

entry({"admin","vpn","ssr_server"},cbi("ssr_server"),_("SSR Python"),4).dependent=true
entry({"admin","vpn","ssr_server","status"},call("act_status")).leaf=true
end

function act_status()
  local e={}
  e.server=luci.sys.call("ps | grep server.py |grep -v grep >/dev/null")==0
  luci.http.prepare_content("application/json")
  luci.http.write_json(e)
end
