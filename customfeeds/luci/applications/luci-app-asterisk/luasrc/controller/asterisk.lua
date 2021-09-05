-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.asterisk", package.seeall)

function index()

	entry({"admin", "services", "asterisk"}, 				  cbi("asterisk"), 			  "Asterisk",				80)

	entry({"admin", "services", "asterisk", "voice"},         cbi("asterisk-voice"),      "Voice Functions",        1)
	entry({"admin", "services", "asterisk", "meetme"},        cbi("asterisk-meetme"),     "Meetme Conferences",     2)

	entry({"admin", "services", "asterisk", "iax-conns"},     cbi("asterisk-iax-connections"), "IAX Connections",   3)
	entry({"admin", "services", "asterisk", "sip-conns"},     cbi("asterisk-sip-connections"), "SIP Connections", 	4)

	entry({"admin", "services", "asterisk", "dialplans"},     cbi("asterisk-dialplans"),  "Dial Plans", 			5)

	entry({"admin", "services", "asterisk", "mod"},           cbi("asterisk-mod-app"),    "Modules", 				4)
	entry({"admin", "services", "asterisk", "mod", "app"},    cbi("asterisk-mod-app"),    "Applications", 			1)
	entry({"admin", "services", "asterisk", "mod", "cdr"},    cbi("asterisk-mod-cdr"),    "Call Detail Records", 	2)
	entry({"admin", "services", "asterisk", "mod", "chan"},   cbi("asterisk-mod-chan"),   "Channels", 				3)
	entry({"admin", "services", "asterisk", "mod", "codec"},  cbi("asterisk-mod-codec"),  "Codecs", 				4)
	entry({"admin", "services", "asterisk", "mod", "format"}, cbi("asterisk-mod-format"), "Format",					5)
	entry({"admin", "services", "asterisk", "mod", "func"},   cbi("asterisk-mod-func"),   "Functions", 				6)
	entry({"admin", "services", "asterisk", "mod", "pbx"},    cbi("asterisk-mod-pbx"),    "PBX", 					7)
	entry({"admin", "services", "asterisk", "mod", "res"},    cbi("asterisk-mod-res"),    "Resources", 				8)
	entry({"admin", "services", "asterisk", "mod", "res", "feature"},
		cbi("asterisk-mod-res-feature"), "Feature Module Configuration", 9 )


	entry({"admin", "asterisk"},                    		cbi("asterisk/main"),        		"Asterisk",  99).i18n = "asterisk"

	entry({"admin", "asterisk", "phones"},          		cbi("asterisk/phones"),      		"Phones",       1)
	entry({"admin", "asterisk", "phones", "sip"},   		cbi("asterisk/phone_sip"),   		nil,            1).leaf = true
	--entry({"admin", "asterisk", "phones", "exten"}, 		cbi("asterisk/phone_exten"), 		"Extensions",   2).leaf = true

	entry({"admin", "asterisk", "trunks"},          		cbi("asterisk/trunks"),      		"Trunks",       2)
	entry({"admin", "asterisk", "trunks", "sip"},   		cbi("asterisk/trunk_sip"),   		nil,            1).leaf = true

	entry({"admin", "asterisk", "voicemail"},          		cbi("asterisk/voicemail"),      	"Voicemail",    3)
	entry({"admin", "asterisk", "voicemail", "mailboxes"},	cbi("asterisk/voicemail"),			"Mailboxes",    1)
	entry({"admin", "asterisk", "voicemail", "settings"},	cbi("asterisk/voicemail_settings"),	"Settings",     2)

	entry({"admin", "asterisk", "meetme"},          		cbi("asterisk/meetme"),      		"MeetMe",		4)
	entry({"admin", "asterisk", "meetme", "rooms"},			cbi("asterisk/meetme"),				"Rooms",		1)
	entry({"admin", "asterisk", "meetme", "settings"},		cbi("asterisk/meetme_settings"),	"Settings",     2)

	entry({"admin", "asterisk", "dialplans"},				call("handle_dialplan"),     		"Call Routing", 5)
	entry({"admin", "asterisk", "dialplans", "out"},		cbi("asterisk/dialplan_out"),		nil,            1).leaf = true
	entry({"admin", "asterisk", "dialplans", "zones"},		call("handle_dialzones"),			"Dial Zones",	2).leaf = true

end


function handle_dialplan()
	local uci = luci.model.uci.cursor()
	local ast = require "luci.asterisk"
	local err = false

	for k, v in pairs(luci.http.formvaluetable("delzone")) do
		local plan = ast.dialplan.plan(k)
		if #v > 0 and plan then
			local newinc = { }

			for _, z in ipairs(plan.zones) do
				if z.name ~= v then
					newinc[#newinc+1] = z.name
				end
			end

			uci:delete("asterisk", plan.name, "include")

			if #newinc > 0 then
				uci:set("asterisk", plan.name, "include", newinc)
			end
		end
	end

	for k, v in pairs(luci.http.formvaluetable("addzone")) do
		local plan = ast.dialplan.plan(k)
		local zone = ast.dialzone.zone(v)
		if #v > 0 and plan and zone then
			local newinc = { zone.name }

			for _, z in ipairs(plan.zones) do
				newinc[#newinc+1] = z.name
			end

			uci:delete("asterisk", plan.name, "include")

			if #newinc > 0 then
				uci:set("asterisk", plan.name, "include", newinc)
			end
		end
	end

	for k, v in pairs(luci.http.formvaluetable("delvbox")) do
		local plan = ast.dialplan.plan(k)
		if #v > 0 and plan then
			uci:delete_all("asterisk", "dialplanvoice",
				{ extension=v, dialplan=plan.name })
		end
	end

	for k, v in pairs(luci.http.formvaluetable("addvbox")) do
		local plan = ast.dialplan.plan(k)
		local vbox = ast.voicemail.box(v)
		if plan and vbox then
			local vext = luci.http.formvalue("addvboxext.%s" % plan.name)
			vext = ( vext and #vext > 0 ) and vext or vbox.number
			uci:section("asterisk", "dialplanvoice", nil, {
				dialplan		= plan.name,
				extension		= vext,
				voicebox		= vbox.number,
				voicecontext	= vbox.context
			})
		end
	end

	for k, v in pairs(luci.http.formvaluetable("delmeetme")) do
		local plan = ast.dialplan.plan(k)
		if #v > 0 and plan then
			uci:delete_all("asterisk", "dialplanmeetme",
				{ extension=v, dialplan=plan.name })
		end
	end

	for k, v in pairs(luci.http.formvaluetable("addmeetme")) do
		local plan = ast.dialplan.plan(k)
		local meetme = ast.meetme.room(v)
		if plan and meetme then
			local mext = luci.http.formvalue("addmeetmeext.%s" % plan.name)
			mext = ( mext and #mext > 0 ) and mext or meetme.room
			uci:section("asterisk", "dialplanmeetme", nil, {
				dialplan	= plan.name,
				extension	= mext,
				room		= meetme.room
			})
		end
	end

	local aname = luci.http.formvalue("addplan")
	if aname and #aname > 0 then
		if aname:match("^[a-zA-Z0-9_]+$") then
			uci:section("asterisk", "dialplan", aname, { })
		else
			err = true
		end
	end

	local dname = luci.http.formvalue("delplan")
	if dname and #dname > 0 then
		if uci:get("asterisk", dname) == "dialplan" then
			uci:delete("asterisk", dname)
			uci:delete_all("asterisk", "dialplanvoice", { dialplan=dname })
			uci:delete_all("asterisk", "dialplanmeetme", { dialplan=dname })
		end
	end

	uci:save("asterisk")
	ast.uci_resync()

	luci.template.render("asterisk/dialplans", { create_error = err })
end

function handle_dialzones()
	local ast = require "luci.asterisk"
	local uci = luci.model.uci.cursor()
	local err = false

	if luci.http.formvalue("newzone") then
		local name = luci.http.formvalue("newzone_name")
		if name and name:match("^[a-zA-Z0-9_]+$") then
			uci:section("asterisk", "dialzone", name, {
				uses  = ast.tools.parse_list(luci.http.formvalue("newzone_uses") or {}),
				match = ast.tools.parse_list(luci.http.formvalue("newzone_match") or {})
			})
			uci:save("asterisk")
		else
			err = true
		end
	end

	if luci.http.formvalue("delzone") then
		local name = luci.http.formvalue("delzone")
		if uci:get("asterisk", name) == "dialzone" then
			uci:delete("asterisk", name)
			uci:save("asterisk")
		end
	end

	luci.template.render("asterisk/dialzones", { create_error = err })
end
