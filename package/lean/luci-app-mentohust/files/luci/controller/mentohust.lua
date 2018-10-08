--[[

LuCI mentohust
Author:ptpt52
Email:ptpt52@gmail.com

]]--

module("luci.controller.mentohust", package.seeall)

function index()

	if not nixio.fs.access("/etc/config/mentohust") then
		return
	end
	local page 
	page = entry({"admin", "services", "mentohust"}, cbi("mentohust/mentohust"), _("MentoHUST"), 10)
	page.i18n = "mentohust"
	page.dependent = true

	entry({"admin", "services", "mentohust", "status"}, call("status")).leaf = true
end

function status()
	local sys  = require "luci.sys"
	local ipkg = require "luci.model.ipkg"
	local http = require "luci.http"
	local uci  = require "luci.model.uci".cursor()

	local status = {
		running = (sys.call("pidof mentohust > /dev/null") == 0),
		h_mentohust = ipkg.installed("mentohust"),
	}

	http.prepare_content("application/json")
	http.write_json(status)
end
