module("luci.controller.qbittorrent",package.seeall)
function index()
	entry({"admin","services","qbittorrent"},cbi("qbittorrent"),_("qbittorrent"))
end