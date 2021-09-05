module("luci.controller.gowebdav", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/gowebdav") then
		return
	end
	entry({"admin", "nas"}, firstchild(), _("NAS") , 45).dependent = false
	local page
	page = entry({"admin", "nas", "gowebdav"}, cbi("gowebdav"), _("GoWebDav"), 100)
	page.dependent = true
	entry({"admin","nas","gowebdav","status"},call("act_status")).leaf=true
end

function act_status()
	local e={}
	e.running=luci.sys.call("pgrep gowebdav >/dev/null")==0
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end
