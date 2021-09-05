-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.mini.index", package.seeall)

function index()
	local root = node()
	if not root.lock then
		root.target = alias("mini")
		root.index = true
	end
	
	entry({"about"}, template("about"))
	
	local page   = entry({"mini"}, alias("mini", "index"), _("Essentials"), 10)
	page.sysauth = "root"
	page.sysauth_authenticator = "htmlauth"
	page.index = true
	
	entry({"mini", "index"}, alias("mini", "index", "index"), _("Overview"), 10).index = true
	entry({"mini", "index", "index"}, form("mini/index"), _("General"), 1).ignoreindex = true
	entry({"mini", "index", "luci"}, cbi("mini/luci", {autoapply=true}), _("Settings"), 10)
	entry({"mini", "index", "logout"}, call("action_logout"), _("Logout"))
end

function action_logout()
	local dsp = require "luci.dispatcher"
	local utl = require "luci.util"
	if dsp.context.authsession then
		utl.ubus("session", "destroy", {
			ubus_rpc_session = dsp.context.authsession
		})
		dsp.context.urltoken.stok = nil
	end

	luci.http.header("Set-Cookie", "sysauth=; path=" .. dsp.build_url())
	luci.http.redirect(luci.dispatcher.build_url())
end
