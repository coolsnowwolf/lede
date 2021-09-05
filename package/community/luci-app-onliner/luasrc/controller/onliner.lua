module("luci.controller.onliner",package.seeall)
nixio=require"nixio"
function index()
entry({"admin","status","onliner"},alias("admin","status","onliner","onliner"),_("Online User"))
entry({"admin","status","onliner","onliner"},template("onliner/onliner"),_("Online User"),1)
entry({"admin", "status","onliner","speed"}, template("onliner/display"), _("speed"), 2)
entry({"admin", "status","onliner","setnlbw"}, call("set_nlbw"))
end
function set_nlbw()
	if nixio.fs.access("/var/run/onsetnlbw") then
		nixio.fs.writefile("/var/run/onsetnlbw","1");
	else
		io.popen("/usr/share/onliner/setnlbw.sh &")
	end
	luci.http.prepare_content("application/json")
	luci.http.write('')
end