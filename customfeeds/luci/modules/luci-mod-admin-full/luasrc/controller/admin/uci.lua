-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2010-2015 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.admin.uci", package.seeall)

function index()
	local redir = luci.http.formvalue("redir", true) or
	  luci.dispatcher.build_url(unpack(luci.dispatcher.context.request))

	entry({"admin", "uci"}, nil, _("Configuration"))
	entry({"admin", "uci", "changes"}, call("action_changes"), _("Changes"), 40).query = {redir=redir}
	entry({"admin", "uci", "revert"}, post("action_revert"), _("Revert"), 30).query = {redir=redir}
	entry({"admin", "uci", "apply"}, post("action_apply"), _("Apply"), 20).query = {redir=redir}
	entry({"admin", "uci", "saveapply"}, post("action_apply"), _("Save &#38; Apply"), 10).query = {redir=redir}
end

function action_changes()
	local uci = luci.model.uci.cursor()
	local changes = uci:changes()

	luci.template.render("admin_uci/changes", {
		changes = next(changes) and changes
	})
end

function action_apply()
	local path = luci.dispatcher.context.path
	local uci = luci.model.uci.cursor()
	local changes = uci:changes()
	local reload = {}

	-- Collect files to be applied and commit changes
	for r, tbl in pairs(changes) do
		table.insert(reload, r)
		if path[#path] ~= "apply" then
			uci:load(r)
			uci:commit(r)
			uci:unload(r)
		end
	end

	luci.template.render("admin_uci/apply", {
		changes = next(changes) and changes,
		configs = reload
	})
end


function action_revert()
	local uci = luci.model.uci.cursor()
	local changes = uci:changes()

	-- Collect files to be reverted
	for r, tbl in pairs(changes) do
		uci:load(r)
		uci:revert(r)
		uci:unload(r)
	end

	luci.template.render("admin_uci/revert", {
		changes = next(changes) and changes
	})
end
