-- Copyright 2019 John Crispin <blogic@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.ltqtapi", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/telephony") then
		return
	end

	page = node("admin", "telephony")
	page.target = firstchild()
	page.title  = _("VoIP")
	page.order  = 90

	entry({"admin", "telephony", "account"}, cbi("luci_ltqtapi/account") , _("Account"), 10)
	entry({"admin", "telephony", "contact"}, cbi("luci_ltqtapi/contact") , _("Contacts"), 20)

	entry({"admin", "telephony", "status"}, call("tapi_status")).leaf = true
end

function tapi_status()
	local st = { }
	local state = require "luci.model.uci".cursor_state()
	state:load("telephony")

	st.status = "Offline";
	if state:get("telephony", "state", "port1", "0") == "0" then
		st.line1 = "Idle";
	else
		st.line1 = "Calling";
	end
	if state:get("telephony", "state", "port2", "0") == "0" then
		st.line2 = "Idle";
	else
		st.line2 = "Calling";
	end
	luci.http.prepare_content("application/json")
	luci.http.write_json(st)
end
