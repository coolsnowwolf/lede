-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

cbimap = Map("asterisk", "asterisk", "")

dialplan = cbimap:section(TypedSection, "dialplan", "Section dialplan", "")
dialplan.addremove = true
dialplan.dynamic = true

include = dialplan:option(MultiValue, "include", "Include zones and plans", "")
cbimap.uci:foreach( "asterisk", "dialplan", function(s) include:value(s['.name']) end )
cbimap.uci:foreach( "asterisk", "dialzone", function(s) include:value(s['.name']) end )

dialplanexten = cbimap:section(TypedSection, "dialplanexten", "Dialplan Extension", "")
dialplanexten.anonymous = true
dialplanexten.addremove = true
dialplanexten.dynamic = true


dialplangeneral = cbimap:section(TypedSection, "dialplangeneral", "Dialplan General Options", "")
dialplangeneral.anonymous = true
dialplangeneral.addremove = true

allowtransfer = dialplangeneral:option(Flag, "allowtransfer", "Allow transfer", "")
allowtransfer.rmempty = true

canreinvite = dialplangeneral:option(ListValue, "canreinvite", "Reinvite/redirect media connections", "")
canreinvite:value("yes", "Yes")
canreinvite:value("nonat", "Yes when not behind NAT")
canreinvite:value("update", "Use UPDATE rather than INVITE for path redirection")
canreinvite:value("no", "No")
canreinvite.rmempty = true

clearglobalvars = dialplangeneral:option(Flag, "clearglobalvars", "Clear global vars", "")
clearglobalvars.rmempty = true


dialplangoto = cbimap:section(TypedSection, "dialplangoto", "Dialplan Goto", "")
dialplangoto.anonymous = true
dialplangoto.addremove = true
dialplangoto.dynamic = true


dialplanmeetme = cbimap:section(TypedSection, "dialplanmeetme", "Dialplan Conference", "")
dialplanmeetme.anonymous = true
dialplanmeetme.addremove = true
dialplanmeetme.dynamic = true


dialplansaytime = cbimap:section(TypedSection, "dialplansaytime", "Dialplan Time", "")
dialplansaytime.anonymous = true
dialplansaytime.addremove = true
dialplansaytime.dynamic = true


dialplanvoice = cbimap:section(TypedSection, "dialplanvoice", "Dialplan Voicemail", "")
dialplanvoice.anonymous = true
dialplanvoice.addremove = true
dialplanvoice.dynamic = true


dialzone = cbimap:section(TypedSection, "dialzone", "Dial Zones for Dialplan", "")
dialzone.addremove = true
dialzone.template = "cbi/tblsection"

addprefix = dialzone:option(Value, "addprefix", "Prefix to add matching dialplans", "")
addprefix.rmempty = true

--international = dialzone:option(DynamicList, "international", "Match International prefix", "")
international = dialzone:option(Value, "international", "Match International prefix", "")
international.rmempty = true

localprefix = dialzone:option(Value, "localprefix", "Prefix (0) to add/remove to/from intl. numbers", "")
localprefix.rmempty = true

localzone = dialzone:option(Value, "localzone", "Dialzone for intl. numbers matched as local", "")
localzone.titleref = luci.dispatcher.build_url( "admin", "services", "asterisk", "dialplans" )
cbimap.uci:foreach( "asterisk", "dialplan", function(s) localzone:value(s['.name']) end )
cbimap.uci:foreach( "asterisk", "dialzone", function(s) localzone:value(s['.name']) end )

match = dialzone:option(Value, "match", "Match plan", "")
match.rmempty = true

uses = dialzone:option(ListValue, "uses", "Connection to use", "")
uses.titleref = luci.dispatcher.build_url( "admin", "services", "asterisk", "sip-conns" )
cbimap.uci:foreach( "asterisk", "sip", function(s) uses:value('SIP/'..s['.name']) end )
cbimap.uci:foreach( "asterisk", "iax", function(s) uses:value('IAX/'..s['.name']) end )


return cbimap
