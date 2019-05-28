module("luci.controller.mia",package.seeall)

function index()
	if not nixio.fs.access("/etc/config/mia")then
		return
	end

	entry({"admin","services","mia"},cbi("mia"),_("Internet Access Schedule Control"),30).dependent=true

end


