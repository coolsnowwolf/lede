-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local ast = require("luci.asterisk")

--
-- SIP trunk info
--
if arg[2] == "info" then
	form = SimpleForm("asterisk", "SIP Trunk Information")
	form.reset  = false
	form.submit = "Back to overview"

	local info, keys = ast.sip.peer(arg[1])
	local data = { }

	for _, key in ipairs(keys) do
		data[#data+1] = {
			key = key,
			val = type(info[key]) == "boolean"
				and ( info[key] and "yes" or "no" )
				or  ( info[key] == nil or #info[key] == 0 )
					and "(none)"
					or  tostring(info[key])
		}
	end

	itbl = form:section(Table, data, "SIP Trunk %q" % arg[1])
	itbl:option(DummyValue, "key", "Key")
	itbl:option(DummyValue, "val", "Value")

	function itbl.parse(...)
		luci.http.redirect(
			luci.dispatcher.build_url("admin", "asterisk", "trunks")
		)
	end

	return form

--
-- SIP trunk config
--
elseif arg[1] then
	cbimap = Map("asterisk", "Edit SIP Trunk")

	peer = cbimap:section(NamedSection, arg[1])
	peer.hidden = {
		type    = "peer",
		qualify = "yes",
	}

	back = peer:option(DummyValue, "_overview", "Back to trunk overview")
	back.value = ""
	back.titleref = luci.dispatcher.build_url("admin", "asterisk", "trunks")

	sipdomain = peer:option(Value, "host", "SIP Domain")
	sipport   = peer:option(Value, "port", "SIP Port")
	function sipport.cfgvalue(...)
		return AbstractValue.cfgvalue(...) or "5060"
	end

	username  = peer:option(Value, "username", "Authorization ID")
	password  = peer:option(Value, "secret", "Authorization Password")
	password.password = true

	outboundproxy = peer:option(Value, "outboundproxy", "Outbound Proxy")
	outboundport  = peer:option(Value, "outboundproxyport", "Outbound Proxy Port")

	register = peer:option(Flag, "register", "Register with peer")
	register.enabled  = "yes"
	register.disabled = "no"

	regext = peer:option(Value, "registerextension", "Extension to register (optional)")
	regext:depends({register="1"})

	didval = peer:option(ListValue, "_did", "Number of assigned DID numbers")
	didval:value("", "(none)")
	for i=1,24 do didval:value(i) end

	dialplan = peer:option(ListValue, "context", "Dialplan Context")
	dialplan:value(arg[1] .. "_inbound", "(default)")
	cbimap.uci:foreach("asterisk", "dialplan",
		function(s) dialplan:value(s['.name']) end)

	return cbimap
end
