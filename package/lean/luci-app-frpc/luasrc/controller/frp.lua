module("luci.controller.frp", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/frp") then
		return
	end

	entry({"admin","vpn","frp"},cbi("frp/frp"), _("Frp Setting"),100).dependent=true
	entry({"admin","vpn","frp","config"},cbi("frp/config")).leaf=true
	entry({"admin","vpn","frp","status"},call("status")).leaf=true
end

function status()
local e={}
e.running=luci.sys.call("pidof frpc > /dev/null")==0
luci.http.prepare_content("application/json")
luci.http.write_json(e)
end
