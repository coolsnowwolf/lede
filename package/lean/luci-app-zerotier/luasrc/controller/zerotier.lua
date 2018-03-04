module("luci.controller.zerotier",package.seeall)
function index()
if not nixio.fs.access("/etc/config/zerotier")then
return
end
entry({"admin","services","zerotier"},cbi("zerotier"),_("ZeroTier"),6).dependent=true
entry({"admin","services","zerotier","status"},call("act_status")).leaf=true
end
function act_status()
local e={}
e.running=luci.sys.call("pgrep /usr/bin/zerotier-one >/dev/null")==0
luci.http.prepare_content("application/json")
luci.http.write_json(e)
end
