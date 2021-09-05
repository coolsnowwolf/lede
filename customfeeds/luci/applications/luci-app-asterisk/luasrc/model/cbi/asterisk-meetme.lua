-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

cbimap = Map("asterisk", "asterisk", "")

meetmegeneral = cbimap:section(TypedSection, "meetmegeneral", "Meetme Conference General Options", "")

audiobuffers = meetmegeneral:option(Value, "audiobuffers", "Number of 20ms audio buffers to be used", "")


meetme = cbimap:section(TypedSection, "meetme", "Meetme Conference", "")
meetme.addremove = true

adminpin = meetme:option(Value, "adminpin", "Admin PIN", "")
adminpin.password = true

pin = meetme:option(Value, "pin", "Meeting PIN", "")
pin.password = true


return cbimap
