module("luci.controller.qbittorrent",package.seeall)
function index()
	entry({"admin","nas","qbittorrent"},cbi("qbittorrent"),_("qbittorrent"))
end
