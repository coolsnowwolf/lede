-- This is a free software, use it under GNU General Public License v3.0.
-- Created By [CTCGFW]Project OpenWRT
-- https://github.com/project-openwrt

module("luci.controller.unblockneteasemusic-mini", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/unblockneteasemusic-mini") then
		return
	end

	local page
	page = entry({"admin", "services", "unblockneteasemusic-mini"}, cbi("unblockneteasemusic-mini"), _("解除网易云音乐播放限制 (Mini)"), 100)
	page.dependent = true
	entry({"admin", "services", "unblockneteasemusic-mini", "status"},call("act_status")).leaf=true
end

function act_status()
	local e={}
	e.running=luci.sys.call("iptables -t nat -L -v -n |grep unblock_netease_music |grep -v grep >/dev/null")==0
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end
