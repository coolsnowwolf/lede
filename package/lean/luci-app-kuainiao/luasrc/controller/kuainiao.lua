module("luci.controller.kuainiao", package.seeall)

function index()
    local fs = luci.fs or nixio.fs
    if not fs.access("/etc/config/kuainiao") then
		return
	end
	
	local page = entry({"admin", "services", "kuainiao"}, cbi("kuainiao"), _("Kuai Niao"),201)
	page.dependent = true
	entry({"admin","services","kuainiao","status"},call("kuainiao_status")).leaf=true
	entry({"admin","services","kuainiao","dwonstatus"},call("down_status")).leaf=true
	entry({"admin","services","kuainiao","upstatus"},call("up_status")).leaf=true
end

function kuainiao_status()
local e={}
e.running=luci.sys.exec("ps |grep -v grep |grep -w '/usr/bin/kuainiao' ")
luci.http.prepare_content("application/json")
luci.http.write_json(e)
end

function up_status()
local e={}
e.upstatus=luci.sys.exec("cat /usr/share/kuainiao/kuainiao_up_state")
luci.http.prepare_content("application/json")
luci.http.write_json(e)
end
function down_status()
local e={}
e.dwonstatus=luci.sys.exec("cat /usr/share/kuainiao/kuainiao_down_state")
luci.http.prepare_content("application/json")
luci.http.write_json(e)
end
