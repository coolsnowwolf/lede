-- Copyright 2009 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

cbimap = Map("asterisk", "MeetMe - Common Settings",
	"Common settings for MeetMe phone conferences.")

meetme = cbimap:section(TypedSection, "meetmegeneral", "General MeetMe Options")
meetme.addremove = false
meetme.anonymous = true

audiobuffers = meetme:option(ListValue, "audiobuffers",
	"Number of 20ms audio buffers to use for conferences")

for i = 2, 32 do audiobuffers:value(i) end


return cbimap
