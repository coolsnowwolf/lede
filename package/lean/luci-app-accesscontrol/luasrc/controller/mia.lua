module("luci.controller.mia",package.seeall)

function index()
	if not nixio.fs.access("/etc/config/mia")then
		return
	end

	entry({"admin","services","mia"},cbi("mia"),_("Internet Access Schedule Control"),30).dependent=true
	entry({"admin","services","mia","status"},call("act_status")).leaf=true

end

function act_status()
  local e={}
	e.running=luci.sys.call("iptables -L INPUT |grep MIA >/dev/null")==0
  luci.http.prepare_content("application/json")
  luci.http.write_json(e)
end
