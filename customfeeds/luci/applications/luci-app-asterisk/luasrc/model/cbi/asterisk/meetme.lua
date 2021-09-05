-- Copyright 2009 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local ast = require "luci.asterisk"

cbimap = Map("asterisk", "MeetMe - Rooms")

meetme = cbimap:section(TypedSection, "meetme", "MeetMe Rooms")
meetme.addremove = true
meetme.anonymous = true
meetme.template = "cbi/tblsection"
meetme:option(Value, "_description", "Description", "Short room description")

room = meetme:option(Value, "room", "Room Number", "Unique room identifier")

function room.write(self, s, val)
	if val and #val > 0 then
		local old = self:cfgvalue(s)
		self.map.uci:foreach("asterisk", "dialplanmeetme",
			function(v)
				if v.room == old then
					self.map:set(v['.name'], "room", val)
				end
			end)
		Value.write(self, s, val)
	end
end


meetme:option(Value, "pin", "PIN", "PIN required to access")
meetme:option(Value, "adminpin", "Admin PIN", "PIN required for administration")

function meetme.remove(self, s)
	return ast.meetme.remove(self.map:get(s, "room"), self.map.uci)
end


return cbimap
