-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local ast = require("luci.asterisk")

cbimap = Map("asterisk", "Registered Trunks")
cbimap.pageaction = false

local sip_peers = { }
cbimap.uci:foreach("asterisk", "sip",
	function(s)
		if s.type == "peer" then
			s.name = s['.name']
			s.info = ast.sip.peer(s.name)
			sip_peers[s.name] = s
		end
	end)


sip_table = cbimap:section(TypedSection, "sip", "SIP Trunks")
sip_table.template    = "cbi/tblsection"
sip_table.extedit     = luci.dispatcher.build_url("admin", "asterisk", "trunks", "sip", "%s")
sip_table.addremove   = true
sip_table.sectionhead = "Extension"

function sip_table.filter(self, s)
	return s and (
		cbimap.uci:get("asterisk", s, "type") == nil or
		cbimap.uci:get_bool("asterisk", s, "provider")
	)
end

function sip_table.create(self, section)
	if TypedSection.create(self, section) then
		created = section
	else
		self.invalid_cts = true
	end
end

function sip_table.parse(self, ...)
	TypedSection.parse(self, ...)
	if created then
		cbimap.uci:tset("asterisk", created, {
			type     = "friend",
			qualify  = "yes",
			provider = "yes"
		})

		cbimap.uci:save("asterisk")
		luci.http.redirect(luci.dispatcher.build_url(
			"admin", "asterisk", "trunks", "sip", created
		))
	end
end


user = sip_table:option(DummyValue, "username", "Username")

host = sip_table:option(DummyValue, "host", "Hostname")
function host.cfgvalue(self, s)
	if sip_peers[s] and sip_peers[s].info.address then
		return "%s:%i" %{ sip_peers[s].info.address, sip_peers[s].info.port }
	else
		return "n/a"
	end
end

context = sip_table:option(DummyValue, "context", "Dialplan")
context.href = luci.dispatcher.build_url("admin", "asterisk", "dialplan")
function context.cfgvalue(...)
	return AbstractValue.cfgvalue(...) or "(default)"
end

online = sip_table:option(DummyValue, "online", "Registered")
function online.cfgvalue(self, s)
	if sip_peers[s] and sip_peers[s].info.online == nil then
		return "n/a"
	else
		return sip_peers[s] and sip_peers[s].info.online
			and "yes" or "no (%s)" %{
				sip_peers[s] and sip_peers[s].info.Status:lower() or "unknown"
			}
	end
end

delay = sip_table:option(DummyValue, "delay", "Delay")
function delay.cfgvalue(self, s)
	if sip_peers[s] and sip_peers[s].info.online then
		return "%i ms" % sip_peers[s].info.delay
	else
		return "n/a"
	end
end

info = sip_table:option(Button, "_info", "Info")
function info.write(self, s)
	luci.http.redirect(luci.dispatcher.build_url(
		"admin", "asterisk", "trunks", "sip", s, "info"
	))
end

return cbimap
