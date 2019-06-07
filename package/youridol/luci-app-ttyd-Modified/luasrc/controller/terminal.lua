module("luci.controller.terminal", package.seeall)

function index()
	if not (luci.sys.call("pidof ttyd > /dev/null") == 0) then
		return
	end
	
	entry({"admin", "services", "terminal"}, template("terminal"), _("TTYD Terminal"), 10).leaf = true
end
