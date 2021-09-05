-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local ast = require("luci.asterisk")

local function find_outgoing_contexts(uci)
	local c = { }
	local h = { }

--	uci:foreach("asterisk", "dialplan",
--		function(s)
--			if not h[s['.name']] then
--				c[#c+1] = { s['.name'], "Dialplan: %s" % s['.name'] }
--				h[s['.name']] = true
--			end
--		end)

	uci:foreach("asterisk", "dialzone",
		function(s)
			if not h[s['.name']] then
				c[#c+1] = { s['.name'], "Dialzone: %s" % s['.name'] }
				h[s['.name']] = true
			end
		end)

	return c
end

local function find_incoming_contexts(uci)
	local c = { }
	local h = { }

	uci:foreach("asterisk", "sip",
		function(s)
			if s.context and not h[s.context] and
			   uci:get_bool("asterisk", s['.name'], "provider")
			then
				c[#c+1] = { s.context, "Incoming: %s" % s['.name'] or s.context }
				h[s.context] = true
			end
		end)

	return c
end

local function find_trunks(uci)
	local t = { }

	uci:foreach("asterisk", "sip",
		function(s)
			if uci:get_bool("asterisk", s['.name'], "provider") then
				t[#t+1] = {
					"SIP/%s" % s['.name'],
					"SIP: %s" % s['.name']
				}
			end
		end)

	uci:foreach("asterisk", "iax",
		function(s)
			t[#t+1] = {
				"IAX/%s" % s['.name'],
				"IAX: %s" % s.extension or s['.name']
			}
		end)

	return t
end

--[[

dialzone {name} - Outgoing zone.
	uses          - Outgoing line to use: TYPE/Name
	match (list)  - Number to match
	countrycode   - The effective country code of this dialzone
	international (list) - International prefix to match
	localzone     - dialzone for local numbers
	addprefix     - Prexix required to dial out.
	localprefix   - Prefix for a local call

]]


--
-- SIP dialzone configuration
--
if arg[1] then
	cbimap = Map("asterisk", "Edit Dialplan Entry")

	entry = cbimap:section(NamedSection, arg[1])

	back = entry:option(DummyValue, "_overview", "Back to dialplan overview")
	back.value = ""
	back.titleref = luci.dispatcher.build_url("admin", "asterisk", "dialplans")

	desc = entry:option(Value, "description", "Description")
	function desc.cfgvalue(self, s, ...)
		return Value.cfgvalue(self, s, ...) or s
	end

	match = entry:option(DynamicList, "match", "Number matches")

	intl = entry:option(DynamicList, "international", "Intl. prefix matches (optional)")

	trunk = entry:option(MultiValue, "uses", "Used trunk")
	for _, v in ipairs(find_trunks(cbimap.uci)) do
		trunk:value(unpack(v))
	end

	aprefix = entry:option(Value, "addprefix", "Add prefix to dial out (optional)")
	--ast.idd.cbifill(aprefix)

	ccode = entry:option(Value, "countrycode", "Effective countrycode (optional)")
	ast.cc.cbifill(ccode)

	lzone = entry:option(ListValue, "localzone", "Dialzone for local numbers")
	lzone:value("", "no special treatment of local numbers")
	for _, v in ipairs(find_outgoing_contexts(cbimap.uci)) do
		lzone:value(unpack(v))
	end

	lprefix = entry:option(Value, "localprefix", "Prefix for local calls (optional)")

	return cbimap
end
