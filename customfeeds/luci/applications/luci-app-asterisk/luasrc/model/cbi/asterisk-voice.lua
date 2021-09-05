-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

cbimap = Map("asterisk", "asterisk", "")

voicegeneral = cbimap:section(TypedSection, "voicegeneral", "Voicemail general options", "")

serveremail = voicegeneral:option(Value, "serveremail", "From Email address of server", "")


voicemail = cbimap:section(TypedSection, "voicemail", "Voice Mail boxes", "")
voicemail.addremove = true

attach = voicemail:option(Flag, "attach", "Email contains attachment", "")
attach.rmempty = true

email = voicemail:option(Value, "email", "Email", "")
email.rmempty = true

name = voicemail:option(Value, "name", "Display Name", "")
name.rmempty = true

password = voicemail:option(Value, "password", "Password", "")
password.rmempty = true

zone = voicemail:option(ListValue, "zone", "Voice Zone", "")
cbimap.uci:foreach( "asterisk", "voicezone", function(s) zone:value(s['.name']) end )


voicezone = cbimap:section(TypedSection, "voicezone", "Voice Zone settings", "")
voicezone.addremove = true

message = voicezone:option(Value, "message", "Message Format", "")
message.rmempty = true

zone = voicezone:option(Value, "zone", "Time Zone", "")
zone.rmempty = true


return cbimap
