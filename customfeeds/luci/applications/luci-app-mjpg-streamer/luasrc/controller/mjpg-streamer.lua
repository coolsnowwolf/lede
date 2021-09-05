-- Copyright 2014 Roger D <rogerdammit@gmail.com>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.mjpg-streamer", package.seeall)

function index()
	require("luci.i18n")
	luci.i18n.loadc("mjpg-streamer")
	if not nixio.fs.access("/etc/config/mjpg-streamer") then
		return
	end

	local page = entry({"admin", "services", "mjpg-streamer"}, cbi("mjpg-streamer"), _("MJPG-streamer"))
	page.i18n = "mjpg-streamer"
	page.dependent = true

end
