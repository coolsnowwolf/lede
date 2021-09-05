-- Copyright 2009 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

module("luci.asterisk", package.seeall)
require("luci.asterisk.cc_idd")

local _io  = require("io")
local uci  = require("luci.model.uci").cursor()
local sys  = require("luci.sys")
local util = require("luci.util")

AST_BIN   = "/usr/sbin/asterisk"
AST_FLAGS = "-r -x"


--- LuCI Asterisk - Resync uci context
function uci_resync()
	uci = luci.model.uci.cursor()
end

--- LuCI Asterisk io interface
-- Handles low level io.
-- @type	module
io = luci.util.class()

--- Execute command and return output
-- @param command	String containing the command to execute
-- @return			String containing the command output
function io.exec(command)
	local fh = _io.popen( "%s %s %q" %{ AST_BIN, AST_FLAGS, command }, "r" )
	assert(fh, "Failed to invoke asterisk")

	local buffer = fh:read("*a")
	fh:close()
	return buffer
end

--- Execute command and invoke given callback for each readed line
-- @param command	String containing the command to execute
-- @param callback	Function to call back for each line
-- @return			Always true
function io.execl(command, callback)
	local ln
	local fh = _io.popen( "%s %s %q" %{ AST_BIN, AST_FLAGS, command }, "r" )
	assert(fh, "Failed to invoke asterisk")

	repeat
		ln = fh:read("*l")
		callback(ln)
	until not ln

	fh:close()
	return true
end

--- Execute command and return an iterator that returns one line per invokation
-- @param command	String containing the command to execute
-- @return			Iterator function
function io.execi(command)
	local fh = _io.popen( "%s %s %q" %{ AST_BIN, AST_FLAGS, command }, "r" )
	assert(fh, "Failed to invoke asterisk")

	return function()
		local ln = fh:read("*l")
		if not ln then fh:close() end
		return ln
	end
end


--- LuCI Asterisk - core status
core = luci.util.class()

--- Retrive version string.
-- @return	String containing the reported asterisk version
function core.version(self)
	local version = io.exec("core show version")
	return version:gsub(" *\n", "")
end


--- LuCI Asterisk - SIP information.
-- @type module
sip = luci.util.class()

--- Get a list of known SIP peers
-- @return		Table containing each SIP peer
function sip.peers(self)
	local head  = false
	local peers = { }

	for line in io.execi("sip show peers") do
		if not head then
			head = true
		elseif not line:match(" sip peers ") then
			local online, delay, id, uid
			local name, host, dyn, nat, acl, port, status =
				line:match("(.-) +(.-) +([D ])   ([N ])   (.)  (%d+) +(.+)")

			if host == '(Unspecified)' then host = nil end
			if port == '0' then port = nil else port = tonumber(port) end

			dyn = ( dyn == 'D' and true or false )
			nat = ( nat == 'N' and true or false )
			acl = ( acl ~= ' ' and true or false )

			online, delay = status:match("(OK) %((%d+) ms%)")

			if online == 'OK' then
				online = true
				delay  = tonumber(delay)
			elseif status ~= 'Unmonitored' then
				online = false
				delay  = 0
			else
				online = nil
				delay  = 0
			end

			id, uid = name:match("(.+)/(.+)")

			if not ( id and uid ) then
				id  = name .. "..."
				uid = nil
			end

			peers[#peers+1] = {
				online  = online,
				delay   = delay,
				name    = id,
				user    = uid,
				dynamic = dyn,
				nat     = nat,
				acl     = acl,
				host    = host,
				port    = port
			}
		end
	end

	return peers
end

--- Get informations of given SIP peer
-- @param peer	String containing the name of the SIP peer
function sip.peer(peer)
	local info = { }
	local keys = { }

	for line in io.execi("sip show peer " .. peer) do
		if #line > 0 then
			local key, val = line:match("(.-) *: +(.*)")
			if key and val then

				key = key:gsub("^ +",""):gsub(" +$", "")
				val = val:gsub("^ +",""):gsub(" +$", "")

				if key == "* Name" then
					key = "Name"
				elseif key == "Addr->IP" then
					info.address, info.port = val:match("(.+) Port (.+)")
					info.port = tonumber(info.port)
				elseif key == "Status" then
					info.online, info.delay = val:match("(OK) %((%d+) ms%)")
					if info.online == 'OK' then
						info.online = true
						info.delay  = tonumber(info.delay)
					elseif status ~= 'Unmonitored' then
						info.online = false
						info.delay  = 0
					else
						info.online = nil
						info.delay  = 0
					end
				end

				if val == 'Yes' or val == 'yes' or val == '<Set>' then
					val = true
				elseif val == 'No' or val == 'no' then
					val = false
				elseif val == '<Not set>' or val == '(none)' then
					val = nil
				end

				keys[#keys+1] = key
				info[key] = val
			end
		end
	end

	return info, keys
end


--- LuCI Asterisk - Internal helpers
-- @type module
tools = luci.util.class()

--- Convert given value to a list of tokens. Split by white space.
-- @param val	String or table value
-- @return		Table containing tokens
function tools.parse_list(v)
	local tokens = { }

	v = type(v) == "table" and v or { v }
	for _, v in ipairs(v) do
		if type(v) == "string" then
			for v in v:gmatch("(%S+)") do
				tokens[#tokens+1] = v
			end
		end
	end

	return tokens
end

--- Convert given list to a collection of hyperlinks
-- @param list	Table of tokens
-- @param url	String pattern or callback function to construct urls (optional)
-- @param sep	String containing the seperator (optional, default is ", ")
-- @return		String containing the html fragment
function tools.hyperlinks(list, url, sep)
	local html

	local function mkurl(p, t)
		if type(p) == "string" then
			return p:format(t)
		elseif type(p) == "function" then
			return p(t)
		else
			return '#'
		end
	end

	list = list or { }
	url  = url  or "%s"
	sep  = sep  or ", "

	for _, token in ipairs(list) do
		html = ( html and html .. sep or '' ) ..
			'<a href="%s">%s</a>' %{ mkurl(url, token), token }
	end

	return html or ''
end


--- LuCI Asterisk - International Direct Dialing Prefixes
-- @type module
idd = luci.util.class()

--- Lookup the country name for the given IDD code.
-- @param country	String containing IDD code
-- @return			String containing the country name
function idd.country(c)
	for _, v in ipairs(cc_idd.CC_IDD) do
		if type(v[3]) == "table" then
			for _, v2 in ipairs(v[3]) do
				if v2 == tostring(c) then
					return v[1]
				end
			end
		elseif v[3] == tostring(c) then
			return v[1]
		end
	end
end

--- Lookup the country code for the given IDD code.
-- @param country	String containing IDD code
-- @return			Table containing the country code(s)
function idd.cc(c)
	for _, v in ipairs(cc_idd.CC_IDD) do
		if type(v[3]) == "table" then
			for _, v2 in ipairs(v[3]) do
				if v2 == tostring(c) then
					return type(v[2]) == "table"
						and v[2] or { v[2] }
				end
			end
		elseif v[3] == tostring(c) then
			return type(v[2]) == "table"
				and v[2] or { v[2] }
		end
	end
end

--- Lookup the IDD code(s) for the given country.
-- @param idd		String containing the country name
-- @return			Table containing the IDD code(s)
function idd.idd(c)
	for _, v in ipairs(cc_idd.CC_IDD) do
		if v[1]:lower():match(c:lower()) then
			return type(v[3]) == "table"
				and v[3] or { v[3] }
		end
	end
end

--- Populate given CBI field with IDD codes.
-- @param field		CBI option object
-- @return			(nothing)
function idd.cbifill(o)
	for i, v in ipairs(cc_idd.CC_IDD) do
		o:value("_%i" % i, util.pcdata(v[1]))
	end

	o.formvalue = function(...)
		local val = luci.cbi.Value.formvalue(...)
		if val:sub(1,1) == "_" then
			val = tonumber((val:gsub("^_", "")))
			if val then
				return type(cc_idd.CC_IDD[val][3]) == "table"
					and cc_idd.CC_IDD[val][3] or { cc_idd.CC_IDD[val][3] }
			end
		end
		return val
	end

	o.cfgvalue = function(...)
		local val = luci.cbi.Value.cfgvalue(...)
		if val then
			val = tools.parse_list(val)
			for i, v in ipairs(cc_idd.CC_IDD) do
				if type(v[3]) == "table" then
					if v[3][1] == val[1] then
						return "_%i" % i
					end
				else
					if v[3] == val[1] then
						return "_%i" % i
					end
				end
			end
		end
		return val
	end
end


--- LuCI Asterisk - Country Code Prefixes
-- @type module
cc = luci.util.class()

--- Lookup the country name for the given CC code.
-- @param country	String containing CC code
-- @return			String containing the country name
function cc.country(c)
	for _, v in ipairs(cc_idd.CC_IDD) do
		if type(v[2]) == "table" then
			for _, v2 in ipairs(v[2]) do
				if v2 == tostring(c) then
					return v[1]
				end
			end
		elseif v[2] == tostring(c) then
			return v[1]
		end
	end
end

--- Lookup the international dialing code for the given CC code.
-- @param cc		String containing CC code
-- @return			String containing IDD code
function cc.idd(c)
	for _, v in ipairs(cc_idd.CC_IDD) do
		if type(v[2]) == "table" then
			for _, v2 in ipairs(v[2]) do
				if v2 == tostring(c) then
					return type(v[3]) == "table"
						and v[3] or { v[3] }
				end
			end
		elseif v[2] == tostring(c) then
			return type(v[3]) == "table"
				and v[3] or { v[3] }
		end
	end
end

--- Lookup the CC code(s) for the given country.
-- @param country	String containing the country name
-- @return			Table containing the CC code(s)
function cc.cc(c)
	for _, v in ipairs(cc_idd.CC_IDD) do
		if v[1]:lower():match(c:lower()) then
			return type(v[2]) == "table"
				and v[2] or { v[2] }
		end
	end
end

--- Populate given CBI field with CC codes.
-- @param field		CBI option object
-- @return			(nothing)
function cc.cbifill(o)
	for i, v in ipairs(cc_idd.CC_IDD) do
		o:value("_%i" % i, util.pcdata(v[1]))
	end

	o.formvalue = function(...)
		local val = luci.cbi.Value.formvalue(...)
		if val:sub(1,1) == "_" then
			val = tonumber((val:gsub("^_", "")))
			if val then
				return type(cc_idd.CC_IDD[val][2]) == "table"
					and cc_idd.CC_IDD[val][2] or { cc_idd.CC_IDD[val][2] }
			end
		end
		return val
	end

	o.cfgvalue = function(...)
		local val = luci.cbi.Value.cfgvalue(...)
		if val then
			val = tools.parse_list(val)
			for i, v in ipairs(cc_idd.CC_IDD) do
				if type(v[2]) == "table" then
					if v[2][1] == val[1] then
						return "_%i" % i
					end
				else
					if v[2] == val[1] then
						return "_%i" % i
					end
				end
			end
		end
		return val
	end
end


--- LuCI Asterisk - Dialzone
-- @type	module
dialzone = luci.util.class()

--- Parse a dialzone section
-- @param zone	Table containing the zone info
-- @return		Table with parsed information
function dialzone.parse(z)
	if z['.name'] then
		return {
			trunks		= tools.parse_list(z.uses),
			name    	= z['.name'],
			description	= z.description or z['.name'],
			addprefix	= z.addprefix,
			matches		= tools.parse_list(z.match),
			intlmatches	= tools.parse_list(z.international),
			countrycode	= z.countrycode,
			localzone	= z.localzone,
			localprefix	= z.localprefix
		}
	end
end

--- Get a list of known dial zones
-- @return		Associative table of zones and table of zone names
function dialzone.zones()
	local zones  = { }
	local znames = { }
	uci:foreach("asterisk", "dialzone",
		function(z)
			zones[z['.name']] = dialzone.parse(z)
			znames[#znames+1] = z['.name']
		end)
	return zones, znames
end

--- Get a specific dial zone
-- @param name	Name of the dial zone
-- @return		Table containing zone information
function dialzone.zone(n)
	local zone
	uci:foreach("asterisk", "dialzone",
		function(z)
			if z['.name'] == n then
				zone = dialzone.parse(z)
			end
		end)
	return zone
end

--- Find uci section hash for given zone number
-- @param idx	Zone number
-- @return		String containing the uci hash pointing to the section
function dialzone.ucisection(i)
	local hash
	local index = 1
	i = tonumber(i)
	uci:foreach("asterisk", "dialzone",
		function(z)
			if not hash and index == i then
				hash = z['.name']
			end
			index = index + 1
		end)
	return hash
end


--- LuCI Asterisk - Voicemailbox
-- @type	module
voicemail = luci.util.class()

--- Parse a voicemail section
-- @param zone	Table containing the mailbox info
-- @return		Table with parsed information
function voicemail.parse(z)
	if z.number and #z.number > 0 then
		local v = {
			id			= '%s@%s' %{ z.number, z.context or 'default' },
			number		= z.number,
			context		= z.context 	or 'default',
			name		= z.name		or z['.name'] or 'OpenWrt',
			zone		= z.zone		or 'homeloc',
			password	= z.password	or '0000',
			email		= z.email		or '',
			page		= z.page		or '',
			dialplans	= { }
		}

		uci:foreach("asterisk", "dialplanvoice",
			function(s)
				if s.dialplan and #s.dialplan > 0 and
				   s.voicebox == v.number
				then
					v.dialplans[#v.dialplans+1] = s.dialplan
				end
			end)

		return v
	end
end

--- Get a list of known voicemail boxes
-- @return		Associative table of boxes and table of box numbers
function voicemail.boxes()
	local vboxes = { }
	local vnames = { }
	uci:foreach("asterisk", "voicemail",
		function(z)
			local v = voicemail.parse(z)
			if v then
				local n = '%s@%s' %{ v.number, v.context }
				vboxes[n]  = v
				vnames[#vnames+1] = n
			end
		end)
	return vboxes, vnames
end

--- Get a specific voicemailbox
-- @param number	Number of the voicemailbox
-- @return			Table containing mailbox information
function voicemail.box(n)
	local box
	n = n:gsub("@.+$","")
	uci:foreach("asterisk", "voicemail",
		function(z)
			if z.number == tostring(n) then
				box = voicemail.parse(z)
			end
		end)
	return box
end

--- Find all voicemailboxes within the given dialplan
-- @param plan	Dialplan name or table
-- @return		Associative table containing extensions mapped to mailbox info
function voicemail.in_dialplan(p)
	local plan  = type(p) == "string" and p or p.name
	local boxes = { }
	uci:foreach("asterisk", "dialplanvoice",
		function(s)
			if s.extension and #s.extension > 0 and s.dialplan == plan then
				local box = voicemail.box(s.voicebox)
				if box then
					boxes[s.extension] = box
				end
			end
		end)
	return boxes
end

--- Remove voicemailbox and associated extensions from config
-- @param box	Voicemailbox number or table
-- @param ctx	UCI context to use (optional)
-- @return		Boolean indicating success
function voicemail.remove(v, ctx)
	ctx = ctx or uci
	local box = type(v) == "string" and v or v.number
	local ok1 = ctx:delete_all("asterisk", "voicemail", {number=box})
	local ok2 = ctx:delete_all("asterisk", "dialplanvoice", {voicebox=box})
	return ( ok1 or ok2 ) and true or false
end


--- LuCI Asterisk - MeetMe Conferences
-- @type	module
meetme = luci.util.class()

--- Parse a meetme section
-- @param room	Table containing the room info
-- @return		Table with parsed information
function meetme.parse(r)
	if r.room and #r.room > 0 then
		local v = {
			room		= r.room,
			pin			= r.pin 			or '',
			adminpin	= r.adminpin		or '',
			description = r._description	or '',
			dialplans	= { }
		}

		uci:foreach("asterisk", "dialplanmeetme",
			function(s)
				if s.dialplan and #s.dialplan > 0 and s.room == v.room then
					v.dialplans[#v.dialplans+1] = s.dialplan
				end
			end)

		return v
	end
end

--- Get a list of known meetme rooms
-- @return		Associative table of rooms and table of room numbers
function meetme.rooms()
	local mrooms = { }
	local mnames = { }
	uci:foreach("asterisk", "meetme",
		function(r)
			local v = meetme.parse(r)
			if v then
				mrooms[v.room] = v
				mnames[#mnames+1] = v.room
			end
		end)
	return mrooms, mnames
end

--- Get a specific meetme room
-- @param number	Number of the room
-- @return			Table containing room information
function meetme.room(n)
	local room
	uci:foreach("asterisk", "meetme",
		function(r)
			if r.room == tostring(n) then
				room = meetme.parse(r)
			end
		end)
	return room
end

--- Find all meetme rooms within the given dialplan
-- @param plan	Dialplan name or table
-- @return		Associative table containing extensions mapped to room info
function meetme.in_dialplan(p)
	local plan  = type(p) == "string" and p or p.name
	local rooms = { }
	uci:foreach("asterisk", "dialplanmeetme",
		function(s)
			if s.extension and #s.extension > 0 and s.dialplan == plan then
				local room = meetme.room(s.room)
				if room then
					rooms[s.extension] = room
				end
			end
		end)
	return rooms
end

--- Remove meetme room and associated extensions from config
-- @param room	Voicemailbox number or table
-- @param ctx	UCI context to use (optional)
-- @return		Boolean indicating success
function meetme.remove(v, ctx)
	ctx = ctx or uci
	local room = type(v) == "string" and v or v.number
	local ok1  = ctx:delete_all("asterisk", "meetme", {room=room})
	local ok2  = ctx:delete_all("asterisk", "dialplanmeetme", {room=room})
	return ( ok1 or ok2 ) and true or false
end


--- LuCI Asterisk - Dialplan
-- @type	module
dialplan = luci.util.class()

--- Parse a dialplan section
-- @param plan	Table containing the plan info
-- @return		Table with parsed information
function dialplan.parse(z)
	if z['.name'] then
		local plan = {
			zones		= { },
			name    	= z['.name'],
			description	= z.description or z['.name']
		}

		-- dialzones
		for _, name in ipairs(tools.parse_list(z.include)) do
			local zone = dialzone.zone(name)
			if zone then
				plan.zones[#plan.zones+1] = zone
			end
		end

		-- voicemailboxes
		plan.voicemailboxes = voicemail.in_dialplan(plan)

		-- meetme conferences
		plan.meetmerooms = meetme.in_dialplan(plan)

		return plan
	end
end

--- Get a list of known dial plans
-- @return		Associative table of plans and table of plan names
function dialplan.plans()
	local plans  = { }
	local pnames = { }
	uci:foreach("asterisk", "dialplan",
		function(p)
			plans[p['.name']] = dialplan.parse(p)
			pnames[#pnames+1] = p['.name']
		end)
	return plans, pnames
end

--- Get a specific dial plan
-- @param name	Name of the dial plan
-- @return		Table containing plan information
function dialplan.plan(n)
	local plan
	uci:foreach("asterisk", "dialplan",
		function(p)
			if p['.name'] == n then
				plan = dialplan.parse(p)
			end
		end)
	return plan
end
