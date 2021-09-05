-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local ast = require("luci.asterisk")
local uci = require("luci.model.uci").cursor()

--[[
	Dialzone overview table
]]

if not arg[1] then
	zonemap = Map("asterisk", "Dial Zones", [[
		Dial zones hold patterns of dialed numbers to match.
		Each zone has one or more trunks assigned. If the first trunk is
		congested, Asterisk will try to use the next available connection.
		If all trunks fail, then the following zones in the parent dialplan
		are tried.
	]])

	local zones, znames = ast.dialzone.zones()

	zonetbl = zonemap:section(Table, zones, "Zone Overview")
	zonetbl.sectionhead = "Zone"
	zonetbl.addremove   = true
	zonetbl.anonymous   = false
	zonetbl.extedit     = luci.dispatcher.build_url(
		"admin", "asterisk", "dialplans", "zones", "%s"
	)

	function zonetbl.cfgsections(self)
		return znames
	end

	function zonetbl.parse(self)
		for k, v in pairs(self.map:formvaluetable(
			luci.cbi.REMOVE_PREFIX .. self.config
		) or {}) do
			if k:sub(-2) == ".x" then k = k:sub(1, #k - 2) end
			uci:delete("asterisk", k)
			uci:save("asterisk")
			self.data[k] = nil
			for i = 1,#znames do
				if znames[i] == k then
					table.remove(znames, i)
					break
				end
			end
		end

		Table.parse(self)
	end

	zonetbl:option(DummyValue, "description", "Description")
	zonetbl:option(DummyValue, "addprefix")

	match = zonetbl:option(DummyValue, "matches")
	function match.cfgvalue(self, s)
		return table.concat(zones[s].matches, ", ")
	end

	trunks = zonetbl:option(DummyValue, "trunk")
	trunks.template = "asterisk/cbi/cell"
	function trunks.cfgvalue(self, s)
		return ast.tools.hyperlinks(zones[s].trunks)
	end

	return zonemap

--[[
	Zone edit form
]]

else
	zoneedit = Map("asterisk", "Edit Dialzone")

	entry = zoneedit:section(NamedSection, arg[1])
	entry.title = "Zone %q" % arg[1];

	back = entry:option(DummyValue, "_overview", "Back to dialzone overview")
	back.value = ""
	back.titleref = luci.dispatcher.build_url(
		"admin", "asterisk", "dialplans", "zones"
	)

	desc = entry:option(Value, "description", "Description")
	function desc.cfgvalue(self, s, ...)
		return Value.cfgvalue(self, s, ...) or s
	end

	trunks = entry:option(MultiValue, "uses", "Used trunks")
	trunks.widget = "checkbox"
	uci:foreach("asterisk", "sip",
		function(s)
			if s.provider == "yes" then
				trunks:value(
					"SIP/%s" % s['.name'],
					"SIP/%s (%s)" %{ s['.name'], s.host or 'n/a' }
				)
			end
		end)


	match = entry:option(DynamicList, "match", "Number matches")

	intl = entry:option(DynamicList, "international", "Intl. prefix matches (optional)")

	aprefix = entry:option(Value, "addprefix", "Add prefix to dial out (optional)")
	ccode = entry:option(Value, "countrycode", "Effective countrycode (optional)")

	lzone = entry:option(ListValue, "localzone", "Dialzone for local numbers")
	lzone:value("", "no special treatment of local numbers")
	for _, z in ipairs(ast.dialzone.zones()) do
		lzone:value(z.name, "%q (%s)" %{ z.name, z.description })
	end
	--for _, v in ipairs(find_outgoing_contexts(zoneedit.uci)) do
	--	lzone:value(unpack(v))
	--end

	lprefix = entry:option(Value, "localprefix", "Prefix for local calls (optional)")

	return zoneedit

end
