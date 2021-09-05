-- Copyright 2012 Gabor Juhos <juhosg@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.minidlna", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/minidlna") then
		return
	end

	local page

	page = entry({"admin", "services", "minidlna"}, cbi("minidlna"), _("miniDLNA"))
	page.dependent = true

	entry({"admin", "services", "minidlna_status"}, call("minidlna_status"))
end

function minidlna_status()
	local sys  = require "luci.sys"
	local uci  = require "luci.model.uci".cursor()
	local port = tonumber(uci:get_first("minidlna", "minidlna", "port"))

	local status = {
		running = (sys.call("pidof minidlnad >/dev/null") == 0),
		audio   = 0,
		video   = 0,
		image   = 0
	}

	if status.running then
		local fd = sys.httpget("http://127.0.0.1:%d/" % (port or 8200), true)
		if fd then
			local html = fd:read("*a")
			if html then
				status.audio = (tonumber(html:match("Audio files</td><td>(%d+)")) or 0)
				status.video = (tonumber(html:match("Video files</td><td>(%d+)")) or 0)
				status.image = (tonumber(html:match("Image files</td><td>(%d+)")) or 0)
			end
			fd:close()
		end
	end

	luci.http.prepare_content("application/json")
	luci.http.write_json(status)
end
